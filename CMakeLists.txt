cmake_minimum_required(VERSION 3.20)
project(FOpenGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WITH_IMGUI "Build with Dear ImGui UI layer" ON)

# -- vcpkg-friendly find_package --
find_package(glfw3 CONFIG REQUIRED)   # vcpkg target: glfw
find_package(GLEW  CONFIG REQUIRED)   # vcpkg target: GLEW::GLEW
find_package(glm   CONFIG REQUIRED)   # vcpkg target: glm::glm

set(HAVE_PKG_IMGUI OFF)
if (WITH_IMGUI)
  find_package(imgui CONFIG QUIET)
  if (TARGET imgui::imgui)
    set(HAVE_PKG_IMGUI ON)
  endif()
endif()

add_library(FOpenGL STATIC
    src/Application/GLApplication.cpp
    src/Application/Time.cpp
    src/Window/GLWindow.cpp
    src/Rendering/Shader.cpp
    src/Rendering/Texture.cpp
    src/Rendering/Mesh.cpp
    src/Camera/Camera.cpp
    src/Light/LightCommon.cpp
    src/Light/DirectionalLight.cpp
    src/Light/PointLight.cpp
    src/Materials/Material.cpp
    src/Materials/MaterialFactory.cpp
    src/ThirdParty/ImGuiHandler.cpp
)
target_include_directories(FOpenGL PUBLIC include)

target_link_libraries(FOpenGL PUBLIC glfw GLEW::GLEW glm::glm)

if (WITH_IMGUI)
  set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui")
  if (EXISTS "${IMGUI_DIR}/imgui.cpp")
    add_library(imgui
      ${IMGUI_DIR}/imgui.cpp
      ${IMGUI_DIR}/imgui_draw.cpp
      ${IMGUI_DIR}/imgui_tables.cpp
      ${IMGUI_DIR}/imgui_widgets.cpp
      ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
      ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
      ${IMGUI_DIR}
      ${IMGUI_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw GLEW::GLEW)

    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

    target_link_libraries(FOpenGL PUBLIC imgui)
    target_compile_definitions(FOpenGL PUBLIC USE_IMGUI=1)
  else()
    message(STATUS "ImGui not found in extern/imgui. Building without UI.")
  endif()
endif()

add_executable(FOpenGL_app src/Application/main.cpp)
target_link_libraries(FOpenGL_app PRIVATE FOpenGL)

add_custom_command(TARGET FOpenGL_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "$<TARGET_FILE_DIR:FOpenGL_app>/shaders"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:FOpenGL_app>/assets")

# ---- tests ----
if (WITH_TESTS)
  include(CTest)
  # Try vcpkg first
  find_package(GTest CONFIG QUIET)
  if (NOT TARGET GTest::gtest)
    message(STATUS "GTest via package manager not found. Using FetchContent.")
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # MSVC CRT
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(FOpenGL_tests
    tests/test_time.cpp
  )
  target_include_directories(FOpenGL_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(FOpenGL_tests PRIVATE FOpenGL glm::glm)

  # Pick correct gtest targets (vcpkg or FetchContent)
  if (TARGET GTest::gtest_main)
    target_link_libraries(FOpenGL_tests PRIVATE GTest::gtest GTest::gtest_main)
  else()
    target_link_libraries(FOpenGL_tests PRIVATE gtest gtest_main)
  endif()

  target_compile_definitions(FOpenGL_tests PRIVATE _CRT_SECURE_NO_WARNINGS)

  include(GoogleTest OPTIONAL RESULT_VARIABLE _has_gtest_mod)
  if (_has_gtest_mod)
    gtest_discover_tests(FOpenGL_tests)
  else()
    add_test(NAME fogl_tests COMMAND fogl_tests)
  endif()
endif()