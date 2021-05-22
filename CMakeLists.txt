cmake_minimum_required(VERSION 3.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# VFX Plarform requirement when using GCC 6.3.x
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# Silence deprecation warning (Gaffer uses this)
add_definitions(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING=1)

# If in debug mode, enable this flag so that you don't get memory corruption
add_compile_options("$<$<CONFIG:DEBUG>:-DWITH_CYCLES_DEBUG=1>")

# Cycles Standalone
set( WITH_CYCLES_STANDALONE_GUI  OFF CACHE INTERNAL "" )
set( WITH_CYCLES_OSL             ON CACHE INTERNAL "" )
set( OPENSUBDIV_ROOT_DIR         ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "" )
set( EMBREE_ROOT_DIR             ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "" )
set( BOOST_ROOT                  ${GAFFER_ROOT} CACHE INTERNAL "" )
#set( LLVM_STATIC                 ON CACHE INTERNAL "" )
#set( LLVM_ROOT_DIR               ${GAFFER_DEPENDENCIES_ROOT} CACHE INTERNAL "" )
set( OPENCOLORIO_ROOT_DIR        ${GAFFER_ROOT} CACHE INTERNAL "" )
set( OPENIMAGEIO_ROOT_DIR        ${GAFFER_ROOT} CACHE INTERNAL "" )
set( OSL_ROOT_DIR                ${GAFFER_ROOT} CACHE INTERNAL "" )
set( OPENEXR_ROOT_DIR            ${GAFFER_ROOT} CACHE INTERNAL "" )
set( JPEG_ROOT_DIR               ${GAFFER_ROOT} CACHE INTERNAL "" )
#set( WITH_CYCLES_CUDA_BINARIES   ON CACHE INTERNAL "" )
set( WITH_OPENCOLORIO            ON CACHE INTERNAL "" )
set( OCIO_PATH                   ${GAFFER_ROOT} CACHE INTERNAL "" )
set( GLEW_ROOT_DIR               ${GAFFER_ROOT} CACHE INTERNAL "" )
set( WITH_OPENVDB                ON CACHE INTERNAL "" )
set( BLOSC_ROOT_DIR              ${GAFFER_ROOT} CACHE INTERNAL "" )
set( OPENVDB_ROOT_DIR            ${GAFFER_ROOT} CACHE INTERNAL "" )
set( TBB_ROOT_DIR                ${GAFFER_ROOT} CACHE INTERNAL "" )
if (MSVC)
    set( ZLIB_ROOT ${GAFFER_ROOT} CACHE INTERNAL "" )
    set( ZLIB_INCLUDE_DIRS ${GAFFER_ROOT}/include CACHE INTERNAL "" )
    set( ZLIB_LIBRARIES ${GAFFER_ROOT}/lib/zlib.lib CACHE INTERNAL "" )
    set( ZLIB_INCLUDE_DIR ${GAFFER_ROOT}/include CACHE INTERNAL "" )
    set( ZLIB_LIBRARY ${GAFFER_ROOT}/lib/zlib.lib CACHE INTERNAL "" )
    set( ZLIB_DIR ${GAFFER_ROOT} CACHE INTERNAL "" )
    set( PNG_LIBRARIES ${GAFFER_ROOT}/lib/libpng.lib CACHE INTERNAL "" )
    set( JPEG_LIBRARIES ${GAFFER_ROOT}/lib/libjpeg.lib CACHE INTERNAL "" )
    set( TIFF_LIBRARY ${GAFFER_ROOT}/lib/libtiff.lib CACHE INTERNAL "" )
endif()
add_subdirectory( cycles )

# Cycles Standalone source/headers
set(CYCLES_LIBRARIES
    cycles_bvh
    cycles_device
    cycles_graph
    cycles_kernel_osl
    cycles_kernel
    cycles_render
    cycles_subd
    cycles_util
    extern_clew
    extern_cuew
    extern_numaapi
    )

set(CYCLES_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/cycles/src
    ${CMAKE_SOURCE_DIR}/cycles/third_party/atomic
    )

project(GafferCycles)

set( CORTEX_LOCATION ${GAFFER_ROOT} CACHE INTERNAL "" )
set( OCIO_PATH ${GAFFER_ROOT} CACHE INTERNAL "" )

list(APPEND CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cycles/src/cmake/Modules"
    "${PROJECT_SOURCE_DIR}/cmake/Modules"
)

find_package(Cortex REQUIRED)
find_package(OpenEXR REQUIRED Imath)
find_package(OSL REQUIRED)
find_package(OpenColorIO REQUIRED)
find_package(OpenImageIO REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)
include_directories( ${Boost_INCLUDE_DIRS} )

# Cycles defines
add_definitions(
    -DCCL_NAMESPACE_BEGIN=namespace\ ccl\ {
    -DCCL_NAMESPACE_END=}
#	-DCCL_NAMESPACE_BEGIN=namespace\ IECoreCycles\ {\ namespace\ ccl\ {
#	-DCCL_NAMESPACE_END=}\}
    -DWITH_OSL=1
    -DWITH_OPENCL=1
    -DWITH_MULTI=1
)

if( MSVC )
    set(CMAKE_CXX_FLAGS "/nologo /J /Gd /EHsc /MP" CACHE STRING "MSVC MD C++ flags " FORCE)
    set(CMAKE_C_FLAGS   "/nologo /J /Gd /MP"       CACHE STRING "MSVC MD C++ flags " FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /RTC1 /MDd /Zi /MP" CACHE STRING "MSVC MD flags " FORCE)
    add_definitions( -DOPENEXR_DLL -DNOMINMAX -D__PRETTY_FUNCTION__=__FUNCSIG__ -DBOOST_ALL_DYN_LINK )
endif()

if( WITH_CYCLES_DEVICE_CUDA )
    add_definitions( -DWITH_CYCLES_CUDA_BINARIES=1 )
endif()

if( WITH_CYCLES_LOGGING )
    find_package(Gflags "2.2.2" REQUIRED)
    find_package(Glog "0.4.0" REQUIRED)
    add_definitions( -DWITH_CYCLES_LOGGING=1 )
endif()

if( WITH_CYCLES_OPENSUBDIV )
    find_package(OpenSubdiv REQUIRED)
    add_definitions( -DWITH_OPENSUBDIV=1 )
endif()

if( WITH_CYCLES_EMBREE )
    find_package(Embree REQUIRED)
    add_definitions( -DWITH_EMBREE=1 )
endif()

if( WITH_CYCLES_DEVICE_OPTIX )
    add_definitions( -DWITH_OPTIX=1 -DWITH_CYCLES_CUDA_BINARIES=1 )
endif()

if( WITH_CYCLES_TEXTURE_CACHE )
    add_definitions( -DWITH_CYCLES_TEXTURE_CACHE=1 )
endif()

find_package(OpenVDB REQUIRED)
add_definitions( -DWITH_OPENVDB=1 -DWITH_OPENVDB_BLOSC=1 )

if( WITH_CYCLES_LIGHTGROUPS )
    add_definitions( -DWITH_CYCLES_LIGHTGROUPS=1 )
endif()

if( WITH_OPENIMAGEDENOISE )
    add_definitions( -DWITH_OPENIMAGEDENOISE=1 )
endif()

set(CMAKE_CXX_STANDARD 11)

link_directories( ${GAFFER_ROOT}/lib )

set( IECORECYCLES_INCLUDE_DIR include/GafferCycles/IECoreCyclesPreview )
set( IECORECYCLES_SRC_DIR src/GafferCycles/IECoreCyclesPreview )

# IECoreCycles
set(IECORECYCLES_CXX_FILES
    ${IECORECYCLES_SRC_DIR}/AttributeAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/CameraAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/CurvesAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/IECoreCycles.cpp
    ${IECORECYCLES_SRC_DIR}/MeshAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/ObjectAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/ParticleAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/ShaderNetworkAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/SocketAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/Renderer.cpp
    ${IECORECYCLES_SRC_DIR}/VDBAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/Mikktspace/mikktspace.c
    ${IECORECYCLES_SRC_DIR}/Mikktspace/mikktspace.h
    )
set(IECORECYCLES_H_FILES
    ${IECORECYCLES_INCLUDE_DIR}/AttributeAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/CameraAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/CurvesAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/Export.h
    ${IECORECYCLES_INCLUDE_DIR}/IECoreCycles.h
    ${IECORECYCLES_INCLUDE_DIR}/MeshAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/ObjectAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/ParticleAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/ShaderNetworkAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/SocketAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/VDBAlgo.h
    )
# GafferCycles
set(GAFFERCYCLES_CXX_FILES
    src/GafferCycles/CyclesAttributes.cpp
    src/GafferCycles/CyclesBackground.cpp
    src/GafferCycles/CyclesLight.cpp
    src/GafferCycles/CyclesMeshLight.cpp
    src/GafferCycles/CyclesOptions.cpp
    src/GafferCycles/CyclesRender.cpp
    src/GafferCycles/CyclesShader.cpp
    src/GafferCycles/InteractiveCyclesRender.cpp
    src/GafferCycles/SocketHandler.cpp
    )
set(GAFFERCYCLES_H_FILES
    include/GafferCycles/CyclesAttributes.h
    include/GafferCycles/CyclesBackground.h
    include/GafferCycles/CyclesLight.h
    include/GafferCycles/CyclesMeshLight.h
    include/GafferCycles/CyclesOptions.h
    include/GafferCycles/CyclesRender.h
    include/GafferCycles/CyclesShader.h
    include/GafferCycles/Export.h
    include/GafferCycles/InteractiveCyclesRender.h
    include/GafferCycles/SocketHandler.h
    include/GafferCycles/TypeIds.h
    )

add_library(GafferCycles SHARED 
    ${IECORECYCLES_CXX_FILES} 
    ${IECORECYCLES_H_FILES} 
    ${GAFFERCYCLES_CXX_FILES} 
    ${GAFFERCYCLES_H_FILES}
    )
target_include_directories(GafferCycles PRIVATE 
    include 
    ${GAFFER_DEPENDENCIES_ROOT}/include 
    ${GAFFER_ROOT}/include 
    ${CYCLES_INCLUDE_DIR} 
    ${OPENSUBDIV_INCLUDE_DIR}
    )
target_link_libraries(GafferCycles 
    ${OPENEXR_IEX_LIBRARY}
    ${OPENEXR_IMATH_LIBRARY}
    ${OCIO_LIBRARIES}
    ${OPENIMAGEIO_LIBRARIES}
    ${OSL_LIBRARIES}
    ${Cortex_IECORE_LIBRARY} 
    ${Cortex_IECORESCENE_LIBRARY} 
    ${Cortex_IECOREIMAGE_LIBRARY}
    Gaffer
    GafferOSL
    GafferScene
    GafferDispatch
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${CYCLES_LIBRARIES} 
    ${OPENSUBDIV_LIBRARIES}
    ${EMBREE_LIBRARIES}
    ${OPENVDB_LIBRARIES}
    ${GLOG_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    )
add_dependencies( GafferCycles cycles )

install(TARGETS GafferCycles DESTINATION lib)
install(FILES ${GAFFERCYCLES_H_FILES}
        DESTINATION include/GafferCycles)
install(FILES ${IECORECYCLES_H_FILES}
        DESTINATION include/GafferCycles/IECoreCyclesPreview)

# GafferCyclesModule
add_library(GafferCyclesModule MODULE 
    src/GafferCyclesModule/GafferCyclesModule.cpp
    )
set_target_properties(GafferCyclesModule PROPERTIES PREFIX "" OUTPUT_NAME "_GafferCycles" )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
    set_target_properties( GafferCyclesModule PROPERTIES SUFFIX ".so" )
elseif( "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" )
    set_target_properties( GafferCyclesModule PROPERTIES SUFFIX ".pyd" )
endif()
target_link_libraries(GafferCyclesModule 
    GafferCycles 
    ${Cortex_IECOREPYTHON_LIBRARY}
    ${CYCLES_LIBRARIES}
    ${GLOG_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    )
string(REPLACE "2" "python2.7" PYTHON_FOLDER ${PYTHON_VARIANT})
string(REPLACE "3" "python3.7m" PYTHON_FOLDER ${PYTHON_FOLDER})
target_include_directories(GafferCyclesModule PRIVATE 
    include ${GAFFER_ROOT}/include 
    ${GAFFER_ROOT}/include/${PYTHON_FOLDER}
    ${CYCLES_INCLUDE_DIR}
    ${EMBREE_INCLUDE_DIRS}
    )
add_dependencies( GafferCyclesModule GafferCycles )

install(TARGETS GafferCyclesModule DESTINATION python/GafferCycles)

file(GLOB GAFFERCYCLES_PY_FILES python/GafferCycles/*.py)

install(FILES ${GAFFERCYCLES_PY_FILES}
        DESTINATION python/GafferCycles)

# GafferCyclesUI
file(GLOB GAFFERCYCLES_UI_PY_FILES python/GafferCyclesUI/*.py)

install(FILES ${GAFFERCYCLES_UI_PY_FILES}
        DESTINATION python/GafferCyclesUI)

# Startup
install(DIRECTORY startup DESTINATION .)
