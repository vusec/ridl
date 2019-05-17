Building for Linux
==================

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
