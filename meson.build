project('learning-vulkan', 'cpp')

cmake = import('cmake')
# message('CMake targets:\n - ' + '\n - '.join(<cmake_subproject>.target_list()))

vk_bootstrap = cmake.subproject('vk-bootstrap')
message('CMake targets:\n - ' + '\n - '.join(vk_bootstrap.target_list()))

deps = [
  dependency('SDL2'),
  dependency('vulkan'),
  dependency('threads'),
  vk_bootstrap.dependency('vk-bootstrap'),
  ]

srcs = files([
  'src/main.cpp',
  'src/throwaway.cpp',
  'src/vk_engine.cpp',
  'src/vk_init.cpp',
  ])


executable(
  'learning-vulkan',
  srcs,
  dependencies : deps
)
