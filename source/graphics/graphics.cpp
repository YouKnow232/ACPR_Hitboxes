#include "graphics.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <d3dcompiler.h>
#include "gearLoader/ggxxacpr.hpp"
#include "logging/logging.h"
#include "settings/settings.h"

namespace ACPRHitboxes {

    constexpr const char* vsCompileTarget = "vs_3_0";
    constexpr const char* psCompileTarget = "ps_3_0";
    
    static IDirect3DVertexBuffer9* vertBuffer;
    static IDirect3DVertexDeclaration9* vertDeclaration;
    
    static IDirect3DVertexShader9* hitboxVS;
    static IDirect3DPixelShader9* hitboxPS;
    
    static const D3DVERTEXELEMENT9 vertDeclarationElements[] = {
        {0, offsetof(Vertex, Position), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION},
        {0, offsetof(Vertex, Color), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR},
        {0, offsetof(Vertex, UV), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD},
        D3DDECL_END()
    };

    inline bool CheckError(HRESULT result, std::string msg) {
        if (result != D3D_OK) {
            std::stringstream ss;
            ss << msg << " 0x" << std::hex << result;
            GetLogger().Log(GearLoader::LogLevel::ERR, ss.str());
            return true;
        }
        return false;
    }
    
    inline HRESULT InitBuffers(IDirect3DDevice9* device) {
        HRESULT result = device->CreateVertexBuffer(
            RECTANGLE_LIMIT * sizeof(Vertex) * VERTICES_PER_RECTANGLE,  // Size
            0,  // Usage
            0,  // VFV
            D3DPOOL_MANAGED,
            &vertBuffer,
            nullptr
        );
        if (CheckError(result, "CreateVertexBuffer failed")) return result;
        
        result = device->CreateVertexDeclaration(&vertDeclarationElements[0], &vertDeclaration);
        if (CheckError(result, "CreateVertexDeclaration failed")) return result;
        
        return result;
    }
    inline HRESULT CompileShader(IDirect3DDevice9* device, std::filesystem::path path, std::string entry, IDirect3DVertexShader9** shader) {
        ID3DBlob* shaderBlob;
        ID3DBlob* shaderErr;

        HRESULT result = D3DCompileFromFile(
            path.wstring().c_str(), NULL, NULL, entry.c_str(),
            vsCompileTarget, 0, 0, &shaderBlob, &shaderErr
        );
        if (CheckError(result, "D3DCompileFromFile failed")) {
            if (shaderErr) GetLogger().Log(
                GearLoader::LogLevel::ERR,
                static_cast<const char*>(shaderErr->GetBufferPointer()));
        }
        
        result = device->CreateVertexShader(reinterpret_cast<DWORD*>(shaderBlob->GetBufferPointer()), shader);
        CheckError(result, "CreatePixelShader failed");

        return result;
    }
    inline HRESULT CompileShader(IDirect3DDevice9* device, std::filesystem::path path, std::string entry, IDirect3DPixelShader9** shader) {
        ID3DBlob* shaderBlob;
        ID3DBlob* shaderErr;

        HRESULT result = D3DCompileFromFile(
            path.wstring().c_str(), NULL, NULL, entry.c_str(),
            psCompileTarget, 0, 0, &shaderBlob, &shaderErr
        );
        if (CheckError(result, "D3DCompileFromFile failed")) {
            if (shaderErr) GetLogger().Log(
                GearLoader::LogLevel::ERR,
                static_cast<const char*>(shaderErr->GetBufferPointer()));
        }

        result = device->CreatePixelShader(reinterpret_cast<DWORD*>(shaderBlob->GetBufferPointer()), shader);
        CheckError(result, "CreatePixelShader failed");

        return result;
    }
    HRESULT CompileShaders(IDirect3DDevice9* device) {
        HRESULT result;
        std::filesystem::path hitboxShaderPath =
            std::filesystem::current_path() / "mods\\Hitboxes\\shaders\\HitboxShader.hlsl";

        result = CompileShader(device, hitboxShaderPath, "HitboxVS", &hitboxVS);
        if (result != 0) { return result; }
        result = CompileShader(device, hitboxShaderPath, "HitboxPS", &hitboxPS);
        if (result != 0) { return result; }
        
        return result;
    }
    
    
    HRESULT InitGraphics(IDirect3DDevice9* device) {
        HRESULT result;
        
        result = InitBuffers(device);
        if (result != 0) return result;
        result = CompileShaders(device);
        if (result != 0) return result;
        
        return result;
    }
    
    inline float* GetScreenSize(IDirect3DDevice9* device) {
        static float screenSize[2];
        D3DVIEWPORT9 vp;
        HRESULT result = device->GetViewport(&vp);
        CheckError(result, "get viewport failed");
        screenSize[0] = static_cast<float>(vp.Width);
        screenSize[1] = static_cast<float>(vp.Height);
        return screenSize;
    }
    
    void SetRenderContext(IDirect3DDevice9* device) {
        HRESULT result;

        result = device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        CheckError(result, "Set D3DRS_ALPHABLENDENABLE failed");
        result = device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        CheckError(result, "Set D3DRS_DESTBLEN failed");
        result = device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CheckError(result, "Set D3DRS_BLENDOP failed");
        // result = device->SetRenderState(D3DRS_SCISSORTESTENABLE, /* check settings*/ FALSE);
        // CheckError(result, "");
        result = device->SetVertexDeclaration(vertDeclaration);
        CheckError(result, "Set vert declaration failed");
        result = device->SetStreamSource(0, vertBuffer, 0, sizeof(Vertex));
        CheckError(result, "Set stream source failed");
        result = device->SetVertexShader(hitboxVS);
        CheckError(result, "Set vert shader failed");
        result = device->SetVertexShaderConstantF(4, GetScreenSize(device), 2);
        CheckError(result, "Set vert shader constant 4 failed");
        result = device->SetPixelShader(hitboxPS);
        CheckError(result, "Set pixel shader failed");
        result = device->SetPixelShaderConstantF(5, &SettingsManager::GetInstance().HitboxBorderThickness, 1);
        CheckError(result, "Set pixel shader constant 5 failed");
    }
    
    void DrawTriListPrimitive(IDirect3DDevice9* device, Vertex *vertices, int numVert, DirectX::XMMATRIX& transform) {
        void* bufferPtr;
        HRESULT result;

        result = vertBuffer->Lock(0, numVert * sizeof(Vertex), &bufferPtr, D3DLOCK_DISCARD);
        CheckError(result, "vertBuffer->Lock() failed");
        memcpy(bufferPtr, vertices, numVert * sizeof(Vertex));
        result = vertBuffer->Unlock();
        CheckError(result, "vertBuffer->Unlock() failed");
        DirectX::XMFLOAT4X4 transformBuffer;
        DirectX::XMStoreFloat4x4(&transformBuffer, DirectX::XMMatrixTranspose(transform));
        device->SetVertexShaderConstantF(0, reinterpret_cast<float*>(&transformBuffer), 4);
        device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, numVert / 3);
    }
    
}
