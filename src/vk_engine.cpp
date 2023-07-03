#include "vk_engine.h"

#include "vk_init.h"
#include "vk_types.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <VkBootstrap.h>
#include <iostream>

// public

void VulkanEngine::init() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags window_flags = (SDL_WINDOW_VULKAN);
  this->window = SDL_CreateWindow(
      "Learning Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      this->window_extent.width, this->window_extent.height, window_flags);

  this->init_vulkan();

  this->is_initialized = true;
}

void VulkanEngine::run() {
  SDL_Event event;

  while (true) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        goto label_VulkanEngine_run_QUIT;
      }
    }
    draw();
  }

label_VulkanEngine_run_QUIT:
  return;
}

void VulkanEngine::draw() {}

void VulkanEngine::cleanup() {
  if (this->is_initialized) {
    SDL_DestroyWindow(this->window);
  }
}

// private

void VulkanEngine::init_vulkan() {
  vkb::InstanceBuilder builder;

  auto maybe_instance = builder.set_app_name("Learning Vulkan")
                            .request_validation_layers()
                            // .require_api_version(VK_MAKE_VERSION(1, 3, 5))
                            .use_default_debug_messenger()
                            .build();

  if (!maybe_instance.has_value()) {
    std::cout << "Failed to build vulkan instance: "
              << maybe_instance.full_error().vk_result << std::endl;
    abort();
  }

  vkb::Instance instance = maybe_instance.value();
  this->instance = instance.instance;
  this->debug_messenger = instance.debug_messenger;
}

void vk_check(VkResult err) {
  if (err) {
    std::cout << "Detected Vulkan error: " << err << std::endl;
    abort();
  }
}
