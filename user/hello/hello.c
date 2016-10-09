/* Simple Hello World example */
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char *argv[])
{
	puts("Hello world!");

	errno = 0;
	if (argc > 1) {
		char buf[256];
		struct passwd pwd;
		struct passwd *pw = NULL;

		memset(&pwd, 0, sizeof(pwd));
		getpwnam_r(argv[1], &pwd, buf, sizeof(buf), &pw);
		if (!pw)
			errx(1, "Failed getting user %s", argv[1]);

		printf("Found user %s with UID %d\n", pw->pw_name, pw->pw_uid);
	}

	return 0;
}
