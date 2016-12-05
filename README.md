# libXDF

static library for loading XDF files

This instruction assumes you have the source code of Sigviewer with XDF feature.

How to use:

1. Clone the repo and build with Qt 5.3 MinGW 4.8

2. Once complete, copy "xdf.h" into sigviewer/external/include

3. Copy debug or release version of libxdf.a into sigviewer/external/lib

4. Build and run Sigviewer!

Note: this repository includes third party libraries Pugixml for XML parsing and Smarc for resampling
