project('learning-vulkan', 'cpp')

cmake = import('cmake')

deps = [
  cmake.subproject('SDL').dependency('SDL2-static'),
]

executable(
  'learning-vulkan', 
  'src/main.cpp',
  dependencies: deps,
  )
