cmake_minimum_required(VERSION 3.1)

include(CheckFunctionExists)

project(mdstool C CXX ASM)

find_package(PkgConfig)
find_package(Threads)
find_package(X11)

file(GLOB SOURCES "source/*.c")

if (${PKG_CONFIG_FOUND})
	pkg_check_modules(FREETYPE freetype2)
endif()

if (${X11_FOUND})
	file(GLOB FILES "source/x11/*.c")
	list(APPEND SOURCES ${FILES})
endif()

add_definitions(-DNK_INCLUDE_DEFAULT_ALLOCATOR)
add_definitions(-DNK_INCLUDE_COMMAND_USERDATA)


if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	file(GLOB FILES "source/x86/gcc/*.c")
	list(APPEND SOURCES ${FILES})
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DUNICODE)
	file(GLOB FILES "source/x86/msvc/*.c")
	list(APPEND SOURCES ${FILES})
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	file(GLOB FILES "source/msw/*.c")
	list(APPEND SOURCES ${FILES})
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	file(GLOB FILES "source/posix/*.c")
	list(APPEND SOURCES ${FILES})
	file(GLOB FILES "source/unix/*.c")
	list(APPEND SOURCES ${FILES})
	file(GLOB FILES "source/linux/*.c")
	list(APPEND SOURCES ${FILES})
endif()

file(GLOB FILES "source/x86/*.c")
list(APPEND SOURCES ${FILES})

check_function_exists(asprintf HAVE_ASPRINTF)
check_function_exists(clock_nanosleep HAVE_CLOCK_NANOSLEEP)
check_function_exists(nanosleep HAVE_NANOSLEEP)
check_function_exists(usleep HAVE_USLEEP)

if(HAVE_CLOCK_NANOSLEEP)
	add_definitions(-DHAVE_CLOCK_NANOSLEEP)
endif()

if (NOT HAVE_ASPRINTF)
	list(APPEND SOURCES "source/compat/asprintf.c")
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	add_definitions(-D_WIN32_WINNT=0x601)
	add_definitions(-DUNICODE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
else ()
	add_definitions(-D_GNU_SOURCE)
endif()

add_executable(mdstool ${SOURCES})

include_directories(nuklear)
include_directories(stb)
include_directories(include)
include_directories(${X11_INCLUDE_DIR} ${X11_Xft_INCLUDE_PATH} ${X11_Xrender_INCLUDE_PATH} ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(mdstool ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${X11_Xft_LIB} ${X11_Xrender_LIB} ${FREETYPE_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	target_link_libraries(mdstool gdiplus ntdll shlwapi)
else()
	target_link_libraries(mdstool m)
endif()
