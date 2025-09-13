
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.12)
project(
  libevdev-sample
  VERSION 0.1
  LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_executable(libevdev-sample main.cpp InputSetter.cpp)

target_link_libraries(libevdev-sample PRIVATE PkgConfig::LIBEVDEV)

target_include_directories(libevdev-sample PRIVATE ${CMAKE_SOURCE_DIR})