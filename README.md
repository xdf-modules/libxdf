# libxdf

## *A static C++ library for loading [XDF](https://github.com/sccn/xdf/wiki/Specifications) files*


Libxdf requires [Qt 5.8 MinGW 5.3](https://www.qt.io/download-open-source/#section-2) to build.

Other Qt 5 versions with a MinGW compiler might work, but is not guaranteed.

Pre-built binaries can be downloaded from the [release](https://github.com/Yida-Lin/libxdf/releases) page.



### How to use in conjunction with [SigViewer](https://github.com/cbrnr/sigviewer):

1. Download `xdf.h` and `libxdf.a` from the [release](https://github.com/Yida-Lin/libxdf/releases) page.

2. Copy `xdf.h` into `sigviewer/external/include`

3. Copy `libxdf.a` into `sigviewer/external/lib`

4. Choose [Qt 5.8 MinGW 5.3](https://www.qt.io/download-open-source/#section-2) as the toolkit to configure SigViewer.

5. Build and run Sigviewer!

![SigViewer using _libxdf_ to display signals in XDF files](docs/Example.png)

Example: SigViewer using _libxdf_ to display signals in an XDF file.

### How to use in other C++ applications:

1. Clone the repo and build with [Qt 5.8 MinGW 5.3](https://www.qt.io/download-open-source/#section-2), or use the pre-built version.

2. Instantiate an object of `Xdf` class and call the load_xdf() method.

Example:

```C++
#include "xdf.h"

Xdf XDFdata;						//the object to store XDF data
XDFdata.load_xdf("C:/example.xdf");	//parameter is the path to the target XDF file
```

If you would like to resample the signals, call:

```C++

XDFdata.resample(100);	//resample to 100Hz

```

The functions in libxdf must be called following certain order. For instance, if you call the `subtractMean()` function before you load any data into 
the object, it will cause undefined behavior.

The recommended order is shown as following. Only `load_xdf()` is mandatory. 

```C++
XDFdata.load_xdf(std::string filepath);
XDFdata.subtractMean();
XDFdata.createLabels();
XDFdata.resample(int sampleRate);
XDFdata.freeUpTimeStamps();
```

Libxdf depends on third party libraries [Pugixml v1.8](http://pugixml.org/) for XML parsing and [Smarc](http://audio-smarc.sourceforge.net/) for resampling.

Detailed documentation was generated via [**Doxygen**](http://www.stack.nl/~dimitri/doxygen/index.html).
