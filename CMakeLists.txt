# CMake file for NuriaTwig
# Source: https://github.com/NuriaProject/Twig
cmake_minimum_required(VERSION 2.8.8)

cmake_policy(SET CMP0020 NEW)

add_subdirectory(lemon)

# Base configuration
PROJECT(NuriaTwig)

# Enable moc
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Dependencies
FIND_PACKAGE(Qt5Core REQUIRED)

if (UNIX)
  set (LEMON_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/lemon/lemon)
elseif (WIN32)
  set (LEMON_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/lemon/lemon.exe)
endif()

# Generate twig.c and twig.h from twig.y
add_custom_target(generate_grammar ALL
                  COMMAND ${LEMON_EXECUTABLE} ../src/grammar/twig.y
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lemon
                  COMMENT "Parser generation")
add_dependencies(generate_grammar lemon)
                  
# 
SET(NuriaTwig_SRC
    src/private/astnodes.cpp
    src/private/astnodes.hpp
    src/private/parser.cpp
    src/private/parser.hpp
    src/private/compiler.cpp
    src/private/compiler.hpp
    src/private/builtins.cpp
    src/private/builtins.hpp
    src/private/tokenizer.cpp
    src/private/tokenizer.hpp
    src/private/templateengine_p.hpp
    src/private/variableaccessor.cpp
    src/private/variableaccessor.hpp
    src/nuria/twig_global.hpp
    src/memorytemplateloader.cpp
    src/nuria/memorytemplateloader.hpp
    src/filetemplateloader.cpp
    src/nuria/filetemplateloader.hpp
    src/templateengine.cpp
    src/nuria/templateengine.hpp
    src/templateloader.cpp
    src/nuria/templateloader.hpp
    src/templateerror.cpp
    src/nuria/templateerror.hpp
    src/templateprogram.cpp
    src/nuria/templateprogram.hpp
)

# Create build target
ADD_LIBRARY(NuriaTwig SHARED ${NuriaTwig_SRC})
target_link_libraries(NuriaTwig NuriaCore)
add_dependencies(NuriaTwig generate_grammar)
QT5_USE_MODULES(NuriaTwig Core)

# 
set_target_properties(
    NuriaTwig PROPERTIES
    VERSION ${NURIA_VERSION}
    SOVERSION ${NURIA_SOVERSION}
)

# Add public include directories to target
set_property(TARGET NuriaTwig APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

# Install target
INSTALL(TARGETS NuriaTwig EXPORT NuriaTwigConfig
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

INSTALL(DIRECTORY src/nuria/ DESTINATION include/nuria
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "private" EXCLUDE
        PATTERN "grammar" EXCLUDE
)

INSTALL(EXPORT NuriaTwigConfig DESTINATION lib/cmake/NuriaTwig)

# Tests
enable_testing()
add_unittest(NAME tst_memorytemplateloader NURIA NuriaTwig)
add_unittest(NAME tst_filetemplateloader NURIA NuriaTwig RESOURCES tests/tst_filetemplateloader_resources.qrc)
add_unittest(NAME tst_templateengine NURIA NuriaTwig RESOURCES tests/tst_templateengine_resources.qrc)
add_unittest(NAME tst_templateengine_caching NURIA NuriaTwig)
add_unittest(NAME tst_templateloader NURIA NuriaTwig)
add_unittest(NAME tst_templatetokenizer NURIA NuriaTwig)
add_unittest(NAME tst_builtins NURIA NuriaTwig)
add_unittest(NAME tst_variableaccessor NURIA NuriaTwig)
