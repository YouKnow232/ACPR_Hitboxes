#pragma once

#include <d3d9.h>
#include "baseMod/baseMod.hpp"

namespace ACPRHitboxes{
    using DrawDelegate = void(*)(BaseMod::Api* api, IDirect3DDevice9* device);

    void RenderFrame(BaseMod::Api* bmApi, IDirect3DDevice9* device);
}