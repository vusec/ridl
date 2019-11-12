#include "boilerplate.h"

int main() {
	BOILERPLATE_INIT();

	char buf[8];
	int fd = open("/proc/version", 0);
	while (1) {
		read(fd, buf, 0);
	}
}
