//
// Created by Zach Lee on 2023/8/20.
//

#include <render/RenderPipeline.h>
#include <render/RHI.h>
#include <render/rdg/TransientObjectPool.h>
#include <render/rdg/TransientMemoryPool.h>

#include <render/rdg/RenderGraphVisitors.h>
#include <render/rdg/RenderGraphExecutor.h>
#include <render/rdg/AccessGraphCompiler.h>
#include <render/rdg/RenderResourceCompiler.h>
#include <render/rdg/RenderSceneVisitor.h>

namespace sky {

    RenderPipeline::RenderPipeline()
    {
        rdgContext = std::make_unique<rdg::RenderGraphContext>();
        rdgContext->pool = std::make_unique<rdg::TransientObjectPool>();
        rdgContext->device = RHI::Get()->GetDevice();
        rdgContext->mainCommandBuffer = rdgContext->device->CreateCommandBuffer({});

        rhi::Fence::Descriptor fenceDesc = {};
        fenceDesc.createSignaled = true;
        rdgContext->fence = rdgContext->device->CreateFence(fenceDesc);
        rdgContext->renderFinish = rdgContext->device->CreateSema({});
    }

    void RenderPipeline::FrameSync()
    {
        rdgContext->fence->WaitAndReset();
        rdgContext->imageAvailableSemaPool.Reset();
        rdgContext->pool->ResetPool();
    }

    void RenderPipeline::Execute(rdg::RenderGraph &rdg)
    {
        using namespace rdg;
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

//        {
//            RenderSceneVisitor sceneVisitor(rdg);
//            sceneVisitor.BuildRenderQueue();
//        }


        auto *queue = rdgContext->device->GetQueue(rhi::QueueType::GRAPHICS);
        {
            rdgContext->mainCommandBuffer->Begin();
            RenderGraphExecutor executor(rdg);
            PmrVector<boost::default_color_type> colors(rdg.vertices.size(), &rdg.context->resources);
            boost::depth_first_search(rdg.graph, executor, ColorMap(colors));
            rdgContext->mainCommandBuffer->End();

            rhi::SubmitInfo submitInfo = {};
            submitInfo.submitSignals.emplace_back(rdgContext->renderFinish);

            auto &semaPool = rdgContext->imageAvailableSemaPool;
            for (uint32_t i = 0; i < semaPool.index; ++i) {
                submitInfo.waits.emplace_back(rhi::PipelineStageBit::COLOR_OUTPUT, semaPool.imageAvailableSemaList[i]);
            }
            submitInfo.fence = rdgContext->fence;
            rdgContext->mainCommandBuffer->Submit(*queue, submitInfo);
        }

        {
            rhi::PresentInfo presentInfo = {};
            presentInfo.semaphores.emplace_back(rdgContext->renderFinish);
            for (auto &swc : rdg.presentPasses) {
                auto &res = rdg.resourceGraph.swapChains[Index(swc.imageID, rdg.resourceGraph)];
                presentInfo.imageIndex = res.desc.imageIndex;
                swc.swapChain->Present(*queue, presentInfo);
            }
        }
    }

} // namespace sky
