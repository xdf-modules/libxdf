cmake_minimum_required(VERSION 3.7)

project(libxdf)

add_library(xdf STATIC
    xdf.h
    xdf.cpp
    pugiconfig.hpp
    pugixml.hpp
    pugixml.cpp
    filtering.h
    filtering.c
    multi_stage.h
    multi_stage.c
    polyfilt.h
    polyfilt.c
    remez_lp.h
    remez_lp.c
    smarc.h
    smarc.c
    stage_impl.h
    stage_impl.c
)

set_property(TARGET xdf PROPERTY CXX_STANDARD 11)
set_property(TARGET xdf PROPERTY C_STANDARD 99)
