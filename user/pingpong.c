#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
	int p1[2], p2[2]; // p1: parent->child  p2: child->parent
	pipe(p1);
	pipe(p2);
	int pid = fork();
	char buf[100];
	if (pid == 0) {
		write(p2[1], "c", 1);
		if (read(p1[0], buf, 1)) {
			printf("%d: received ping\n", getpid());
		}
		close(p2[1]);
		close(p1[0]);
	} else {
		write(p1[1], "p", 1);
		wait(0);
		if (read(p2[0], buf, 1)) {
			printf("%d: received pong\n", getpid());
		}
		close(p1[1]);
		close(p2[0]);
	}
	exit(0);
}
