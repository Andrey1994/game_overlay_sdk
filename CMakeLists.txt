cmake_minimum_required (VERSION 3.12)
project (game_overlay_sdk)

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

configure_msvc_runtime ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  MESSAGE("64 bits compiler detected")
  SET(EX_PLATFORM 64)
  SET(EX_PLATFORM_NAME "x64")

  SET(GAME_OVERLAY_NAME "GameOverlay64")
  SET(INJECTOR_NAME "DLLInjection64")

  find_library(Vulkan_LIBRARY
    NAMES vulkan-1
    PATHS
      "$ENV{VULKAN_SDK}/Bin"
      "$ENV{VULKAN_SDK}/Lib"
  )
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
  MESSAGE("32 bits compiler detected")
  SET(EX_PLATFORM 32)
  SET(EX_PLATFORM_NAME "x86")

  SET(GAME_OVERLAY_NAME "GameOverlay32")
  SET(INJECTOR_NAME "DLLInjection32")

  find_library(Vulkan_LIBRARY
    NAMES vulkan-1
    PATHS
      "$ENV{VULKAN_SDK}/Bin32"
      "$ENV{VULKAN_SDK}/Lib32"
  )

endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

IF(EX_PLATFORM EQUAL 64)
MESSAGE("Outputting to lib64 and bin64")

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/python/game_overlay_sdk/lib
  CACHE PATH
  "Single Directory for all Libraries"
)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/python/game_overlay_sdk/lib
  CACHE PATH
  "Single Directory for all Executables."
)

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/lib/64
  CACHE PATH
  "Single Directory for all static libraries."
)

ELSE( EX_PLATFORM EQUAL 64 )

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/python/game_overlay_sdk/lib
  CACHE PATH
  "Single Directory for all Libraries"
)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/python/game_overlay_sdk/lib
  CACHE PATH
  "Single Directory for all Executables."
)

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
   ${CMAKE_SOURCE_DIR}/lib/32
   CACHE PATH
   "Single Directory for all static libraries."
)
ENDIF( EX_PLATFORM EQUAL 64 )

find_path(Vulkan_INCLUDE_DIR
  NAMES vulkan/vulkan.h
  PATHS
    "$ENV{VULKAN_SDK}/Include"
)

#---------------- INJECTOR START -----------------#
add_library (
  ${INJECTOR_NAME} SHARED
  src/Utils/Win32Handle.cpp
  src/Utils/ProcessHelpers.cpp
  src/Utils/StringUtils.cpp
  src/Utils/suspend_threads.cpp
  src/DLLInjection/DLLInjection.cpp
  src/DLLInjection/Monitor.cpp
  src/DLLInjection/MonitorProcessCreation.cpp
)

target_include_directories (${INJECTOR_NAME} PUBLIC
  inc
  src/Utils
  src/DLLInjection/inc
)

#----------------- INJECTOR END -------------------#

#------------------ COMMON START ------------------#
file(GLOB_RECURSE CommonSrc "src/DLLToInject/Commons/*.cpp")

add_library (
  Commons STATIC
  ${CommonSrc}
)

target_include_directories (Commons PUBLIC
  ${Vulkan_INCLUDE_DIR}
)

target_compile_definitions(Commons PUBLIC
  VK_USE_PLATFORM_WIN32_KHR
  _UNICODE
  UNICODE
)

target_link_libraries(Commons ${Vulkan_LIBRARY})
#-------------------- COMMON END --------------------#

#-------------------- MINHOOK START -----------------#
file(GLOB_RECURSE MinHookSrc "src/DLLToInject/MinHook/*.c")

add_library(
  MinHook STATIC
  ${MinHookSrc}
)
#-------------------- MINHOOK END -------------------#

#----------------- GameOverlay START ----------------#
file(GLOB_RECURSE GameOverlayD3DSrc "src/DLLToInject/GameOverlay/d3d/*.cpp")

add_library (
  ${GAME_OVERLAY_NAME} SHARED
  ${GameOverlayD3DSrc}
)

target_link_libraries (${GAME_OVERLAY_NAME} MinHook Commons d3d12 d2d1 dwrite ${Vulkan_LIBRARY})

target_include_directories ( ${GAME_OVERLAY_NAME} PUBLIC
  src/DLLToInject/Commons
  src/DLLToInject/MinHook/include
  src/DLLToInject/LibOVR/include
  src/DLLToInject/openvr/headers
  src/DLLToInject/GameOverlay/d3d/source
  ${Vulkan_INCLUDE_DIR}
)

target_compile_definitions(${GAME_OVERLAY_NAME} PUBLIC
  VK_USE_PLATFORM_WIN32_KHR
  _UNICODE
  UNICODE
)
#----------------- GameOverlay END ------------------#