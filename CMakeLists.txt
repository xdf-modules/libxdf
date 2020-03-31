cmake_minimum_required(VERSION 3.7)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)

project(libxdf)

option(XDF_NO_SYSTEM_PUGIXML "Force use of bundled pugixml" OFF)

set(SOURCES
    xdf.h
    xdf.cpp
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

add_library(xdf ${SOURCES})

find_package(pugixml 1.9 QUIET)
if(TARGET pugixml AND NOT XDF_NO_SYSTEM_PUGIXML)
	message(STATUS "Using system pugixml")
	target_link_libraries(xdf PRIVATE pugixml)
else()
	message(STATUS "Using bundled pugixml")
	target_sources(xdf PRIVATE pugixml/pugixml.cpp)
	target_include_directories(xdf PRIVATE pugixml)
endif()

target_compile_features(xdf PUBLIC cxx_std_11)
set_target_properties(xdf PROPERTIES OUTPUT_NAME xdf PUBLIC_HEADER xdf.h)
target_include_directories(xdf PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	$<INSTALL_INTERFACE:include>
)
include(GNUInstallDirs)
install(TARGETS xdf
	EXPORT "${PROJECT_NAME}Config"
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT "${PROJECT_NAME}Config"
	COMPONENT ${PROJECT_NAME}
	NAMESPACE "XDF::"
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}
)
