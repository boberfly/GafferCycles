cmake_minimum_required(VERSION 3.0)

# Cycles Standalone
set( WITH_CYCLES_STANDALONE_GUI  OFF CACHE INTERNAL "" )
set( WITH_CYCLES_OSL             ON CACHE INTERNAL "" )
#set( WITH_CYCLES_LOGGING         ON CACHE INTERNAL "" )
set( WITH_CYCLES_DEBUG           ON CACHE INTERNAL "" )
set( WITH_CYCLES_OPENSUBDIV      OFF CACHE INTERNAL "" )
set( OPENSUBDIV_ROOT_DIR         ${GAFFER_ROOT} CACHE INTERNAL "" )
set( BOOST_ROOT                  ${GAFFER_ROOT} CACHE INTERNAL "" )
set( LLVM_STATIC                 ON CACHE INTERNAL "" )
set( LLVM_ROOT_DIR               ${GAFFER_DEPENDENCIES_ROOT} CACHE INTERNAL "" )
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
)

set(CMAKE_CXX_STANDARD 11)

link_directories( ${GAFFER_ROOT}/lib )

# IECoreCycles
set(IECORECYCLES_CXX_FILES
    src/GafferCycles/IECoreCyclesPreview/AttributeAlgo.cpp
    src/GafferCycles/IECoreCyclesPreview/CameraAlgo.cpp
    src/GafferCycles/IECoreCyclesPreview/CurvesAlgo.cpp
    src/GafferCycles/IECoreCyclesPreview/InstancingConverter.cpp
    src/GafferCycles/IECoreCyclesPreview/MeshAlgo.cpp
    src/GafferCycles/IECoreCyclesPreview/ObjectAlgo.cpp
    #src/GafferCycles/IECoreCyclesPreview/Renderer.cpp
    )
set(IECORECYCLES_H_FILES
    include/GafferCycles/IECoreCyclesPreview/AttributeAlgo.h
    include/GafferCycles/IECoreCyclesPreview/CameraAlgo.h
    include/GafferCycles/IECoreCyclesPreview/CurvesAlgo.h
    include/GafferCycles/IECoreCyclesPreview/Export.h
    include/GafferCycles/IECoreCyclesPreview/InstancingConverter.h
    include/GafferCycles/IECoreCyclesPreview/MeshAlgo.h
    include/GafferCycles/IECoreCyclesPreview/ObjectAlgo.h
    include/GafferCycles/IECoreCyclesPreview/SocketAlgo.h
    )
# GafferCycles
set(GAFFERCYCLES_CXX_FILES
    src/GafferCycles/CyclesAttributes.cpp
    src/GafferCycles/CyclesOptions.cpp
    src/GafferCycles/CyclesRender.cpp
    src/GafferCycles/InteractiveCyclesRender.cpp
    )
set(GAFFERCYCLES_H_FILES
    include/GafferCycles/CyclesAttributes.h
    include/GafferCycles/CyclesOptions.h
    include/GafferCycles/CyclesRender.h
    include/GafferCycles/Export.h
    include/GafferCycles/InteractiveCyclesRender.h
    include/GafferCycles/TypeIds.h
    )

add_library(GafferCycles SHARED ${IECORECYCLES_CXX_FILES} ${IECORECYCLES_H_FILES} ${GAFFERCYCLES_CXX_FILES} ${GAFFERCYCLES_H_FILES})
target_include_directories(GafferCycles PRIVATE include ${GAFFER_DEPENDENCIES_ROOT}/include ${GAFFER_ROOT}/include ${CYCLES_INCLUDE_DIR} ${OPENSUBDIV_INCLUDE_DIR})
target_link_libraries(GafferCycles ${Cortex_IECORE_LIBRARY} ${Cortex_IECORESCENE_LIBRARY} ${CYCLES_LIBRARIES} ${OPENSUBDIV_LIBRARIES})
add_dependencies( GafferCycles cycles )

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
add_dependencies( GafferCyclesModule GafferCycles )

install(TARGETS GafferCyclesModule DESTINATION python/GafferCycles)

file(GLOB GAFFERCYCLES_PY_FILES python/GafferCycles/*.py)

install(FILES ${GAFFERCYCLES_PY_FILES}
        DESTINATION python/GafferCycles)

# GafferCyclesUI
file(GLOB GAFFERCYCLES_UI_FILES python/GafferCyclesUI/*.py)

install(FILES ${GAFFERCYCLES_UI_FILES}
        DESTINATION python/GafferCyclesUI)