project('learning-vulkan', 'cpp')

cmake = import('cmake')

deps = [
  dependency('SDL2'),
  dependency('vulkan'),
  dependency('threads'),
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
