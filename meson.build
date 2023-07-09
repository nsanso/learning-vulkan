project('learning-vulkan', 'cpp')

cmake = import('cmake')

vk_bootstrap = cmake.subproject('vk-bootstrap')
message('CMake targets:\n - ' + '\n - '.join(vk_bootstrap.target_list()))

deps = [
  dependency('SDL2'),
  dependency('vulkan'),
  dependency('threads'),
  vk_bootstrap.dependency('vk-bootstrap'),
  ]

shaders = [
	'src/shaders/triangle.vert',
	'src/shaders/triangle.frag',
]

glslang = find_program('glslangValidator')
shader_targets = []
foreach s : shaders
	name = s.split('/').get(-1).underscorify()
	shader_targets += custom_target(
	  name,
	  command: [glslang, '-V', '@INPUT@', '-o', '@OUTPUT@', '--vn', name],
	  input: s,
	  output: '@PLAINNAME@.h'
	  )
endforeach

srcs = [
  'src/main.cpp',
  'src/engine/vk_engine.cpp',
  'src/engine/vk_init.cpp',
  shader_targets,
  ]

executable(
  'learning-vulkan',
  srcs,
  dependencies : deps
)
