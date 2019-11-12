CORES=3,7

all: cross_transaction_leak verw_bypass alignment_write ridl_basic ridl_invalidpage taa_basic taa_read pgtable_leak pgtable_leak_notsx loadport

clean:
	rm cross_transaction_leak verw_bypass alignment_write ridl_basic ridl_invalidpage taa_basic taa_read pgtable_leak pgtable_leak_notsx loadport

run: all
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

%: %.c ridl.h leak_code.h
	gcc -O3 -Wall -o $@ $<

