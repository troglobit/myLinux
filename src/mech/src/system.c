/* SPDX-License-Identifier: Apache-2.0 */

#include <augeas.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <shadow.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define XMLNS "urn:ietf:params:xml:ns:yang:ietf-system"
#define MYSYS_NS "urn:mylinux:system"

#define LOGIN_SHELL "/bin/sh"
#define ADMIN_USER  "admin"		/* reserved super-user, until NACM */
#define ADMIN_GROUP "wheel"		/* UNIX group with admin access */
#define SHADOW_FILE "/etc/shadow"

static augeas *aug;
static char   *ver = NULL;
static char   *rel = NULL;
static char   *sys = NULL;
static char   *os  = NULL;

/**
 * Like system(), but takes a formatted string as argument.
 * @param fmt  printf style format list to command to run
 *
 * This system() wrapper greatly simplifies operations that usually
 * consist of composing a command from parts into a dynamic buffer
 * before calling it.  The return value from system() is also parsed,
 * checking for proper exit and signals.
 *
 * @returns If the command exits normally, the return code of the command
 * is returned.  Otherwise, if the command is signalled, the return code
 * is -1 and @a errno is set to @c EINTR.
 */
static int run(const char *fmt, ...)
{
	va_list ap;
	char *cmd;
	int len;
	int rc;

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	cmd = alloca(++len);
	if (!cmd) {
		errno = ENOMEM;
		return -1;
	}

	va_start(ap, fmt);
	vsnprintf(cmd, len, fmt, ap);
	va_end(ap);

	rc = system(cmd);
	if (rc == -1)
		return -1;

	if (WIFEXITED(rc)) {
		errno = 0;
		rc = WEXITSTATUS(rc);
	} else if (WIFSIGNALED(rc)) {
		errno = EINTR;
		rc = -1;
	}

//	clixon_debug(CLIXON_DBG_DEFAULT, "ietf-system:run(%s) => %d", cmd, rc);

	return rc;
}

static bool is_true(cxobj *xp, char *name)
{
	cxobj *obj;

	if (!xp || !name)
		return false;

	obj = xml_find(xp, name);
	if (obj && !strcmp(xml_body(obj), "true"))
		return true;

	return false;
}

static char *get_str(cxobj *xp, char *name)
{
	cxobj *obj;

	if (!xp || !name)
		return NULL;

	obj = xml_find(xp, name);
	if (obj)
		return xml_body(obj);

	return NULL;
}

static int sys_reload_services(void)
{
	return run("initctl -nbq touch sysklogd lldpd");
}

int ietf_sys_tr_begin(clixon_handle h, transaction_data td)
{
	return aug_load(aug);
}

int ietf_sys_tr_commit_hostname(cxobj *src, cxobj *tgt)
{
	const char *host, *new, *tmp;
	int err, i, nhosts;
	char **hosts, *old;

	if (src && xml_flag(src, XML_FLAG_DEL))
		new = "default";	/* XXX: derive from global "options.h" */
	else if (tgt && xml_flag(tgt, XML_FLAG_ADD|XML_FLAG_CHANGE))
		new = xml_body(tgt);
	else
		return 0;

	aug_get(aug, "etc/hostname/hostname", &tmp);
	old = strdup(tmp);

	err = sethostname(new, strlen(new));
	err = err ? : aug_set(aug, "etc/hostname/hostname", new);

	nhosts = aug_match(aug, "etc/hosts/*/canonical", &hosts);
	for (i = 0; i < nhosts; i++) {
		aug_get(aug, hosts[i], &host);
		if (!strcmp(host, old))
			err = err ? : aug_set(aug, hosts[i], new);
		free(hosts[i]);
	}
	free(hosts);
	free(old);

	if (src)
		err = err ? : sys_reload_services();

	return err;
}

/*
 * On GLIBC systems the file /etc/timezone contains the name of the
 * current timezone, the file /etc/localtime is a copy or symlink to
 * the file /usr/share/zoneinfo/$(cat /etc/timezone)
 */
int ietf_sys_tr_commit_clock(cxobj *src, cxobj *tgt)
{
	char *timezone;
	char cmd[512];
	cxobj *obj;

	if (!tgt)
		return 0;

	obj = xml_find(tgt, "timezone-name");
	if (!obj)
		return 0;

	timezone = xml_body(obj);

#ifdef __UCLIBC__
	snprintf(cmd, sizeof(cmd), "ln -sf /usr/share/zoneinfo/uclibc/%s /etc/TZ", timezone);
	if (run(cmd)) {
		WARN("ietf-system: failed setting timezone %s", timezone);
		return -1;
	}
	return 0;
#else
	const char *fn = "/etc/timezone";
	FILE *fp;

	snprintf(cmd, sizeof(cmd), "cp /usr/share/zoneinfo/%s /etc/localtime", timezone);
	if (run(cmd)) {
		WARN("ietf-system: failed setting timezone %s", timezone);
		return -1;
	}

	fp = fopen(fn, "w");
	if (!fp) {
		WARN("ietf-system: failed updating %s: %s", fn, strerror(errno));
		return -1;
	}
	fprintf(fp, "%s\n", timezone);

	return fclose(fp);
#endif
}

/* True unless file `fn` already holds exactly `want`. */
static int file_differs(const char *fn, const char *want)
{
	char buf[1024];
	size_t n;
	FILE *fp;

	fp = fopen(fn, "r");
	if (!fp)
		return 1;
	n = fread(buf, 1, sizeof(buf) - 1, fp);
	fclose(fp);
	buf[n] = 0;

	return strcmp(buf, want) != 0;
}

/*
 * sntpd is myLinux's NTP client.  It has no config file; the server list
 * goes on its command line, so the finit service pulls it from
 * SNTPD_OPTIONS in the env file written here.  Each server is rendered
 * host[:port][,iburst][,prefer].  association-type is not represented:
 * sntpd treats the list as a failover set and fans a hostname out over
 * its resolved addresses, which covers 'pool'; 'peer' (symmetric active)
 * is out of scope for SNTP.
 */
int ietf_sys_tr_commit_ntp(cxobj *src, cxobj *tgt)
{
	const char *fn = "/etc/default/sntpd";
	cxobj *srv = NULL;
	char *enabled;
	int n = 0, changed;
	cbuf *cb;

	if (!tgt)
		goto disable;

	cb = cbuf_new();
	if (!cb) {
		clixon_err(OE_UNIX, errno, "ietf-system:cbuf_new");
		return -1;
	}

	cprintf(cb, "SNTPD_OPTIONS=\"");
	while ((srv = xml_child_each(tgt, srv, CX_ELMNT))) {
		char *addr, *port;
		cxobj *udp;

		if (strcmp(xml_name(srv), "server"))
			continue;

		udp = xml_find(srv, "udp");
		addr = udp ? get_str(udp, "address") : NULL;
		if (!addr)
			continue;

		cprintf(cb, "%s%s", n ? " " : "", addr);
		port = get_str(udp, "port");
		if (port && strcmp(port, "123"))
			cprintf(cb, ":%s", port);
		if (is_true(srv, "iburst"))
			cprintf(cb, ",iburst");
		if (is_true(srv, "prefer"))
			cprintf(cb, ",prefer");
		n++;
	}
	cprintf(cb, "\"\n");

	/* Rewrite (and restart sntpd) only when the list actually changes,
	 * so an unrelated commit does not reset its phase-lock loop. */
	changed = file_differs(fn, cbuf_get(cb));
	if (changed) {
		FILE *fp = fopen(fn, "w");

		if (!fp) {
			WARN("ietf-system: failed updating %s: %s", fn, strerror(errno));
			cbuf_free(cb);
			return -1;
		}
		fputs(cbuf_get(cb), fp);
		fclose(fp);
	}
	cbuf_free(cb);

	/* 'enabled' defaults to true; only an explicit false disables. */
	enabled = get_str(tgt, "enabled");
	if (n && (!enabled || strcmp(enabled, "false"))) {
		if (changed)
			run("initctl -nbq touch sntpd");
		return run("initctl -nbq enable sntpd");
	}
disable:
	return run("initctl -nbq disable sntpd");
}

/*
 * dnsmasq is the resolver, it gets its nameservers from various
 * sources: static, dhcp, vpn, via the resolvconf wrapper.
 */
int ietf_sys_tr_commit_dns(cxobj *src, cxobj *tgt)
{
	const char *fn = "/etc/resolv.conf.head";
	cxobj *obj, *opt = NULL, *srv = NULL;
	int first = 1;
	FILE *fp;

	/* No dns-resolver config: drop the head, leaving resolvconf to fall
	 * back to whatever the interfaces (e.g. DHCP) provide. */
	if (!tgt) {
		remove(fn);
		goto reload;
	}

	fp = fopen(fn, "w");
	if (!fp) {
		WARN("ietf-system: failed updating %s: %s", fn, strerror(errno));
		return -1;
	}

	obj = xml_find(tgt, "options");
	if (obj) {
		char *str;

		fprintf(fp, "options");
		str = get_str(obj, "timeout");
		if (str)
			fprintf(fp, " timeout:%s", str);
		str = get_str(obj, "attempts");
		if (str)
			fprintf(fp, " attempts:%s ", str);
		fprintf(fp, "\n");
	}

	while ((opt = xml_child_each(tgt, opt, CX_ELMNT))) {
		if (strcmp(xml_name(opt), "search"))
			continue;

		if (first) {
			fprintf(fp, "search ");
			first = 0;
		}
		fprintf(fp, "%s ", xml_body(opt));
	}
	if (!first)
		fprintf(fp, "\n");

	/* Tag each server with its YANG key in a comment resolvconf ignores,
	 * so the list round-trips back into operational state (below). */
	while ((srv = xml_child_each(tgt, srv, CX_ELMNT))) {
		char *name;
		cxobj *tmp;

		if (strcmp(xml_name(srv), "server"))
			continue;

		tmp = xml_find(srv, "udp-and-tcp");
		obj = tmp ? xml_find(tmp, "address") : NULL;
		if (!obj)
			continue;

		name = get_str(srv, "name");
		if (name)
			fprintf(fp, "# id:%s\n", name);
		fprintf(fp, "nameserver %s\n", xml_body(obj));
	}
	fclose(fp);

reload:
	/* resolvconf rebuilds resolv.conf from the head we just wrote, but
	 * only once the system is up (the udhcpc hook has run). */
	if (run("initctl cond get hook/sys/up"))
		return 0;

	return run("resolvconf -u");
}

/*
 * User names reach system() and file paths below, so constrain them to
 * a safe character set (letters, digits, and _-.) before use -- no '/'
 * to escape /home, no shell metacharacters to inject commands.
 */
static int is_valid_username(const char *user)
{
	size_t i;

	if (!user || !user[0])
		return 0;

	if (!isalpha((unsigned char)user[0]) && user[0] != '_')
		return 0;

	for (i = 1; user[i]; i++) {
		char c = user[i];

		if (!isalnum((unsigned char)c) && c != '_' && c != '-' && c != '.')
			return 0;
	}

	return 1;
}

/*
 * Install a user's password hash straight into /etc/shadow.  BusyBox
 * chpasswd is not built, so we rewrite the file directly, clearing the
 * aging fields since the wall clock is unreliable at boot.
 * An empty hash disables password login ("*") but still permits key auth.
 */
static int set_password(const char *user, const char *hash)
{
	const char *pw = (hash && *hash) ? hash : "*";
	struct spwd *sp;
	FILE *fp;
	int fd;

	if (lckpwdf()) {
		WARN("ietf-system: cannot lock %s", SHADOW_FILE);
		return -1;
	}

	setspent();
	fp = fopen(SHADOW_FILE "+", "w");
	if (!fp)
		goto fail;

	fd = fileno(fp);
	if (fd == -1 || fchown(fd, 0, 0) || fchmod(fd, 0600)) {
		fclose(fp);
		goto fail;
	}

	while ((sp = getspent())) {
		struct spwd usp;

		if (!strcmp(sp->sp_namp, user)) {
			usp = *sp;
			usp.sp_pwdp   = (char *)pw;
			usp.sp_lstchg = -1;
			usp.sp_min    = -1;
			usp.sp_max    = -1;
			usp.sp_warn   = -1;
			usp.sp_inact  = -1;
			usp.sp_expire = -1;
			sp = &usp;
		}

		if (putspent(sp, fp)) {
			fclose(fp);
			goto fail;
		}
	}

	if (fclose(fp) || rename(SHADOW_FILE "+", SHADOW_FILE))
		goto fail;

	endspent();
	ulckpwdf();

	return 0;
fail:
	endspent();
	ulckpwdf();
	WARN("ietf-system: failed setting password for user %s", user);

	return -1;
}

/*
 * Public keys go in a root-owned authorized_keys so sshd's StrictModes is
 * satisfied and users cannot tamper with their own key set.  This moves to
 * /run/ssh/%u once we wire AuthorizedKeysFile.
 */
static void set_authorized_keys(cxobj *user, const char *name)
{
	cxobj *key = NULL;
	char path[256];
	FILE *fp;

	run("mkdir -p /home/%s/.ssh", name);

	snprintf(path, sizeof(path), "/home/%s/.ssh/authorized_keys", name);
	fp = fopen(path, "w");
	if (!fp) {
		WARN("ietf-system: cannot write %s", path);
		return;
	}

	while ((key = xml_child_each(user, key, CX_ELMNT))) {
		const char *alg, *data, *kn;

		if (strcmp(xml_name(key), "authorized-key"))
			continue;

		alg  = get_str(key, "algorithm");
		data = get_str(key, "key-data");
		kn   = get_str(key, "name");

		/* Guard against a stray newline forging extra key lines. */
		if (!alg || !data || strpbrk(alg, " \t\r\n") || strpbrk(data, " \t\r\n"))
			continue;
		if (kn && strpbrk(kn, "\r\n"))
			kn = NULL;

		fprintf(fp, "%s %s %s\n", alg, data, kn ? kn : name);
	}

	fclose(fp);
	chmod(path, 0644);
}

static int usr_del(const char *name)
{
	if (!is_valid_username(name))
		return 0;

	if (run("deluser --remove-home %s", name))
		WARN("ietf-system: failed deleting user %s", name);

	return 0;
}

/*
 * The mylinux-system augment adds an identityref login shell.  The value
 * may arrive prefixed (e.g. "mysys:false"); anything but the no-login
 * identity maps to the default POSIX shell.  A shell change only takes on
 * the next boot, when the ephemeral /etc rebuilds the account from config.
 */
static const char *usr_shell(cxobj *user)
{
	const char *sh = get_str(user, "shell");

	if (sh && (strstr(sh, "false") || strstr(sh, "nologin")))
		return "/bin/false";
	if (sh && strstr(sh, "clish"))
		return "/usr/bin/clish";

	return LOGIN_SHELL;
}

static int usr_set(cxobj *user)
{
	const char *name = get_str(user, "name");

	if (!is_valid_username(name)) {
		WARN("ietf-system: rejecting invalid user name \"%s\"", name ? name : "(null)");
		return 0;
	}

	if (!getpwnam(name)) {
		if (run("adduser -D -s %s %s", usr_shell(user), name)) {
			WARN("ietf-system: failed creating user %s", name);
			return 0;
		}

		/* For now only the reserved admin user gets admin rights;
		 * later this follows NACM group membership. */
		if (!strcmp(name, ADMIN_USER))
			run("adduser %s %s", name, ADMIN_GROUP);
	}

	set_password(name, get_str(user, "password"));
	set_authorized_keys(user, name);

	return 0;
}

/*
 * RFC 7317 local users.  Deletions come from the source (running) tree,
 * additions and changes from the target (candidate).  On the startup
 * commit every user is flagged as an addition -- that is how the factory
 * admin user is created on first boot.
 */
int ietf_sys_tr_commit_auth(cxobj *src, cxobj *tgt)
{
	cxobj *user;

	user = NULL;
	while (src && (user = xml_child_each(src, user, CX_ELMNT))) {
		if (strcmp(xml_name(user), "user"))
			continue;
		if (xml_flag(user, XML_FLAG_DEL))
			usr_del(get_str(user, "name"));
	}

	user = NULL;
	while (tgt && (user = xml_child_each(tgt, user, CX_ELMNT))) {
		if (strcmp(xml_name(user), "user"))
			continue;
		if (xml_flag(user, XML_FLAG_ADD | XML_FLAG_CHANGE))
			usr_set(user);
	}

	return 0;
}

int ietf_sys_tr_commit(clixon_handle h, transaction_data td)
{
	cxobj *src = transaction_src(td), *tgt = transaction_target(td);
	yang_stmt *yspec = clicon_dbspec_yang(h);
	int slen = 0, tlen = 0, err = -EINVAL;
	cxobj **ssys, **tsys;

//	show_transaction("ietf-system", td, true);
	if (src && clixon_xml_find_instance_id(src, yspec, &ssys, &slen,
					       "/sys:system") < 0)
		goto err;

	if (tgt && clixon_xml_find_instance_id(tgt, yspec, &tsys, &tlen,
					       "/sys:system") < 0)
		goto err;

	err = ietf_sys_tr_commit_hostname(slen ? xml_find(ssys[0], "hostname") : NULL,
					  tlen ? xml_find(tsys[0], "hostname") : NULL);
	if (err)
		goto err;

	err = ietf_sys_tr_commit_clock(slen ? xml_find(ssys[0], "clock") : NULL,
				       tlen ? xml_find(tsys[0], "clock") : NULL);
	if (err)
		goto err;

	err = ietf_sys_tr_commit_ntp(slen ? xml_find(ssys[0], "ntp") : NULL,
				     tlen ? xml_find(tsys[0], "ntp") : NULL);
	if (err)
		goto err;

	err = ietf_sys_tr_commit_dns(slen ? xml_find(ssys[0], "dns-resolver") : NULL,
				     tlen ? xml_find(tsys[0], "dns-resolver") : NULL);
	if (err)
		goto err;

	err = ietf_sys_tr_commit_auth(slen ? xml_find(ssys[0], "authentication") : NULL,
				      tlen ? xml_find(tsys[0], "authentication") : NULL);
	if (err)
		goto err;
err:
	err = err ? : aug_save(aug);
	return err;
}

static char *strip_quotes(char *str)
{
	char *ptr;

	while (*str && (isspace(*str) || *str == '"'))
		str++;

	for (ptr = str + strlen(str); ptr > str; ptr--) {
		if (*ptr != '"')
			continue;

		*ptr = 0;
		break;
	}

	return str;
}

static void setvar(char *line, const char *nm, char **var)
{
	char *ptr;

	if (!strncmp(line, nm, strlen(nm)) && (ptr = strchr(line, '='))) {
		if (*var)
			free(*var);
		*var = strdup(strip_quotes(++ptr));
	}
}

static void os_init(void)
{
	struct utsname uts;
	char line[80];
	FILE *fp;

	if (!uname(&uts)) {
		os  = strdup(uts.sysname);
		ver = strdup(uts.release);
		rel = strdup(uts.release);
		sys = strdup(uts.machine);
	}

	fp = fopen("/etc/os-release", "r");
	if (!fp) {
		fp = fopen("/usr/lib/os-release", "r");
		if (!fp)
			return;
	}

	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = 0; /* drop \n */
		setvar(line, "NAME", &os);
		setvar(line, "VERSION_ID", &ver);
		setvar(line, "BUILD_ID", &rel);
		setvar(line, "ARCHITECTURE", &sys);
	}
	fclose(fp);
}

static char *fmtime(time_t t, char *buf, size_t len)
{
        const char *isofmt = "%FT%T%z";
        struct tm tm;
        size_t i, n;

        localtime_r(&t, &tm);
        n = strftime(buf, len, isofmt, &tm);
        i = n - 5;
        if (buf[i] == '+' || buf[i] == '-') {
                buf[i + 6] = buf[i + 5];
                buf[i + 5] = buf[i + 4];
                buf[i + 4] = buf[i + 3];
                buf[i + 3] = ':';
        }

        return buf;
}

/* Reverse of usr_shell(): map an /etc/passwd login shell to its identity. */
static const char *shell_ident(const char *sh)
{
	if (sh && strstr(sh, "clish"))
		return "clish";
	if (sh && (strstr(sh, "false") || strstr(sh, "nologin")))
		return "false";

	return "sh";
}

/*
 * The applied time-zone name: uClibc records it in the /etc/TZ symlink
 * (-> .../zoneinfo/uclibc/<name>), glibc in /etc/timezone.  Fills buf and
 * returns 0 on success.
 */
static int get_timezone(char *buf, size_t len)
{
	const char *mark = "/zoneinfo/uclibc/";
	char link[256];
	ssize_t n;
	FILE *fp;
	char *p;

	fp = fopen("/etc/timezone", "r");
	if (fp) {
		p = fgets(buf, len, fp);
		fclose(fp);
		if (p) {
			buf[strcspn(buf, "\r\n")] = 0;
			if (buf[0])
				return 0;
		}
	}

	n = readlink("/etc/TZ", link, sizeof(link) - 1);
	if (n > 0) {
		link[n] = 0;
		p = strstr(link, mark);
		if (p) {
			snprintf(buf, len, "%s", p + strlen(mark));
			return 0;
		}
	}

	return -1;
}

static void cprint_timezone(cbuf *cb)
{
	char tz[128];

	if (get_timezone(tz, sizeof(tz)) == 0)
		cprintf(cb, "  <clock><timezone-name>%s</timezone-name></clock>\n", tz);
}

/* Emit one server from an sntpd command-line token host[:port][,opts]. */
static void cprint_ntp_server(cbuf *cb, char *tok)
{
	char *host = tok, *port = NULL, *opts;

	opts = strchr(tok, ',');
	if (opts)
		*opts++ = 0;

	if (*host == '[') {			/* [v6]:port */
		char *end = strchr(host, ']');

		if (!end)
			return;
		host++;
		*end++ = 0;
		if (*end == ':')
			port = end + 1;
	} else {
		char *colon = strchr(host, ':');

		/* a lone colon is host:port; several means a bare v6 */
		if (colon && !strchr(colon + 1, ':')) {
			*colon = 0;
			port = colon + 1;
		}
	}

	cprintf(cb, "    <server><name>%s</name><udp><address>%s</address>",
		host, host);
	if (port)
		cprintf(cb, "<port>%s</port>", port);
	cprintf(cb, "</udp>");
	if (opts && strstr(opts, "iburst"))
		cprintf(cb, "<iburst>true</iburst>");
	if (opts && strstr(opts, "prefer"))
		cprintf(cb, "<prefer>true</prefer>");
	cprintf(cb, "</server>\n");
}

/*
 * NTP client state, read from the live sntpd process (its /proc entry)
 * rather than the env file, so operational shows what sntpd is really
 * running with.  Its command line carries no server names, so each server
 * is keyed by its address; association-type is not tracked (see
 * tr_commit_ntp).
 */
static void cprint_ntp(cbuf *cb)
{
	char cmdline[1024], path[280];
	struct dirent *de;
	int open = 0;
	DIR *proc;

	proc = opendir("/proc");
	if (!proc)
		return;

	while ((de = readdir(proc))) {
		char comm[32] = "";
		char *p;
		size_t n;
		FILE *fp;

		if (de->d_name[0] < '1' || de->d_name[0] > '9')
			continue;

		snprintf(path, sizeof(path), "/proc/%s/comm", de->d_name);
		fp = fopen(path, "r");
		if (!fp)
			continue;
		p = fgets(comm, sizeof(comm), fp);
		fclose(fp);
		comm[strcspn(comm, "\r\n")] = 0;
		if (!p || strcmp(comm, "sntpd"))
			continue;

		snprintf(path, sizeof(path), "/proc/%s/cmdline", de->d_name);
		fp = fopen(path, "r");
		if (!fp)
			continue;
		n = fread(cmdline, 1, sizeof(cmdline) - 1, fp);
		fclose(fp);
		cmdline[n] = 0;

		/* Walk the nul-separated argv; servers are the non-option args.
		 * cprint_ntp_server() splits the token in place, so advance by
		 * its original length, captured before the call. */
		for (p = cmdline; p < cmdline + n; ) {
			size_t len = strlen(p);

			if (*p && *p != '/' && *p != '-') {
				if (!open) {
					cprintf(cb, "  <ntp><enabled>true</enabled>\n");
					open = 1;
				}
				cprint_ntp_server(cb, p);
			}
			p += len + 1;
		}
		break;			/* one supervised sntpd */
	}
	closedir(proc);

	if (open)
		cprintf(cb, "  </ntp>\n");
}

/*
 * The applied DNS config, read back from the resolv.conf head mech wrote.
 * Each server carries its YANG key in a "# id:" comment so the name-keyed
 * list round-trips; a server without one (should not happen) falls back to
 * its address as the name.
 */
static void cprint_dns(cbuf *cb)
{
	char line[512], name[128] = "";
	int open = 0;
	FILE *fp;

	fp = fopen("/etc/resolv.conf.head", "r");
	if (!fp)
		return;

	while (fgets(line, sizeof(line), fp)) {
		char *tok;

		line[strcspn(line, "\r\n")] = 0;

		if (!strncmp(line, "# id:", 5)) {
			snprintf(name, sizeof(name), "%s", line + 5);
			continue;
		}
		if (!strncmp(line, "search ", 7)) {
			for (tok = strtok(line + 7, " \t"); tok; tok = strtok(NULL, " \t")) {
				if (!open) {
					cprintf(cb, "  <dns-resolver>\n");
					open = 1;
				}
				cprintf(cb, "    <search>%s</search>\n", tok);
			}
			continue;
		}
		if (!strncmp(line, "nameserver ", 11)) {
			tok = line + 11;
			while (*tok == ' ' || *tok == '\t')
				tok++;
			if (!open) {
				cprintf(cb, "  <dns-resolver>\n");
				open = 1;
			}
			cprintf(cb, "    <server><name>%s</name>"
				"<udp-and-tcp><address>%s</address></udp-and-tcp>"
				"</server>\n", name[0] ? name : tok, tok);
			name[0] = 0;
		}
	}
	fclose(fp);

	if (open)
		cprintf(cb, "  </dns-resolver>\n");
}

/*
 * Each managed user's applied login shell and password hash, read back
 * from /etc/passwd and /etc/shadow, so operational reflects what the
 * account actually got -- not merely what sits in running.  Confined to
 * real login users (uid >= 1000), leaving root and system accounts out.
 * The password is a hash; RFC 7317 leaves it readable (deny-write only),
 * and NACM is what restricts who may see it.
 */
static void cprint_users(cbuf *cb)
{
	struct passwd *pw;

	cprintf(cb, "  <authentication>\n");
	setpwent();
	while ((pw = getpwent())) {
		struct spwd *sp;

		if (pw->pw_uid < 1000 || pw->pw_uid >= 65534)
			continue;
		cprintf(cb, "    <user><name>%s</name>", pw->pw_name);
		sp = getspnam(pw->pw_name);
		if (sp && sp->sp_pwdp && sp->sp_pwdp[0] == '$')
			cprintf(cb, "<password>%s</password>", sp->sp_pwdp);
		cprintf(cb, "<shell xmlns=\"%s\">%s</shell></user>\n",
			MYSYS_NS, shell_ident(pw->pw_shell));
	}
	endpwent();
	cprintf(cb, "  </authentication>\n");
}

int ietf_sys_statedata(clixon_handle h, cvec *nsc, char *xpath, cxobj *xstate)
{
        struct sysinfo si;
        time_t now, boot;
        char buf[42];
	char host[256];
	cbuf *cb;
	int rc;

	cb = cbuf_new();
	if (!cb) {
		clixon_err(OE_UNIX, errno, "ietf-system:cbuf_new");
		return -1;
	}

        tzset();
        sysinfo(&si);
        now = time(NULL);
        boot = now - si.uptime;

	/* The applied hostname, so operational reflects what is in effect
	 * -- not just the config-false leaves under system-state. */
	if (gethostname(host, sizeof(host)))
		host[0] = 0;
	host[sizeof(host) - 1] = 0;

	cprintf(cb, "<system xmlns=\"%s\">", XMLNS);
	cprintf(cb, "  <hostname>%s</hostname>\n", host);
	cprint_timezone(cb);
	cprint_ntp(cb);
	cprint_dns(cb);
	cprint_users(cb);
	cprintf(cb, "</system>");

	cprintf(cb, "<system-state xmlns=\"%s\">", XMLNS);
	cprintf(cb, "  <platform>\n");
	cprintf(cb, "    <os-name>%s</os-name>\n", os);
	cprintf(cb, "    <os-version>%s</os-version>\n", ver);
	if (rel)
		cprintf(cb, "    <os-release>%s</os-release>\n", rel);
	cprintf(cb, "    <machine>%s</machine>\n", sys);
	cprintf(cb, "  </platform>\n");
	cprintf(cb, "  <clock>\n");
	cprintf(cb, "    <current-datetime>%s</current-datetime>\n", fmtime(now, buf, sizeof(buf)));
	cprintf(cb, "    <boot-datetime>%s</boot-datetime>\n", fmtime(boot, buf, sizeof(buf)));
	cprintf(cb, "  </clock>\n");
	cprintf(cb, "</system-state>");
	rc = clixon_xml_parse_string(cbuf_get(cb), YB_NONE, NULL, &xstate, NULL) < 0 ? -1 : 0;
	cbuf_free(cb);

	return rc;
}

static int set_datetime(cxobj *xe)
{
	const char *isofmt = "%FT%T%z";
        struct timeval tv;
        struct tm tm;
	char tz[24];
	int rc = -1;
	cxobj *obj;
	char *buf;
	size_t n;

	obj = xml_find(xe, "current-datetime");
	if (!obj)
		return 0;

	buf = strdup(xml_body(obj));
	if (!buf)
		return -1;

	n = strlen(buf);
	if (buf[n - 3] == ':' && (buf[n - 6] == '+' || buf[n - 6] == '-')) {
                buf[n - 3] = buf[n - 2];
                buf[n - 2] = buf[n - 1];
                buf[n - 1] = 0;
        } else
                isofmt = "%FT%TZ";

        memset(&tm, 0, sizeof(tm));
        if (!strptime(buf, isofmt, &tm)) {
                WARN("ietf-system:failed strptime: %s", strerror(errno));
		goto done;
	}

	snprintf(tz, sizeof(tz), "UTC%s%ld", tm.tm_gmtoff > 0 ? "+" : "", tm.tm_gmtoff / 3600);
	setenv("TZ", tz, 1);
//	ERR("time %s gmtoff %ld TZ=%s", buf, tm.tm_gmtoff, tz);

        tv.tv_sec = mktime(&tm);
        tv.tv_usec = 0;
        if (settimeofday(&tv, NULL)) {
                WARN("ietf-system:failed settimeofday: %s", strerror(errno));
		goto done;
	}

	rc = 0;
done:
	unsetenv("TZ");
        free(buf);
	return rc;
}

static int do_rpc(clixon_handle h,            /* Clicon handle */
		  cxobj        *xe,           /* Request: <rpc><xn></rpc> */
		  cbuf         *cbret,        /* Reply eg <rpc-reply>... */
		  void         *arg,          /* client_entry */
		  void         *regarg)       /* Argument given at register */
{
	char *namespace;
	char *op;

	if ((namespace = xml_find_type_value(xe, NULL, "xmlns", CX_ATTR)) == NULL) {
		clixon_err(OE_XML, ENOENT, "No namespace given in RPC %s", xml_name(xe));
		return -1;
	}
	if (strcmp(namespace, XMLNS))
		return 0;	/* not for us */

	cprintf(cbret, "<rpc-reply xmlns=\"%s\"><ok/></rpc-reply>", NETCONF_BASE_NAMESPACE);

	op = xml_name(xe);
	if (!strcmp(op, "set-current-datetime"))
		return set_datetime(xe);
	if (!strcmp(op, "system-shutdown"))
		return run("poweroff");
	if (!strcmp(op, "system-restart"))
		return run("reboot");
	return 0;
}

static clixon_plugin_api ietf_system_api = {
	.ca_name = "ietf-system",
	.ca_init = clixon_plugin_init,

	.ca_trans_begin = ietf_sys_tr_begin,
	.ca_trans_commit = ietf_sys_tr_commit,

	.ca_statedata = ietf_sys_statedata,
};

static void register_rpc(clixon_handle h, const char *rpc)
{
	int rc;

	rc = rpc_callback_register(h, do_rpc, NULL, XMLNS, rpc);
	if (rc < 0)
		clixon_err(OE_PLUGIN, EINVAL, "ietf-system: failed registering RPC %s", rpc);
}

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	os_init();

	aug = aug_init(NULL, "", 0);
	if (!aug ||
	    aug_load_file(aug, "/etc/hostname") ||
	    aug_load_file(aug, "/etc/hosts")) {
		clixon_err(OE_UNIX, EINVAL,
			   "ietf-system: Augeas initialization failed");
		return NULL;
	}

	register_rpc(h, "set-current-datetime");
	register_rpc(h, "system-restart");
	register_rpc(h, "system-shutdown");

	return &ietf_system_api;
}
