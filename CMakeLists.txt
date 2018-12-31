cmake_minimum_required(VERSION 3.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# VFX Plarform requirement when using GCC 6.3.x
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# Silence deprecation warning (Gaffer uses this)
add_definitions(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING=1)

# Cycles Standalone
set( WITH_CYCLES_STANDALONE_GUI  OFF CACHE INTERNAL "" )
set( WITH_CYCLES_OSL             ON CACHE INTERNAL "" )
#set( WITH_CYCLES_LOGGING         ON CACHE INTERNAL "" )
set( WITH_CYCLES_DEBUG           ON CACHE INTERNAL "" )
#set( WITH_CYCLES_OPENSUBDIV      OFF CACHE INTERNAL "" )
#set( OPENSUBDIV_ROOT_DIR         ${GAFFER_ROOT} CACHE INTERNAL "" )
set( BOOST_ROOT                  ${GAFFER_ROOT} CACHE INTERNAL "" )
#set( LLVM_STATIC                 ON CACHE INTERNAL "" )
#set( LLVM_ROOT_DIR               ${GAFFER_DEPENDENCIES_ROOT} CACHE INTERNAL "" )
set( OPENIMAGEIO_ROOT_DIR        ${GAFFER_ROOT} CACHE INTERNAL "" )
set( OSL_ROOT_DIR                ${GAFFER_ROOT} CACHE INTERNAL "" )
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
    )

set(CYCLES_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/cycles/src
    ${CMAKE_SOURCE_DIR}/cycles/third_party/atomic
    )

project(GafferCycles)

set( CORTEX_LOCATION ${GAFFER_ROOT} CACHE INTERNAL "" )

list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/cycles/src/cmake/Modules"
    "${PROJECT_SOURCE_DIR}/cmake/Modules"
)
find_package(OpenSubdiv REQUIRED)
find_package(Cortex REQUIRED)

# Cycles defines
add_definitions(
	-DCCL_NAMESPACE_BEGIN=namespace\ ccl\ {
	-DCCL_NAMESPACE_END=}
    -DWITH_OSL=1
)

set(CMAKE_CXX_STANDARD 11)

link_directories( ${GAFFER_ROOT}/lib )

set( IECORECYCLES_INCLUDE_DIR include/GafferCycles/IECoreCyclesPreview )
set( IECORECYCLES_SRC_DIR src/GafferCycles/IECoreCyclesPreview )

# IECoreCycles
set(IECORECYCLES_CXX_FILES
    ${IECORECYCLES_SRC_DIR}/AttributeAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/CameraAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/CurvesAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/MeshAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/ObjectAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/ShaderNetworkAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/SocketAlgo.cpp
    ${IECORECYCLES_SRC_DIR}/Renderer.cpp
    )
set(IECORECYCLES_H_FILES
    ${IECORECYCLES_INCLUDE_DIR}/AttributeAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/CameraAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/CurvesAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/Export.h
    ${IECORECYCLES_INCLUDE_DIR}/MeshAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/ObjectAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/ShaderNetworkAlgo.h
    ${IECORECYCLES_INCLUDE_DIR}/SocketAlgo.h
    )
# GafferCycles
set(GAFFERCYCLES_CXX_FILES
    src/GafferCycles/CyclesAttributes.cpp
    src/GafferCycles/CyclesLight.cpp
    src/GafferCycles/CyclesOptions.cpp
    src/GafferCycles/CyclesRender.cpp
    src/GafferCycles/CyclesShader.cpp
    src/GafferCycles/InteractiveCyclesRender.cpp
    src/GafferCycles/SocketHandler.cpp
    )
set(GAFFERCYCLES_H_FILES
    include/GafferCycles/CyclesAttributes.h
    include/GafferCycles/CyclesLight.h
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
    ${Cortex_IECORE_LIBRARY} 
    ${Cortex_IECORESCENE_LIBRARY} 
    ${CYCLES_LIBRARIES} 
    ${OPENSUBDIV_LIBRARIES}
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
target_link_libraries(GafferCyclesModule 
    GafferCycles 
    ${CYCLES_LIBRARIES}
    )
target_include_directories(GafferCyclesModule PRIVATE 
    include ${GAFFER_ROOT}/include 
    ${GAFFER_ROOT}/include/python2.7 
    ${CYCLES_INCLUDE_DIR}
    )
add_dependencies( GafferCyclesModule GafferCycles )

install(TARGETS GafferCyclesModule DESTINATION python/GafferCycles)

file(GLOB GAFFERCYCLES_PY_FILES python/GafferCycles/*.py)

install(FILES ${GAFFERCYCLES_PY_FILES}
        DESTINATION python/GafferCycles)

# GafferCyclesUI
file(GLOB GAFFERCYCLES_UI_FILES python/GafferCyclesUI/*.py)

install(FILES ${GAFFERCYCLES_UI_FILES}
        DESTINATION python/GafferCyclesUI)
