#pragma once
#include "Renderer/DX11Layer.h"

namespace Nickel::Renderer {
    struct VertexBuffer {
        std::unique_ptr<ID3D11Buffer, DxDeleter> buffer;
        //ID3D11Buffer* buffer;
        UINT stride;
        UINT offset;
        bool isDynamic;

        template <typename VertexT>
        void Create(ID3D11Device1* device, std::span<VertexT> vertexData, bool dynamic) {
            Assert(buffer.get() == nullptr);
            auto data = D3D11_SUBRESOURCE_DATA{ .pSysMem = vertexData.data() };

            auto rawBuffer = DXLayer11::CreateVertexBuffer(device, (sizeof(VertexT) * vertexData.size()), dynamic, &data);
            //buffer = rawBuffer;
            buffer = std::unique_ptr<ID3D11Buffer, DxDeleter>(rawBuffer);
            stride = sizeof(VertexT);
            offset = 0;
            isDynamic = dynamic;
        }

        template <typename VertexT>
        void Update(ID3D11DeviceContext1* ctx, std::span<VertexT const*> vertexData) {
            auto& b = b.get();
            if constexpr (_DEBUG) {
                Assert(isDynamic);
                D3D11_BUFFER_DESC buffer_desc;
                b->GetDesc(&buffer_desc);
                Assert(buffer_desc.Usage == D3D11_USAGE_DYNAMIC && buffer_desc.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE);
            }

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            void* dataPtr;
            ctx->Map(b, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            dataPtr = mappedResource.pData;
            memcpy(dataPtr, vertexData.data(), vertexData.size());
            ctx->Unmap(b, 0);
        }
    }; 
}