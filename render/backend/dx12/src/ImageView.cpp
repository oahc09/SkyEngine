//
// Created by Zach Lee on 2022/11/5.
//

#include <dx12/ImageView.h>

namespace sky::dx {

    ImageView::ImageView(Device &dev) : DevObject(dev)
    {
    }

    ImageView::~ImageView()
    {
    }

    std::shared_ptr<rhi::ImageView> ImageView::CreateView(const rhi::ImageViewDesc &desc) const
    {
        return source->CreateView(desc);
    }

    rhi::PixelFormat ImageView::GetFormat() const
    {
        return source->GetDescriptor().format;
    }

    const rhi::Extent3D &ImageView::GetExtent() const
    {
        return source->GetDescriptor().extent;
    }

}