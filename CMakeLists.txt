cmake_minimum_required(VERSION 3.16)
project(tuxracer VERSION 0.61 LANGUAGES C CXX)
if(WIN32)
  enable_language(RC)
endif()

# Dependencies are built statically by scripts/build-deps.sh into
# deps-prefix/<platform>; pass it with -DCMAKE_PREFIX_PATH=...
find_package(SDL2 REQUIRED CONFIG)
foreach(p ${CMAKE_PREFIX_PATH})
  list(APPEND CMAKE_MODULE_PATH "${p}/lib/cmake/SDL2_mixer")
endforeach()
find_package(SDL2_mixer REQUIRED CONFIG)

set(TCL_PREFIX "" CACHE PATH "Prefix where static Tcl 8.6 is installed")
find_path(TCL_INCLUDE_DIR tcl.h HINTS "${TCL_PREFIX}/include" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_library(TCL_LIBRARY NAMES tcl8.6 tcl86s tcl86
             HINTS "${TCL_PREFIX}/lib" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(NOT TCL_INCLUDE_DIR OR NOT TCL_LIBRARY)
  message(FATAL_ERROR "Static Tcl not found in TCL_PREFIX=${TCL_PREFIX}")
endif()

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)

file(GLOB TR_SOURCES src/*.c src/*.cpp)
if(WIN32)
  add_executable(tuxracer WIN32 ${TR_SOURCES} packaging/windows/tuxracer.rc)
elseif(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE tuxracer.icns)
  set(ICON packaging/macos/tuxracer.icns)
  set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  add_executable(tuxracer MACOSX_BUNDLE ${TR_SOURCES} ${ICON})
  set_target_properties(tuxracer PROPERTIES
    OUTPUT_NAME "Tux Racer"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "Tux Racer"
    MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.tuxracer"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.61"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.61")
else()
  add_executable(tuxracer ${TR_SOURCES})
endif()

target_include_directories(tuxracer PRIVATE src ${TCL_INCLUDE_DIR})
target_compile_definitions(tuxracer PRIVATE
  VERSION="0.61-sdl2" PACKAGE="tuxracer"
  HAVE_SDL=1 HAVE_SDL_MIXER=1 HAVE_SDL_JOYSTICKOPEN=1
  "$<$<COMPILE_LANGUAGE:C,CXX>:TCL_HEADER=<tcl.h$<ANGLE-R>>" STATIC_BUILD=1
  HAVE_ISNAN=1 HAVE_GETCWD=1 HAVE_STRDUP=1 HAVE_UNISTD_H=1
  GL_SILENCE_DEPRECATION=1)
if(NOT WIN32)
  target_compile_definitions(tuxracer PRIVATE HAVE_GETTIMEOFDAY=1 HAVE_SYS_TIME_H=1)
endif()

# 2001-era C: keep it compiling with today's compilers
set(OLD_C_FLAGS -fcommon -fno-strict-aliasing -w)
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  list(APPEND OLD_C_FLAGS -Wno-error=implicit-function-declaration
       -Wno-error=incompatible-function-pointer-types -Wno-error=int-conversion)
else()
  list(APPEND OLD_C_FLAGS -fpermissive)
endif()
target_compile_options(tuxracer PRIVATE
  $<$<COMPILE_LANGUAGE:C>:-std=gnu89 ${OLD_C_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++98 -fno-strict-aliasing -w>)

if(TARGET SDL2::SDL2main)
  target_link_libraries(tuxracer PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(tuxracer PRIVATE SDL2_mixer::SDL2_mixer-static SDL2::SDL2-static
                      ${TCL_LIBRARY} OpenGL::GLU OpenGL::GL)

if(WIN32)
  target_link_libraries(tuxracer PRIVATE netapi32 userenv ws2_32 shlwapi)
  target_link_options(tuxracer PRIVATE -static -static-libgcc -static-libstdc++)
elseif(APPLE)
  target_link_libraries(tuxracer PRIVATE z "-framework CoreFoundation")
else()
  target_link_libraries(tuxracer PRIVATE z pthread dl m)
  target_link_options(tuxracer PRIVATE -static-libgcc -static-libstdc++)
endif()

option(TR_AUTOTEST "Test build: automatic key presses and screenshots" OFF)
if(TR_AUTOTEST)
  target_compile_definitions(tuxracer PRIVATE TR_AUTOTEST=1)
endif()

# Game data next to the executable (inside the bundle on macOS)
if(APPLE)
  add_custom_command(TARGET tuxracer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data
            "$<TARGET_BUNDLE_CONTENT_DIR:tuxracer>/Resources/data")
else()
  add_custom_command(TARGET tuxracer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data
            "$<TARGET_FILE_DIR:tuxracer>/data")
endif()
