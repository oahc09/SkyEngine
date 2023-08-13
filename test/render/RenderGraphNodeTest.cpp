//
// Created by Zach Lee on 2021/12/26.
//

#include <core/logger/Logger.h>
#include <gtest/gtest.h>
#include <render/RHI.h>
#include <render/rdg/RenderGraph.h>
#include <render/rdg/RenderGraphVisitors.h>
#include <render/rdg/RenderGraphExecutor.h>
#include <render/rdg/AccessGraphCompiler.h>
#include <render/rdg/RenderResourceCompiler.h>
#include <render/rdg/TransientObjectPool.h>

#include <memory>

using namespace sky;
using namespace sky::rdg;

TEST(RenderGraphTest, NodeGraphTest01)
{
#ifdef __APPLE__
    RHI::Get()->InitInstance({"test", "", true, rhi::API::METAL});
#else
    RHI::Get()->InitInstance({"test", "", true, rhi::API::VULKAN});
#endif
    RHI::Get()->InitDevice({});

    RenderGraphContext context;
    context.pool = std::make_unique<TransientObjectPool>();
    context.device = RHI::Get()->GetDevice();
    context.mainCommandBuffer = context.device->CreateCommandBuffer({});

    RenderGraph rdg(&context);
    auto       &rg = rdg.resourceGraph;


    /**
     *                    color0            color1           color2
     *                         [                  ]
     *  test                                     R
     *    test_1
     *      test_1_1           W
     *      test_1_2           W
     *    test_2
     *      test_2_1           W
     *      test_2_2           W
     *                         [                                   ]
     *  test2                  W                RW               RW
     */


    rg.AddImage("test",
        GraphImage{{128, 128, 1}, 2, 2, rhi::PixelFormat::RGBA8_UNORM, rhi::ImageUsageFlagBit::RENDER_TARGET | rhi::ImageUsageFlagBit::SAMPLED, rhi::SampleCount::X1, rhi::ImageViewType::VIEW_2D_ARRAY});
    rg.AddImageView("test_1", "test", GraphImageView{{0, 1, 0, 2, rhi::AspectFlagBit::COLOR_BIT, rhi::ImageViewType::VIEW_2D_ARRAY}});
    rg.AddImageView("test_1_1", "test_1", GraphImageView{{0, 1, 0, 1, rhi::AspectFlagBit::COLOR_BIT}});
    rg.AddImageView("test_1_2", "test_1", GraphImageView{{0, 1, 1, 1, rhi::AspectFlagBit::COLOR_BIT}});
    rg.AddImageView("test_2", "test", GraphImageView{{1, 1, 0, 2, rhi::AspectFlagBit::COLOR_BIT, rhi::ImageViewType::VIEW_2D_ARRAY}});
    rg.AddImageView("test_2_1", "test_2", GraphImageView{{1, 1, 0, 1, rhi::AspectFlagBit::COLOR_BIT}});
    rg.AddImageView("test_2_2", "test_2", GraphImageView{{1, 1, 1, 1, rhi::AspectFlagBit::COLOR_BIT}});
    rg.AddImage("test2",
                GraphImage{{128, 128, 1}, 1, 1, rhi::PixelFormat::D24_S8, rhi::ImageUsageFlagBit::DEPTH_STENCIL | rhi::ImageUsageFlagBit::SAMPLED});

    auto pass1 = rdg.AddRasterPass("color0", 128, 128)
        .AddAttachment({"test_1_1", rhi::LoadOp::CLEAR, rhi::StoreOp::STORE}, rhi::ClearValue(0.f, 0.f, 0.f, 0.f))
        .AddAttachment({"test_1_2", rhi::LoadOp::CLEAR, rhi::StoreOp::STORE}, rhi::ClearValue(0.f, 0.f, 0.f, 0.f))
        .AddAttachment({"test_2_1", rhi::LoadOp::CLEAR, rhi::StoreOp::STORE}, rhi::ClearValue(0.f, 0.f, 0.f, 0.f))
        .AddAttachment({"test_2_2", rhi::LoadOp::CLEAR, rhi::StoreOp::STORE}, rhi::ClearValue(0.f, 0.f, 0.f, 0.f))
        .AddAttachment({"test2", rhi::LoadOp::CLEAR, rhi::StoreOp::STORE}, rhi::ClearValue(1.f, 0));
    pass1.AddRasterSubPass("color0_sub0")
        .AddColor("test_1_1", ResourceAccessBit::WRITE)
        .AddColor("test_1_2", ResourceAccessBit::WRITE)
        .AddColor("test_2_1", ResourceAccessBit::WRITE)
        .AddColor("test_2_2", ResourceAccessBit::WRITE)
        .AddDepthStencil("test2", ResourceAccessBit::WRITE)
        .AddQueue("queue1", "v1");

    auto pass2 = rdg.AddRasterPass("color1", 128, 128)
        .AddAttachment({"test2", rhi::LoadOp::LOAD, rhi::StoreOp::STORE}, rhi::ClearValue(1.f, 0));
    pass2.AddRasterSubPass("color1_sub0")
        .AddDepthStencil("test2", ResourceAccessBit::WRITE)
        .AddComputeView("test", {"_", ComputeType::SRV, ResourceAccessBit::READ});

    auto pass3 = rdg.AddRasterPass("color2", 128, 128)
        .AddAttachment({"test2", rhi::LoadOp::LOAD, rhi::StoreOp::STORE}, rhi::ClearValue(1.f, 0));
    pass3.AddRasterSubPass("color2_sub0")
        .AddDepthStencil("test2", ResourceAccessBit::WRITE);

    {
        AccessCompiler             compiler(rdg);
        PmrVector<boost::default_color_type> colors(rdg.accessGraph.vertices.size(), &rdg.context->resources);
        boost::depth_first_search(rdg.accessGraph.graph, compiler, ColorMap(colors));
    }


    {
        RenderResourceCompiler               compiler(rdg);
        PmrVector<boost::default_color_type> colors(rdg.vertices.size(), &rdg.context->resources);
        boost::depth_first_search(rdg.graph, compiler, ColorMap(colors));
    }

    {
        RenderGraphPassCompiler              compiler(rdg);
        PmrVector<boost::default_color_type> colors(rdg.vertices.size(), &rdg.context->resources);
        boost::depth_first_search(rdg.graph, compiler, ColorMap(colors));
    }

    {
        context.mainCommandBuffer->Begin();
        RenderGraphExecutor executor(rdg);
        PmrVector<boost::default_color_type> colors(rdg.vertices.size(), &rdg.context->resources);
        boost::depth_first_search(rdg.graph, executor, ColorMap(colors));
        context.mainCommandBuffer->End();

        auto *queue = context.device->GetQueue(rhi::QueueType::GRAPHICS);
        context.mainCommandBuffer->Submit(*queue, {});
    }
    context.device->WaitIdle();
}
