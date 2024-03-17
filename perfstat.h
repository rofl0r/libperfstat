#ifndef PERFSTAT_H
#define PERFSTAT_H

#include <unistd.h>

struct perfstat {
	pid_t pproc;
	int fifo[2];
};

int perfstat_init(struct perfstat *c, char* const args[]);
int perfstat_enable(struct perfstat *c);
int perfstat_disable(struct perfstat *c);
int perfstat_fini(struct perfstat *c);

#pragma RcB2 DEP "perfstat.c"

#endif
