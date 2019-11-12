SOURCES := $(wildcard source/*.c)
BINARIES := $(patsubst source/%.c, binaries/%.bin, $(SOURCES))

all: $(BINARIES)

clean:
	@rm -f binaries/*.bin

binaries/%.bin: source/%.c $(DEPS)
	@mkdir -p binaries
	$(CC) -O2 -o $@ $< $(VICTIMFLAGS)

