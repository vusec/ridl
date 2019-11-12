BIN ?= bin
OBJ ?= obj

CFLAGS += -DSTRIDE=1024 -DSTRIDE_SHIFT=10
CFLAGS += -Iinclude -O2
LDFLAGS += -O2
LIBS += -lmemkit

obj-y += \
	source/args.c.o \
	source/utils.c.o \
	source/utils.S.o
objs = $(addprefix $(OBJ)/, $(obj-y))

.PHONY: all clean

all: $(BIN)/rdcl $(BIN)/rdcl-tsx
all: $(BIN)/l1tf $(BIN)/l1tf-tsx
all: $(BIN)/mfbds $(BIN)/mfbds-ret $(BIN)/mfbds-tsx $(BIN)/mfbds-sse $(BIN)/mfbds-avx
all: $(BIN)/mlpds $(BIN)/mlpds-sse $(BIN)/mlpds-avx
all: $(BIN)/msbds $(BIN)/msbds-tsx
all: $(BIN)/mdsum-victim
all: $(BIN)/mdsum $(BIN)/mdsum-tsx
all: $(BIN)/lfbtf $(BIN)/lfbtf-tsx
all: $(BIN)/taa $(BIN)/taa-ac $(BIN)/taa-tsx
all: $(BIN)/hello $(BIN)/movdqu

$(OBJ)/%.c.o: %.c
	@echo "CC $<"
	@mkdir -p "$(dir $@)"
	@$(CC) -c $< -o $@ $(CFLAGS) -MT $@ -MMD -MP -MF $(@:.o=.d)

$(OBJ)/%.S.o: %.S
	@echo "AS $<"
	@mkdir -p "$(dir $@)"
	@$(CC) -c $< -o $@ $(CFLAGS) -MT $@ -MMD -MP -MF $(@:.o=.d)

$(BIN)/rdcl: source/rdcl.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/rdcl-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/rdcl-tsx: source/rdcl.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/l1tf: source/l1tf.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/l1tf-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/l1tf-tsx: source/l1tf.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mfbds: source/mfbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mfbds-ret: CFLAGS += -DRETSPEC
$(BIN)/mfbds-ret: source/mfbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mfbds-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/mfbds-tsx: source/mfbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mfbds-sse: CFLAGS += -DUSE_SSE
$(BIN)/mfbds-sse: source/mfbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mfbds-avx: CFLAGS += -DUSE_AVX
$(BIN)/mfbds-avx: source/mfbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mlpds: source/mlpds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mlpds-sse: CFLAGS += -DUSE_SSE
$(BIN)/mlpds-sse: source/mlpds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mlpds-avx: CFLAGS += -DUSE_AVX
$(BIN)/mlpds-avx: source/mlpds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/msbds: source/msbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/msbds-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/msbds-tsx: source/msbds.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mdsum-victim: source/mdsum-victim.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mdsum: source/mdsum.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/mdsum-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/mdsum-tsx: source/mdsum.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/lfbtf: source/lfbtf.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/lfbtf-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/lfbtf-tsx: source/lfbtf.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/taa: source/taa.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/taa-ac: CFLAGS += -DSTRICT_AC
$(BIN)/taa-ac: source/taa.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/taa-tsx: CFLAGS += -DTSX -mrtm
$(BIN)/taa-tsx: source/taa.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/hello: source/hello.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

$(BIN)/movdqu: source/movdqu.c $(objs)
	@echo "LD $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	@rm -rf $(OBJ) $(BIN)

