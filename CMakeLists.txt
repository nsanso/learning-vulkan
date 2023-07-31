cmake_minimum_required(VERSION 3.20)

project(learning-vulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Sources
list(APPEND sources
        src/main.cpp
        src/graphics/application.cpp
        src/graphics/application.h
        src/graphics/device.cpp
        src/graphics/device.h
        src/graphics/engine.cpp
        src/graphics/engine.h
        src/graphics/mesh.cpp
        src/graphics/mesh.h
        src/graphics/pipeline.cpp
        src/graphics/pipeline.h
        src/graphics/swapchain.cpp
        src/graphics/swapchain.h
        src/graphics/utils.h
)

# Shaders
list(APPEND shaders
        src/shaders/mesh.vert
        src/shaders/mesh.frag
)

# Assets
cmake_path(SET assets "${CMAKE_SOURCE_DIR}/vendor/vulkan-guide/assets/")

# External dependencies
find_package(Threads REQUIRED)
list(APPEND libs ${CMAKE_THREAD_LIBS_INIT})

if (WIN32)
list(APPEND includes $ENV{VULKAN_SDK}/Include)
list(APPEND CMAKE_PREFIX_PATH $ENV{VULKAN_SDK}/Lib)
find_library(SDL2 NAMES SDL2 REQUIRED)
list(APPEND libs ${SDL2})
else()
find_package(SDL2 REQUIRED)
list(APPEND libs SDL2)
endif()

find_package(Vulkan REQUIRED)
list(APPEND libs ${Vulkan_LIBRARY})

add_subdirectory(vendor/VulkanMemoryAllocator)
list(APPEND libs VulkanMemoryAllocator)

add_subdirectory(vendor/glm)
list(APPEND libs glm::glm)

add_subdirectory(vendor/tinyobjloader)
list(APPEND libs tinyobjloader)

# Compile shaders
set(shader_dir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
list(APPEND includes ${shader_dir})
find_program(glsl glslangValidator)
foreach(s ${shaders})
        set(in ${CMAKE_CURRENT_SOURCE_DIR}/${s})
        set(out ${shader_dir}/${s}.h)
        cmake_path(GET s FILENAME filename)
        string(REPLACE . _ name ${filename})
        add_custom_command(
                COMMAND ${glsl} -V ${in} -o ${out} --vn ${name}
                DEPENDS ${in}
                OUTPUT  ${out}
        )
        list(APPEND shader_headers ${out})
endforeach(s)
add_custom_target(shader_target DEPENDS ${shader_headers})

# Executable target
if (WIN32)
        add_executable(${PROJECT_NAME} WIN32 ${sources})
else()
        add_executable(${PROJECT_NAME} ${sources})
endif()
target_link_libraries(${PROJECT_NAME} ${libs})
add_dependencies(${PROJECT_NAME} shader_target)
target_include_directories(${PROJECT_NAME} PRIVATE ${includes}) 
target_compile_definitions(${PROJECT_NAME} PRIVATE "ASSETS_PATH=\"${assets}\"")
