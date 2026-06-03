cmake_minimum_required(VERSION 3.22)

project(timeseries_viewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CTest)

option(TSV_BUILD_APP "Build the desktop application" ON)
option(TSV_BUILD_TESTS "Build the test suite" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_library(timeseries_core INTERFACE)
target_include_directories(timeseries_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(Lua REQUIRED)

target_link_libraries(timeseries_core INTERFACE nlohmann_json::nlohmann_json SQLite::SQLite3 sol2::sol2 ${LUA_LIBRARIES})
target_include_directories(timeseries_core INTERFACE ${LUA_INCLUDE_DIR})

add_library(timeseries_app_model STATIC
  src/app/app_model.cpp
)
target_link_libraries(timeseries_app_model PUBLIC timeseries_core)
target_include_directories(timeseries_app_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(TSV_BUILD_APP)
  find_package(OpenGL REQUIRED)
  find_package(glad CONFIG REQUIRED)
  find_package(glfw3 CONFIG REQUIRED)
  find_package(imgui CONFIG REQUIRED)
  find_package(implot CONFIG REQUIRED)
  find_package(nfd CONFIG REQUIRED)

  add_executable(timeseries_viewer
    src/app/main.cpp
    src/third_party/imgui/backends/imgui_impl_glfw.cpp
    src/third_party/imgui/backends/imgui_impl_opengl3.cpp
  )

  target_include_directories(timeseries_viewer PRIVATE src/third_party/imgui/backends)
  target_link_libraries(timeseries_viewer PRIVATE
    timeseries_app_model
    glad::glad
    glfw
    imgui::imgui
    implot::implot
    nfd::nfd
    OpenGL::GL
    ${CMAKE_DL_LIBS}
  )

  target_compile_definitions(timeseries_viewer PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

if(TSV_BUILD_TESTS)
  find_package(Catch2 CONFIG REQUIRED)

  add_executable(timeseries_viewer_tests
    tests/core_tests.cpp
    tests/app_model_tests.cpp
    tests/gui_render_tests.cpp
  )

  target_compile_definitions(timeseries_viewer_tests PRIVATE TSV_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(timeseries_viewer_tests PRIVATE
    timeseries_app_model
    Catch2::Catch2WithMain
  )

  include(Catch)
  catch_discover_tests(timeseries_viewer_tests)
endif()
