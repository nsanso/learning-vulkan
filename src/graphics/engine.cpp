#include "engine.h"

#include <src/shaders/mesh.frag.h>
#include <src/shaders/mesh.vert.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdio>

#include "device.h"
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

    uint32_t sdl_extension_count;
    SDL_Vulkan_GetInstanceExtensions(m_window, &sdl_extension_count, nullptr);

    std::vector<const char *> sdl_extensions(sdl_extension_count);
    SDL_Vulkan_GetInstanceExtensions(m_window, &sdl_extension_count,
                                     sdl_extensions.data());

    m_application = GraphicsApplicationBuilder()
                        .set_application_name("Learning Vulkan")
                        ->set_application_version(0, 1, 0)
                        ->add_instance_extension(sdl_extensions)
                        ->build();

    SDL_Vulkan_CreateSurface(m_window, m_application.instance, &m_surface);

    m_qfamily_graphics = m_application.get_queue_family(VK_QUEUE_GRAPHICS_BIT);

    m_device = GraphicsDeviceBuilder(m_application.device)
                   .add_queue(m_qfamily_graphics, .99f)
                   ->build();

    m_q_graphics = m_device.get_queue(m_qfamily_graphics);

    // create allocator
    VmaAllocatorCreateInfo allocator_info{
        .physicalDevice = m_application.device,
        .device = m_device.device,
        .instance = m_application.instance,
    };
    vmaCreateAllocator(&allocator_info, &m_allocator);

    m_swapchain = GraphicsSwapchainBuilder(m_application.device, m_allocator,
                                           m_device.device, m_surface)
                      .set_extent(m_window_extent)
                      ->build();

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
        .format = m_swapchain.format.format,
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

    VkSubpassDependency color_dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkAttachmentDescription depth_attachment{
        .format = m_swapchain.depth_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_attachment_ref{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDependency depth_dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref,
    };

    VkAttachmentDescription attachments[] = {color_attachment,
                                             depth_attachment};
    VkSubpassDependency dependencies[] = {color_dependency, depth_dependency};
    VkRenderPassCreateInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = sizeof(attachments) / sizeof(attachments[0]),
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = sizeof(dependencies) / sizeof(dependencies[0]),
        .pDependencies = dependencies,
    };
    vk_check(vkCreateRenderPass(m_device.device, &render_pass_info, nullptr,
                                &m_render_pass));

    // Get Frame buffer
    VkFramebufferCreateInfo fb_info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = m_render_pass,
        .width = m_window_extent.width,
        .height = m_window_extent.height,
        .layers = 1,
    };

    m_frame_bufs = std::vector<VkFramebuffer>{m_swapchain.images.size()};

    for (size_t i = 0; i < m_swapchain.images.size(); i++) {
        VkImageView attachments[]{m_swapchain.views[i], m_swapchain.depth_view};

        fb_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        fb_info.pAttachments = attachments;

        vk_check(vkCreateFramebuffer(m_device.device, &fb_info, nullptr,
                                     &m_frame_bufs[i]));
    }

    // Create Synchronization structures
    VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    vk_check(
        vkCreateFence(m_device.device, &fence_info, nullptr, &m_fence_render));

    VkSemaphoreCreateInfo semph_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vk_check(vkCreateSemaphore(m_device.device, &semph_info, nullptr,
                               &m_semph_present));
    vk_check(vkCreateSemaphore(m_device.device, &semph_info, nullptr,
                               &m_semph_render));

    // Create the pipeline
    m_pipeline = GraphicsPipelineBuilder(m_device.device)
                     .add_shader(VK_SHADER_STAGE_VERTEX_BIT, mesh_vert,
                                 sizeof(mesh_vert))
                     ->add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, mesh_frag,
                                  sizeof(mesh_frag))
                     ->set_extent(m_window_extent)
                     ->set_render_pass(m_render_pass)
                     ->add_push_constant_range({
                         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                         .offset = 0,
                         .size = sizeof(glm::mat4),
                     })
                     ->build();

    // load mesh
    m_meshes.push_back(
        Mesh(m_allocator,
             std::vector<Vertex>{
                 Vertex{.position{.5f, 0.f, 0.f}, .color{1.f, 0.f, 0.f}},
                 Vertex{.position{-.5f, 0.f, 0.f}, .color{0.f, 1.f, 0.f}},
                 Vertex{.position{0.f, 1.f, 0.f}, .color{0.f, 0.f, 1.f}},
             }));

    auto monkey = Mesh::from_obj(m_allocator, ASSETS_PATH, "monkey_smooth.obj");

    if (monkey) {
        m_meshes.push_back(monkey.value());
    }

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
    vk_check(vkWaitForFences(m_device.device, 1, &m_fence_render, true,
                             one_second_ns));

    // Destroy in the inverse order of creation
    for (auto m : m_meshes) {
        m.destroy();
    };
    m_pipeline.destroy();
    vkDestroySemaphore(m_device.device, m_semph_render, nullptr);
    vkDestroySemaphore(m_device.device, m_semph_present, nullptr);
    vkDestroyFence(m_device.device, m_fence_render, nullptr);
    vkDestroyCommandPool(m_device.device, m_cmd_pool, nullptr);
    vkDestroyRenderPass(m_device.device, m_render_pass, nullptr);
    for (size_t i = 0; i < m_frame_bufs.size(); i++) {
        vkDestroyFramebuffer(m_device.device, m_frame_bufs[i], nullptr);
    }
    m_swapchain.destroy();
    vmaDestroyAllocator(m_allocator);
    m_device.destroy();
    vkDestroySurfaceKHR(m_application.instance, m_surface, nullptr);
    m_application.destroy();
    SDL_DestroyWindow(m_window);

    printf("VulkanEngine::cleanup OK\n");
}

void GraphicsEngine::run() {
    SDL_Event event;

    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
        }
        draw();
    }
}

void GraphicsEngine::draw() {
    vk_check(vkWaitForFences(m_device.device, 1, &m_fence_render, true,
                             one_second_ns));
    vk_check(vkResetFences(m_device.device, 1, &m_fence_render));
    vk_check(vkResetCommandBuffer(m_cmd_buf, 0));

    uint32_t swap_img_idx;
    vk_check(vkAcquireNextImageKHR(m_device.device, m_swapchain.swapchain,
                                   one_second_ns, m_semph_present, nullptr,
                                   &swap_img_idx));

    VkCommandBufferBeginInfo cmd_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vk_check(vkBeginCommandBuffer(m_cmd_buf, &cmd_begin_info));

    VkClearValue clear_values[]{
        // color
        VkClearValue{.color{.float32{.0f, 0.f, 0.f, 0.f}}},
        // depth
        VkClearValue{
            .depthStencil{.depth = 1.f},
        }};

    VkRenderPassBeginInfo renderpass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_render_pass,
        .framebuffer = m_frame_bufs[swap_img_idx],
        .renderArea{
            .extent = m_window_extent,
        },
        .clearValueCount = sizeof(clear_values) / sizeof(clear_values[0]),
        .pClearValues = clear_values,
    };
    vkCmdBeginRenderPass(m_cmd_buf, &renderpass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Draw
    vkCmdBindPipeline(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_pipeline.pipeline);

    glm::vec3 camera_position{0.f, 0.f, -2.f};
    glm::mat4 view = glm::translate(glm::mat4(1.f), camera_position);
    glm::mat4 proj =
        glm::perspective(glm::radians(90.f), 16.f / 9.f, 0.1f, 200.f);
    proj[1][1] *= -1;  // Make the y axis point upwards
    glm::mat4 model =
        glm::rotate(glm::mat4{1.f}, glm::radians(m_frame_count * 0.4f),
                    glm::vec3{0.f, 1.f, 0.f});
    glm::mat4 transform = proj * view * model;

    vkCmdPushConstants(m_cmd_buf, m_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(glm::mat4), &transform);

    VkDeviceSize offset = 0;

    // // Draw Triangle
    // vkCmdBindVertexBuffers(m_cmd_buf, 0, 1,
    //                        &m_meshes.at(0).vertex_buffer.buffer, &offset);
    // vkCmdDraw(m_cmd_buf, m_meshes.at(0).vertices.size(), 1, 0, 0);

    // Draw Monke
    vkCmdBindVertexBuffers(m_cmd_buf, 0, 1,
                           &m_meshes.at(1).vertex_buffer.buffer, &offset);
    vkCmdDraw(m_cmd_buf, m_meshes.at(1).vertices.size(), 1, 0, 0);

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
        .pSwapchains = &m_swapchain.swapchain,
        .pImageIndices = &swap_img_idx,
    };
    vk_check(vkQueuePresentKHR(m_q_graphics, &present_info));

    m_frame_count++;
}
