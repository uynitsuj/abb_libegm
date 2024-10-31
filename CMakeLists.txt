cmake_minimum_required(VERSION 3.15)


# Read version from the package.xml file.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/package.xml package_xml_str)
if(NOT package_xml_str MATCHES "<version>([0-9]+.[0-9]+.[0-9]+)</version>")
  message(FATAL_ERROR "Could not parse project version from package.xml. Aborting.")
endif()

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--disable-new-dtags")


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

# RPATH Configuration: Only use relative paths for portability
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

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

add_subdirectory(pybind11_protobuf)

add_library(${PROJECT_NAME} ${SRC_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

set(ABSL_ENABLE_INSTALL ON)

include(FetchContent)
FetchContent_Declare(
  absl
  GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
  GIT_TAG 20230125.3
  FIND_PACKAGE_ARGS ${_absl_package_args} NAMES absl)

FetchContent_MakeAvailable(absl)

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pybind11_protobuf>"
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
  ${PROTOBUF_INCLUDE_DIRS}
  ${absl_SOURCE_DIR}
  pybind11_native_proto_caster
)

target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--disable-new-dtags"
  Boost::regex
  Boost::system
  Boost::thread
  ${PROTOBUF_LIBRARIES}
  Threads::Threads
  absl::strings       
  absl::base          
  absl::flat_hash_map
  absl::flat_hash_set
  absl::hash
  absl::optional
  absl::city             
  absl::low_level_hash    
  absl::strings_internal  
  absl::raw_logging_internal 
)

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
      INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
      BUILD_RPATH "$ORIGIN:$ORIGIN/../lib"
      BUILD_WITH_INSTALL_RPATH TRUE
      INSTALL_RPATH_USE_LINK_PATH TRUE
    )

    set_target_properties(_abb_libegm PROPERTIES
        INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
        BUILD_RPATH "$ORIGIN:$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )

    # Include directories for Python module
    target_include_directories(_abb_libegm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/pybind11_protobuf
        pybind11_native_proto_caster
    )

    # Link libraries for Python module
    target_link_libraries(_abb_libegm PRIVATE "-Wl,--disable-new-dtags"
        ${PROJECT_NAME}
        pybind11::headers
        pybind11_native_proto_caster
        absl::strings       
        absl::base          
        absl::flat_hash_map
        absl::flat_hash_set
        absl::hash
        absl::optional
        absl::city             
        absl::low_level_hash    
        absl::strings_internal  
        absl::raw_logging_internal 
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