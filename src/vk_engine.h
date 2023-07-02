#pragma once

#include "vk_types.h"

struct VulkanEngine {
  bool isInitialized{false};
  size_t frameNumber{0};

  VkExtent2D windowExtent{1280, 720};

  struct SDL_Window *window{nullptr};

  void init();
  void cleanup();
  void draw();
  void run();
};
