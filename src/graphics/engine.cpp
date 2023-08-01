#include "engine.h"

#include <src/shaders/color.frag.h>
#include <src/shaders/mesh.vert.h>
#include <src/shaders/normal.frag.h>

#include <algorithm>
#include <cstdio>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

const int64_t one_second_ns = 1'000'000'000;

// public
GraphicsEngine::GraphicsEngine()
    : m_frame_count(),
      m_window_extent({1280, 720}),
      m_window(),
      m_surface(),
      m_application(),
      m_device(),
      m_qfamily_graphics(uint32_t(~0)),
      m_q_graphics(),
      m_allocator(),
      m_swapchain(),
      m_render(),
      m_cmd_pool(),
      m_cmd_buf(),
      m_semph_present(),
      m_semph_render(),
      m_fence_render(),
      m_pipelines(),
      m_meshes() {
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

    // Get Render pass
    m_render = GraphicsRenderBuilder(m_swapchain, m_device.device).build();

    // Get Command pool
    VkCommandPoolCreateInfo command_pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_qfamily_graphics,
    };
    assert(!vkCreateCommandPool(m_device.device, &command_pool_info, nullptr,
                                &m_cmd_pool));

    // Get Command buffer
    VkCommandBufferAllocateInfo cmdbuf_alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    assert(!vkAllocateCommandBuffers(m_device.device, &cmdbuf_alloc_info,
                                     &m_cmd_buf));

    // Create Synchronization structures
    VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    assert(
        !vkCreateFence(m_device.device, &fence_info, nullptr, &m_fence_render));

    VkSemaphoreCreateInfo semph_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    assert(!vkCreateSemaphore(m_device.device, &semph_info, nullptr,
                              &m_semph_present));
    assert(!vkCreateSemaphore(m_device.device, &semph_info, nullptr,
                              &m_semph_render));

    // Create the drawables
    Drawable monkey{};
    const uint32_t triangle_rows = 50;
    const uint32_t triangle_cols = 50;
    std::vector<Drawable> triangles(triangle_rows * triangle_cols);

    // Create the pipeline
    m_pipelines.push_back(GraphicsPipelineBuilder(m_device.device)
                              .set_extent(m_window_extent)
                              ->set_render_pass(m_render.renderpass)
                              ->add_push_constant_range({
                                  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                  .offset = 0,
                                  .size = sizeof(glm::mat4),
                              })
                              ->add_shader(VK_SHADER_STAGE_VERTEX_BIT,
                                           mesh_vert, sizeof(mesh_vert))
                              ->add_shader(VK_SHADER_STAGE_FRAGMENT_BIT,
                                           normal_frag, sizeof(normal_frag))
                              ->build());
    monkey.material_hdl = m_pipelines.size() - 1;

    m_pipelines.push_back(GraphicsPipelineBuilder(m_device.device)
                              .set_extent(m_window_extent)
                              ->set_render_pass(m_render.renderpass)
                              ->add_push_constant_range({
                                  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                  .offset = 0,
                                  .size = sizeof(glm::mat4),
                              })
                              ->add_shader(VK_SHADER_STAGE_VERTEX_BIT,
                                           mesh_vert, sizeof(mesh_vert))
                              ->add_shader(VK_SHADER_STAGE_FRAGMENT_BIT,
                                           color_frag, sizeof(color_frag))
                              ->build());
    for (auto &t : triangles) {
        t.material_hdl = m_pipelines.size() - 1;
    }

    // load mesh
    m_meshes.push_back(
        Mesh(m_allocator,
             std::vector<Vertex>{
                 Vertex{.position{.5f, 0.f, 0.f}, .color{1.f, 0.f, 0.f}},
                 Vertex{.position{-.5f, 0.f, 0.f}, .color{0.f, 1.f, 0.f}},
                 Vertex{.position{0.f, 1.f, 0.f}, .color{0.f, 0.f, 1.f}},
             }));
    for (auto &t : triangles) {
        t.mesh_hdl = m_meshes.size() - 1;
    }

    glm::mat4 flip = glm::mat4{1.f};
    flip[1][1] *= -1;

    m_meshes.push_back(
        Mesh::from_obj(m_allocator, ASSETS_PATH, "monkey_smooth.obj").value());
    monkey.mesh_hdl = m_meshes.size() - 1;

    monkey.model = glm::translate(glm::vec3{0.f, -1.5f, 0.f}) * flip;

    float triangle_span_x = 25.f;
    float triangle_span_z = 25.f;
    for (size_t i = 0; i < triangle_cols; i++) {
        float x = triangle_span_x / triangle_cols * i - triangle_span_x / 2;

        for (size_t j = 0; j < triangle_rows; j++) {
            float z = triangle_span_z / triangle_rows * j - triangle_span_z / 2;

            triangles[i * triangle_rows + j].model =
                glm::translate(glm::vec3{x, 0, z}) * flip *
                glm::scale(glm::vec3{.25f});
        }
    }

    m_drawables.push_back(monkey);
    m_drawables.insert(m_drawables.end(), triangles.begin(), triangles.end());

    printf("VulkanEngine::init OK\n");
}

GraphicsEngine::~GraphicsEngine() {
    // Wait for the gpu to finish the pending work
    assert(!vkWaitForFences(m_device.device, 1, &m_fence_render, true,
                            one_second_ns));

    // Destroy in the inverse order of creation
    for (auto m : m_meshes) {
        m.destroy();
    };
    for (auto p : m_pipelines) {
        p.destroy();
    };
    vkDestroySemaphore(m_device.device, m_semph_render, nullptr);
    vkDestroySemaphore(m_device.device, m_semph_present, nullptr);
    vkDestroyFence(m_device.device, m_fence_render, nullptr);
    vkDestroyCommandPool(m_device.device, m_cmd_pool, nullptr);
    m_render.destroy();
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
    assert(!vkWaitForFences(m_device.device, 1, &m_fence_render, true,
                            one_second_ns));
    assert(!vkResetFences(m_device.device, 1, &m_fence_render));
    assert(!vkResetCommandBuffer(m_cmd_buf, 0));

    uint32_t swap_img_idx;
    assert(!vkAcquireNextImageKHR(m_device.device, m_swapchain.swapchain,
                                  one_second_ns, m_semph_present, nullptr,
                                  &swap_img_idx));

    VkCommandBufferBeginInfo cmd_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    assert(!vkBeginCommandBuffer(m_cmd_buf, &cmd_begin_info));

    VkClearValue clear_values[]{
        // color
        VkClearValue{.color{.float32{.0f, 0.f, 0.f, 0.f}}},
        // depth
        VkClearValue{
            .depthStencil{.depth = 1.f},
        }};

    VkRenderPassBeginInfo renderpass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_render.renderpass,
        .framebuffer = m_render.framebuffers[swap_img_idx],
        .renderArea{
            .extent = m_window_extent,
        },
        .clearValueCount = sizeof(clear_values) / sizeof(clear_values[0]),
        .pClearValues = clear_values,
    };
    vkCmdBeginRenderPass(m_cmd_buf, &renderpass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Draw
    glm::vec3 camera_position{0.f, -2.f, 5.f};
    glm::mat4 view = glm::inverse(glm::rotate(glm::radians(m_frame_count * .2f),
                                              glm::vec3{0.f, 1.f, 0.f}) *
                                  glm::translate(camera_position));
    glm::mat4 proj =
        glm::perspective(glm::radians(90.f), 16.f / 9.f, 0.1f, 200.f);

    Handle current_mesh = -1;
    Handle current_material = -1;
    for (auto d : m_drawables) {
        if (d.material_hdl != current_material) {
            current_material = d.material_hdl;
            vkCmdBindPipeline(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipelines.at(current_material).pipeline);
        }
        if (d.mesh_hdl != current_mesh) {
            current_mesh = d.mesh_hdl;
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(
                m_cmd_buf, 0, 1,
                &m_meshes.at(current_mesh).vertex_buffer.buffer, &offset);
        }

        glm::mat4 transform = proj * view * d.model;
        vkCmdPushConstants(m_cmd_buf, m_pipelines.at(current_material).layout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                           &transform);
        vkCmdDraw(m_cmd_buf, m_meshes.at(current_mesh).vertices.size(), 1, 0,
                  0);
    }

    // finalize the render pass and the command buffer
    vkCmdEndRenderPass(m_cmd_buf);
    assert(!vkEndCommandBuffer(m_cmd_buf));

    // prepare the submission to the queue.
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_semph_present,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_cmd_buf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_semph_render,
    };
    assert(!vkQueueSubmit(m_q_graphics, 1, &submit, m_fence_render));

    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_semph_render,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain.swapchain,
        .pImageIndices = &swap_img_idx,
    };
    assert(!vkQueuePresentKHR(m_q_graphics, &present_info));

    m_frame_count++;
}
