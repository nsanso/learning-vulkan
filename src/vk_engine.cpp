#include "vk_engine.h"

// Forward declarations
template <typename T>
const T &vkb_value_or_abort(vkb::Result<T> res);
VkResult vk_check(VkResult err);

// public
void VulkanEngine::init() {
    // Get window
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow("Learning Vulkan", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, m_window_extent.width,
                                m_window_extent.height, SDL_WINDOW_VULKAN);

    // Get Vulkan instance
    m_instance = vkb_value_or_abort(vkb::InstanceBuilder{}
                                        .set_app_name("Learning Vulkan")
                                        .request_validation_layers()
                                        .require_api_version(vk_version)
                                        .use_default_debug_messenger()
                                        .build());

    // Get surface
    SDL_Vulkan_CreateSurface(m_window, m_instance, &m_surface);

    // Get device
    m_gpu = vkb_value_or_abort(
        vkb::PhysicalDeviceSelector{m_instance}
            .set_minimum_version(VK_API_VERSION_MAJOR(vk_version),
                                 VK_API_VERSION_MINOR(vk_version))
            .set_surface(m_surface)
            .select());

    m_device = vkb_value_or_abort(vkb::DeviceBuilder{m_gpu}.build());

    // Get Queues
    m_q_graphics =
        vkb_value_or_abort(m_device.get_queue(vkb::QueueType::graphics));
    m_qfamily_graphics =
        vkb_value_or_abort(m_device.get_queue_index(vkb::QueueType::graphics));

    // Get swapchain
    m_swapchain = vkb_value_or_abort(
        vkb::SwapchainBuilder{m_gpu, m_device, m_surface}
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(m_window_extent.width, m_window_extent.height)
            .build());
    m_swapchain_images = vkb_value_or_abort(m_swapchain.get_images());
    m_swapchain_views = vkb_value_or_abort(m_swapchain.get_image_views());

    // Get Command pool
    VkCommandPoolCreateInfo command_pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_qfamily_graphics,
    };
    vk_check(vkCreateCommandPool(m_device.device, &command_pool_info, nullptr,
                                 &m_cmd_pool));

    // Get Command buffer
    VkCommandBufferAllocateInfo cmdbuf_alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vk_check(vkAllocateCommandBuffers(m_device.device, &cmdbuf_alloc_info,
                                      &m_cmd_buf));

    // Get Render pass
    VkAttachmentDescription color_attachment{
        .format = m_swapchain.image_format,
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
    vk_check(vkCreateRenderPass(m_device.device, &render_pass_info, nullptr,
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
        vk_check(vkCreateFramebuffer(m_device.device, &fb_info, nullptr,
                                     &m_frame_bufs[i]));
    }

    // done
    m_initialized = true;
    printf("VulkanEngine::init OK\n");
}

void VulkanEngine::cleanup() {
    if (!m_initialized) {
        printf("VulkanEngine::cleanup: not initialized\n");
        return;
    }
    // Destroy in the inverse order of creation
    vkDestroyCommandPool(m_device.device, m_cmd_pool, nullptr);
    vkb::destroy_swapchain(m_swapchain);
    vkDestroyRenderPass(m_device.device, m_render_pass, nullptr);
    for (size_t i = 0; i < m_frame_bufs.size(); i++) {
        vkDestroyFramebuffer(m_device.device, m_frame_bufs[i], nullptr);
        vkDestroyImageView(m_device.device, m_swapchain_views[i], nullptr);
    }
    vkb::destroy_device(m_device);
    vkb::destroy_surface(m_instance, m_surface);
    vkb::destroy_instance(m_instance);
    SDL_DestroyWindow(m_window);

    printf("VulkanEngine::cleanup OK\n");
}

void VulkanEngine::run() {
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

void VulkanEngine::draw() {}

VkResult vk_check(VkResult err) {
    if (err) {
        printf("Detected Vulkan error: %d\n", err);
        abort();
    }
    return err;
}

template <typename T>
const T &vkb_value_or_abort(vkb::Result<T> res) {
    if (!res.has_value()) {
        printf("Detected VKB error: %s\n",
               res.full_error().type.message().c_str());
        abort();
    }
    return res.value();
}
