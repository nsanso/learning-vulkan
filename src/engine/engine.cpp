#include "engine.h"

#include <src/shaders/mesh.frag.h>
#include <src/shaders/mesh.vert.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdio>

#include "pipeline.h"
#include "utils.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

const int64_t one_second_ns = 1'000'000'000;

// public
GraphicsEngine::GraphicsEngine() {
    // NOTE: vk-bootstrap is giving me problems on windows.
    //       Using raw vulkan fixes all issues, even though it is more complex.

    // Get window
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow("Learning Vulkan", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, m_window_extent.width,
                                m_window_extent.height, SDL_WINDOW_VULKAN);

    VkApplicationInfo application_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Learning Vulkan",
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "Custom",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = vk_version,
    };

    // TODO: add custom logger
    std::vector<const char *> instance_extensions{
        // #ifndef NDEBUG
        //        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        // #endif
    };

    uint32_t sdl_extension_count;
    SDL_Vulkan_GetInstanceExtensions(m_window, &sdl_extension_count, nullptr);

    size_t extension_count = instance_extensions.size();
    instance_extensions.resize(extension_count + sdl_extension_count);
    SDL_Vulkan_GetInstanceExtensions(m_window, &sdl_extension_count,
                                     &instance_extensions[extension_count]);

    const std::vector<const char *> validation_layers{
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
    };

    VkInstanceCreateInfo instance_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = (uint32_t)validation_layers.size(),
        .ppEnabledLayerNames = validation_layers.data(),
        .enabledExtensionCount = (uint32_t)instance_extensions.size(),
        .ppEnabledExtensionNames = instance_extensions.data(),
    };

    vk_check(vkCreateInstance(&instance_info, nullptr, &m_instance));

    // Get surface
    SDL_Vulkan_CreateSurface(m_window, m_instance, &m_surface);

    // Get device
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

    for (auto device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        uint32_t qfam_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_count, nullptr);

        std::vector<VkQueueFamilyProperties> qfamilies(qfam_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_count,
                                                 qfamilies.data());

        // TODO: add checks that select a suitable gpu instead of just taking
        // the first one
        if (true) {
            m_gpu = device;

            for (int i = qfam_count - 1; i >= 0; i--) {
                // loop in reverse order to make sure that you take the suitable
                // queue with the lowest index possible
                if (qfamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    m_qfamily_graphics = i;
                }
            }
            break;
        }
    }

    // Get logical device
    float queue_info_priority = .99f;
    VkDeviceQueueCreateInfo queue_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_qfamily_graphics,
        .queueCount = 1,
        .pQueuePriorities = &queue_info_priority,
    };
    VkPhysicalDeviceFeatures features{};
    std::vector<const char *> device_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
    };
    VkDeviceCreateInfo device_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledExtensionCount = (uint32_t)device_extensions.size(),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = &features,
    };
    vk_check(vkCreateDevice(m_gpu, &device_info, nullptr, &m_device));

    // Get queues
    vkGetDeviceQueue(m_device, m_qfamily_graphics, 0, &m_q_graphics);

    // Get swapchain
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface,
                                                       &surface_capabilities));

    uint32_t surface_format_count;
    vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_gpu, m_surface, &surface_format_count, nullptr));
    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_gpu, m_surface, &surface_format_count, surface_formats.data()));
    m_surface_format = surface_formats.at(0);

    uint32_t present_mode_count;
    vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_gpu, m_surface, &present_mode_count, nullptr));
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_gpu, m_surface, &present_mode_count, present_modes.data()));

    VkSwapchainCreateInfoKHR swapchain_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = std::clamp(surface_capabilities.minImageCount,
                                    surface_capabilities.minImageCount + 1,
                                    surface_capabilities.maxImageCount),
        .imageFormat = m_surface_format.format,
        .imageColorSpace = m_surface_format.colorSpace,
        .imageExtent = m_window_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
    };
    vk_check(
        vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain));

    // Get swapchain images
    uint32_t swapchain_image_count;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count,
                            nullptr);
    m_swapchain_images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count,
                            m_swapchain_images.data());

    // Get swapchain image views
    m_swapchain_views.resize(m_swapchain_images.size());
    for (size_t i = 0; i < m_swapchain_images.size(); i++) {
        VkImageViewCreateInfo view_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_swapchain_images.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_surface_format.format,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        vk_check(vkCreateImageView(m_device, &view_info, nullptr, &m_swapchain_views[i]));
    }

    // Get Command pool
    VkCommandPoolCreateInfo command_pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_qfamily_graphics,
    };
    vk_check(vkCreateCommandPool(m_device, &command_pool_info, nullptr,
                                 &m_cmd_pool));

    // Get Command buffer
    VkCommandBufferAllocateInfo cmdbuf_alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vk_check(
        vkAllocateCommandBuffers(m_device, &cmdbuf_alloc_info, &m_cmd_buf));

    // Get Render pass
    VkAttachmentDescription color_attachment{
        .format = m_surface_format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_attachment_ref{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
    };

    VkRenderPassCreateInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    vk_check(vkCreateRenderPass(m_device, &render_pass_info, nullptr,
                                &m_render_pass));

    // Get Frame buffer
    VkFramebufferCreateInfo fb_info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = m_render_pass,
        .attachmentCount = 1,
        .width = m_window_extent.width,
        .height = m_window_extent.height,
        .layers = 1,
    };

    m_frame_bufs = std::vector<VkFramebuffer>{m_swapchain_images.size()};

    for (size_t i = 0; i < m_swapchain_images.size(); i++) {
        fb_info.pAttachments = &m_swapchain_views[i];
        vk_check(
            vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_frame_bufs[i]));
    }

    // Create Synchronization structures
    VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    vk_check(vkCreateFence(m_device, &fence_info, nullptr, &m_fence_render));

    VkSemaphoreCreateInfo semph_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vk_check(
        vkCreateSemaphore(m_device, &semph_info, nullptr, &m_semph_present));
    vk_check(
        vkCreateSemaphore(m_device, &semph_info, nullptr, &m_semph_render));

    // Create the pipeline
    GraphicsPipelineBuilder(m_device)
        .add_shader(VK_SHADER_STAGE_VERTEX_BIT, mesh_vert, sizeof(mesh_vert))
        ->add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, mesh_frag, sizeof(mesh_frag))
        ->set_extent(m_window_extent)
        ->set_render_pass(m_render_pass)
        ->add_push_constant_range({
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(glm::mat4),
        })
        ->build(&m_pipeline);

    // create allocator
    VmaAllocatorCreateInfo allocator_info{
        .physicalDevice = m_gpu,
        .device = m_device,
        .instance = m_instance,
    };
    vmaCreateAllocator(&allocator_info, &m_allocator);

    // load mesh
    m_meshes.push_back(
        Mesh(m_allocator,
             std::vector<Vertex>{
                 Vertex{.position{.5f, 0.f, 0.f}, .color{1.f, 0.f, 0.f}},
                 Vertex{.position{-.5f, 0.f, 0.f}, .color{0.f, 1.f, 0.f}},
                 Vertex{.position{0.f, 1.f, 0.f}, .color{0.f, 0.f, 1.f}},
             }));

    // done
    m_initialized = true;
    printf("VulkanEngine::init OK\n");
}

GraphicsEngine::~GraphicsEngine() {
    if (!m_initialized) {
        printf("VulkanEngine::cleanup: not initialized\n");
        return;
    }
    // Wait for the gpu to finish the pending work
    vk_check(
        vkWaitForFences(m_device, 1, &m_fence_render, true, one_second_ns));

    // Destroy in the inverse order of creation
    for (auto m : m_meshes) {
        m.destroy();
    };
    vmaDestroyAllocator(m_allocator);
    m_pipeline.destroy();
    vkDestroySemaphore(m_device, m_semph_render, nullptr);
    vkDestroySemaphore(m_device, m_semph_present, nullptr);
    vkDestroyFence(m_device, m_fence_render, nullptr);
    vkDestroyCommandPool(m_device, m_cmd_pool, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);
    for (size_t i = 0; i < m_frame_bufs.size(); i++) {
        vkDestroyFramebuffer(m_device, m_frame_bufs[i], nullptr);
        vkDestroyImageView(m_device, m_swapchain_views[i], nullptr);
    }
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);

    printf("VulkanEngine::cleanup OK\n");
}

void GraphicsEngine::run() {
    SDL_Event event;

    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto run_QUIT;
            }
        }
        draw();
    }

run_QUIT:
    return;
}

void GraphicsEngine::draw() {
    vk_check(
        vkWaitForFences(m_device, 1, &m_fence_render, true, one_second_ns));
    vk_check(vkResetFences(m_device, 1, &m_fence_render));
    vk_check(vkResetCommandBuffer(m_cmd_buf, 0));

    uint32_t swap_img_idx;
    vk_check(vkAcquireNextImageKHR(m_device, m_swapchain, one_second_ns,
                                   m_semph_present, nullptr, &swap_img_idx));

    VkCommandBufferBeginInfo cmd_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vk_check(vkBeginCommandBuffer(m_cmd_buf, &cmd_begin_info));

    VkClearValue clear_value{.color{.float32{.0f, 0.f, 0.f, 0.f}}};
    VkRenderPassBeginInfo renderpass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_render_pass,
        .framebuffer = m_frame_bufs[swap_img_idx],
        .renderArea{
            .extent = m_window_extent,
        },
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    vkCmdBeginRenderPass(m_cmd_buf, &renderpass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Draw
    vkCmdBindPipeline(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_pipeline.pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_cmd_buf, 0, 1,
                           &m_meshes.at(0).vertex_buffer.buffer, &offset);

    glm::vec3 camera_position{0.f, 0.f, -2.f};
    glm::mat4 view = glm::translate(glm::mat4(1.f), camera_position);
    glm::mat4 proj =
        glm::perspective(glm::radians(90.f), 16.f / 9.f, 0.1f, 200.f);
    glm::mat4 model =
        glm::rotate(glm::mat4{1.f}, glm::radians(m_frame_count * 0.4f),
                    glm::vec3{0.f, 1.f, 0.f});
    glm::mat4 transform = proj * view * model;

    vkCmdPushConstants(m_cmd_buf, m_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(glm::mat4), &transform);

    vkCmdDraw(m_cmd_buf, m_meshes.at(0).vertices.size(), 1, 0, 0);

    // finalize the render pass and the command buffer
    vkCmdEndRenderPass(m_cmd_buf);
    vk_check(vkEndCommandBuffer(m_cmd_buf));

    // prepare the submission to the queue.
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_semph_present,
        .pWaitDstStageMask = &wait_stage,  // you'll learn about this later
        .commandBufferCount = 1,
        .pCommandBuffers = &m_cmd_buf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_semph_render,
    };
    vk_check(vkQueueSubmit(m_q_graphics, 1, &submit, m_fence_render));

    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_semph_render,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &swap_img_idx,
    };
    vk_check(vkQueuePresentKHR(m_q_graphics, &present_info));

    m_frame_count++;
}
