#include "perfstat.h"
#include <stdio.h>

int main() {
	int i;
	struct perfstat ps;
	if(perfstat_init(&ps, (char*const[]){"--freq=max", "-g", 0}) != 0)
		return -1;
	if(perfstat_enable(&ps) != 0) return -2;
	for(i = 0; i < 0x1ffff; ++i);
	if(perfstat_disable(&ps) != 0) return -3;
	if(perfstat_fini(&ps) == 0) {
		puts("success - now run perf report to watch the report");
		return 0;
	}
	return -4;
}
