cmake_minimum_required(VERSION 4.3)
project(GStreamerApps LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST           REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_BASE      REQUIRED IMPORTED_TARGET gstreamer-base-1.0)
pkg_check_modules(GST_APP       REQUIRED IMPORTED_TARGET gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO     REQUIRED IMPORTED_TARGET gstreamer-video-1.0)
pkg_check_modules(GST_AUDIO     REQUIRED IMPORTED_TARGET gstreamer-audio-1.0)

# Create a common INTERFACE library that all apps will use
add_library(gst_common INTERFACE)
target_link_libraries(gst_common INTERFACE
    PkgConfig::GST
    PkgConfig::GST_BASE
    PkgConfig::GST_APP
    PkgConfig::GST_VIDEO
    PkgConfig::GST_AUDIO
)
target_compile_options(gst_common INTERFACE -Wall -Wextra)


add_subdirectory(hello-world)
add_subdirectory(hello-world-extended)
add_subdirectory(basic-tutorial-2)
add_subdirectory(basic-tutorial-3)
