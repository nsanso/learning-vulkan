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


shaders = [
	'src/shaders/triangle.vert',
	'src/shaders/triangle.frag',
]

shader_headers = []
glslang = find_program('glslangValidator')
foreach shader : shaders
	name = shader.underscorify() + '_data'
	args = [glslang, '-V', '@INPUT@', '-o', '@OUTPUT@', '--vn', name]
	header = custom_target(
		shader + '_spv',
		output: shader + '.h',
		input: shader,
		command: args)
	shader_headers += [header]
endforeach

srcs = files([
  'src/main.cpp',
  'src/engine/vk_engine.cpp',
  'src/engine/vk_init.cpp',
  shader_headers,
  ])

executable(
  'learning-vulkan',
  srcs,
  dependencies : deps
)
