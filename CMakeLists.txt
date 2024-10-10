if( ${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR} )
   message( FATAL_ERROR "Build must be in a shadow directory, such as ${PROJECT_SOURCE_DIR}/build." )
endif()

# Define project specific name, description, and version
set(PROJ Git2QtTests)
set(PROJ_DESCRIPTION "Tests and examples for libgit2qt")
set(PROJ_VERSION 1.0.1)

# Enable Qt code generation
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project($PROJ LANGUAGES CXX)

file(GLOB_RECURSE PROJ_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

qt_add_executable(
    ${PROJ} ${PROJ_SOURCES}
)

get_directory_property(PARENT_DIR PARENT_DIRECTORY)
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/libgit2qt/include)

target_include_directories(${PROJ} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJ})

# Additional Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core)
target_link_libraries(${PROJ} PRIVATE Qt6::Core)
target_link_libraries(${PROJ} PRIVATE git2qt KanoopCommon)
