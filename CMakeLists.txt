cmake_minimum_required(VERSION 3.17)

project(NanoEngine)
set(PROJECT_NAME NanoEngine)
set(CMAKE_CXX_STANDARD 17)

set(ROOT_NAMESPACE NanoEngine)
set(CMAKE_VS_PLATFORM_NAME "x64")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")


################################################################################zZ
# Source groups
################################################################################
set(Headers
    "")
source_group("Headers" FILES ${Headers})

set(Sources
    "src/main.cpp")

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

if(LINUX) # If the build is mac
    message("Starting the Linux Build")
    set(VULKAN_SDK "")

    # include directories
    target_include_directories(${PROJECT_NAME} PUBLIC
      ""
    )

    # lib directories
    target_link_directories(${PROJECT_NAME} PRIVATE
      ""
    )

    # library files
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
            ""
    )
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/external/windows/assimp/dll/assimp-vc143-mt.dll
#         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# file(COPY ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
#         DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
