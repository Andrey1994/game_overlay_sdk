project(Commons CXX)

################################################################################
# Source groups
################################################################################
set(Config
    Config/BlackList.cpp
    Config/BlackList.h
    Config/Config.cpp
    Config/Config.h
)
source_group("Config" FILES ${Config})

set(Logging
    Logging/MessageLog.cpp
    Logging/MessageLog.h
)
source_group("Logging" FILES ${Logging})

set(Overlay
    Overlay/DLLInjection.cpp
    Overlay/DLLInjection.h
    Overlay/GlobalHook.cpp
    Overlay/GlobalHook.h
    Overlay/Hook.cpp
    Overlay/Hook.h
    Overlay/Overlay.cpp
    Overlay/Overlay.h
    Overlay/OverlayInterface.cpp
    Overlay/OverlayInterface.h
    Overlay/OverlayMessage.cpp
    Overlay/OverlayMessage.h
    Overlay/OverlayPosition.cpp
    Overlay/OverlayPosition.h
    Overlay/VK_Environment.cpp
    Overlay/VK_Environment.h
)
source_group("Overlay" FILES ${Overlay})

set(Recording
    Recording/Capturing.cpp
    Recording/Capturing.h
    Recording/OverlayThread.cpp
    Recording/OverlayThread.h
    Recording/PerformanceCounter.cpp
    Recording/PerformanceCounter.hpp
    Recording/RecordingState.cpp
    Recording/RecordingState.h
)
source_group("Recording" FILES ${Recording})

set(Rendering
    Rendering/ConstantBuffer.h
    Rendering/OverlayBitmap.cpp
    Rendering/OverlayBitmap.h
    Rendering/TextMessage.cpp
    Rendering/TextMessage.h
)
source_group("Rendering" FILES ${Rendering})

set(Utility
    Utility/Constants.h
    Utility/DirectoryType.h
    Utility/FileDirectory.cpp
    Utility/FileDirectory.h
    Utility/FileUtils.cpp
    Utility/FileUtils.h
    Utility/IniParser.cpp
    Utility/IniParser.h
    Utility/ProcessHelper.cpp
    Utility/ProcessHelper.h
    Utility/ProcessTermination.cpp
    Utility/ProcessTermination.h
    Utility/SmartHandle.cpp
    Utility/SmartHandle.h
    Utility/StringUtils.cpp
    Utility/StringUtils.h
)
source_group("Utility" FILES ${Utility})

set(ALL_FILES
    ${Config}
    ${Logging}
    ${Overlay}
    ${Recording}
    ${Rendering}
    ${Utility}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Commons)

################################################################################
# Target name
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUG   "${PROJECT_NAME}$ENV{PlatformArchitecture}$ENV{TargetExt}"
        TARGET_NAME_RELEASE "${PROJECT_NAME}$ENV{PlatformArchitecture}$ENV{TargetExt}"
    )
endif()
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>/Bin"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>/Bin"
    )
endif()
################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$ENV{VK_SDK_PATH}/include"
    )
endif()

################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od;
                ${DEFAULT_CXX_DEBUG_RUNTIME_LIBRARY}
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
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
