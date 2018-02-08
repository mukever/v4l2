cmake_minimum_required(VERSION 3.5)
project(v4l2)

set( CMAKE_CXX_FLAGS "-std=c++14  -pthread  ")
set(CMAKE_CXX_STANDARD 14 )
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})
set( CMAKE_CXX_FLAGS "-std=c++14  -pthread" )




set(SOURCE_FILES main.cpp yuvproc.cpp video.cpp window.cpp ysqfd.cpp fcvimage.cpp oaidsys.cpp oaid_base.cpp queue.cpp
yuvproc.h video.h  window.h ysqfd.h fcvimage.h oaidsys.h oaid_base.h list.h midlayer.h perf.h pixel.h  queue.h sysdef.h
connector.cpp connector.hpp controller.hpp terminal.cpp terminal.hpp stringx.cpp stringx.hpp  )
include_directories(${GTK_INCLUDE_DIRS})


add_executable(v4l2 ${SOURCE_FILES})
target_link_libraries(v4l2 ${GTK3_LIBRARIES})

