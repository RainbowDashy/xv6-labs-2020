#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void fork1() {
	int forked = 0;
	int p[2];
	int prime;
	read(0, &prime, 4);
	printf("prime %d\n", prime);
	int n;
	while (read(0, &n, 4)) {
		if (n % prime) {
			if (!forked) {
				pipe(p);
				if (fork() == 0) {
					close(0);
					dup(p[0]);
					close(p[0]);
					close(p[1]);
					prime = n;
					printf("prime %d\n", prime);
				} else {
					close(p[0]);
					forked = 1;
					write(p[1], &n, 4);
				}
			} else {
				write(p[1], &n, 4);
			}
			
		}		
	}
	if (forked) {
		close(p[1]);
	}
}

void fork2() {
	int forked = 0, first = 1;
	int p[2];
	int prime, n;
	while (read(0, &n, 4)) {
		if (first) {
			first = 0;
			prime = n;
			printf("prime %d\n", prime);
			continue;
		}
		if (n % prime) {
			if (!forked) {
				pipe(p);
				if (fork() == 0) {
					close(0);
					dup(p[0]);
					close(p[0]);
					close(p[1]);
					first = 1;
				} else {
					close(p[0]);
					forked = 1;
				}
			}
			write(p[1], &n, 4);
		}
	}
	if (forked) {
		close(p[1]);
	}
}

int main(int argc, char* argv[]) {
	int p[2];
	pipe(p);
	
	if (fork() == 0) {
		close(0);
		dup(p[0]);
		close(p[0]);
		close(p[1]);
		fork2();
	} else {
		close(p[0]);
		for (int i = 2; i <= 35; ++i)
			write(p[1], &i, 4);
		close(p[1]);

	}
	wait(0);
	exit(0);
}
