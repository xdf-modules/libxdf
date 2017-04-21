cmake_minimum_required(VERSION 3.7)

project(libxdf)

add_library(xdf STATIC
    xdf.h
    xdf.cpp
    pugixml/pugiconfig.hpp
    pugixml/pugixml.hpp
    pugixml/pugixml.cpp
    smarc/filtering.h
    smarc/filtering.c
    smarc/multi_stage.h
    smarc/multi_stage.c
    smarc/polyfilt.h
    smarc/polyfilt.c
    smarc/remez_lp.h
    smarc/remez_lp.c
    smarc/smarc.h
    smarc/smarc.c
    smarc/stage_impl.h
    smarc/stage_impl.c
)

set_property(TARGET xdf PROPERTY CXX_STANDARD 11)
set_property(TARGET xdf PROPERTY C_STANDARD 99)
