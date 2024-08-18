if( ${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR} )
   message( FATAL_ERROR "Build must be in a shadow directory, such as ${PROJECT_SOURCE_DIR}/build." )
endif()

# Define project specific name, description, and version
set(PROJ testutil)
set(PROJ_DESCRIPTION "Test Utility")
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
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/KanoopCommon/include)
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/liblog/include)
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/libtesscommon/include)
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/libtessdiag/include)
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/dbc/include)

target_include_directories(${PROJ} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJ})

# Additional Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core SerialBus Mqtt Sql Xml)
target_link_libraries(${PROJ} PRIVATE Qt6::Core Qt6::SerialBus Qt6::Mqtt Qt6::Sql Qt6::Xml)
target_link_libraries(${PROJ} PRIVATE tesscommon log KanoopCommon sdv dbc tessdiag ssl crypto)
