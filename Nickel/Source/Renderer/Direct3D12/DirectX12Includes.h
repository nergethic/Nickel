#pragma once

#pragma warning(push, 0)
#pragma comment(lib, "d3d12.lib")
#pragma warning(pop)

#pragma error(push, 0)
#include "../include/d3d12.h"
#include "../include/d3dx12.h"
#pragma error(pop)

#include "../CommonDirectXIncludes.h"

#include "../RendererInterface.h"
#include "../../Window.h"

namespace Nickel::Renderer::DXLayer12 {
#include "DirectXTK12/WICTextureLoader.h"
}
