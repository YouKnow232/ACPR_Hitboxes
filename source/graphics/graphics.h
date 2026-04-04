#include <d3d9.h>
#include "directx/DirectXMath.h"
#include "graphicsTypes.h"

namespace ACPRHitboxes {
    #define RECTANGLE_LIMIT 512
    #define VERTICES_PER_RECTANGLE 6

    void SetRenderContext(IDirect3DDevice9* device);
    HRESULT InitGraphics(IDirect3DDevice9* device);
    void DrawTriListPrimitive(IDirect3DDevice9* device, Vertex* vertices, int numVert, DirectX::XMMATRIX& transform);
}
