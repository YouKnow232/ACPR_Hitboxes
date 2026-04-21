#pragma once

#include "graphicsTypes.h"


namespace ACPRHitboxes {
    
    int CombineGeometry(
        Vertex* inBuffer, int numVertices,
        Vertex* outBuffer, int outBufferSize,
        float borderOffset
    );
}
