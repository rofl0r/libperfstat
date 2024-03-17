#include "perfstat.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

int perfstat_init(struct perfstat *c, char* const args[]) {
	int expipe[2];
	int cmdpipe[2];
	int ackpipe[2];
	if(pipe(cmdpipe) < 0) return -1;
	if(pipe(ackpipe) < 0) return -1;
	if(pipe(expipe) < 0) return -1;
	pid_t mypid = getpid();
	pid_t pid = fork();
	if(pid) {
		char buf;
		c->pproc = pid;
		close(cmdpipe[0]);
		close(ackpipe[1]);
		close(expipe[1]);
		/* wait for exec to close the CLOEXEC'd pipe */
		read(expipe[0], &buf, 1);
		close(expipe[0]);
		c->fifo[0] = cmdpipe[1];
		c->fifo[1] = ackpipe[0];
		/* check that process didn't end prematurely - for example
		   any perf version before kernel 5.10 don't recognize the
		   IPC options we need */
		int st;
		pid_t ret = waitpid(pid, &st, WNOHANG);
		if(ret == 0) return 0;
		return -1;
	} else {
		char buf[64];
		char pbuf[16];
		unsigned ucnt = 0, acnt, i;
		c->fifo[0] = cmdpipe[0];
		close(cmdpipe[1]);
		c->fifo[1] = ackpipe[1];
		close(ackpipe[0]);
		sprintf(pbuf, "%d", mypid);
		sprintf(buf, "fd:%d,%d", c->fifo[0], c->fifo[1]);
		close(expipe[0]);
		fcntl(expipe[1], F_SETFD, FD_CLOEXEC);
		close(1); // prevent potential stdout spam
		close(2); // prevent definite  stderr spam
		if(args) for(;args[ucnt]; ++ucnt);
		char *argv_f[] = {
			"perf", "record", "--delay=-1",
			"-p", pbuf, "--control", buf,
		};
		acnt = sizeof(argv_f)/sizeof(argv_f[0]);
		char **argv = calloc(sizeof(char*), (ucnt+acnt+1));
		for(i = 0; i < acnt; ++i) argv[i] = argv_f[i];
		for(i = 0; i < ucnt; ++i) argv[acnt+i] = args[i];
		argv[acnt+i] = 0;
		execvp("perf", argv);
		return -1;
	}
	return -1;
}

int perfstat_enable(struct perfstat *c) {
	char buf[64];
	write(c->fifo[0], "enable\n", 8);
	read(c->fifo[1], buf, sizeof buf);
	return 0;
}

int perfstat_disable(struct perfstat *c) {
	char buf[64];
	write(c->fifo[0], "disable\n", 9);
	read(c->fifo[1], buf, sizeof buf);
	return 0;
}

int perfstat_fini(struct perfstat *c) {
	char buf[64];
	close(c->fifo[0]);
	close(c->fifo[1]);
	kill(c->pproc, SIGINT);
	int r, ret = waitpid(c->pproc, &r, 0);
	if(ret == -1) return -1;
	if(ret != c->pproc) return -2;
	if(WIFSIGNALED(r) && WTERMSIG(r) == SIGINT)
		return 0;
	if(WIFEXITED(r) == 0) return -3;
	return WEXITSTATUS(r);
}
