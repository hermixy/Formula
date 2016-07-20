project(formula)
cmake_minimum_required(VERSION 2.8)

# Debug setting
#ADD_DEFINITIONS("-DDEBUG")

# Checking C++11 Support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Setting compiler flags for MSVC to generate program of release version
if (MSVC)
	# To link MSVC libraries statically, uncomment below instructions
	# set(CompilerFlags
	# CMAKE_CXX_FLAGS
	# CMAKE_CXX_FLAGS_DEBUG
	# CMAKE_CXX_FLAGS_RELEASE
	# CMAKE_C_FLAGS
	# CMAKE_C_FLAGS_DEBUG
	# CMAKE_C_FLAGS_RELEASE
	# )
	# foreach(CompilerFlag ${CompilerFlags})
	# string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	# endforeach()

	# Link MSVC runtime libraries(dll)
	set(CMAKE_C_FLAGS_DEBUG   "   /MD /EHsc")
	set(CMAKE_CXX_FLAGS_DEBUG " /MD /EHsc")
	set(CMAKE_C_FLAGS_RELEASE   "/MD /EHsc")
	set(CMAKE_CXX_FLAGS_RELEASE " /MD /EHsc")
	set(CMAKE_C_FLAGS   "/MD /EHsc")
	set(CMAKE_CXX_FLAGS " /MD /EHsc")
endif(MSVC)

# Generating lexer and parser using flex & bison
find_package(BISON)
find_package(FLEX)

if(PARSERPATH)
else(PARSERPATH)
	set(PARSERPATH frontend)
endif(PARSERPATH)

bison_target(Parser ${PARSERPATH}/parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp)
flex_target(Lexer ${PARSERPATH}/lexer.l  ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp)
add_flex_bison_dependency(Lexer Parser)

#message("---------------------------------")
#message(${CMAKE_CURRENT_SOURCE_DIR})
#message(${BISON_Parser_OUTPUTS})
#message(${FLEX_Lexer_OUTPUTS})
#message("---------------------------------")

include_directories(. frontend backend)

# Building CLI interpreter 
aux_source_directory(. SOURCES)
add_executable(formula
	${BISON_Parser_OUTPUTS}
	${FLEX_Lexer_OUTPUTS}
	backend/Operand.cpp
	backend/Function.cpp
	backend/Code.cpp
	backend/VM.cpp
	${SOURCES})
