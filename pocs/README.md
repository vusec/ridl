These are (slightly cleaned-up) versions of a few old RIDL PoCs.

To run all of them, you'll need TSX support, up-to-date microcode (with the mitigations), and you need to have SMT (hyper-threading) enabled.
You also need huge pages (or remove HUGETLB from mmap calls); manually allocate some like this:

```
echo 16 | sudo tee /proc/sys/vm/nr_hugepages
```

You can run these PoCs using `make run` (remember to change the cores in the Makefile to make sense for your machine).

