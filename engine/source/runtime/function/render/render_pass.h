#pragma once

#include "runtime/function/render/render_common.h"
#include "runtime/function/render/render_pass_base.h"
#include "runtime/function/render/render_resource.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace Piccolo
{
    class VulkanRHI;

    enum
    {
        // attachments
        _main_camera_pass_gbuffer_a                     = 0,
        _main_camera_pass_gbuffer_b                     = 1,
        _main_camera_pass_gbuffer_c                     = 2,
        _main_camera_pass_backup_buffer_odd             = 3,
        _main_camera_pass_backup_buffer_even            = 4,
        _main_camera_pass_post_process_buffer_odd       = 5,
        _main_camera_pass_post_process_buffer_even      = 6,
        _main_camera_pass_depth                         = 7,
        _main_camera_pass_swap_chain_image              = 8,
        // attachment count
        _main_camera_pass_custom_attachment_count       = 5,
        _main_camera_pass_post_process_attachment_count = 2,
        _main_camera_pass_attachment_count              = 9,
    };

    // All subPass of main_camera_pass
    // A renderpass can include multiple serial subpasses
    // 枚举了subpass
    enum
    {
        _main_camera_subpass_basepass = 0,
        _main_camera_subpass_deferred_lighting,
        _main_camera_subpass_forward_lighting,
        _main_camera_subpass_tone_mapping,
        _main_camera_subpass_color_grading,
        _main_camera_subpass_fxaa,
        _main_camera_subpass_ui,
        _main_camera_subpass_combine_ui,
        _main_camera_subpass_count
    };

    // 用Nodes表示可见场景
    struct VisiableNodes
    {
        std::vector<RenderMeshNode>*              p_directional_light_visible_mesh_nodes {nullptr};   // directional_light 可见
        std::vector<RenderMeshNode>*              p_point_lights_visible_mesh_nodes {nullptr};        // point_lights 可见
        std::vector<RenderMeshNode>*              p_main_camera_visible_mesh_nodes {nullptr};         // main camera 可见
        RenderAxisNode*                           p_axis_node {nullptr};                              // 坐标轴节点
    };

    // 定义RenderPass的模板，对Vulkan的相关对象和概念进行了再包装
    // 为RenderPass提供了通用的基类支持
    class RenderPass : public RenderPassBase
    {
    public:
        struct FrameBufferAttachment
        {
            RHIImage*        image;
            RHIDeviceMemory* mem;
            RHIImageView*    view;
            RHIFormat       format;
        };

        struct Framebuffer
        {
            int           width;
            int           height;
            RHIFramebuffer* framebuffer;
            RHIRenderPass*  render_pass;

            std::vector<FrameBufferAttachment> attachments;
        };

        struct Descriptor
        {
            RHIDescriptorSetLayout* layout;
            RHIDescriptorSet*       descriptor_set;
        };

        struct RenderPipelineBase
        {
            RHIPipelineLayout* layout;
            RHIPipeline*       pipeline;
        };

        
        GlobalRenderResource*      m_global_render_resource {nullptr};   // 记录了可能有用的全局资源

        std::vector<Descriptor>         m_descriptor_infos;
        std::vector<RenderPipelineBase> m_render_pipelines;
        Framebuffer                     m_framebuffer;

        void initialize(const RenderPassInitInfo* init_info) override;
        void postInitialize() override;

        virtual void draw();

        virtual RHIRenderPass*                       getRenderPass() const;
        virtual std::vector<RHIImageView*>           getFramebufferImageViews() const;
        virtual std::vector<RHIDescriptorSetLayout*> getDescriptorSetLayouts() const;

        static VisiableNodes m_visiable_nodes;

    private:
    };
} // namespace Piccolo
