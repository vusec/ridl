all: binaries/may10leak victims

clean:
	@rm -f binaries/*
	@cd victims && make clean

clean-attacker:
	@rm -f binaries/may10leak

.PHONY: victims
victims:
	@mkdir -p victims/binaries
	@cd victims && make

binaries/may10leak: may10leak.c
	@mkdir -p binaries
	gcc -g -O3 $(ATTACKERFLAGS) may10leak.c -o $@ # ~/.local/lib/x86_64-linux-gnu/libpfc.so

