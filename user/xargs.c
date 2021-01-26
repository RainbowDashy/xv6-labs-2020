#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char* argv[]) {
	if (argc > MAXARG) {
		fprintf(2, "argument number exceeds\n");
		exit(1);
	}
	char buf[512], *arg[MAXARG], *p = buf;
	int n = argc - 1;
	for (int i = 1; i < argc; ++i)
		arg[i-1] = argv[i];
	while (read(0, p++, sizeof(*p))) {
		if (*(p-1) == '\n') {
		
			*(p-1) = ' ';
			for (char *c = buf, *pre = c; c != p; ++c) {
				if (*c == ' ') {
					if (n+1 > MAXARG) {
						fprintf(2, "argument number exceeds\n");
						exit(1);	
					}
					*c = 0;
					arg[n++] = pre;
					pre = c + 1;
				}
			}

			if (fork() == 0) {
				arg[n] = 0;
				exec(arg[0], arg);
				exit(0);
			} else {
				wait(0);
				n = argc - 1;
				p = buf;	
			}
		}
	}
	exit(0);
}
