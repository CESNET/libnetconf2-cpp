# C++ bindings for libnetconf2

![License](https://img.shields.io/github/license/CESNET/libnetconf2-cpp)
[![Gerrit](https://img.shields.io/badge/patches-via%20Gerrit-blue)](https://gerrit.cesnet.cz/q/project:CzechLight/libnetconf2-cpp)
[![Zuul CI](https://img.shields.io/badge/zuul-checked-blue)](https://zuul.gerrit.cesnet.cz/t/public/buildsets?project=CzechLight/libnetconf2-cpp)

Object-oriented bindings of the [`libnetconf2`](https://github.com/CESNET/libnetconf2) library.

## Installation
- C++20 compiler (e.g., GCC 10.x+, clang 10+)
- CMake 3.19+
- [`pkg-config`](https://www.freedesktop.org/wiki/Software/pkg-config/)
- [`libnetconf2`](https://github.com/CESNET/libnetconf2)
- [C++ bindings for `libyang`](https://github.com/CESNET/libyang-cpp)
- optionally, Doxygen for building the documentation
- optionally, [Doctest](https://github.com/doctest/doctest/) as a C++ unit test framework
- optionally, [Boost](https://www.boost.org/) for the test suite

The build process uses [CMake](https://cmake.org/runningcmake/).
A quick-and-dirty build with no fancy options can be as simple as `mkdir build && cd build && cmake .. && make && make install`.

## Contributing
The current version wraps just enough to get a NETCONF client running over a file descriptor.
That's enough for our use case.
If you need more and are willing to maintain your contribution, patches welcome.

The development is being done on Gerrit [here](https://gerrit.cesnet.cz/q/project:CzechLight/libnetconf2-cpp).
Instructions on how to submit patches can be found [here](https://gerrit.cesnet.cz/Documentation/intro-gerrit-walkthrough-github.html).
GitHub Pull Requests are not used.
