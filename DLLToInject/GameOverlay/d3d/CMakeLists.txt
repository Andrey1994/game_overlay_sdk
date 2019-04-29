project(GameOverlay CXX)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    res/resource.h
    source/main.cpp
)
source_group("" FILES ${no_group_source_files})

set(Hook
    source/critical_section.hpp
    source/hook.cpp
    source/hook.hpp
    source/hook_manager.cpp
    source/hook_manager.hpp
)
source_group("Hook" FILES ${Hook})

set(d3d
    source/d3d/d3d11_renderer.cpp
    source/d3d/d3d11_renderer.hpp
    source/d3d/d3d12_renderer.cpp
    source/d3d/d3d12_renderer.hpp
    source/d3d/dxgi.cpp
    source/d3d/dxgi_swapchain.cpp
    source/d3d/dxgi_swapchain.hpp
    source/d3d/DXGIWrapper.cpp
    source/d3d/DXGIWrapper.h
)
source_group("d3d" FILES ${d3d})

set(d3d__Compositor
    source/d3d/oculus.cpp
    source/d3d/oculus.h
    source/d3d/steamvr.cpp
    source/d3d/steamvr.h
)
source_group("d3d\\Compositor" FILES ${d3d__Compositor})

set(d3d__Shaders
    OverlayPS_Byte.h
    OverlayVS_Byte.h
)
source_group("d3d\\Shaders" FILES ${d3d__Shaders})

set(ALL_FILES
    ${no_group_source_files}
    ${Hook}
    ${d3d}
    ${d3d__Compositor}
    ${d3d__Shaders}
)

################################################################################
# Target
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "overlay")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "deps/DirectX.cmake")
use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "deps/Vulkan.cmake")

################################################################################
# Target name
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUG   "${PROJECT_NAME}$ENV{PlatformArchitecture}"
        TARGET_NAME_RELEASE "${PROJECT_NAME}$ENV{PlatformArchitecture}"
    )
endif()
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY"${CMAKE_SOURCE_DIR}/x64/$<CONFIG>/Bin/"
)
################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATCommons;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATMinHook/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATLibOVR/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/source;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATopenvr/headers"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od;
                /Zi
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi;
                /Gy
            >
            ${DEFAULT_CXX_RUNTIME_LIBRARY};
            /W3;
            /std:c++latest;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG:FULL
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    libMinHook
    Commons
)

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "$(OutDir)Commons$(PlatformArchitecture);"
            "$(SolutionDir)lib\Debug\libMinHook.x64;"
            "d3d12;"
            "d2d1;"
            "dwrite"
        ">"
        "$<$<CONFIG:Release>:"
            "$(OutDir)Commons$(PlatformArchitecture);"
            "$(SolutionDir)lib\Release\libMinHook.x64;"
            "d3d12;"
            "d2d1;"
            "dwrite"
        ">"
    )
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
    )
endif()

