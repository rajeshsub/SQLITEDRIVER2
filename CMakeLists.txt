#============================================================================
# CSIRO LGPL header to be included
#============================================================================


cmake_minimum_required(VERSION 3.0)
project(SQLITEDRIVER2)


include_directories(${SQLITEDRIVER2_SOURCE_DIR})
if (NOT ${SQLITEDRIVER2_SOURCE_DIR} STREQUAL ${SQLITEDRIVER2_BINARY_DIR})
    include_directories(${SQLITEDRIVER2_BINARY_DIR})
endif()

find_package(Qt5Core)
find_package(Qt5Sql)
set(QT_LIBRARIES Qt5::Core;Qt5::Sql)

set(HEADERS
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2.h
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlite3.h
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlite3ext.h
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2_functions.h
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2_api.h
)

set(INSTALL_HEADERS
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2.h 
)

set(MOC_HEADERS
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2.h
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2_functions.h
)

set(SOURCES
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2.cpp
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlitedriver2_functions.cpp
    ${SQLITEDRIVER2_SOURCE_DIR}/sqlite3.c
    ${SQLITEDRIVER2_SOURCE_DIR}/shell.c
)

qt5_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})
qt5_add_resources(RES_SOURCES ${RESOURCES})

add_library(SQLITEDRIVER2 SHARED ${SOURCES} ${HEADERS} ${MOC_SOURCES} ${MOC_HEADERS})
target_link_libraries(SQLITEDRIVER2 ${QT_LIBRARIES})