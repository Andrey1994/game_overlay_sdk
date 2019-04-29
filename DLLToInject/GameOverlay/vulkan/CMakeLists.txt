project(VK_LAYER_OCAT_overlay CXX)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    resource.h
    src/AppResMapping.cpp
    src/AppResMapping.h
    src/HashMap.h
    src/OverlayImageData.cpp
    src/OverlayImageData.h
    src/Rendering.cpp
    src/Rendering.h
    src/shader.comp
    src/shader.frag
    src/shader.vert
    src/SwapchainImageData.h
    src/SwapchainImageMapping.h
    src/SwapchainMapping.cpp
    src/SwapchainMapping.h
    src/SwapchainQueueMapping.h
    src/VK_LAYER_OCAT_overlay.cpp
    VK_LAYER_OCAT_overlay.def
)
source_group("" FILES ${no_group_source_files})

set(Compositor
    src/Compositor/vk_oculus.cpp
    src/Compositor/vk_oculus.h
    src/Compositor/vk_steamvr.cpp
    src/Compositor/vk_steamvr.h
)
source_group("Compositor" FILES ${Compositor})

set(ALL_FILES
    ${no_group_source_files}
    ${Compositor}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "overlay")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE VK_LAYER_AMD_overlay)

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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>/Bin/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>/Bin/"
    )
endif()
################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATCommons;"
        "$ENV{VK_SDK_PATH}/include;"
        "$ENV{VK_SDK_PATH}/Source/layers;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATGameOverlay/d3d/source;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATLibOVR/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../OCATopenvr/headers"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_CRT_SECURE_NO_WARNINGS;"
        "API_NAME=\"Vulkan\";"
        "_MBCS"
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
                ${DEFAULT_CXX_DEBUG_RUNTIME_LIBRARY};
                /Zi
            >
            $<$<CONFIG:Release>:
                /GL;
                /O2;
                /Oi;
                ${DEFAULT_CXX_RUNTIME_LIBRARY};
                /Gy
            >
            /W3;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /LTCG
            >
            /DEBUG:FULL
        )
    endif()
endif()

################################################################################
# Post build events
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    add_custom_command_if(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMANDS
        COMMAND   $<CONFIG:Debug> xcopy /Y /D "VK_LAYER_AMD_overlay64.json" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>"
        COMMAND   $<CONFIG:Debug> "$ENV{VK_SDK_PATH}"\\Bin\\glslangValidator.exe -V src\\shader.vert -o src\\vert.spv
        COMMAND   $<CONFIG:Debug> "$ENV{VK_SDK_PATH}"\\Bin\\glslangValidator.exe -V src\\shader.frag -o src\\frag.spv
        COMMAND   $<CONFIG:Debug> "$ENV{VK_SDK_PATH}"\\Bin\\glslangValidator.exe -V src\\shader.comp -o src\\comp.spv
        COMMAND   $<CONFIG:Debug> xcopy /Y /D src\\*spv "$<SHELL_PATH:${OUTPUT_DIRECTORY}>"
        COMMAND $<CONFIG:Release> xcopy /Y /D "VK_LAYER_OCAT_overlay64.json" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>"
        COMMAND $<CONFIG:Release> "$ENV{VK_SDK_PATH}"\\Bin\\glslangValidator.exe -V src\\shader.vert -o src\\vert.spv
        COMMAND $<CONFIG:Release> "$ENV{VK_SDK_PATH}"\\Bin\\glslangValidator.exe -V src\\shader.frag -o src\\frag.spv
        COMMAND $<CONFIG:Release> "$ENV{VK_SDK_PATH}"\\Bin\\glslangValidator.exe -V src\\shader.comp -o src\\comp.spv
        COMMAND $<CONFIG:Release> xcopy /Y /D src\\*spv "$<SHELL_PATH:${OUTPUT_DIRECTORY}>"
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    Commons
    GameOverlay
)

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "$(OutDir)Commons$(PlatformArchitecture);"
            "$(OutDir)GameOverlay$(PlatformArchitecture);"
            "d2d1;"
            "dwrite"
        ">"
        "$<$<CONFIG:Release>:"
            "$(OutDir)Commons$(PlatformArchitecture);"
            "$(OutDir)GameOverlay$(PlatformArchitecture);"
            "d2d1;"
            "dwrite"
        ">"
    )
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

