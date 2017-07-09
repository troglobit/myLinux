/* Silly boot logo animation
 *
 * Copyright (c) 2017  Joachim Nilsson <troglobit@gmail.com>
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

#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <finit/finit.h>
#include <finit/plugin.h>

static pid_t pid = 0;
static const char *logo[] = {
	"                                                               ______\n",
	"                          __                       .-------.  /\\     \\\n",
	"                         |  |                     /   o   /| /o \\  o  \\\n",
	" _______                 |  |_______ _______     /_______/o|/   o\\_____\\\n",
	"|       .----.-----.-----|  |   _   |   _   |    | o     | |\\o   /o    /\n",
	"|\e[1m.\e[0m|   | |   _|  _  |  _  |  |\e[1m.\e[0m  |   |   1___|    |   o   |o/ \\ o/  o  /\n",
	"'-|\e[1m.\e[0m  |-|__| |_____|___  |__|\e[1m.\e[0m  |   |____   |    |     o |/   \\/____o/\n",
	"  |\e[1m:\e[0m  |            |_____|  |\e[1m:\e[0m  1   |\e[1m:\e[0m  1   |    '-------'\n",
	"  |\e[1m::.\e[0m| Chaos Rel.          |\e[1m::.. .\e[0m |\e[1m::.. .\e[0m |    Troglobit Software Inc.\n",
	"  '---'                     '-------'-------'    \e[4mhttp://troglobit.com\e[0m\n",
	NULL
};

extern int screen_rows;
extern int screen_cols;

#define DRAW_DELAY   2000
#define SCREEN_WIDTH screen_cols
#include <lite/conio.h>

static int screen_colo  = 1;
static int screen_rowo  = 1;
static int logo_cols    = 0;
static int logo_rows    = 0;

static int lenparse(const char *line)
{
	int len = 0, esc = 0;

	for (int i = 0; line[i]; i++) {
		if (line[i] == '\n')
			break;

		if (esc || line[i] == '\e') {
			esc = 1;

			if (isalpha(line[i]))
				esc = 0;
		} else
			len++;
	}

	return len;
}

static int logo_init(int center)
{
	int i, len;

	for (i = 0; logo[i]; i++) {
		len = lenparse(logo[i]);

		if (len > logo_cols)
			logo_cols = len;
	}

	if (i > logo_rows)
		logo_rows = i;

	if (screen_cols < logo_cols)
		return 1;

	if (center) {
		screen_rowo = (screen_rows - logo_rows) / 2;
		screen_colo = (screen_cols - logo_cols) / 2;
	}

	return 0;
}

static void cursor(int x, int y)
{
	gotoxy(x, y);
	fputs("   \e[7m   \e[0m   ", stdout);
}

static int logo_show(int center)
{
	int l = 0;
	int x, y, i;

	if (logo_init(center))
		return 1;

	hidecursor();
	clrscr();
	for (y = screen_rowo; y < screen_rows && logo[l]; y++) {
	     for (i = 0; i < DRAW_DELAY; i++) {
		     x = (rand() % logo_cols) + screen_colo;
		     if (x > 5)
			     x -= 5;

		     cursor(x, y);
	     }

	     gotoxy(screen_colo, y);
	     delline();
	     if (logo[l])
		     puts(logo[l++]);
	}
	showcursor();

	return 0;
}

static void logo_start(void *arg)
{
	pid = fork();
	if (pid)
		return;

	logo_show(1);

	exit(0);
}

static void logo_stop(void *arg)
{
	kill(pid, 9);
	waitpid(pid, NULL, WNOHANG);
}

static plugin_t plugin = {
	.hook[HOOK_BANNER] = { .cb = logo_start },
	.hook[HOOK_SVC_UP] = { .cb = logo_stop  },
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
