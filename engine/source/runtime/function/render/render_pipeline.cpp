#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

#include "runtime/function/render/passes/color_grading_pass.h"
#include "runtime/function/render/passes/vignette_pass.h"
#include "runtime/function/render/passes/combine_ui_pass.h"
#include "runtime/function/render/passes/directional_light_pass.h"
#include "runtime/function/render/passes/main_camera_pass.h"
#include "runtime/function/render/passes/pick_pass.h"
#include "runtime/function/render/passes/point_light_pass.h"
#include "runtime/function/render/passes/tone_mapping_pass.h"
#include "runtime/function/render/passes/ui_pass.h"
#include "runtime/function/render/passes/particle_pass.h"

#include "runtime/function/render/debugdraw/debug_draw_manager.h"

#include "runtime/core/base/macro.h"

namespace Piccolo
{
    // 在render_system内调用
    void RenderPipeline::initialize(RenderPipelineInitInfo init_info)
    {
        // 初始化会用到的pass/subpass
        m_point_light_shadow_pass = std::make_shared<PointLightShadowPass>();
        m_directional_light_pass  = std::make_shared<DirectionalLightShadowPass>();
        m_main_camera_pass        = std::make_shared<MainCameraPass>();
        m_tone_mapping_pass       = std::make_shared<ToneMappingPass>();
        m_color_grading_pass      = std::make_shared<ColorGradingPass>();
        m_vignette_pass           = std::make_shared<VignettePass>();
        m_ui_pass                 = std::make_shared<UIPass>();
        m_combine_ui_pass         = std::make_shared<CombineUIPass>();
        m_pick_pass               = std::make_shared<PickPass>();
        m_fxaa_pass               = std::make_shared<FXAAPass>();
        m_particle_pass           = std::make_shared<ParticlePass>();

        // RenderPassCommonInfo 包含rhi和rendersource的信息，在render_pass_base.cpp内定义
        RenderPassCommonInfo pass_common_info;
        pass_common_info.rhi             = m_rhi;
        pass_common_info.render_resource = init_info.render_resource;

        // 将rhi和renderResource信息传给各个Pass
        m_point_light_shadow_pass->setCommonInfo(pass_common_info);
        m_directional_light_pass->setCommonInfo(pass_common_info);
        m_main_camera_pass->setCommonInfo(pass_common_info);
        m_tone_mapping_pass->setCommonInfo(pass_common_info);
        m_color_grading_pass->setCommonInfo(pass_common_info);
        m_vignette_pass->setCommonInfo(pass_common_info);
        m_ui_pass->setCommonInfo(pass_common_info);
        m_combine_ui_pass->setCommonInfo(pass_common_info);
        m_pick_pass->setCommonInfo(pass_common_info);
        m_fxaa_pass->setCommonInfo(pass_common_info);
        m_particle_pass->setCommonInfo(pass_common_info);

        // 计算ShadowMap的两个pass初始化
        m_point_light_shadow_pass->initialize(nullptr);
        m_directional_light_pass->initialize(nullptr);

        // 静态转型
        std::shared_ptr<MainCameraPass> main_camera_pass = std::static_pointer_cast<MainCameraPass>(m_main_camera_pass);
        std::shared_ptr<RenderPass>     _main_camera_pass = std::static_pointer_cast<RenderPass>(m_main_camera_pass); // 为了可以调用基类RenderPass的函数，用来get它的其他subpass
        std::shared_ptr<ParticlePass> particle_pass = std::static_pointer_cast<ParticlePass>(m_particle_pass);

        // 初始化Particle Pass
        ParticlePassInitInfo particle_init_info{};
        particle_init_info.m_particle_manager = g_runtime_global_context.m_particle_manager;
        m_particle_pass->initialize(&particle_init_info);

        // 初始化Shadow Mapping Pass的 Image View
        main_camera_pass->m_point_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_point_light_shadow_pass)->getFramebufferImageViews()[0];
        main_camera_pass->m_directional_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_directional_light_pass)->m_framebuffer.attachments[0].view;

        // 为main_camera 设置fxaa和Particle
        MainCameraPassInitInfo main_camera_init_info;
        main_camera_init_info.enble_fxaa = init_info.enable_fxaa;
        main_camera_pass->setParticlePass(particle_pass);
        m_main_camera_pass->initialize(&main_camera_init_info);

        std::static_pointer_cast<ParticlePass>(m_particle_pass)->setupParticlePass();

        // 设置Shadow Mapping Pass的descriptor_layouts,并初始化
        std::vector<RHIDescriptorSetLayout*> descriptor_layouts = _main_camera_pass->getDescriptorSetLayouts();
        std::static_pointer_cast<PointLightShadowPass>(m_point_light_shadow_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);
        std::static_pointer_cast<DirectionalLightShadowPass>(m_directional_light_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);

        m_point_light_shadow_pass->postInitialize();
        m_directional_light_pass->postInitialize();

        // 初始化ToneMappingPass
        ToneMappingPassInitInfo tone_mapping_init_info;
        tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
        tone_mapping_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        m_tone_mapping_pass->initialize(&tone_mapping_init_info);

        // 初始化ColorGradingPass
        ColorGradingPassInitInfo color_grading_init_info;
        color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
        color_grading_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_color_grading_pass->initialize(&color_grading_init_info);

        // 初始化VignettePass
        VignettePassInitInfo vignette_init_info;
        vignette_init_info.render_pass = _main_camera_pass->getRenderPass();
        vignette_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        m_vignette_pass->initialize(&vignette_init_info);

        // 初始化UIPass
        UIPassInitInfo ui_init_info;
        ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        m_ui_pass->initialize(&ui_init_info);

        // 初始化CombineUIPass
        CombineUIPassInitInfo combine_ui_init_info;
        combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        combine_ui_init_info.scene_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        combine_ui_init_info.ui_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        m_combine_ui_pass->initialize(&combine_ui_init_info);

        // 初始化PickPass
        PickPassInitInfo pick_init_info;
        pick_init_info.per_mesh_layout = descriptor_layouts[MainCameraPass::LayoutType::_per_mesh];
        m_pick_pass->initialize(&pick_init_info);

        // 初始化FXAAPass
        FXAAPassInitInfo fxaa_init_info;
        fxaa_init_info.render_pass = _main_camera_pass->getRenderPass();
        fxaa_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_post_process_buffer_even];
        m_fxaa_pass->initialize(&fxaa_init_info);

    }

    // Familiar Vulkan!!! 
    void RenderPipeline::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        vulkan_rhi->waitForFences();

        vulkan_rhi->resetCommandPool();

        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }

        // draw shadowmap pass
        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();
        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        // contain several sub pass
        // get 返回的是subpass的指针, 通过static_cast 和解引用得到对象来初始化该subpass的引用
        // 这里需要使用static_cast还有点疑问
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        VignettePass&     vignette_pass      = *(static_cast<VignettePass*>(m_vignette_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());

        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->drawForward(color_grading_pass,
                          vignette_pass,
                          fxaa_pass,
                          tone_mapping_pass,
                          ui_pass,
                          combine_ui_pass,
                          particle_pass,
                          vulkan_rhi->m_current_swapchain_image_index);

        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
    }

    // forward的是drawForward，deffered的是draw
    void RenderPipeline::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        vulkan_rhi->waitForFences();

        vulkan_rhi->resetCommandPool();

        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }

        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();
        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        VignettePass&     vignette_pass      = *(static_cast<VignettePass*>(m_vignette_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());

        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->draw(color_grading_pass,
                   vignette_pass,
                   fxaa_pass,
                   tone_mapping_pass,
                   ui_pass,
                   combine_ui_pass,
                   particle_pass,
                   vulkan_rhi->m_current_swapchain_image_index);
                   
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
    }


    // 有odd和even两套framebuffer
    // 有待解决 怎么判断该用哪个buffer odd还是even
    void RenderPipeline::passUpdateAfterRecreateSwapchain()
    {
        MainCameraPass&   main_camera_pass   = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        VignettePass& vignette_pass          = *(static_cast<VignettePass*>(m_vignette_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        PickPass&         pick_pass          = *(static_cast<PickPass*>(m_pick_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        // main_camera_pass 先更新
        // odd 和 even 是subpass之间的一些缓冲区
        main_camera_pass.updateAfterFramebufferRecreate();
        tone_mapping_pass.updateAfterFramebufferRecreate(main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);
        color_grading_pass.updateAfterFramebufferRecreate(main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        vignette_pass.updateAfterFramebufferRecreate(main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);
        fxaa_pass.updateAfterFramebufferRecreate(main_camera_pass.getFramebufferImageViews()[_main_camera_pass_post_process_buffer_even]);
        combine_ui_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even],
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);
        pick_pass.recreateFramebuffer();
        particle_pass.updateAfterFramebufferRecreate();
        g_runtime_global_context.m_debugdraw_manager->updateAfterRecreateSwapchain();
    }

    // 通过UV坐标获得选中物体的id
    uint32_t RenderPipeline::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        PickPass& pick_pass = *(static_cast<PickPass*>(m_pick_pass.get()));
        return pick_pass.pick(picked_uv);
    }

    // 设置是否显示坐标轴
    void RenderPipeline::setAxisVisibleState(bool state)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_is_show_axis  = state;
    }

    // 设置selected_axis
    void RenderPipeline::setSelectedAxis(size_t selected_axis)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_selected_axis = selected_axis;
    }
} // namespace Piccolo
