cmake_minimum_required(VERSION 3.15)


# Read version from the package.xml file.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/package.xml package_xml_str)
if(NOT package_xml_str MATCHES "<version>([0-9]+.[0-9]+.[0-9]+)</version>")
  message(FATAL_ERROR "Could not parse project version from package.xml. Aborting.")
endif()


# At this point we either have a proper version string, or we've errored
# out with a FATAL_ERROR above. So assume CMAKE_MATCH_1 contains our
# package's version.
project(abb_libegm VERSION ${CMAKE_MATCH_1} LANGUAGES CXX)
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

include(GNUInstallDirs)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

#########################
## Boost C++ Libraries ##
#########################
find_package(Boost REQUIRED COMPONENTS regex system thread)

#############################
## Google Protocol Buffers ##
#############################

# Temporary workaround for https://github.com/ms-iot/ROSOnWindows/issues/218
if(WIN32)
  foreach(prefix IN ITEMS $ENV{CMAKE_PREFIX_PATH})
    if(${prefix} STREQUAL "C:/opt/rosdeps/x64")
      list(APPEND CMAKE_PROGRAM_PATH "C:/opt/rosdeps/x64/tools/protobuf")
    endif()
  endforeach()
endif()

find_package(Protobuf REQUIRED)

# Make sure protoc is present, as apparently the above find_package() doesn't check that.
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Cannot find required 'protoc', cannot process Protobuf files without it. Aborting.")
endif()

# Generate C++ for protocol classes (headers and sources # get written to the CMAKE_CURRENT_BINARY_DIR location).
set(EgmProtoFiles proto/egm.proto proto/egm_wrapper.proto proto/egm_wrapper_trajectory.proto)
if(NOT QUIET)
  message(STATUS "Generating protobuf C++ for: ${EgmProtoFiles}")
  message(STATUS "Protobuf output directory: ${CMAKE_CURRENT_BINARY_DIR}")
endif()
if(MSVC)
  # Add export macro when using Microsoft Visual C++ compiler.
  protobuf_generate_cpp(EgmProtoSources EgmProtoHeaders EXPORT_MACRO ABB_LIBEGM_EXPORT ${EgmProtoFiles})
else()
  protobuf_generate_cpp(EgmProtoSources EgmProtoHeaders ${EgmProtoFiles})
  message(STATUS "Generating protobuf without MSVC at : " ${EgmProtoSources} ${EgmProtoHeaders})
endif()


#############
## Threads ##
#############
find_package(Threads REQUIRED)

# Work around Protobuf exporting 'lpthread' as a library: we let the
# previous find_package(...) determine the system's thread library.
list(REMOVE_ITEM PROTOBUF_LIBRARIES "-lpthread")

###########
## Build ##
###########
option(BUILD_SHARED_LIBS "Build dynamically-linked binaries" ON)
option(USE_PYTHON "Build the Python interface" ON)

set(
  SRC_FILES
    src/egm_base_interface.cpp
    src/egm_common.cpp
    src/egm_common_auxiliary.cpp
    src/egm_controller_interface.cpp
    src/egm_interpolator.cpp
    src/egm_logger.cpp
    src/egm_udp_server.cpp
    src/egm_trajectory_interface.cpp
    ${EgmProtoSources}
)

#if (USE_PYTHON)
#  add_subdirectory(python)
# endif()

add_library(${PROJECT_NAME} ${SRC_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}>"
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
  ${PROTOBUF_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  Boost::regex
  Boost::system
  Boost::thread
  ${PROTOBUF_LIBRARIES}
  Threads::Threads
)

#if(NOT BUILD_SHARED_LIBS)
#  target_compile_definitions(${PROJECT_NAME} PUBLIC "ABB_LIBEGM_STATIC_DEFINE")
#endif()

#if(MSVC)
#  # Force include the export header when using Microsoft Visual C++ compiler.
#  target_compile_options(${PROJECT_NAME} PUBLIC "/FI${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h")
#endif()

# Python bindings
if(SKBUILD OR USE_PYTHON)
    # Python module
    python_add_library(_abb_libegm MODULE 
        src/bindings.cpp
        src/controller.cpp
        src/common.cpp
        src/udp.cpp
        WITH_SOABI
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
    )

    set_target_properties(_abb_libegm PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )

    # Include directories for Python module
    target_include_directories(_abb_libegm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROTOBUF_INCLUDE_DIRS}
    )

    # Link libraries for Python module
    target_link_libraries(_abb_libegm PRIVATE
        ${PROJECT_NAME}
        pybind11::headers
    )

    # Add version info
    target_compile_definitions(_abb_libegm PRIVATE VERSION_INFO=${PROJECT_VERSION})

    # Installation
    install(TARGETS _abb_libegm ${PROJECT_NAME}
    LIBRARY DESTINATION ${SKBUILD_PLATLIB_DIR}/abb_libegm
    )

    # Install Python package files
    install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/src/abb_libegm/__init__.py
          DESTINATION ${SKBUILD_PLATLIB_DIR}/abb_libegm
    )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/abb_libegm/
        DESTINATION ${SKBUILD_PLATLIB_DIR}/abb_libegm
        FILES_MATCHING PATTERN "*.py"
    )
    
endif()