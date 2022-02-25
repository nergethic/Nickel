#pragma once
#include "Renderer/Direct3D11/DX11Layer.h"

namespace Nickel::Renderer {
    struct IndexBuffer {
        std::unique_ptr<ID3D11Buffer, DxDeleter> buffer;
        // ID3D11Buffer* buffer;
        u32 offset;

        inline void Create(ID3D11Device1* device, std::span<u32> indexData) {
            Assert(buffer.get() == nullptr);
            auto data = D3D11_SUBRESOURCE_DATA{ .pSysMem = indexData.data() };
            auto rawBuffer = DX11Layer::CreateIndexBuffer(device, (sizeof(indexData[0]) * indexData.size()), &data);
            //buffer = rawBuffer;
            buffer = std::unique_ptr<ID3D11Buffer, DxDeleter>(rawBuffer);
            offset = 0;
        }
    };
}