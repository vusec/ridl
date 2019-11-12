Prerequisites
=============

See `./memkit` for instructions on how to build and load the kernel module.
Many of the toy programs will actually depend on the memkit kernel module to allocate kernel memory or to make modifications to page tables.

Building
========

Run the following command to build the toy programs:

```
make
```

Cross-thread Attacks
====================

To run the cross-thread attacks, you have to figure out which logical CPU cores are part of the same physical CPU core
This can be done by reading `/proc/cpuinfo` using `cat /proc/cpuinfo`.
To find two logical CPU cores that are part of the same physical CPU core, you can look at the `core id`.
If the `core id` is the same, then the logical CPU cores are part of the same physical CPU core.
The `processor` number can then be used as an argument for `taskset` to run the victim program on one thread and the attacker on another thread.
For instance, on an Intel Core i7-6700K (Skylake) processor, the threads 0 and 4 are part of the same physical CPU core.
Hence you can run the victim and attacker as follows:

```
taskset -c 0 ./victim
taskset -c 4 ./attack
```

Rogue Data Cache Load
=====================

Rogue Data Cache Load (RDCL) or Meltdown shows that it is possible to use out-of-order execution, speculative execution or transactions to leak data from kernel virtual addresses, as long as the cache lines are resident in the L1d cache.

The RDCL toy program works by allocating some kernel memory and writing a secret to it.
Normally, the idea is that there is some data in the kernel that gets touched by the kernel, such that is in the L1d cache.
The attacker tries to call some system call that will make sure that the data gets cached in the L1d cache, and then tries to leak the data by speculatively reading from the kernel virtual address.
In regular execution, this would normally trigger a segmentation fault, but in the case of speculative execution, this would leak the data.

You can run `./bin/rdcl` or `./bin/rdcl-tsx` (if you have TSX).

L1 Terminal Fault
=================

L1 Terminal Fault (L1TF) or Foreshadow shows that it is possible to use speculative execution or transactions to leak cache lines from any virtual address of which the physical address aliases with any cache line resident in the L1d cache, even if the page is not present.
That is, even when just the physical address is stored in the Page Table Entry (PTE), it is possible to leak the corresponding data, as long as the data that belongs to that physical address is resident in the L1d cache.

The L1TF toy program is pretty much an extension of the Meltdown one, except it sets up a page that aliases with the kernel memory.
The attacker program still tries to get the kernel memory cached in the L1d cache, but instead uses the aliasing page to leak the data.

You can run `./bin/l1tf` or `./bin/l1tf-tsx` (if you have TSX).

In addition, there is a cross-thread implementation called LFBTF (Line Fill Buffer Terminal Fault).
The `./bin/mdsum-victim` can be used to allocate kernel memory, write a secret to the kernel memory and then keep the data loaded into the LFB in another thread.

How can we be sure that it is not in the L1d cache?
Well, you can run the following command: `taskset -c 0 ./bin/mdsum-victim --uncacheable`.

Now, to perform the attack, you have to copy over the kernel virtual address and run either `./bin/lfbtf` or `./bin/lfbtf-tsx` with the kernel address and the number of bytes to leak:

```
taskset -c 4 ./bin/lfbtf 0xffffb9a5019f7000 36
```

The attacker program will set up an aliasing page to the kernel memory that is marked as not present and try to leak from that.

Why does this program leak from the Line Fill Buffer (LFB)?
One of the responsibilities of the LFB is to perform load squashing and write combining, which in the case of Intel CPUs even happens across threads.
What this means is that the speculative load used by the attacker may alias with an entry in the LFB, which ultimately causes the leakage.

Micro-architectural Data Sampling Uncached Memory
=================================================

Micro-architectural Data Sampling Uncached Memory (MDSUM) is a cross-thread attack that again shows leakage from the LFB.
We again use the `./bin/mdsum-victim` as follows: `taskset -c 0 ./bin/mdsum-victim --uncacheable`.

Then we run the attacker `./bin/mdsum` as follows:

```
taskset -c 4 ./bin/mdsum 0xffffb9a5019f7000 36
```

This toy program just tries to speculatively leak from the kernel virtual address.
Again what happens is that the speculative load aliases with the LFB entry, which is the cause of leakage.

Micro-architectural Fill Buffer Data Sampling
=============================================

MDSUM shows one form of leaking from the LFBs, but there are other ways to leak from the LFB, such as Micro-architectural Fill Buffer Data Sampling (MFBDS).
This experiment again consists of a victim that runs on one thread: `taskset -c 0 ./bin/hello`.
Now we run the attacker `taskset -c 4 ./bin/mfbds-tsx 36` on another thread.
What the attacker does is it tries to speculatively leak from either an invalid pointer, such as a `NULL` pointer or a valid page that has not been backed by a physical page yet.
The last phenomenon is called demand paging, where the operating system only allocates a physical page to back the virtual address upon the first access to the virtual address.
In the end, it means that the PTE that corresponds to the virtual address equals 0, which is the same as using an invalid address.

Why does MFBDS leak from the LFBs?
When performing the speculative load, the CPU consults both the L1d cache and the LFB simultaneously.
If the cache line cannot be found in the L1d cache, it will allocate an entry in the LFB to keep track of the address.
This form of book keeping is normally done to send out the memory request asynchronously, such that the CPU pipeline can keep running other instructions until the memory request completes and the data comes back.
Unfortunately, when LFB entries are being released upon the completion of a memory requests and any dependent loads/stores, the data in the LFB entry is not zeroed out.
What this means is that when you re-allocate the entry, the data of a previous load/store may still be present and this is exactly what is being leaked using the speculative load.

There are other toy programs, such as `./bin/mfbds-ret`, which uses speculation instead of transactions.
In addition, to use demand paging, you can pass the `--demand` argument to any of the toy programs:

```
./bin/mfbds --demand 36
./bin/mfbds-sse --demand 36
./bin/mfbds-avx --demand 36
```

Micro-architectural Store Buffer Data Sampling
==============================================

Micro-architectural Store Buffer Data Sampling (MSBDS) shows that you can leak pending stores from the store buffer.
You can run the program as follows: `./bin/msbds` or `./bin/msbds-tsx` (if you have TSX).
The attacker will first do 56 stores to a page offset that does not match with the page offset that the attacker wants to leak.
This is done to fill the store buffer to increase the chance of capturing the stores performed by the victim, because if the store buffer is close to being depleted, there is a chance we won't capture anything.
Now we let the victim run, which will just do a store to offset 0x30a.
The victim could also be some kernel code executing stores that we want to capture, as long as KPTI is not enabled.
The reason why KPTI has to be disabled for this, is because changing the `cr3` register waits for the store buffer to be depleted.
This is because the store buffer keeps tracks of the virtual addresses, and changing the address space simply changes the semantics of the virtual addresses.
Now that the victim ran, the attacker tries to speculatively read from a non-canonical address or a page that has the access bit cleared.
Speculatively loading such addresses causes the page offset of the load to be aliased with the page offset of the store in the buffer.
Thus, we leak stores from the store buffer.

Micro-architectural Load Port Data Sampling
===========================================

Micro-architectural Load Port Data Sampling (MLPDS) shows that you can leak loads from the buffers used by load ports.
Because x86(-64) has to support unaligned loads and stores, it is possible that a load spans two cache lines or even two pages.
Intel CPUs accommodate for this by splitting loads and stores into two separate loads and stores that load the two separate cache lines.
However, while speculating it is possible that the second part returns stale data from the buffer in the load port.

The victim uses `movdqu` to load the value `'N'` into the load port.
Run the victim as follows: `taskset -c 0 ./bin/movdqu`.

The attacker now allocates two pages to leak from.
The reason why is because we are trying to have a load that crosses the page boundary, but doing this while crossing cache lines should also work.
Then we enable alignment checks for normal loads or use unaligned SSE/AVX loads to perform a load crossing a page boundary speculatively.
This way we leak data from the load port.

Run the attacker as follows: `taskset -c 4 ./bin/mlpds` (or `./bin/mlpds-sse` or `./bin/mlpds-avx`).

TSX Asynchronous Abort
======================

TSX Asynchronous Abort (TAA) shows that you can leak data by triggering an asynchronous abort during a TSX transaction.
The way the attack works is that before entering the transaction, the attacker issues pending cache flushes on cache lines that will be used by the transaction.
During the transaction the attacker will try to speculatively load from a cache line that is about to get flushed.
This results in an asynchronous abort of the TSX, but will still speculatively leak data.

To run this experiment, you can run `./bin/hello` as the victim, and then `./bin/taa 36`, `./bin/taa-ac 36` or `./bin/taa-tsx 36` (if you have TSX).

