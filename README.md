# Libxdf – a C++ library for loading [XDF](https://github.com/sccn/xdf/wiki/Specifications "Extensible Data Format") files

[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![License](https://img.shields.io/github/license/xdf-modules/libxdf)](https://opensource.org/licenses/BSD-2-Clause)

* [Introduction](#intro)
* [Quick-Start Guide](#quick)
    * [Download](#download)
* [Documentation](#doc)
* [Reference](#reference)
* [Support](#support)


## <a name="intro"></a>Introduction

Libxdf is a cross-platform C++ library for loading multimodal, multi-rate signals stored in [XDF](https://github.com/sccn/xdf/wiki/Specifications  "Extensible Data Format") files.
Libxdf is used in the biosignal viewing application [SigViewer](https://github.com/cbrnr/sigviewer) and the LSL application [XDFStreamer](https://github.com/labstreaminglayer/App-XDFStreamer/). It can also be integrated into other C++ applications.

Libxdf is open-source, free, and actively maintained.

## <a name="quick"></a>Quick-Start Guide

### <a name="download"></a>Download

* Find Source and Prebuilt Binaries on the [releases page](https://github.com/Yida-Lin/libxdf/releases).
    * You may need to expand the list of Assets to find the downloads.
* For Linux Debian (Ubuntu) users: `sudo dpkg -i libxdf-{version}-Linux.deb`
* For Windows and Mac users: simply extract the archive somewhere convenient.

### Building libxdf

If the release does not have assets for your platform or they do not work for you for some other reason, then
Libxdf can be conveniently built either using `qmake` or `cmake`. Configuration files for both build tools are included with the source.

To build with cmake from command prompt / terminal:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=${PWD}/build/install
cmake --build build --config Release -j --target install
```

`cmake` builds a static library by default, but you can build a shared library
by setting the
[`BUILD_SHARED_LIBS`](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html)
variable (e.g. add `-DBUILD_SHARED_LIBS=ON` to the first cmake command above).

### Use in conjunction with [SigViewer](https://github.com/cbrnr/sigviewer)

Libxdf is a built-in component of [SigViewer](https://github.com/cbrnr/sigviewer). If you wish to build SigViewer from source, follow these steps:

1. Download `xdf.h` and `libxdf.a` from the [release](https://github.com/Yida-Lin/libxdf/releases) page.
2. Copy `xdf.h` into `sigviewer/external/include`
3. Copy `libxdf.a` into `sigviewer/external/lib`
4. Build and run Sigviewer


![SigViewer using _libxdf_ to display signals in XDF files](docs/Example.png)

Example: SigViewer using _libxdf_ to display signals in an XDF file.

### Use in other C++ applications

1. Install a prebuilt binary release or build from source as above.
2. For CMake users:
    * In your project's CMakeLists.txt, use the following snippet:
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
    * If the libxdf package was installed or extracted into a folder other than a standard system library folder, you will have to pass a cmake command line argument to indicate where to find it: `-DXDF_INSTALL_ROOT=path/to/libxdf`
3. In your source code, `#include "xdf.h"`, instantiate an object of the `Xdf` class and call the `load_xdf` method.

Example:

```C++
#include "xdf.h"

Xdf XDFdata;
XDFdata.load_xdf("C:/example.xdf");
```

To resample the signals to e.g. 100Hz:

```C++
XDFdata.resample(100);
```

The functions in libxdf must be called following a certain order. For instance, if you call the `subtractMean` function before you load any data, it will cause undefined behavior.

The recommended order is shown here. Only `load_xdf` is mandatory.

```C++
XDFdata.load_xdf(std::string filepath);
XDFdata.subtractMean();
XDFdata.createLabels();
XDFdata.resample(int sampleRate);
XDFdata.freeUpTimeStamps();
```

Libxdf depends on third party libraries [Pugixml v1.8](http://pugixml.org/) for XML parsing and [Smarc](http://audio-smarc.sourceforge.net/) for resampling.

## <a name="doc"></a> Documentation
Detailed documentation was generated via [Doxygen](http://www.stack.nl/~dimitri/doxygen/index.html) and is available [here](docs/html/class_xdf.html).

## <a name="SigViewer"></a> SigViewer Online Repo
SigViewer Online Repository is [here](repository/Updates.xml).

## <a name="reference"></a> Reference
If you use this code in your project, please cite:
```
Yida Lin, Clemens Brunner, Paul Sajda and Josef Faller. SigViewer: Visualizing Multimodal Signals Stored in XDF (Extensible Data Format) Files. The 39th Annual International Conference of the IEEE Engineering in Medicine and Biology Society.
```
Direct link: https://arxiv.org/abs/1708.06333

Bibtex format:
```
@article{lin2017sigviewer,
  title={SigViewer: visualizing multimodal signals stored in XDF (Extensible Data Format) files},
  author={Lin, Yida and Brunner, Clemens and Sajda, Paul and Faller, Josef},
  journal={arXiv},
  pages={arXiv--1708},
  year={2017}
}
```

## <a name="support"></a>Support
[Email author](mailto:yl3842@columbia.edu) or report a new [issue](https://github.com/Yida-Lin/libxdf/issues).
