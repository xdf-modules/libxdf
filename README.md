# LibXDF – a C++ library for loading XDF files

LibXDF is a cross-platform C++ library for loading multimodal signals stored in [XDF](https://github.com/sccn/xdf/wiki/Specifications  "Extensible Data Format") files.
It is used in the biosignal viewing application [SigViewer](https://github.com/cbrnr/sigviewer) and the LSL application [XDFStreamer](https://github.com/labstreaminglayer/App-XDFStreamer/) and can also be integrated into other C++ applications.


## Quick start

The source code and prebuilt binaries are available as [releases](https://github.com/Yida-Lin/libxdf/releases) (you may need to expand the list of assets to find the downloads).

If a particular release does not have assets for your platform or they do not work for you for some other reason, then
libXDF can be built with [CMake](https://cmake.org) using the following two commands:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=${PWD}/build/install -DCMAKE_BUILD_TYPE=Release
cmake --build build -j --target install
```

This builds and installs a static library in `./build/install`, but you can build a shared library by adding `-DBUILD_SHARED_LIBS=ON` to the first CMake command.


## Integrating into C++ applications

If you want to use libXDF in C++ applications, follow these steps:

1. Install a prebuilt binary release or build libXDF from source as described above.
2. If you use CMake, add the following lines to your `CMakeLists.txt` to find and link against libXDF:

    ```CMake
    find_package(libxdf REQUIRED
        HINTS ${XDF_INSTALL_ROOT}
        PATH_SUFFIXES share
    )
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        # ... other dependencies
        XDF::xdf
    )
    ```

    If `libxdf` is not in a standard system library location, pass `-DXDF_INSTALL_ROOT=path/to/libxdf` to specify its path.
3. In your source code, `#include "xdf.h"`, instantiate an object of the `Xdf` class, and call the `load_xdf` method.

    For example:

    ```C++
    #include "xdf.h"

    Xdf XDFdata;
    XDFdata.load_xdf("example.xdf");
    ```

Functions must be called in a specific order. For example, calling `subtractMean` before loading data leads to undefined behavior.

The recommended order is shown below:

```C++
XDFdata.load_xdf(std::string filepath);
XDFdata.subtractMean();
XDFdata.createLabels();
XDFdata.resample(int sampleRate);
XDFdata.freeUpTimeStamps();
```


## Documentation

Detailed documentation is available [here](docs/html/class_xdf.html).


## References

If you use libXDF in your project, please consider citing the following [conference paper](https://arxiv.org/abs/1708.06333):

> Yida Lin, Clemens Brunner, Paul Sajda and Josef Faller. *SigViewer: Visualizing Multimodal Signals Stored in XDF (Extensible Data Format) Files.* The 39th Annual International Conference of the IEEE Engineering in Medicine and Biology Society.

LibXDF depends on third party libraries [pugixml](http://pugixml.org/) for XML parsing and [Smarc](http://audio-smarc.sourceforge.net/) for resampling.
