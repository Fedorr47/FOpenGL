cmake_minimum_required(VERSION 3.23)
project(FOpenGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

option(WITH_IMGUI           "Build with Dear ImGui UI layer"    ON)
option(WITH_ASAN            "Enable AddressSanitizer"           OFF)
option(WITH_TESTS           "Build tests"                       OFF)
option(WITH_CLANG_TIDY      "Run clang-tidy"                    OFF)
option(WITH_MSVC_ANALYZE    "Enable MSVC code analysis"         OFF)

# --- deps (vcpkg) ---
find_package(glfw3 CONFIG REQUIRED) # vcpkg target: glfw 
find_package(GLEW CONFIG REQUIRED) # vcpkg target: GLEW::GLEW 
find_package(glm CONFIG REQUIRED) # vcpkg target: glm::glm 
find_package(assimp CONFIG REQUIRED) # vcpkg target: assimp

# --- main lib ---
add_library(FOpenGL STATIC
    src/Application/GLApplication.cpp
    src/Application/Time.cpp
    src/Window/GLWindow.cpp
    src/Rendering/Shader.cpp
    src/Texture/Texture.cpp
    src/Model/Mesh.cpp
    src/Model/Model.cpp
    src/Camera/Camera.cpp
    src/Light/LightCommon.cpp
    src/Light/DirectionalLight.cpp
    src/Light/PointLight.cpp
    src/Materials/Material.cpp
    src/Materials/MaterialFactory.cpp
    src/ThirdParty/ImGuiHandler.cpp
)
target_include_directories(FOpenGL PUBLIC include)
target_link_libraries(FOpenGL PUBLIC glfw GLEW::GLEW glm::glm assimp::assimp)

# --- ImGui (extern/imgui) ---
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
    
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
    target_link_libraries(imgui PUBLIC glfw GLEW::GLEW)

    target_link_libraries(FOpenGL PUBLIC imgui)
    target_compile_definitions(FOpenGL PUBLIC USE_IMGUI=1)
  else()
    message(STATUS "ImGui not found in extern/imgui. Building without UI.")
  endif()
endif()

if (WITH_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy)
  if (CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY};
      -checks=modernize-*,bugprone-*,performance-*,readability-*,cppcoreguidelines-*;
      -warnings-as-errors=modernize-*,bugprone-*,performance-*)
  endif()
endif()

if (MSVC AND WITH_MSVC_ANALYZE)
  target_compile_options(FOpenGL PRIVATE /analyze)
endif()

# --- exe ---
add_executable(FOpenGL_app src/Application/main.cpp)
target_link_libraries(FOpenGL_app PRIVATE FOpenGL)

add_custom_command(TARGET FOpenGL_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:FOpenGL_app>/shaders"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/assets"  "$<TARGET_FILE_DIR:FOpenGL_app>/assets"
)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
  add_custom_command(TARGET FOpenGL_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:FOpenGL_app> $<TARGET_FILE_DIR:FOpenGL_app>
    COMMAND_EXPAND_LISTS
  )
endif()

# --- AddressSanitizer
if (WITH_ASAN)
  if (MSVC)
    target_compile_options(FOpenGL     PRIVATE /fsanitize=address /Zi)
    target_compile_options(FOpenGL_app PRIVATE /fsanitize=address /Zi)
    target_link_options(FOpenGL     PRIVATE /INCREMENTAL:NO)
    target_link_options(FOpenGL_app PRIVATE /INCREMENTAL:NO)

    if(DEFINED ENV{VCToolsInstallDir})
      set(ASAN_BIN "$ENV{VCToolsInstallDir}/bin/Hostx64/x64")
      set(ASAN_LIB "$ENV{VCToolsInstallDir}/lib/x64")
      target_link_directories(FOpenGL     PRIVATE "${ASAN_LIB}")
      target_link_directories(FOpenGL_app PRIVATE "${ASAN_LIB}")
      target_link_libraries(FOpenGL     PRIVATE clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib)
      target_link_libraries(FOpenGL_app PRIVATE clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib)
      add_custom_command(TARGET FOpenGL_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${ASAN_BIN}/clang_rt.asan_dynamic-x86_64.dll"
          "$<TARGET_FILE_DIR:FOpenGL_app>")
    endif()
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(FOpenGL     PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options  (FOpenGL     PRIVATE -fsanitize=address)
    target_compile_options(FOpenGL_app PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options  (FOpenGL_app PRIVATE -fsanitize=address)
  endif()
endif()

# --- tests ---
if (WITH_TESTS)
  include(CTest)
  find_package(GTest CONFIG QUIET)
  if (NOT TARGET GTest::gtest)
    message(STATUS "GTest via package manager not found. Using FetchContent.")
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(FOpenGL_tests tests/test_time.cpp)
  target_include_directories(FOpenGL_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(FOpenGL_tests PRIVATE FOpenGL glm::glm)

  if (TARGET GTest::gtest_main)
    target_link_libraries(FOpenGL_tests PRIVATE GTest::gtest GTest::gtest_main)
  else()
    target_link_libraries(FOpenGL_tests PRIVATE gtest gtest_main)
  endif()

  include(GoogleTest OPTIONAL RESULT_VARIABLE _has_gtest_mod)
  if (_has_gtest_mod)
    gtest_discover_tests(FOpenGL_tests)
  else()
    add_test(NAME FOpenGL_tests COMMAND FOpenGL_tests)
  endif()
endif()
