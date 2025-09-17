
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

set(SOURCES
${CMAKE_SOURCE_DIR}/include/SocketServer.cpp
${CMAKE_SOURCE_DIR}/include/ProcessCTRL.cpp

)


include_directories(
  ${CMAKE_SOURCE_DIR}/include
)

pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_executable(libevdev-sample main.cpp InputSetter.cpp ${SOURCES})

target_link_libraries(libevdev-sample PRIVATE PkgConfig::LIBEVDEV)

target_include_directories(libevdev-sample PRIVATE ${CMAKE_SOURCE_DIR})


add_executable(INPUT_GETTER ipcMain.cpp InputSetter.cpp ${SOURCES})

target_link_libraries(INPUT_GETTER PRIVATE PkgConfig::LIBEVDEV)

target_include_directories(INPUT_GETTER PRIVATE ${CMAKE_SOURCE_DIR})