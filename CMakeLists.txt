cmake_minimum_required(VERSION 3.1)

project(libxdf)

include_directories("${PROJECT_SOURCE_DIR}")

set(SOURCE_FILES
    xdf.cpp
    pugixml.cpp
    filtering.c
    multi_stage.c
    polyfilt.c
    remez_lp.c
    smarc.c
    stage_impl.c
)

set(HEADER_FILES
    xdf.h
    pugiconfig.hpp
    pugixml.hpp
    filtering.h
    multi_stage.h
    polyfilt.h
    remez_lp.h
    smarc.h
    stage_impl.h
)

add_library(xdf STATIC ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET xdf PROPERTY CXX_STANDARD 11)
set_property(TARGET xdf PROPERTY C_STANDARD 99)
