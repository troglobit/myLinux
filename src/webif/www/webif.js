/* myLinux web interface: a RESTCONF client, all of it runs in the browser */
'use strict';

const view = $('#view');

const SESSION = 24;		/* hours a login lasts, at the most */

let auth = sessionStorage.getItem('auth');
let since = Number(sessionStorage.getItem('since')) || Date.now();	/* when we logged in */
let seen = Number(sessionStorage.getItem('seen')) || Date.now();	/* when somebody last was at the controls */
let idle = Number(localStorage.getItem('idle') ?? 15);			/* minutes without that before we log out */
let heard = 0;			/* when the device last answered us */
let current = null;		/* name of the view on show */
let renders = 0;		/* tells a render that a later one overtook it */
let stream = null;		/* AbortController of the event stream */
let own = 0;			/* change events to expect from our own saves */
let stale = false;		/* changed elsewhere since the forms were filled in */
const edited = new Set();	/* forms with unsaved edits */

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

	heard = Date.now();

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

/* Rejects unless the device answers, and RESTCONF on it, within five seconds */
async function alive() {
	const res = await fetch('/.well-known/host-meta', { cache: 'no-store', signal: AbortSignal.timeout(5000) });

	if (!res.ok)
		throw new Error(res.status);
	heard = Date.now();
}

async function listen() {
	const ctl = stream = new AbortController();
	let gap = false;	/* out of touch for a while, events may be lost */

	while (!ctl.signal.aborted) {
		const begun = Date.now();
		const link = new AbortController();
		let ended = false;
		let pulse;

		try {
			/* Asked first, an absent device costs us five seconds
			 * and not as long as the browser cares to wait */
			await alive();

			/* A device that is switched off or out of reach leaves
			 * the stream open and silent, the same as when nothing
			 * happens.  So when we have heard nothing from it for a
			 * while we ask, and give the stream up if it is alone. */
			pulse = setInterval(() => {
				if (!document.hidden && Date.now() - heard >= 5000)
					alive().catch(() => link.abort());
			}, 5000);

			const res = await fetch('/streams/NETCONF', {
				headers: { Authorization: auth, Accept: 'text/event-stream' },
				signal: AbortSignal.any([ctl.signal, link.signal]),
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
				heard = Date.now();
				buf += value;

				const events = buf.split(/\r?\n\r?\n/);

				buf = events.pop();
				events.filter((ev) => ev.includes('netconf-config-change')).forEach(changed);
			}
		} catch (err) {
			/* Device restarting or out of reach, try again below */
		} finally {
			clearInterval(pulse);
		}

		if (ctl.signal.aborted)
			break;

		/* The web server ends a stream that has been up for long, for
		 * us to log in again.  Nothing is lost if we do so at once,
		 * unless the event of a save of ours was on its way. */
		if (ended && !own && Date.now() - begun > 10000)
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

/* The type of an interface, an identity less the module it is from */
const kind = (ifc) => local(ifc?.type || '');
const isLoopback = (ifc) => kind(ifc) === 'softwareLoopback';

/* Link light and state of an interface; one that is only configured has none */
const link = (ifc) => h('span', { class: 'state' + (isUp(ifc) ? ' up' : '') }, ifc['oper-status'] || 'not there');

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
				list.filter((ifc) => kind(ifc) === 'ethernetCsmacd').map((ifc) =>
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
						h('td', null, ifc.name, h('small', null, about(ifc))),
						h('td', null, link(ifc)),
						h('td', { class: 'addr' }, addresses(ifc).map((addr) => h('div', null, addr))),
						h('td', { class: 'addr' }, ifc['phys-address'] || ''),
						h('td', { class: 'num' }, bytes(Number(ifc.statistics?.['in-octets'] || 0))),
						h('td', { class: 'num' }, bytes(Number(ifc.statistics?.['out-octets'] || 0))))))))),
	];
}

/*
 * System
 */

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
function settings(save, fields, label = 'Save changes', more = []) {
	const msg = h('span', { role: 'status' });
	const btn = h('button', { class: 'primary' }, label);

	async function onsubmit(ev) {
		const form = ev.currentTarget;

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
			await write(save);
			edited.delete(form);
			msg.className = 'saved';
			msg.textContent = 'Saved';
		} catch (err) {
			msg.textContent = err.message;
		}
		btn.disabled = false;
	}

	return h('form', { class: 'settings', onsubmit }, fields, h('div', { class: 'actions' }, btn, more, msg));
}

/* What the system page shows of ietf-system, and what it calls it */
const SYSTEM = {
	hide: ['system/authentication', 'system/ntp/enabled', 'system/ntp/server/name', 'system/dns-resolver/server/name'],
	labels: {
		'system/hostname': 'Host name',
		'system/clock': 'Clock',
		'system/clock/timezone-name': 'Time zone',
		'system/ntp': 'Set the clock from time servers (NTP)',
		'system/ntp/server': 'Time servers',
		'system/ntp/server/iburst': 'Burst at start',
		'system/ntp/server/prefer': 'Preferred',
		'system/dns-resolver': 'Name resolution (DNS)',
		'system/dns-resolver/search': 'Search domains',
		'system/dns-resolver/server': 'Name servers',
		'system/dns-resolver/options/timeout': 'Timeout',
		'system/dns-resolver/options/attempts': 'Attempts',
	},
	items: {
		'system/ntp/server': 'server',
		'system/dns-resolver/search': 'domain',
		'system/dns-resolver/server': 'server',
	},
	/* A server is listed by a name of its own, for which its address will do */
	fill: {
		'system/ntp/server': (srv) => { srv.name ??= srv.udp?.address; },
		'system/dns-resolver/server': (srv) => { srv.name ??= srv['udp-and-tcp']?.address; },
	},
};

async function system() {
	const sys = await get('ietf-system:system?content=config') || {};
	const body = group(schemaAt('ietf-system:system'), sys, SYSTEM, 'system');

	/* All of the container is written back, so what we do not show,
	 * the users for one, is kept and what was cleared here goes away. */
	async function save() {
		const next = structuredClone(sys);

		body.apply(next);
		await put('ietf-system:system', { 'ietf-system:system': next });
		hostname();
	}

	return [h('section', null,
		h('h1', null, 'System'),
		settings(save, body.el))];
}

/*
 * Interfaces
 */

const MYIF = 'mylinux-interfaces:';

/* The types there is a way to create, as the model spells them */
const CREATED = {
	'iana-if-type:bridge': 'Bridge',
	'iana-if-type:l2vlan': 'VLAN',
	[MYIF + 'veth']: 'veth pair',
	[MYIF + 'dummy']: 'Dummy',
};

/* What the interfaces page shows of an interface, and what it calls it */
const IFACES = {
	hide: ['interface/name', 'interface/type'],
	labels: {
		'interface/ietf-ip:ipv4': 'IPv4',
		'interface/ietf-ip:ipv4/address': 'Addresses',
		'interface/ietf-ip:ipv4/address/ip': 'Address',
		[`interface/ietf-ip:ipv4/${MYIF}dhcp`]: 'Address from DHCP',
		'interface/ietf-ip:ipv6': 'IPv6',
		'interface/ietf-ip:ipv6/address': 'Addresses',
		'interface/ietf-ip:ipv6/address/ip': 'Address',
		[`interface/${MYIF}bridge-port`]: 'Port of a bridge',
		[`interface/${MYIF}vlan`]: 'VLAN',
		[`interface/${MYIF}vlan/id`]: 'VLAN ID',
		[`interface/${MYIF}vlan/lower-layer-if`]: 'On top of',
		[`interface/${MYIF}veth`]: 'veth pair',
		[`interface/${MYIF}veth/peer`]: 'Name of the other end',
	},
	/* There with an address or DHCP in them, and not without */
	implied: ['interface/ietf-ip:ipv4', 'interface/ietf-ip:ipv6'],
};

const opened = new Set();	/* rows folded out, to be so again after a render */

/* A row that folds out to the settings of what it is about, made when first asked for */
function fold(key, summary, make) {
	const el = h('details', { class: 'row', open: opened.has(key), ontoggle }, h('summary', null, summary));

	function ontoggle() {
		opened[el.open ? 'add' : 'delete'](key);
		if (el.open && el.children.length === 1)
			el.append(make());
	}
	if (el.open)
		el.append(make());

	return el;
}

/* What an interface is, in a few words */
function about(ifc) {
	const vlan = ifc[MYIF + 'vlan'];
	const peer = ifc[MYIF + 'veth']?.peer;

	if (vlan)
		return `VLAN ${vlan.id} on ${vlan['lower-layer-if']}`;
	if (peer)
		return `veth, other end ${peer}`;

	return CREATED[ifc.type] ?? (kind(ifc) === 'ethernetCsmacd' ? 'Ethernet' : kind(ifc));
}

async function interfaces() {
	const [oper, conf] = await Promise.all([
		get('ietf-interfaces:interfaces'),
		get('ietf-interfaces:interfaces?content=config'),
	]);
	const config = conf?.interface || [];
	const all = new Map((oper?.interface || []).map((ifc) => [ifc.name, ifc]));
	const msg = h('p', { class: 'error', role: 'alert' });
	const sn = schemaAt('ietf-interfaces:interfaces/interface');

	/* What is configured counts, also for an interface that is not there */
	for (const ifc of config)
		all.set(ifc.name, { ...all.get(ifc.name), ...ifc });

	const list = [...all.values()].filter((ifc) => !isLoopback(ifc));
	const names = list.map((ifc) => ifc.name);
	const bridges = config.filter((ifc) => kind(ifc) === 'bridge').map((ifc) => ifc.name);
	const path = (name) => 'ietf-interfaces:interfaces/interface=' + encodeURIComponent(name);

	function row(ifc) {
		let cfg = config.find((it) => it.name === ifc.name);
		const type = cfg?.type || ifc.type;
		const created = type in CREATED;
		const port = `interface/${MYIF}bridge-port`;
		let body;

		const form = () => {
			body = group(sn, cfg, {
				...IFACES,
				entry: { type },
				/* Which veth is at the other end is not for changing,
				 * and without a bridge there is nothing to be a port of */
				hide: [...IFACES.hide, `interface/${MYIF}veth`, ...(bridges.length ? [] : [port])],
				refs: (at) => (at.startsWith(port) ? bridges : names).filter((name) => name !== ifc.name),
			}, 'interface');

			return settings(save, body.el, 'Save changes',
				created ? h('button', { type: 'button', class: 'danger', onclick: remove }, 'Remove') : []);
		};

		async function save() {
			const next = { ...structuredClone(cfg), name: ifc.name, type };

			body.apply(next);

			/* Nothing but its name and type left: no configuration */
			if (Object.keys(next).length > 2 || created) {
				await put(path(ifc.name), { 'ietf-interfaces:interface': [next] });
				cfg = next;
			} else if (cfg) {
				await del(path(ifc.name));
				cfg = undefined;
			} else {
				return false;
			}
		}

		/* Goes with what cannot be without it: the other end of a veth
		 * pair, and the place its ports have in a bridge. */
		async function remove() {
			const peer = cfg[MYIF + 'veth']?.peer;
			const upper = config.find((it) => it[MYIF + 'vlan']?.['lower-layer-if'] === ifc.name);

			msg.textContent = '';
			if (upper) {
				msg.textContent = `Remove ${upper.name} first, it is on top of ${ifc.name}.`;
				return;
			}
			if (!await confirm(`Remove ${ifc.name}?`, peer ? `${peer}, the other end, goes with it.`
					   : 'The interface and its settings are deleted.', 'Remove'))
				return;

			try {
				/* All of them are written back, so as they are now */
				const now = await get('ietf-interfaces:interfaces?content=config');
				const rest = now.interface.filter((it) => it.name !== ifc.name && it.name !== peer).map((it) => {
					const { [MYIF + 'bridge-port']: member, ...other } = it;

					return member?.bridge === ifc.name ? other : it;
				});

				await write(() => put('ietf-interfaces:interfaces',
					{ 'ietf-interfaces:interfaces': { interface: rest } }));
				render();
			} catch (err) {
				msg.textContent = err.message;
			}
		}

		return fold(ifc.name, [
			h('b', null, ifc.name), h('span', { class: 'muted' }, about(ifc)),
			link(ifc),
			h('span', { class: 'addr' }, addresses(ifc).join(', ')),
		], form);
	}

	/* A new interface is its type and name, and what its type cannot be without */
	function adder() {
		const type = h('select', { name: 'type', onchange: typed },
			Object.entries(CREATED).map(([id, text]) => option(id, text)));
		const name = h('input', { name: 'name', maxlength: 15, autocomplete: 'off', autocapitalize: 'none' });
		const needs = [`interface/${MYIF}vlan`, `interface/${MYIF}veth`];
		const slot = h('div', { class: 'fields' });
		let body;

		function typed() {
			body = group(sn, undefined, {
				...IFACES,
				entry: { type: type.value },
				only: needs,
				/* The other end of a veth pair is not there to pick, we make it */
				refs: (at) => (at.endsWith('veth/peer') ? null : names),
			}, 'interface');
			slot.replaceChildren(body.el);
		}
		typed();

		async function add() {
			const ifc = { name: name.value.trim(), type: type.value };
			const made = [ifc];

			body.apply(ifc);

			const vlan = ifc[MYIF + 'vlan'];
			const peer = ifc[MYIF + 'veth']?.peer;

			if (vlan)
				ifc.name ||= `${vlan['lower-layer-if']}.${vlan.id}`;
			if (peer)
				made.push({ name: peer, type: ifc.type, [MYIF + 'veth']: { peer: ifc.name } });
			if (!ifc.name)
				throw new Error('Give the interface a name.');
			if (made.some((it) => all.has(it.name)))
				throw new Error('There is an interface with that name already.');

			await patch('ietf-interfaces:interfaces', { 'ietf-interfaces:interfaces': { interface: made } });
			opened.add(ifc.name);
			render();
		}

		return settings(add, [
			h('div', { class: 'fields' },
				h('label', null, 'Type', type),
				h('label', null, 'Name', h('small', null, 'A VLAN left without one is named after what it is on and its ID.'), name)),
			slot,
		], 'Add interface');
	}

	return [
		h('section', null,
			h('h1', null, 'Interfaces'),
			h('div', null, list.map(row)),
			msg),
		h('section', null,
			h('h2', null, 'Add an interface'),
			adder()),
	];
}

/*
 * Users
 */

/* Who we are logged in as */
const whoami = () => atob(auth.slice(6)).split(':')[0];

const basic = (user, pass) =>
	/* Basic wants bytes, btoa() wants Latin-1 */
	'Basic ' + btoa(String.fromCharCode(...new TextEncoder().encode(`${user}:${pass}`)));

/* What the users page shows of a user, and what it calls it */
const USERS = {
	hide: ['user/name'],
	labels: {
		'user/mylinux-system:shell': 'Login shell',
		'user/authorized-key': 'SSH keys',
		'user/authorized-key/key-data': 'Key',
	},
	names: { 'mylinux-system:sh': 'Shell', 'mylinux-system:clish': 'Command line interface', 'mylinux-system:false': 'No login' },
	items: { 'user/authorized-key': 'key' },
};

async function users() {
	const list = (await get('ietf-system:system/authentication?content=config'))?.user || [];
	const self = whoami();
	const msg = h('p', { class: 'error', role: 'alert' });
	const sn = schemaAt('ietf-system:system/authentication/user');
	const path = (name) => 'ietf-system:system/authentication/user=' + encodeURIComponent(name);

	/* Our own password is what we log in with from here on */
	function relogin(user) {
		if (user.name !== self || !user.password?.startsWith('$0$'))
			return;
		auth = basic(self, user.password.slice(3));
		sessionStorage.setItem('auth', auth);
	}

	function row(user) {
		let body;

		const form = () => {
			body = group(sn, user, USERS, 'user');

			return settings(save, body.el, 'Save changes',
				user.name === self ? [] : h('button', { type: 'button', class: 'danger', onclick: remove }, 'Remove'));
		};

		async function save() {
			const next = structuredClone(user);

			body.apply(next);
			await put(path(user.name), { 'ietf-system:user': [next] });
			relogin(next);

			/* For the hash the device made of a new password */
			render();
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

		return fold('user ' + user.name, [
			h('b', null, user.name),
			h('span', { class: 'muted' }, user.name === self ? 'You' : ''),
		], form);
	}

	/* A new user is a name and a password, the rest is set on its row */
	const fresh = group(sn, undefined, {
		...USERS,
		hide: [],
		only: ['user/name', 'user/password'],
		labels: { 'user/name': 'User name' },
	}, 'user');

	async function add() {
		const user = {};

		fresh.apply(user);
		if (!user.name)
			throw new Error('Give the user a name.');
		await post('ietf-system:system/authentication', { 'ietf-system:user': [user] });
		opened.add('user ' + user.name);
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
			settings(add, fresh.el, 'Add user')),
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
		schema ??= await (await fetch('schema.json')).json();
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
	$('#whoami').textContent = whoami();
	live(false);
	hostname();
	render();
	listen();
}

function logout(why = '') {
	auth = null;
	sessionStorage.removeItem('auth');
	stream?.abort();
	view.replaceChildren();
	$('#confirm').close();
	$('#app').hidden = true;
	$('#login').hidden = false;
	$('#login .error').textContent = why;
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

	since = Date.now();
	sessionStorage.setItem('auth', auth);
	sessionStorage.setItem('since', since);
	form.elements.pass.value = '';
	start();
});

$('#logout').addEventListener('click', () => logout());
$('#notice button').addEventListener('click', () => render());
view.addEventListener('input', (ev) => edited.add(ev.target.closest('form')));
window.addEventListener('hashchange', () => render());

/* Kept with the login, so that reloading the page does not start the clock over */
function active() {
	const now = Date.now();

	if (now - seen > 10000)
		sessionStorage.setItem('seen', now);
	seen = now;
}

/* Has whoever logged in left, or been in for too long?  Then out we go, and say why. */
function left() {
	const now = Date.now();

	if (now - since > SESSION * 3600000)
		logout(`Logged out, ${SESSION} hours after logging in.`);
	else if (idle && now - seen > idle * 60000)
		logout(`Logged out after ${idle} minutes without activity.`);

	return !auth;
}

for (const type of ['pointermove', 'pointerdown', 'keydown', 'scroll'])
	window.addEventListener(type, active, { capture: true, passive: true });

setInterval(() => {
	/* Counters and link state move without a configuration change */
	if (auth && !left() && current === 'status' && !document.hidden)
		render(true);
}, 5000);

/* Light or dark; with neither chosen the style sheet leaves it to the browser */
$('#theme').value = document.documentElement.style.colorScheme = localStorage.getItem('theme') || '';
$('#theme').addEventListener('change', (ev) => {
	document.documentElement.style.colorScheme = ev.target.value;
	localStorage.setItem('theme', ev.target.value);
});

$('#idle').value = idle;
$('#idle').addEventListener('change', (ev) => {
	idle = Number(ev.target.value);
	localStorage.setItem('idle', idle);
});

/* The menu folds up when the click, or the Escape, is not for it */
window.addEventListener('click', (ev) => {
	if (!$('#user').contains(ev.target) || ev.target.closest('a, button'))
		$('#user').open = false;
});
window.addEventListener('keydown', (ev) => {
	if (ev.key === 'Escape')
		$('#user').open = false;
});

if (!auth)
	logout();
else if (!left())
	start();
