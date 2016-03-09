# Area #

Area is a CAM-related software for pocketing operation.

This project provides library and associated python-module to compute pocket operations.

Written by _Dan Heeks_ <danheeks@gmail.com>, derived from the [kbool](http://boolean.klaasholwerda.nl/bool.html) library written by _Klaas Holwerda_

Here is some tool-path created with [Pocket](Pocket.md) ( Displayed using [HeeksCNC](https://github.com/Heeks/heekscnc) ).

## Installation ##

### Dependencies ###
To build libarea, you need:
  * CMake
  * Python (with developers files)
  * Python Boost (with developers files)

### Fetch sources ###
```
git clone git@github.com:Heeks/libarea.git
```

### Build ###

#### Under MacOSX, GNU/Linux, `*`BSD and probably a lot of POSIX systems ####
```
cd libarea
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## Contribute ##
You can contribute to libarea many ways:
  * Catch bugs and report them to issue tracker
  * Write patches and report them on associated issue or by email
  * Improve it: optimizations, build enhancements, documentation, etc. are welcome
  * Package it software for your preferred distributions
