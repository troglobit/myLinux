/*
 * Forms from the YANG models.  schema.json is the configuration part of
 * the models as the device has them, written by yang2json at build time:
 * what there is to set, of what type, and what refers to what.  A page
 * says which part of it to show and what to call things; the widgets and
 * what they accept follow from the model.
 */
'use strict';

const $ = (sel) => document.querySelector(sel);

let schema = null;

/* Build an element; text always goes in as text, never as markup */
function h(tag, attrs, ...kids) {
	const el = document.createElement(tag);

	for (const [key, val] of Object.entries(attrs || {})) {
		if (key.startsWith('on'))
			el.addEventListener(key.slice(2), val);
		else if (val === true)
			el.setAttribute(key, '');
		else if (val !== false && val != null)
			el.setAttribute(key, val);
	}
	el.append(...kids.flat());

	return el;
}

/* Set, or with nothing to set remove, a member of a configuration object */
function set(obj, key, value) {
	if (value === '' || value == null)
		delete obj[key];
	else
		obj[key] = value;
}

const option = (value, text, selected) => h('option', { value, selected: !!selected }, text);
const local = (name) => name.split(':').pop();
const isEmpty = (obj) => Object.keys(obj).length === 0;

/* "prefix-length" is "Prefix length" until a page knows a better name */
function label(ctx, path) {
	const name = local(path.split('/').pop()).replaceAll('-', ' ');

	return ctx.labels?.[path] ?? name[0].toUpperCase() + name.slice(1);
}

/* The first sentence of a description does for a hint */
const hint = (sn) => (sn.description || '').replace(/\s+/g, ' ').match(/^.*?\.(?= |$)/)?.[0];

/* The data nodes of a schema node by name.  Choices and cases are not data. */
function members(sn, map = new Map()) {
	for (const [name, child] of Object.entries(sn.children || {})) {
		if (child.kind === 'choice' || child.kind === 'case')
			members(child, map);
		else
			map.set(name, child);
	}

	return map;
}

/* The schema node at a data path, like ietf-system:system/ntp/server */
function schemaAt(path) {
	let sn = { children: schema.data };

	for (const name of path.split('/')) {
		sn = members(sn).get(name);
		if (!sn)
			throw new Error(`The model has no ${path}.`);
	}

	return sn;
}

/* An identity and all that derive from it */
function derived(base, list = []) {
	for (const id of schema.identities[base] || []) {
		list.push(id);
		derived(id, list);
	}

	return list;
}

/*
 * A node with a when statement is there for some only.  What our models
 * ask is of what type the list entry is; anything else we let through,
 * and the device has the last word.
 */
function applies(sn, ctx) {
	return (sn.when || []).flatMap((expr) => expr.split(' and ')).every((expr) => {
		const m = expr.match(/^(not\()?derived-from-or-self\((?:[\w-]+:)?([\w-]+), *'([^']+)'\)\)?$/);

		if (!m)
			return true;

		const value = ctx.entry?.[m[2]];
		const is = value === m[3] || derived(m[3]).includes(value);

		return m[1] ? !is : is;
	});
}

/* A page hides some of what there is, or says what only to show */
const shown = (sn, ctx, path) =>
	!ctx.hide?.includes(path) && applies(sn, ctx) &&
	(!ctx.only || ctx.only.some((at) => at === path || path.startsWith(at + '/')));

/*
 * A container that is all there is to a case says nothing its parent
 * does not, like the udp in the transport of a time server.  Its fields
 * are shown as if they were the parent's.
 */
const inlined = (sn, parent) =>
	sn.kind === 'container' && !sn.presence && parent.kind === 'case' && Object.keys(parent.children).length === 1;

/* The cases of a choice that have something to show */
function cases(sn, ctx, path) {
	return Object.entries(sn.children || {}).filter(([, cs]) =>
		[...members(cs)].some(([name, child]) => shown(child, ctx, `${path}/${name}`)));
}

/* How many plain fields a list entry is, or NaN if it is more than fields */
function plain(sn, ctx, path) {
	let count = 0;

	for (const [name, child] of Object.entries(sn.children || {})) {
		const at = `${path}/${name}`;

		if (child.kind === 'choice') {
			const live = cases(child, ctx, path);

			if (live.length > 1)
				return NaN;
			if (live.length)
				count += plain(live[0][1], ctx, path);
		} else if (!shown(child, ctx, at)) {
			continue;
		} else if (inlined(child, sn)) {
			count += plain(child, ctx, at);
		} else if (child.kind === 'leaf' && child.type.typedef !== 'crypt-hash') {
			count++;
		} else {
			return NaN;
		}
	}

	return count;
}

/* A list entry of a few plain fields fits on a line */
const isFlat = (sn, ctx, path) => plain(sn, ctx, path) <= 4;

/*
 * Controls: one value of a leaf or leaf-list.  Each has the element to
 * show and read(), which returns the value as RESTCONF wants it, nothing
 * for no value, and throws what to tell the user if it is no good.
 */

function selectOf(values, value, sn, names = {}) {
	const el = h('select', null,
		sn.mandatory || sn.default !== undefined ? [] : option('', '', value === undefined),
		/* What is set stays on offer, also when it is not among the choices */
		[...new Set(value === undefined ? values : [...values, value])].map((val) =>
			option(val, names[val] ?? local(val), val === (value ?? sn.default))));

	/* A default that is on show is not thereby configured */
	return { el, read: () => (value === undefined && el.value === sn.default ? undefined : el.value || undefined) };
}

/* Zones as the browser knows them; the device may know others, so only suggestions */
function zones() {
	if (!$('#zones'))
		document.body.append(h('datalist', { id: 'zones' },
			(Intl.supportedValuesOf?.('timeZone') || []).map((zone) => option(zone, zone))));

	return 'zones';
}

/* A password is set, never shown.  The device hashes one that is marked as clear text. */
function secret(value) {
	const attrs = { type: 'password', autocomplete: 'new-password' };
	const pass = h('input', { ...attrs, placeholder: value ? 'Unchanged' : 'None', 'aria-label': 'New password' });
	const again = h('input', { ...attrs, placeholder: 'Repeat it', 'aria-label': 'Repeat the new password' });

	return {
		el: h('span', { class: 'pair' }, pass, again),
		read() {
			if (pass.value !== again.value)
				throw new Error('The two passwords differ.');

			return pass.value ? '$0$' + pass.value : value;
		},
	};
}

function control(sn, value, ctx, path) {
	const type = sn.type;
	const base = type.base === 'leafref' ? type.type.base : type.base;
	const refs = type.base === 'leafref' && ctx.refs?.(path);
	let el;

	if (refs)
		return selectOf(refs, value, sn);
	if (base === 'identityref') {
		const ids = type.bases.flatMap((id) => derived(id));

		/* An identity of the leaf's own module comes without the module */
		return selectOf(ids, ids.find((id) => local(id) === value) ?? value, sn, ctx.names);
	}
	if (base === 'enumeration')
		return selectOf(type.enum, value, sn);
	if (type.typedef === 'crypt-hash')
		return secret(value);

	if (base === 'boolean' || base === 'empty') {
		const dflt = sn.default === undefined ? undefined : sn.default === 'true';

		el = h('input', { type: 'checkbox', checked: base === 'empty' ? value !== undefined : value ?? dflt ?? false });

		return {
			el,
			read() {
				if (base === 'empty')
					return el.checked ? [null] : undefined;

				/* Not what is so anyway, nor a no where nothing was said */
				return el.checked === dflt || (!el.checked && value === undefined) ? undefined : el.checked;
			},
		};
	}

	if (/^u?int/.test(base)) {
		el = h('input', { type: 'number', value, placeholder: sn.default,
				  min: type.range?.[0][0], max: type.range?.at(-1)[1] });

		return {
			el,
			read() {
				if (!el.checkValidity())
					throw new Error(`${label(ctx, path)}: ${el.validationMessage}`);

				return el.value === '' ? undefined : Number(el.value);
			},
		};
	}

	if (base === 'binary')
		el = h('textarea', { class: 'addr', spellcheck: 'false', rows: 2 }, value ?? '');
	else
		el = h('input', { value, placeholder: sn.default, maxlength: type.length?.at(-1)[1],
				  list: type.typedef === 'timezone-name' ? zones() : null,
				  class: /address|host/.test(type.typedef) ? 'addr' : null,
				  spellcheck: 'false', autocapitalize: 'none' });

	return { el, read: () => el.value.trim().replace(base === 'binary' ? /\s+/g : '', '') || undefined };
}

/*
 * Parts: what there is to see of one node, and apply(obj) which puts
 * what the user made of it into the configuration object it is part of.
 */

function leaf(name, sn, value, ctx, path) {
	const ctl = control(sn, value, ctx, path);
	const text = label(ctx, path);
	const box = ctl.el.type === 'checkbox';
	const title = hint(sn);

	ctl.el.title = title || '';
	if (ctx.bare && !box) {
		ctl.el.placeholder = sn.default === undefined ? text : `${text} (${sn.default})`;
		ctl.el.setAttribute('aria-label', text);
	}

	return {
		el: box ? h('label', { class: 'check', title }, ctl.el, text)
			: ctx.bare ? ctl.el
			: h('label', { title }, sn.units ? `${text}, ${sn.units}` : text, ctl.el),
		apply(obj) {
			const val = ctl.read();

			if (val === undefined && sn.mandatory)
				throw new Error(`${text} is needed.`);
			set(obj, name, val);
		},
	};
}

/* Rows that come and go: the entries of a list, the values of a leaf-list */
function rows(title, items, make, what) {
	const list = h('div', { class: 'rows' });
	const parts = new Set();

	function add(item, fresh) {
		const part = make(item, fresh, () => {
			parts.delete(part);
			part.el.remove();
			list.dispatchEvent(new Event('input', { bubbles: true }));
		});

		part.fresh = fresh;
		parts.add(part);
		list.append(part.el);
		if (fresh)
			part.el.querySelector('input, select, textarea')?.focus();
	}

	items.forEach((item) => add(item, false));

	return {
		el: h('fieldset', null, h('legend', null, title), list,
			h('button', { type: 'button', class: 'quiet add', onclick: () => add(undefined, true) }, `Add ${what}`)),
		parts,
	};
}

/* Added, and then nothing was typed in it */
function isBlank(part) {
	const typed = [...part.el.querySelectorAll('input:not([type=checkbox]), textarea')];

	return part.fresh && typed.length > 0 && typed.every((el) => !el.value);
}

const remover = (text, onclick) =>
	h('button', { type: 'button', class: 'quiet remove', title: text, 'aria-label': text, onclick }, '×');

function leafList(name, sn, values, ctx, path) {
	const text = label(ctx, path);
	const { el, parts } = rows(text, values || [], (value, fresh, remove) => {
		const ctl = control(sn, value, ctx, path);

		ctl.el.setAttribute('aria-label', text);

		return { el: h('div', { class: 'line' }, ctl.el, remover('Remove', remove)), read: ctl.read };
	}, ctx.items?.[path] ?? local(name));

	return {
		el,
		apply(obj) {
			const list = [...parts].map((part) => part.read()).filter((val) => val !== undefined);

			set(obj, name, list.length ? list : null);
		},
	};
}

function list(name, sn, entries, ctx, path) {
	const what = ctx.items?.[path] ?? local(name);
	const flat = isFlat(sn, ctx, path);
	const { el, parts } = rows(label(ctx, path), entries || [], (entry, fresh, remove) => {
		const sub = { ...ctx, entry: entry || {}, bare: flat };
		const body = group(sn, entry, sub, path);
		const title = () => sn.keys.map((key) => entry?.[key]).join(' ') || `New ${what}`;
		const part = {
			read() {
				const next = structuredClone(entry || {});

				body.apply(next);
				ctx.fill?.[path]?.(next);

				return next;
			},
		};

		part.el = flat ? h('div', { class: 'line' }, body.el, remover(`Remove ${title()}`, remove))
			: h('details', { class: 'entry', open: fresh },
				h('summary', null, title()), body.el,
				h('button', { type: 'button', class: 'quiet', onclick: remove }, 'Remove'));

		return part;
	}, what);

	return {
		el,
		apply(obj) {
			const next = [...parts].filter((part) => !isBlank(part)).map((part) => part.read());

			set(obj, name, next.length ? next : null);
		},
	};
}

function container(name, sn, value, ctx, path, parent) {
	const body = group(sn, value, ctx, path);
	const text = label(ctx, path);
	const implied = ctx.implied?.includes(path);
	let box = null;
	let el = body.el;

	if (sn.presence && !implied) {
		const toggle = () => { body.el.hidden = !box.checked; };

		box = h('input', { type: 'checkbox', checked: value !== undefined, onchange: toggle });
		el = h('label', { class: 'check', title: hint(sn) }, box, text);

		/* With something in it, that is shown when it is there */
		if (!isEmpty(sn.children || {})) {
			toggle();
			el = h('fieldset', null, h('legend', null, el), body.el);
		}
	} else if (!inlined(sn, parent)) {
		el = h('fieldset', null, h('legend', null, text), body.el);
	}

	return {
		el,
		inline: el === body.el,
		apply(obj) {
			const next = structuredClone(value || {});

			if (box && !box.checked)
				return set(obj, name, null);
			body.apply(next);

			/* Empty it is there only if being there is what it says */
			set(obj, name, isEmpty(next) && !box ? null : next);
		},
	};
}

/* One of several cases: which, and the fields of that one */
function choice(sn, value, ctx, path, live) {
	const has = (cs) => [...members(cs).keys()].some((name) => value?.[name] !== undefined);
	const now = (live.find(([, cs]) => has(cs)) || live[0])[0];
	const pick = h('select', { onchange: show }, live.map(([name]) => option(name, label(ctx, `${path}/${name}`), name === now)));
	const slot = h('div', { class: 'fields' });
	let body;

	function show() {
		body = group(sn.children[pick.value], value, ctx, path);
		slot.replaceChildren(body.el);
	}
	show();

	return {
		el: h('fieldset', null, h('legend', null, h('label', null, label(ctx, path), pick)), slot),
		apply(obj) {
			for (const [name, cs] of live) {
				if (name !== pick.value)
					members(cs).forEach((_, member) => delete obj[member]);
			}
			body.apply(obj);
		},
	};
}

/*
 * The fields of a container, a list entry or a case.  value is what is
 * configured of it now.  ctx is the page's say: hide, labels, options,
 * refs() for what a leafref may point at, and entry, the list entry we
 * are in, which is what a when statement asks about.
 */
function group(sn, value, ctx, path) {
	const el = h('div', { class: ctx.bare ? 'bare' : 'fields' });
	const parts = [];

	for (const [name, child] of Object.entries(sn.children || {})) {
		const at = `${path}/${name}`;
		let part;

		if (child.kind === 'choice') {
			const live = cases(child, ctx, path);

			if (!live.length)
				continue;
			part = live.length > 1 ? choice(child, value, ctx, path, live)
				: { ...group(live[0][1], value, ctx, path), inline: true };
		} else if (!shown(child, ctx, at)) {
			continue;
		} else if (child.kind === 'leaf') {
			part = leaf(name, child, value?.[name], ctx, at);
		} else if (child.kind === 'leaf-list') {
			part = leafList(name, child, value?.[name], ctx, at);
		} else if (child.kind === 'list') {
			part = list(name, child, value?.[name], ctx, at);
		} else {
			part = container(name, child, value?.[name], ctx, at, sn);
		}

		parts.push(part);
		/* What is shown as ours joins our fields, without a box of its own */
		el.append(...(part.inline ? part.el.childNodes : [part.el]));
	}

	return { el, apply: (obj) => parts.forEach((part) => part.apply(obj)) };
}
