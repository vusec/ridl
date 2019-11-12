Microsoft Windows
=================

Fully supported. Build instructions will follow.

Linux
=====

Clone the repository:

```
git clone https://github.com/vusec/ridl
cd ridl
```

Install the submodules (dependencies):

```
git submodule init
git submodule update
```

Generate the Makefile using CMake:

```
mkdir build
cd build
cmake ..
```

Build it using `make`:

```
make
```

If everything went well, you should now have `mdstool` and `mdstool-cli`.

Mac OS X
========

Clone the repository:

```
git clone https://github.com/vusec/ridl
cd ridl
```

Install the submodules (dependencies):

```
git submodule init
git submodule update
```

Generate the Makefile using CMake:

```
mkdir build
cd build
cmake ..
```

Build it using `make`:

```
make
```

If everything went well, you should now have `mdstool-cli`.

FreeBSD
=======

TODO

FAQ
===

Q) Can I run this in a VM?

This program relies on the `cpuid` instruction, which may report the wrong information in a VM. Make sure to run this on the actual hardware without virtualization.
