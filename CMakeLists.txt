cmake_minimum_required(VERSION 2.8)
project(sauron)

# Dir with the found modules for the libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" )

# Removing some warnings on Visual Studio
if (MSVC)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS)
	add_definitions(/D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories( ${PROJECT_SOURCE_DIR}/src )

add_executable( eye WIN32
	src/eye/main.cpp
	src/eye/resource.h
	src/eye/eye.rc
)
