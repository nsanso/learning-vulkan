cmake_minimum_required(VERSION 3.20)

project(learning-vulkan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Sources
list(APPEND sources
        src/main.cpp
        src/engine/engine.cpp
        src/engine/mesh.cpp
        src/engine/pipeline.cpp
)

# Shaders
list(APPEND shaders
        src/shaders/mesh.vert
        src/shaders/triangle.vert
        src/shaders/triangle.frag
)

# External dependencies
find_package(Threads REQUIRED)
list(APPEND libs ${CMAKE_THREAD_LIBS_INIT})

find_package(SDL2 REQUIRED)
list(APPEND libs SDL2)

find_package(Vulkan REQUIRED)
list(APPEND libs vulkan)

add_subdirectory(vendor/vk-bootstrap)
list(APPEND libs vk-bootstrap)

add_subdirectory(vendor/VulkanMemoryAllocator)
list(APPEND libs VulkanMemoryAllocator)

add_subdirectory(vendor/glm)
list(APPEND libs glm::glm)

# Compile shaders
set(shader_dir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
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
set(target learning-vulkan)
add_executable(${target} ${sources})
target_link_libraries(${target} ${libs})
add_dependencies(${target} shader_target)
target_include_directories(${target} PRIVATE ${shader_dir}) 
