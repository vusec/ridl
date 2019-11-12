BUILD ?= obj
PREFIX ?= /usr
LIBDIR ?= lib

includedir = $(PREFIX)/include
libdir = $(PREFIX)/$(LIBDIR)

#KERNEL_SRC_DIR = /usr/src/linux
KERNEL_SRC_DIR = /lib/modules/`uname -r`/build
#EXTRA_CFLAGS = --verbose

ccflags-y += -I$(PWD)/include

obj-m += memkit.o

memkit-objs += \
	source/kmem.o \
	source/main.o \
	source/mem.o \
	source/pat.o \
	source/tlb.o \
	source/umem.o

lib-obj-y += \
	lib/kmem.o \
	lib/memkit.o \
	lib/pat.o \
	lib/pte.o \
	lib/tlb.o

lib-objs = $(addprefix $(BUILD)/, $(lib-obj-y))

deps = $(libs-objs:.o=.d)

.PHONY: all install uninstall clean

all: clean memkit.ko libmemkit.so

-include $(deps)

memkit.ko: clean
	@make -C $(KERNEL_SRC_DIR) M=`pwd` modules

libmemkit.so: ccflags-y += -fPIC -Iinclude
libmemkit.so: LDFLAGS += -shared
libmemkit.so: $(lib-objs)
	@echo "LD $(notdir $@)"
	@mkdir -p $(dir $@)
	@$(CC) $(lib-objs) -o $@ $(LDFLAGS) $(ccflags-y) $(LIBS)

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@mkdir -p "$(dir $@)"
	@$(CC) -c $< -o $@ $(ccflags-y) -MT $@ -MMD -MP -MF $(@:.o=.d)

install:
	@install -m 755 -d "$(DESTDIR)$(libdir)"
	@install -m 644 include/memkit.h "$(DESTDIR)$(includedir)/"
	@install -m 644 libmemkit.so "$(DESTDIR)$(libdir)/libmemkit.so"

uninstall:
	@rm "$(DESTDIR)$(libdir)/libmemkit.so"
	@rm "$(DESTDIR)$(includedir)/memkit.h"

clean: 
	@make -C /lib/modules/`uname -r`/build M=$(PWD) clean
	@rm -rf $(BUILD)
	@rm -rf libmemkit.so

