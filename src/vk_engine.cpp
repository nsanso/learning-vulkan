#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "vk_init.h"
#include "vk_types.h"

void VulkanEngine::init() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_WindowFlags window_flags = (SDL_WINDOW_VULKAN);

  this->window = SDL_CreateWindow(
      "Learning Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      this->windowExtent.width, this->windowExtent.height, window_flags);

  this->isInitialized = true;
}

void VulkanEngine::cleanup() {
  if (this->isInitialized) {
    SDL_DestroyWindow(this->window);
  }
}

void VulkanEngine::draw() {}

void VulkanEngine::run() {
  SDL_Event event;

  while (true) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        goto QUIT;
      }
    }
    draw();
  }

QUIT:
  return;
}
