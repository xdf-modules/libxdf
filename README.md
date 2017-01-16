# libxdf

## *A static C++ library for loading [XDF](https://github.com/sccn/xdf/wiki/Specifications) files*

Libxdf requires [Qt 5.7 MinGW 5.3](https://www.qt.io/download-open-source/#section-2) to build.

Other Qt 5 versions with a MinGW compiler might work, but is not guaranteed.


### How to use in conjunction with [Sigviewer](https://github.com/cbrnr/sigviewer):

1. Clone the repo and build with [Qt 5.7 MinGW 5.3](https://www.qt.io/download-open-source/#section-2)

2. Once complete, copy `xdf.h` into `sigviewer/external/include`

3. Copy debug or release version of `libxdf.a` into `sigviewer/external/lib`

4. Build and run Sigviewer!

### How to use in other C++ applications:

1. Clone the repo and build with [Qt 5.7 MinGW 5.3](https://www.qt.io/download-open-source/#section-2)

2. Instantiate an object of `Xdf` class and call the load_xdf() method.

Example:

```C++
#include "xdf.h"

Xdf XDFdata;						//the object to store XDF data
XDFdata.load_xdf("C:/example.xdf");	//parameter is the path to the target XDF file
```

If you would like to resample the signals, call:

```C++

int desiredSampleRate = 100;			//choose any sample rate you would like
XDFdata.resampleXDF(desiredSampleRate);	//resamples the data to the desired sample rate

```

The functions in libxdf must be called following certain order. For instance, if you call the `subtractMean()` function before you load any data into 
the object, it will cause undefined behavior.

The recommended order is shown as following. Only `load_xdf()` is mandatory. 

```C++
XDFdata.load_xdf(std::string filepath);
XDFdata.subtractMean();
XDFdata.createLabels();
XDFdata.resampleXDF(int sampleRate);
XDFdata.freeUpTimeStamps();
```

Libxdf depends on third party libraries [Pugixml v1.8](http://pugixml.org/) for XML parsing and [Smarc](http://audio-smarc.sourceforge.net/) for resampling.

Detailed documentation was generated via [**Doxygen**](http://www.stack.nl/~dimitri/doxygen/index.html).
