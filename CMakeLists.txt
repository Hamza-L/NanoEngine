cmake_minimum_required(VERSION 3.17)

# must be set before project()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VS_PLATFORM_NAME "x64")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE "Debug")

project(NanoEngine)
set(PROJECT_NAME NanoEngine)
set(ROOT_NAMESPACE NanoEngine)

################################################################################zZ
# Source groups
################################################################################
set(Headers
    "src/NanoEngine.hpp"
    "src/NanoWindow.hpp"
    "src/NanoGraphics.hpp"
    "src/NanoError.hpp"
    "src/NanoUtility.hpp"
    "src/NanoLogger.hpp"
)

source_group("Headers" FILES ${Headers})

set(Sources
    "src/NanoEngine.cpp"
    "src/NanoWindow.cpp"
    "src/NanoGraphics.cpp"
    "src/NanoLogger.cpp"
    "src/main.cpp"
)

source_group("Sources" FILES ${Sources})

set(ALL_FILES
    ${Headers}
    ${Sources})

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

################################################################################
# Include directories and Dependencies
################################################################################

if(WIN32)
    message("Starting the windows Build")

target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLM"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GFLW/win/include"
        "$ENV{VULKAN_SDK}/Include")

    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GFLW/win/lib-vc2022"
        "$ENV{VULKAN_SDK}/Lib")

    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "glfw3.lib"
        "vulkan-1.lib")
elseif(LINUX)
    message("Starting the Linux Build")

target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GLM"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GFLW/linux/include")

    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/GFLW/linux/lib")

    set(ADDITIONAL_LIBRARY_DEPENDENCIES
            "libglfw3.a")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/external/windows/assimp/dll/assimp-vc143-mt.dll
#         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# file(COPY ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
#         DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
