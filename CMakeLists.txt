cmake_minimum_required(VERSION 3.0)

# Cycles Standalone
set( WITH_CYCLES_STANDALONE_GUI  OFF CACHE INTERNAL "" )
set( WITH_CYCLES_OSL             ON CACHE INTERNAL "" )
#set( WITH_CYCLES_LOGGING         ON CACHE INTERNAL "" )
set( WITH_CYCLES_DEBUG           ON CACHE INTERNAL "" )
set( WITH_CYCLES_OPENSUBDIV      ON CACHE INTERNAL "" )
set( OPENSUBDIV_ROOT_DIR         ${GAFFER_ROOT} CACHE INTERNAL "" )
set( BOOST_ROOT                  ${GAFFER_ROOT} CACHE INTERNAL "" )
set( LLVM_STATIC                 ON CACHE INTERNAL "" )
set( LLVM_ROOT_DIR               ${GAFFER_ROOT} CACHE INTERNAL "" )
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

set(CYCLES_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/cycles/src)

project(GafferCycles)

set(CMAKE_CXX_STANDARD 11)

# IECoreCycles
set(IECORECYCLES_CXX_FILES
    src/IECoreCyclesPreview/CameraAlgo.cpp
    src/IECoreCyclesPreview/CurvesAlgo.cpp
    src/IECoreCyclesPreview/MeshAlgo.cpp
    src/IECoreCyclesPreview/NodeAlgo.cpp
    src/IECoreCyclesPreview/Renderer.cpp
    )
set(IECORECYCLES_H_FILES
    include/GafferCycles/IECoreCyclesPreview/Export.h 
    include/GafferCycles/IECoreCyclesPreview/NodeAlgo.h 
    include/GafferCycles/IECoreCyclesPreview/SocketAlgo.h 
    )
# GafferCycles
set(GAFFERCYCLES_CXX_FILES
    src/CyclesAttributes.cpp
    src/CyclesOptions.cpp
    src/CyclesRender.cpp
    src/InteractiveCyclesRender.cpp
    )
set(GAFFERCYCLES_H_FILES
    include/CyclesAttributes.h 
    include/CyclesOptions.h 
    include/CyclesRender.h 
    include/Export.h 
    include/InteractiveCyclesRender.h 
    include/TypeIds.h 
    )

add_library(GafferCycles SHARED ${IECORECYCLES_CXX_FILES} ${IECORECYCLES_H_FILES} ${GAFFERCYCLES_CXX_FILES} ${GAFFERCYCLES_H_FILES})
target_include_directories(GafferCycles PRIVATE include ${GAFFER_ROOT}/include ${CYCLES_SOURCE_DIR})
target_link_libraries(GafferCycles ${CYCLES_LIBRARIES})

install(TARGETS GafferCycles DESTINATION lib)
install(FILES ${GAFFERCYCLES_H_FILES}
        DESTINATION include/GafferCycles)
install(FILES ${IECORECYCLES_H_FILES}
        DESTINATION include/GafferCycles/IECoreCyclesPreview)

# GafferCyclesModule
add_library(GafferCyclesModule MODULE src/GafferCyclesModule/GafferCyclesModule.cpp)
set_target_properties(GafferCyclesModule PROPERTIES PREFIX "" OUTPUT_NAME "_GafferCycles" )

target_link_libraries(GafferCyclesModule GafferCycles)

target_include_directories(GafferCyclesModule PRIVATE include ${GAFFER_ROOT}/include ${GAFFER_ROOT}/include/python2.7)

install(TARGETS GafferCyclesModule DESTINATION python/GafferCycles)

file(GLOB GAFFERCYCLES_PY_FILES python/GafferCycles/*.py)

install(FILES ${GAFFERCYCLES_PY_FILES}
        DESTINATION python/GafferCycles)

# GafferCyclesUI
file(GLOB GAFFERCYCLES_UI_FILES python/GafferCyclesUI/*.py)

install(FILES ${GAFFERCYCLES_UI_FILES}
        DESTINATION python/GafferCyclesUI)