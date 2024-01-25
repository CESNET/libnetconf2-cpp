# C++ bindings for libnetconf2

![License](https://img.shields.io/github/license/CESNET/libnetconf2-cpp)
[![Gerrit](https://img.shields.io/badge/patches-via%20Gerrit-blue)](https://gerrit.cesnet.cz/q/project:CzechLight/libnetconf2-cpp)
[![Zuul CI](https://img.shields.io/badge/zuul-checked-blue)](https://zuul.gerrit.cesnet.cz/t/public/buildsets?project=CzechLight/libnetconf2-cpp)

Object-oriented bindings of the [`libnetconf2`](https://github.com/CESNET/libnetconf2) library written in C++20. Memory is managed with RAII.

## Dependencies
- [libyang-cpp](https://github.com/CESNET/libyang-cpp) >= 1.1.0 - C++ bindings for *libyang*
- [libnetconf2](https://github.com/CESNET/libnetconf2-cpp) >= 2.1.40,  < 3. Last version compatible is [2.1.40](https://github.com/CESNET/libnetconf2/tree/99fd7ad8643717fa760388599e90064ac964f7e4)
- C++20 compiler (e.g., GCC 10.x+, clang 10+)
- CMake >= 3.19

### optional for tests
- [doctest](https://github.com/doctest/doctest) >= 2.4.8
- boost

### optional for docs
- doxygen

## Building
*libnetconf2-cpp* uses *CMake* for building.
One way of building *libyang-cpp* looks like this:
```
cmake . -B build
cmake --build build
cmake --install build
```

## Usage

Check the [test suite in `tests/`](tests/) for usage examples.

## Contributing
The development is being done on Gerrit [here](https://gerrit.cesnet.cz/q/project:CzechLight/libnetconf2-cpp).
Instructions on how to submit patches can be found [here](https://gerrit.cesnet.cz/Documentation/intro-gerrit-walkthrough-github.html).
