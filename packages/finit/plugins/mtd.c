/* Simple MTD erase plugin that runs if mtd:Config is not mounted properly
 *
 * Copyright (c) 2015-2020  Joachim Nilsson <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sched.h>		/* sched_yield() */
#include <unistd.h>             /* SEEK_SET */
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <mtd/mtd-user.h>

#include <finit/finit.h>
#include <finit/helpers.h>
#include <finit/plugin.h>

#define MTD_CONFIG_LABEL  "Config"
#define MTD_OVERLAY_DIRS  {			\
		"/mnt/etc",  "/mnt/.etc",	\
		"/mnt/var",  "/mnt/.var",	\
		"/mnt/home", "/mnt/.home",	\
		NULL }

static int match_line(char *needle, char *line, size_t len, FILE *fp)
{
	while (fgets(line, len, fp)) {
		if (strstr(line, needle))
			return 1;
	}

	return 0;
}

static int is_mounted(char *label)
{
	FILE *fp;
	int match;
	char line[64];

	fp = fopen("/proc/mounts", "r");
	if (!fp)
		return 1;

	match = match_line(label, line, sizeof(line), fp);
	fclose(fp);

	return match;
}

static int find_mtd(char *label, char *dev, size_t len)
{
	int found;
	FILE *fp;
	char line[64];

	fp = fopen("/proc/mtd", "r");
	if (!fp)
		return 0;

	found = match_line(label, line, sizeof(line), fp);
	fclose(fp);
	
	if (found) {
		char *ptr;

		ptr = strchr(line, ':');
		if (!ptr) {
			found = 0;
		} else {
			*ptr = 0;
			snprintf(dev, len, "/dev/%s", line);
		}
	}

	return found;
}

static int do_erase(char *dev)
{
	int fd;
	size_t i;
	mtd_info_t mtd;

	fd = open(dev, O_RDWR | O_SYNC);
	if (-1 == fd)
		return 1;

	memset(&mtd, 0, sizeof(mtd));
	if (ioctl(fd, MEMGETINFO, &mtd) < 0 || mtd.erasesize <= 0) {
		close(fd);
		return 1;
	}
		
	for (i = 0; i < mtd.size / mtd.erasesize; i++) {
		erase_info_t this = {
			.start  = i * mtd.erasesize,
			.length = mtd.erasesize
		};

		/* Yield CPU once in a while, to not starve watchdogd */
		sched_yield();

		if (ioctl(fd, MEMERASE, &this) < 0) {
			close(fd);
			return 1;
		}
	}
	
	return close(fd);
}

static int check_overlayfs(void)
{
	int exists = 1, created = 0;
	char *dirs[] = MTD_OVERLAY_DIRS;

	for (int i = 0; dirs[i]; i++) {
		if (!fexist(dirs[i]))
			exists = 0;
	}

	if (exists)
		return 0;

	print_desc("Creating OverlayFS directories in mtd:" MTD_CONFIG_LABEL, NULL);

	for (int i = 0; dirs[i]; i++) {
		if (!fexist(dirs[i])) {
			if (mkdir(dirs[i], 0755) || !fexist(dirs[i])) {
				_pe("Failed creating %s", dirs[i]);
				continue;
			}
			sync();
			created++;
		}
	}

	print_result(created == 0);

	return created;
}

static void mount_error(void *arg)
{
	char dev[10];

	if (is_mounted("mtd:" MTD_CONFIG_LABEL)) {
		if (check_overlayfs())
			reboot(RB_AUTOBOOT);

		return;
	}

	if (!find_mtd(MTD_CONFIG_LABEL, dev, sizeof(dev)))
		return;

	print_desc("Erasing corrupt configuration partition", NULL);
	print_result(do_erase(dev));

	/* Finit has not setup any signals yet, sys_reboot() won't work */
	reboot(RB_AUTOBOOT);
}

static plugin_t plugin = {
	.hook[HOOK_MOUNT_ERROR] = { .cb = mount_error },
};

PLUGIN_INIT(plugin_init)
{
	plugin_register(&plugin);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&plugin);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
