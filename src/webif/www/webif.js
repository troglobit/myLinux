/* myLinux web interface: a RESTCONF client, all of it runs in the browser */
'use strict';

const $ = (sel) => document.querySelector(sel);
const view = $('#view');

let auth = sessionStorage.getItem('auth');
let current = null;		/* name of the view on show */
let renders = 0;		/* tells a render that a later one overtook it */
let stream = null;		/* AbortController of the event stream */
let own = 0;			/* change events to expect from our own saves */
let stale = false;		/* changed elsewhere since the forms were filled in */
const edited = new Set();	/* forms with unsaved edits */

/* Build an element; text always goes in as text, never as markup */
function h(tag, attrs, ...kids) {
	const el = document.createElement(tag);

	for (const [key, val] of Object.entries(attrs || {})) {
		if (key.startsWith('on'))
			el.addEventListener(key.slice(2), val);
		else if (val === true)
			el.setAttribute(key, '');
		else if (val !== false)
			el.setAttribute(key, val);
	}
	el.append(...kids.flat());

	return el;
}

/*
 * RESTCONF
 */

class HttpError extends Error {
	constructor(status, message) {
		super(message);
		this.status = status;
	}
}

async function request(method, path, body) {
	const headers = { Authorization: auth, Accept: 'application/yang-data+json' };

	if (body !== undefined)
		headers['Content-Type'] = 'application/yang-data+json';

	const res = await fetch('/restconf/' + path, {
		method, headers, body: body === undefined ? undefined : JSON.stringify(body),
	});
	const text = await res.text();
	let data = null;

	try {
		data = text ? JSON.parse(text) : null;
	} catch (err) {
		/* An error page from the web server, not from RESTCONF */
	}

	if (res.status === 401) {
		logout();
		throw new HttpError(401, 'Logged out.');
	}
	if (!res.ok) {
		/* A list by the RFC, clixon sends the one error as it is */
		const err = [data?.['ietf-restconf:errors']?.error].flat()[0];

		throw new HttpError(res.status, err?.['error-message'] || `The device answered ${res.status}.`);
	}

	return data;
}

/* A subtree that is not there is a 404, which for us is an empty one */
async function get(path) {
	try {
		const data = await request('GET', 'data/' + path);

		return data ? Object.values(data)[0] : null;
	} catch (err) {
		if (err.status === 404)
			return null;
		throw err;
	}
}

const put    = (path, body) => request('PUT', 'data/' + path, body);
const post   = (path, body) => request('POST', 'data/' + path, body);
const patch  = (path, body) => request('PATCH', 'data/' + path, body);
const del    = (path) => request('DELETE', 'data/' + path);
const invoke = (rpc) => request('POST', 'operations/' + rpc);

/*
 * Notifications.  EventSource cannot send an Authorization header, so
 * read the event stream with fetch.
 */

function live(on) {
	const el = $('#live');

	el.classList.toggle('up', on);
	el.textContent = on ? 'Live' : 'Offline';
	el.title = on ? 'Changes made elsewhere show up here at once'
		      : 'Not connected for live updates';
}

async function listen() {
	const ctl = stream = new AbortController();
	let gap = false;	/* out of touch for a while, events may be lost */

	while (!ctl.signal.aborted) {
		const since = Date.now();
		let ended = false;

		try {
			const res = await fetch('/streams/NETCONF', {
				headers: { Authorization: auth, Accept: 'text/event-stream' },
				signal: ctl.signal,
			});

			if (res.status === 401)
				return logout();
			if (!res.ok)
				throw new Error(res.status);

			live(true);
			if (gap) {
				own = 0;
				changed();
			}
			gap = false;

			const reader = res.body.pipeThrough(new TextDecoderStream()).getReader();
			let buf = '';

			for (;;) {
				const { value, done } = await reader.read();

				ended = done;
				if (done)
					break;
				buf += value;

				const events = buf.split(/\r?\n\r?\n/);

				buf = events.pop();
				events.filter((ev) => ev.includes('netconf-config-change')).forEach(changed);
			}
		} catch (err) {
			/* Device restarting or out of reach, try again below */
		}

		if (ctl.signal.aborted)
			break;

		/* The web server ends a stream that has been up for long, for
		 * us to log in again.  Nothing is lost if we do so at once,
		 * unless the event of a save of ours was on its way. */
		if (ended && !own && Date.now() - since > 10000)
			continue;

		live(false);
		gap = true;
		await new Promise((resolve) => setTimeout(resolve, 3000));
	}
}

/* The configuration changed, here or elsewhere */
function changed() {
	if (own > 0) {
		own--;
		return;
	}
	hostname();
	stale = edited.size > 0;
	if (stale)
		notice('The configuration was changed elsewhere. Reloading shows it and drops your edits here.');
	else
		render();
}

function notice(text) {
	$('#notice span').textContent = text;
	$('#notice').hidden = false;
}

/*
 * Status
 */

function duration(sec) {
	const d = Math.floor(sec / 86400);
	const hr = Math.floor(sec % 86400 / 3600);
	const min = Math.floor(sec % 3600 / 60);

	if (d)
		return `${d} d ${hr} h`;
	if (hr)
		return `${hr} h ${min} min`;

	return `${min} min`;
}

function bytes(num) {
	const units = ['B', 'kB', 'MB', 'GB', 'TB'];
	let i = 0;

	while (num >= 1000 && i < units.length - 1) {
		num /= 1000;
		i++;
	}

	return `${i ? num.toFixed(1) : num} ${units[i]}`;
}

const FAMILIES = ['ietf-ip:ipv4', 'ietf-ip:ipv6'];

function addresses(ifc) {
	const list = [];

	for (const fam of FAMILIES) {
		for (const addr of ifc[fam]?.address || [])
			list.push(`${addr.ip}/${addr['prefix-length']}`);
	}

	return list;
}

const fact = (label, value) => h('div', null, h('dt', null, label), h('dd', null, value));
const isUp = (ifc) => ifc['oper-status'] === 'up';
const isLoopback = (ifc) => (ifc.type || '').endsWith('softwareLoopback');

async function status() {
	const [state, ifs] = await Promise.all([
		get('ietf-system:system-state'),
		get('ietf-interfaces:interfaces'),
	]);
	const list = ifs?.interface || [];
	const platform = state?.platform || {};
	const clock = state?.clock || {};
	const up = (Date.parse(clock['current-datetime']) - Date.parse(clock['boot-datetime'])) / 1000;

	return [
		h('section', null,
			h('h1', null, 'Status'),
			h('div', { class: 'faceplate', role: 'img', 'aria-label': 'Ports and their link state' },
				list.filter((ifc) => !isLoopback(ifc)).map((ifc) =>
					h('div', { class: 'jack' + (isUp(ifc) ? ' up' : '') +
							  (ifc['admin-status'] === 'down' ? ' off' : '') },
						h('i'), ifc.name))),
			h('dl', { class: 'facts' },
				fact('Software', `${platform['os-name'] || ''} ${platform['os-version'] || ''}`),
				fact('Kernel', `${platform['os-release'] || ''} ${platform.machine || ''}`),
				fact('Running for', isNaN(up) ? 'unknown' : duration(up)),
				fact('Device time', (clock['current-datetime'] || '').replace('T', ' ').slice(0, 19)))),
		h('section', null,
			h('h2', null, 'Interfaces'),
			h('div', { class: 'scroll' },
				h('table', null,
					h('thead', null, h('tr', null,
						h('th', null, 'Name'), h('th', null, 'Link'),
						h('th', null, 'Addresses'), h('th', null, 'Hardware address'),
						h('th', { class: 'num' }, 'Received'), h('th', { class: 'num' }, 'Sent'))),
					h('tbody', null, list.map((ifc) => h('tr', null,
						h('td', null, ifc.name),
						h('td', null, h('span', { class: 'state' + (isUp(ifc) ? ' up' : '') },
							ifc['oper-status'] || 'unknown')),
						h('td', { class: 'addr' }, addresses(ifc).map((addr) => h('div', null, addr))),
						h('td', { class: 'addr' }, ifc['phys-address'] || ''),
						h('td', { class: 'num' }, bytes(Number(ifc.statistics?.['in-octets'] || 0))),
						h('td', { class: 'num' }, bytes(Number(ifc.statistics?.['out-octets'] || 0))))))))),
	];
}

/*
 * System
 */

const lines = (text) => text.split(/[\s,]+/).filter(Boolean);

/* Set, or with nothing to set remove, a member of a configuration object */
function set(obj, key, value) {
	if (value === '' || value == null)
		delete obj[key];
	else
		obj[key] = value;
}

/*
 * A write of ours comes back as a change event, to be told from those
 * of others.  A write that turns out to write nothing returns false.
 */
async function write(fn) {
	own++;
	try {
		if (await fn() === false)
			own--;
	} catch (err) {
		own--;
		throw err;
	}
}

/* A form with a save button and its result next to it; save does the saving */
function settings(cls, save, fields, label = 'Save changes') {
	const msg = h('span', { role: 'status' });
	const btn = h('button', { class: 'primary' }, label);

	async function onsubmit(ev) {
		const form = ev.target;

		ev.preventDefault();
		msg.className = 'error';
		msg.textContent = '';

		/* Saving writes back more than the form shows, as it was read */
		if (stale) {
			msg.textContent = 'Reload first, the configuration was changed elsewhere.';
			return;
		}

		btn.disabled = true;
		try {
			await write(() => save((name) => form.elements[name].value.trim(), form));
			edited.delete(form);
			msg.className = 'saved';
			msg.textContent = 'Saved';
		} catch (err) {
			msg.textContent = err.message;
		}
		btn.disabled = false;
	}

	return h('form', { class: cls, onsubmit }, fields, h('div', { class: 'actions' }, btn, msg));
}

async function system() {
	const sys = await get('ietf-system:system?content=config') || {};
	const ntp = (sys.ntp?.server || []).map((srv) => srv.udp?.address).filter(Boolean);
	const dns = (sys['dns-resolver']?.server || []).map((srv) => srv['udp-and-tcp']?.address).filter(Boolean);

	/* Write back all of the container, so what we do not show, the
	 * users for one, is kept and what was cleared here goes away. */
	async function save(val) {
		const next = structuredClone(sys);
		const servers = lines(val('ntp'));
		const resolvers = lines(val('dns'));
		const search = lines(val('search'));

		set(next, 'hostname', val('hostname'));
		set(next, 'contact', val('contact'));
		set(next, 'location', val('location'));
		set(next, 'clock', val('timezone') ? { ...next.clock, 'timezone-name': val('timezone') } : null);
		set(next, 'ntp', servers.length ? {
			...next.ntp,
			server: servers.map((addr) => ({ name: addr, udp: { address: addr } })),
		} : null);
		set(next, 'dns-resolver', resolvers.length || search.length ? {
			...next['dns-resolver'],
			search: search.length ? search : undefined,
			server: resolvers.map((addr) => ({ name: addr, 'udp-and-tcp': { address: addr } })),
		} : null);

		await put('ietf-system:system', { 'ietf-system:system': next });
		hostname();
	}

	return [h('section', null,
		h('h1', null, 'System'),
		settings('settings', save, [
			h('label', null, 'Host name',
				h('input', { name: 'hostname', value: sys.hostname || '', maxlength: 63,
					     placeholder: $('#hostname').textContent })),
			h('label', null, 'Contact',
				h('input', { name: 'contact', value: sys.contact || '' })),
			h('label', null, 'Location',
				h('input', { name: 'location', value: sys.location || '' })),
			h('label', null, 'Time zone',
				h('input', { name: 'timezone', value: sys.clock?.['timezone-name'] || '',
					     placeholder: 'Europe/Stockholm' })),
			h('label', null, 'Time servers',
				h('small', null, 'NTP servers, one per line. Leave empty to not sync the clock.'),
				h('textarea', { name: 'ntp', spellcheck: 'false' }, ntp.join('\n'))),
			h('label', null, 'Name servers',
				h('small', null, 'DNS servers, one address per line. Leave empty to use what DHCP provides.'),
				h('textarea', { name: 'dns', spellcheck: 'false' }, dns.join('\n'))),
			h('label', null, 'Search domains',
				h('input', { name: 'search', value: (sys['dns-resolver']?.search || []).join(' ') }))]))];
}

/*
 * Interfaces
 */

/* The port mech leaves to the system, see MGMT_IF in its interfaces.c */
const MGMT_IF = 'eth0';

async function interfaces() {
	const [oper, conf] = await Promise.all([
		get('ietf-interfaces:interfaces'),
		get('ietf-interfaces:interfaces?content=config'),
	]);
	const config = new Map((conf?.interface || []).map((ifc) => [ifc.name, ifc]));
	const names = new Map((oper?.interface || []).map((ifc) => [ifc.name, ifc]));

	for (const [name, ifc] of config) {
		if (!names.has(name))
			names.set(name, ifc);
	}

	function row(ifc) {
		const cfg = config.get(ifc.name);
		const addrs = addresses(cfg || {});

		if (ifc.name === MGMT_IF) {
			return h('div', { class: 'iface' },
				h('h2', null, ifc.name),
				h('p', { class: 'muted fixed' }, 'The management port. The system sets it up ' +
					'with DHCP, it cannot be changed here.'));
		}

		async function save(val, form) {
			const next = { ...cfg, name: ifc.name, type: cfg?.type || ifc.type };
			const list = lines(val('addresses'));
			const path = 'ietf-interfaces:interfaces/interface=' + encodeURIComponent(ifc.name);

			if (!list.every((addr) => /^[0-9a-f:.]+\/\d+$/i.test(addr)))
				throw new Error('Give each address with its prefix length, like 192.168.1.1/24.');

			set(next, 'enabled', form.elements.enabled.checked ? null : false);
			set(next, 'description', val('description'));
			for (const fam of FAMILIES) {
				const af = { ...next[fam] };
				const mine = list.filter((addr) => addr.includes(':') === fam.endsWith('6'));

				set(af, 'address', mine.length ? mine.map((addr) => {
					const [ip, len] = addr.split('/');

					return { ip, 'prefix-length': Number(len) };
				}) : null);
				set(next, fam, Object.keys(af).length ? af : null);
			}

			/* Nothing but its name and type left: no configuration */
			if (Object.keys(next).length > 2)
				await put(path, { 'ietf-interfaces:interface': [next] });
			else if (cfg)
				await del(path);
			else
				return false;
		}

		return settings('iface', save, [
			h('h2', null, ifc.name),
			h('label', null, 'Description',
				h('input', { name: 'description', value: cfg?.description || '' })),
			h('label', null, 'Addresses',
				h('textarea', { name: 'addresses', class: 'addr', spellcheck: 'false',
						rows: Math.max(2, addrs.length),
						placeholder: '192.168.1.1/24\n2001:db8::1/64',
						title: 'IPv4 and IPv6 addresses with prefix length, one per line' },
					addrs.join('\n'))),
			h('label', { class: 'check' },
				h('input', { name: 'enabled', type: 'checkbox', checked: cfg?.enabled !== false }),
				'Enabled')]);
	}

	return [h('section', null,
		h('h1', null, 'Interfaces'),
		h('div', null, [...names.values()].filter((ifc) => !isLoopback(ifc)).map(row)))];
}

/*
 * Users
 */

const basic = (user, pass) =>
	/* Basic wants bytes, btoa() wants Latin-1 */
	'Basic ' + btoa(String.fromCharCode(...new TextEncoder().encode(`${user}:${pass}`)));

/* The device hashes a password that is marked as clear text with $0$ */
function password(form) {
	const pass = form.elements.password.value;

	if (pass !== form.elements.repeat.value)
		throw new Error('The two passwords differ.');

	return '$0$' + pass;
}

const secret = (label, name) => h('label', null, label,
	h('input', { name, type: 'password', autocomplete: 'new-password', required: true }));
const passwords = () => [secret('New password', 'password'), secret('Repeat it', 'repeat')];

async function users() {
	const list = (await get('ietf-system:system/authentication?content=config'))?.user || [];
	const self = atob(auth.slice(6)).split(':')[0];
	const msg = h('p', { class: 'error', role: 'alert' });
	const path = (name) => 'ietf-system:system/authentication/user=' + encodeURIComponent(name);

	function row(user) {
		async function save(val, form) {
			await patch(path(user.name),
				{ 'ietf-system:user': [{ name: user.name, password: password(form) }] });

			/* Our own: it is what we log in with from here on */
			if (user.name === self) {
				auth = basic(self, form.elements.password.value);
				sessionStorage.setItem('auth', auth);
			}
			form.reset();
		}

		async function remove() {
			if (!await confirm(`Remove ${user.name}?`,
					   'The account and its home directory are deleted.', 'Remove'))
				return;

			msg.textContent = '';
			try {
				await write(() => del(path(user.name)));
				render();
			} catch (err) {
				msg.textContent = err.message;
			}
		}

		return settings('user', save, [
			h('h2', null, user.name),
			...passwords(),
			...(user.name === self ? [] : [
				h('button', { type: 'button', class: 'remove', onclick: remove }, 'Remove')]),
		], 'Set password');
	}

	async function add(val, form) {
		await post('ietf-system:system/authentication',
			{ 'ietf-system:user': [{ name: val('name'), password: password(form) }] });
		render();
	}

	return [
		h('section', null,
			h('h1', null, 'Users'),
			h('p', { class: 'muted' }, 'Only admin can log in here and change settings. ' +
				'Other users get an account on the device, for SSH.'),
			h('div', null, list.map(row)),
			msg),
		h('section', null,
			h('h2', null, 'Add a user'),
			settings('settings', add, [
				h('label', null, 'User name',
					h('input', { name: 'name', required: true, maxlength: 32,
						     autocomplete: 'off', autocapitalize: 'none',
						     pattern: '[A-Za-z_][A-Za-z0-9_.\\-]*',
						     title: 'Letters, digits, and _ - . starting with a letter' })),
				...passwords(),
			], 'Add user')),
	];
}

/*
 * Maintenance
 */

function confirm(title, text, action) {
	const dlg = $('#confirm');

	dlg.querySelector('h2').textContent = title;
	dlg.querySelector('p').textContent = text;
	dlg.querySelector('[value=ok]').textContent = action;
	dlg.returnValue = 'cancel';
	dlg.showModal();

	return new Promise((resolve) => {
		dlg.addEventListener('close', () => resolve(dlg.returnValue === 'ok'), { once: true });
	});
}

async function maintenance() {
	const msg = h('p', { class: 'error', role: 'alert' });

	const run = (fn) => async () => {
		msg.textContent = '';
		try {
			await fn();
		} catch (err) {
			msg.textContent = err.message;
		}
	};

	const download = run(async () => {
		const data = await request('GET', 'data?content=config');
		const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
		const link = h('a', { href: URL.createObjectURL(blob),
				      download: `${$('#hostname').textContent}-config.json` });

		link.click();
		URL.revokeObjectURL(link.href);
	});

	const restart = run(async () => {
		if (!await confirm('Restart the device?',
				   'Everything it does stops for as long as the restart takes.', 'Restart'))
			return;
		await invoke('ietf-system:system-restart');
		notice('Restarting. This page picks up again when the device is back.');
	});

	const reset = run(async () => {
		if (!await confirm('Reset to factory settings?',
				   'All settings are replaced with the factory ones, users and passwords ' +
				   'included. You may have to log in again, or find the device at another address.',
				   'Reset'))
			return;
		await invoke('ietf-factory-default:factory-reset');
	});

	const task = (title, text, button) => h('div', { class: 'task' },
		h('div', null, h('h2', null, title), h('p', { class: 'muted' }, text)), button);

	return [h('section', null,
		h('h1', null, 'Maintenance'),
		h('div', null,
			task('Configuration', 'Download the current settings as a JSON file. It holds password hashes.',
				h('button', { onclick: download }, 'Download configuration')),
			task('Restart', 'Restart the device. Settings are kept.',
				h('button', { onclick: restart }, 'Restart')),
			task('Factory settings', 'Throw away all settings and start over.',
				h('button', { class: 'danger', onclick: reset }, 'Reset'))),
		msg)];
}

/*
 * Frame
 */

const views = { status, system, interfaces, users, maintenance };

function hostname() {
	get('ietf-system:system/hostname').then((host) => {
		$('#hostname').textContent = host || 'myLinux';
		document.title = host ? `${host} - myLinux` : 'myLinux';
	}).catch(() => {});
}

/* A quiet render is the status refresh: it keeps what is there if it fails */
async function render(quiet) {
	const name = location.hash.slice(1) in views ? location.hash.slice(1) : 'status';
	const seq = ++renders;
	let kids;

	try {
		kids = await views[name]();
	} catch (err) {
		if (quiet === true)
			return;
		kids = [h('p', { class: 'error', role: 'alert' }, err.message)];
	}
	if (!auth || seq !== renders)
		return;

	current = name;
	view.replaceChildren(...kids);
	if (quiet === true)
		return;

	edited.clear();
	stale = false;
	$('#notice').hidden = true;
	for (const link of document.querySelectorAll('nav a'))
		link.toggleAttribute('aria-current', link.hash === '#' + name);
}

function start() {
	$('#login').hidden = true;
	$('#app').hidden = false;
	live(false);
	hostname();
	render();
	listen();
}

function logout() {
	auth = null;
	sessionStorage.removeItem('auth');
	stream?.abort();
	view.replaceChildren();
	$('#app').hidden = true;
	$('#login').hidden = false;
	$('#login').elements.user.focus();
}

$('#login').addEventListener('submit', async (ev) => {
	const form = ev.target;
	const err = form.querySelector('.error');

	ev.preventDefault();
	err.textContent = '';

	auth = basic(form.elements.user.value, form.elements.pass.value);
	try {
		const res = await fetch('/restconf/data/ietf-system:system/hostname', {
			headers: { Authorization: auth, Accept: 'application/yang-data+json' },
		});

		if (res.status === 401) {
			const wait = res.headers.get('Retry-After');

			throw new Error(wait ? `Too many failed attempts. Try again in ${wait} seconds.`
					     : 'Wrong user name or password, or not an administrator.');
		}
		if (!res.ok && res.status !== 404)
			throw new Error(`The device answered ${res.status}.`);
	} catch (e) {
		auth = null;
		err.textContent = e instanceof TypeError ? 'Cannot reach the device.' : e.message;
		return;
	}

	sessionStorage.setItem('auth', auth);
	form.elements.pass.value = '';
	start();
});

$('#logout').addEventListener('click', logout);
$('#notice button').addEventListener('click', () => render());
view.addEventListener('input', (ev) => edited.add(ev.target.form));
window.addEventListener('hashchange', () => render());

/* Counters and link state move without a configuration change */
setInterval(() => {
	if (auth && current === 'status' && !document.hidden)
		render(true);
}, 5000);

if (auth)
	start();
else
	logout();
