
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

pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)
pkg_check_modules(LIBNUMA REQUIRED IMPORTED_TARGET numa)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${LIBNUMA_INCLUDE_DIRS}
)

link_libraries(
  ${LIBNUMA_LIBRARIES}
)

include(FetchContent)

FetchContent_Declare(
  NHJson
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(NHJson)
include_directories(${nlohmann_json_SOURCE_DIR}/include)


add_executable(libevdev-sample main.cpp InputSetter.cpp ${SOURCES})

target_link_libraries(libevdev-sample PRIVATE PkgConfig::LIBEVDEV nlohmann_json::nlohmann_json)

target_include_directories(libevdev-sample PRIVATE ${CMAKE_SOURCE_DIR})


add_executable(INPUT_GETTER ipcMain.cpp InputSetter.cpp ${SOURCES})

target_link_libraries(INPUT_GETTER PRIVATE PkgConfig::LIBEVDEV nlohmann_json::nlohmann_json)

target_include_directories(INPUT_GETTER PRIVATE ${CMAKE_SOURCE_DIR})

