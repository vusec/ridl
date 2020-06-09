CORES=3,7

all: cross_transaction_leak verw_bypass alignment_write ridl_basic ridl_invalidpage taa_basic taa_read pgtable_leak pgtable_leak_notsx loadport verw_bypass_l1des vrs cpuid_leak_taa cpuid_leak

clean:
	rm cross_transaction_leak verw_bypass alignment_write ridl_basic ridl_invalidpage taa_basic taa_read pgtable_leak pgtable_leak_notsx loadport verw_bypass_l1des vrs cpuid_leak_taa cpuid_leak

run: all
	# should leak cpuid buffer
	taskset -c $(CORES) ./cpuid_leak_taa
	# slower non-TAA version
	taskset -c $(CORES) ./cpuid_leak
	#
	# should leak 89 (from aborted TSX transaction)
	./cross_transaction_leak
	#
	# should leak 42 (across VERW)
	./verw_bypass
	#
	# should leak 89 (alignment trap)
	taskset -c $(CORES) ./alignment_write
	#
	# should leak 89 (TAA -> cross-thread write leak)
	taskset -c $(CORES) ./taa_basic
	#
	# should leak 89 (TAA -> cross-thread read leak)
	taskset -c $(CORES) ./taa_read
	#
	# should leak 81 (NULL read w/mfence -> cross-thread write leak)
	taskset -c $(CORES) ./ridl_basic
	#
	# should leak 81 (ridl_basic, but with non-NULL read)
	taskset -c $(CORES) ./ridl_invalidpage
	#
	# should leak 67 (userspace page table flags)
	taskset -c $(CORES) ./pgtable_leak
	
	taskset -c $(CORES) ./loadport

	# should leak 67 (userspace page table flags, no SMT/TSX)
	./pgtable_leak_notsx | sort -n
	
	# l1des
	./verw_bypass_l1des
	
	# vrs
	taskset -c $(CORES) ./vrs

cpuid_leak_taa: cpuid_leak.c ridl.h leak_code.h
	gcc -O3 -Wall -DTAA -o $@ $<

%: %.c ridl.h ridl2.h leak_code.h
	gcc -O3 -o $@ $<

