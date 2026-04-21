#include "geometry.h"
#include <vector>
#include <unordered_set>
#include "clipper2/clipper.h"
#include "CDT.h"
#include "graphicsTypes.h"

using namespace ACPRHitboxes;
namespace C2 = Clipper2Lib;

/* Library Conversions */

inline void EnsureWinding(C2::PathsD& paths, bool winding) {
    for (auto& path : paths) {
        if (C2::IsPositive(path) != winding) {
            std::reverse(path.begin(), path.end());
        }
    }
}
inline bool AlmostEqual(double a, double b, double eps = 1e-9)
{
    return std::abs(a - b) < eps;
}
inline void EliminateDuplicateVertices(C2::PathsD& paths,  double eps = 1e-9) {
    for (auto& path : paths) {
        C2::PathD cleaned;
        
        for (size_t i = 0; i < path.size(); ++i)
        {
            const auto& curr = path[i];
            const auto& prev = path[(i + path.size() - 1) % path.size()];
            
            if (!AlmostEqual(curr.x, prev.x, eps) ||
                !AlmostEqual(curr.y, prev.y, eps))
            {
                cleaned.push_back(curr);
            }
        }
    
        path = std::move(cleaned);
    }
}

inline Vertex ToVertex(CDT::V2d<double> vert, float z, D3DCOLOR color) {
    return {{ static_cast<float>(vert.x), static_cast<float>(vert.y), z }, color};
}

inline C2::PathsD ToPaths(Vertex* buffer, int numVertices) {
    C2::PathsD paths = {};

    for (int i = 0; (i+5) < numVertices; i += 6) {
        C2::PathD path = {};

        path.push_back(C2::PointD(buffer[i+0].Position[0], buffer[i+0].Position[1]));
        path.push_back(C2::PointD(buffer[i+1].Position[0], buffer[i+1].Position[1]));
        path.push_back(C2::PointD(buffer[i+5].Position[0], buffer[i+5].Position[1]));
        path.push_back(C2::PointD(buffer[i+2].Position[0], buffer[i+2].Position[1]));

        paths.push_back(path);
    }

    return paths;
}
inline void ToV2dVector(
    C2::PathsD& paths,
    std::vector<CDT::V2d<double>>& vertices,
    std::vector<CDT::Edge>& edges)
{
    for (auto& path : paths) {
        if (path.size() < 3) continue;

        size_t base = vertices.size();

        for (const auto& p: path) {
            vertices.push_back({p.x, p.y});
        }

        for (size_t i = 0; i < path.size(); i++) {
            size_t a = base + i;
            size_t b = base + (i + 1) % path.size();
            edges.push_back({
                static_cast<CDT::VertInd>(a),
                static_cast<CDT::VertInd>(b),
            });
        }
    }
}
inline int TriangulationToVertices(
    CDT::Triangulation<double>& triangulation,
    Vertex* outBuffer,
    int outBufferIndex,
    int outBufferSize,
    float z,
    D3DCOLOR vertColor
) {
    int i = outBufferIndex;
    for (auto& t : triangulation.triangles) {
        if (i + 3 >= outBufferSize) break;

        outBuffer[i++] = ToVertex(triangulation.vertices[t.vertices[0]], z, vertColor);
        outBuffer[i++] = ToVertex(triangulation.vertices[t.vertices[1]], z, vertColor);
        outBuffer[i++] = ToVertex(triangulation.vertices[t.vertices[2]], z, vertColor);
    }

    return i;
}


int ACPRHitboxes::CombineGeometry(
    Vertex* inBuffer, int numVertices,
    Vertex* outBuffer, int outBufferSize,
    float borderOffset
) {
    C2::PathsD paths = ToPaths(inBuffer, numVertices);
    EnsureWinding(paths, true);
    C2::PathsD unionContours = C2::Union(paths, C2::FillRule::NonZero);

    auto cdt = CDT::Triangulation<double>();

    std::vector<CDT::V2d<double>> triangulationVertices = {};
    std::vector<CDT::Edge> triangulationEdges = {};
    ToV2dVector(unionContours, triangulationVertices, triangulationEdges);

    cdt.insertVertices(triangulationVertices);
    cdt.insertEdges(triangulationEdges);
    cdt.eraseOuterTriangles();

    D3DCOLOR colorSample = inBuffer[0].Color;
    float zSample = inBuffer[0].Position[2];
    int outBufferIndex = TriangulationToVertices(cdt, outBuffer, 0, outBufferSize, zSample, colorSample);

    if (borderOffset <= 0.0f) return outBufferIndex;

    C2::PathsD innerBorderContour = C2::InflatePaths(
        unionContours,
        -borderOffset,
        C2::JoinType::Miter,
        C2::EndType::Polygon,
        2.0f, // miter_limit
        8     // Precision
    );
    EliminateDuplicateVertices(innerBorderContour);
    EnsureWinding(innerBorderContour, false);
    unionContours.insert(unionContours.end(), innerBorderContour.begin(), innerBorderContour.end());

    auto borderCdt = CDT::Triangulation<double>();

    std::vector<CDT::V2d<double>> borderTriangulationVertices = {};
    std::vector<CDT::Edge> borderTriangulationEdges = {};
    ToV2dVector(unionContours, borderTriangulationVertices, borderTriangulationEdges);

    borderCdt.insertVertices(borderTriangulationVertices);
    borderCdt.insertEdges(borderTriangulationEdges);
    borderCdt.eraseOuterTrianglesAndHoles();

    D3DCOLOR borderColor = colorSample | 0xFF000000;
    return TriangulationToVertices(borderCdt, outBuffer, outBufferIndex, outBufferSize, zSample, borderColor);
}
