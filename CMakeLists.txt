cmake_minimum_required(VERSION 4.0)
project(FOpenGL LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

option(USE_VCPKG        "Prefer vcpkg packages when available" ON)
option(USE_SUBMODULES   "Use git submodules fallback when vcpkg is unavailable" ON)

option(WITH_IMGUI       "Build with Dear ImGui UI layer"    ON)
option(WITH_ASAN        "Enable AddressSanitizer"           OFF)
option(WITH_TESTS       "Build tests"                       OFF)
option(WITH_CLANG_TIDY  "Run clang-tidy"                    OFF)
option(WITH_MSVC_ANALYZE "Enable MSVC code analysis"        OFF)

# OpenGL loader/lib (needed on many platforms, and for ImGui OpenGL backend)
find_package(OpenGL REQUIRED)

# ---------------------------------------------
# Try vcpkg first (CONFIG packages)
# ---------------------------------------------
set(HAVE_VCPKG_DEPS OFF)

if (USE_VCPKG)
  find_package(glfw3  CONFIG QUIET)
  find_package(GLEW   CONFIG QUIET)
  find_package(glm    CONFIG QUIET)
  find_package(assimp CONFIG QUIET)

  # vcpkg can expose glfw as "glfw" or "glfw3::glfw" depending on port/version
  set(_HAS_GLFW OFF)
  if (TARGET glfw OR TARGET glfw3::glfw)
    set(_HAS_GLFW ON)
  endif()

  if (_HAS_GLFW AND TARGET GLEW::GLEW AND (TARGET glm::glm OR TARGET glm) AND (TARGET assimp::assimp OR TARGET assimp))
    set(HAVE_VCPKG_DEPS ON)
  endif()
endif()

# ---------------------------------------------
# Fallback: submodules / FetchContent
# ---------------------------------------------
if (NOT HAVE_VCPKG_DEPS)
  if (NOT USE_SUBMODULES)
    message(FATAL_ERROR "Dependencies not found via vcpkg and USE_SUBMODULES=OFF")
  endif()

  message(STATUS "Using submodules/FetchContent dependencies")

  # GLFW
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/CMakeLists.txt")
    add_subdirectory(extern/glfw)
  else()
    FetchContent_Declare(glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
  endif()

  # GLM (header-only)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/glm/CMakeLists.txt")
    add_subdirectory(extern/glm)
  else()
    FetchContent_Declare(glm
      GIT_REPOSITORY https://github.com/g-truc/glm.git
      GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)
  endif()

  # Assimp
  set(ASSIMP_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_INSTALL            OFF CACHE BOOL "" FORCE)

  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/assimp/CMakeLists.txt")
    add_subdirectory(extern/assimp)
  else()
    FetchContent_Declare(assimp
      GIT_REPOSITORY https://github.com/assimp/assimp.git
      GIT_TAG v5.4.3
    )
    FetchContent_MakeAvailable(assimp)
  endif()

  # GLEW
  # NOTE: upstream git repo does not necessarily contain generated headers.
  # Using release tarball guarantees include/GL/*.h exist.
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/glew/build/cmake/CMakeLists.txt"
      AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/glew/include/GL/wglew.h")
    set(ONLY_LIBS  ON CACHE BOOL "" FORCE)
    set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
    add_subdirectory(extern/glew/build/cmake)
  else()
    set(ONLY_LIBS   ON  CACHE BOOL "" FORCE)
    set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
    
    FetchContent_Declare(glew_src
      URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz
      SOURCE_SUBDIR build/cmake
    )
    
    FetchContent_MakeAvailable(glew_src)
  endif()
endif()

# ---------------------------------------------
# Resolve dependency targets (works for vcpkg and fallback)
# ---------------------------------------------
set(GLFW_TARGET "")
if (TARGET glfw)
  set(GLFW_TARGET glfw)
elseif (TARGET glfw3::glfw)
  set(GLFW_TARGET glfw3::glfw)
else()
  message(FATAL_ERROR "GLFW target not found")
endif()

set(GLM_TARGET "")
if (TARGET glm::glm)
  set(GLM_TARGET glm::glm)
elseif (TARGET glm)
  set(GLM_TARGET glm)
else()
  message(FATAL_ERROR "GLM target not found")
endif()

set(ASSIMP_TARGET "")
if (TARGET assimp::assimp)
  set(ASSIMP_TARGET assimp::assimp)
elseif (TARGET assimp)
  set(ASSIMP_TARGET assimp)
else()
  message(FATAL_ERROR "Assimp target not found")
endif()

set(GLEW_TARGET "")
if (TARGET GLEW::GLEW)
  set(GLEW_TARGET GLEW::GLEW)
elseif (TARGET glew_s)
  set(GLEW_TARGET glew_s)
elseif (TARGET libglew_static)
  set(GLEW_TARGET libglew_static)
elseif (TARGET glew)
  set(GLEW_TARGET glew)
else()
  message(FATAL_ERROR "GLEW target not found")
endif()

# If we ended up with a static GLEW target, consumers need GLEW_STATIC
if (GLEW_TARGET STREQUAL "glew_s" OR GLEW_TARGET STREQUAL "libglew_static")
  add_compile_definitions(GLEW_STATIC=1)
endif()

# ---------------------------------------------
# Main library
# ---------------------------------------------
add_library(FOpenGL STATIC
  src/Application/GLApplication.cpp
  src/Application/Time.cpp
  src/Window/GLWindow.cpp
  src/Rendering/Shader.cpp
  src/Rendering/Shadow/ShadowMap.cpp
  src/Rendering/Shadow/OmniShadowMap.cpp
  src/Texture/Texture.cpp
  src/Skybox/Skybox.cpp
  src/Model/Mesh.cpp
  src/Model/Model.cpp
  src/Camera/Camera.cpp
  src/Light/LightCommon.cpp
  src/Light/DirectionalLight.cpp
  src/Light/PointLight.cpp
  src/Light/SpotLight.cpp
  src/Materials/Material.cpp
  src/Materials/MaterialFactory.cpp
  src/ThirdParty/ImGuiHandler.cpp
  #src/Utils/AssetLoader.cpp
)

target_include_directories(FOpenGL
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(FOpenGL PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/glew/include"
)

# Headers for IDE convenience
file(GLOB_RECURSE FOPENGL_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)
target_sources(FOpenGL PRIVATE ${FOPENGL_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${FOPENGL_HEADERS})

# Shaders for IDE convenience
file(GLOB_RECURSE FOPENGL_SHADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom"
)
target_sources(FOpenGL PRIVATE ${FOPENGL_SHADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" PREFIX "Shader Files" FILES ${FOPENGL_SHADERS})

# Link core dependencies once (always)
target_link_libraries(FOpenGL
  PUBLIC
    ${GLFW_TARGET}
    ${GLM_TARGET}
    ${ASSIMP_TARGET}
    ${GLEW_TARGET}
    OpenGL::GL
)

# ---------------------------------------------
# ImGui (extern/imgui)
# ---------------------------------------------
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

    # Tell ImGui OpenGL backend which loader is used
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

    # Ensure imgui itself sees GLFW and GLEW headers via target usage requirements
    target_link_libraries(imgui PUBLIC ${GLFW_TARGET} ${GLEW_TARGET} OpenGL::GL)

    # Link imgui into the engine lib
    target_link_libraries(FOpenGL PUBLIC imgui)
    target_compile_definitions(FOpenGL PUBLIC USE_IMGUI=1)
  else()
    message(STATUS "ImGui not found in extern/imgui. Building without UI.")
  endif()
endif()

# ---------------------------------------------
# Tooling
# ---------------------------------------------
if (WITH_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy)
  if (CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
      ${CLANG_TIDY}
      -checks=modernize-*,bugprone-*,performance-*,readability-*,cppcoreguidelines-*
      -warnings-as-errors=modernize-*,bugprone-*,performance-*
    )
  endif()
endif()

if (MSVC AND WITH_MSVC_ANALYZE)
  target_compile_options(FOpenGL PRIVATE /analyze)
endif()

# ---------------------------------------------
# Executable
# ---------------------------------------------
add_executable(FOpenGL_app src/Application/main.cpp)
target_link_libraries(FOpenGL_app PRIVATE FOpenGL)

add_custom_command(TARGET FOpenGL_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:FOpenGL_app>/shaders"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/assets"  "$<TARGET_FILE_DIR:FOpenGL_app>/assets"
)

if (WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
  set(COPY_RUNTIME_DEPS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/copy_runtime_deps.cmake")
  file(WRITE "${COPY_RUNTIME_DEPS_SCRIPT}" [=[
# Copy runtime dependencies if any exist (safe when the list is empty)
if(NOT DEPS)
  message(STATUS "No runtime DLLs to copy.")
  return()
endif()

foreach(dll IN LISTS DEPS)
  if(EXISTS "${dll}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${dll}" "${DEST}")
  endif()
endforeach()
]=])

  add_custom_command(TARGET FOpenGL_app POST_BUILD
    COMMAND ${CMAKE_COMMAND}
      -DDEPS="$<TARGET_RUNTIME_DLLS:FOpenGL_app>"
      -DDEST="$<TARGET_FILE_DIR:FOpenGL_app>"
      -P "${COPY_RUNTIME_DEPS_SCRIPT}"
    COMMAND_EXPAND_LISTS
  )
endif()

# ---------------------------------------------
# AddressSanitizer
# ---------------------------------------------
if (WITH_ASAN)
  if (MSVC)
    target_compile_options(FOpenGL     PRIVATE /fsanitize=address /Zi)
    target_compile_options(FOpenGL_app PRIVATE /fsanitize=address /Zi)
    target_link_options(FOpenGL     PRIVATE /INCREMENTAL:NO)
    target_link_options(FOpenGL_app PRIVATE /INCREMENTAL:NO)

    # Optional manual copy of ASan runtime (may vary by VS install)
    if(DEFINED ENV{VCToolsInstallDir})
      set(ASAN_BIN "$ENV{VCToolsInstallDir}/bin/Hostx64/x64")
      add_custom_command(TARGET FOpenGL_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${ASAN_BIN}/clang_rt.asan_dynamic-x86_64.dll"
          "$<TARGET_FILE_DIR:FOpenGL_app>"
      )
    endif()
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(FOpenGL     PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options  (FOpenGL     PRIVATE -fsanitize=address)
    target_compile_options(FOpenGL_app PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options  (FOpenGL_app PRIVATE -fsanitize=address)
  endif()
endif()

# ---------------------------------------------
# Tests
# ---------------------------------------------
if (WITH_TESTS)
  include(CTest)

  find_package(GTest CONFIG QUIET)
  if (NOT TARGET GTest::gtest)
    message(STATUS "GTest via package manager not found. Using FetchContent.")
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(FOpenGL_tests tests/test_time.cpp)
  target_include_directories(FOpenGL_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(FOpenGL_tests PRIVATE FOpenGL ${GLM_TARGET})

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
