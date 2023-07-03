#pragma once

#include "vk_types.h"

class VulkanEngine {
public:
  void init();
  void cleanup();
  void draw();
  void run();

private:
  bool is_initialized{false};
  size_t frame_number{0};
  VkExtent2D window_extent{1280, 720};
  struct SDL_Window *window{nullptr};
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkPhysicalDevice chosen_gpu;
  VkDevice device;
  VkSurfaceKHR surface;

  void init_vulkan();
};
