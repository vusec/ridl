# Installation

To build the memkit kernel module and the corresponding library, run the following:

```
make
sudo make install
```

Then load the kernel module as follows:

```
sudo insmod memkit.ko
```

Beware that you should not use this kernel module on any production system as it gives normal users access to features that are meant for research purposes and that make your system unsafe.
The features include: full access to physical and kernel virtual memory, the ability to traverse and modify page tables, allocate/free kernel memory, etc.

