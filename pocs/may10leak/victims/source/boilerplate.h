#define _GNU_SOURCE
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/prctl.h>
#define PR_GET_SPECULATION_CTRL 52
#define PR_SET_SPECULATION_CTRL 53
#define PR_SPEC_DISABLE_FOREVER 8
	
#define ROUNDUP(x,y)   ((((x)+(y)-1)/(y))*(y))

#ifdef SPEC_BYPASS
#define BOILERPLATE_INIT() do { \
	prctl(PR_SET_SPECULATION_CTRL, 0, PR_SPEC_DISABLE_FOREVER, 0, 0);\
	} while (0);
#else
#define BOILERPLATE_INIT() { }
#endif

#define STRINGIZE(s) #s
#define TOSTRING(s) STRINGIZE(s)

