# libXDF

static library for loading XDF files

This instruction assumes you have the source code of Sigviewer with XDF feature.

How to use:

Clone the repo and build with Qt 5.3 MinGW 4.8
Once complete, copy "xdf.h" into sigviewer/external/include
Copy debug or release version of liblibxdf.a into sigviewer/external/lib
Build and run Sigviewer!

Note: this repository includes third party libraries Pugixml for XML parsing and Smarc for resampling
