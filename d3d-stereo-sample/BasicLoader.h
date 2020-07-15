#pragma once
#include "BasicReaderWriter.h"

// A simple loader class that provides support for loading shaders, textures,
// and meshes from files on disk. Provides synchronous and asynchronous methods.
class BasicLoader
{
public:
    BasicLoader(
        winrt::com_ptr<ID3D11Device> const& d3dDevice,
        winrt::com_ptr<IWICImagingFactory2> wicFactory
    );

    void LoadTexture(
        std::wstring const& filename,
        _Out_opt_ ID3D11Texture2D** texture,
        _Out_opt_ ID3D11ShaderResourceView** textureView
    );

    winrt::Windows::Foundation::IAsyncAction LoadTextureAsync(
        std::wstring const& filename,
        _Out_opt_ ID3D11Texture2D** texture,
        _Out_opt_ ID3D11ShaderResourceView** textureView
    );

    void LoadShader(
        std::wstring const& filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    );

    void LoadShader(
        std::wstring const& filename,
        _Out_ ID3D11PixelShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _Out_ ID3D11PixelShader** shader
    );

    void LoadShader(
        std::wstring const& filename,
        _Out_ ID3D11ComputeShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _Out_ ID3D11ComputeShader** shader
    );

    void LoadShader(
        std::wstring const& filename,
        _Out_ ID3D11GeometryShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _Out_ ID3D11GeometryShader** shader
    );

    void LoadShader(
        std::wstring const& filename,
        _In_reads_opt_(numEntries) const D3D11_SO_DECLARATION_ENTRY* streamOutDeclaration,
        _In_ uint32_t numEntries,
        _In_reads_opt_(numStrides) const uint32_t* bufferStrides,
        _In_ uint32_t numStrides,
        _In_ uint32_t rasterizedStream,
        _Out_ ID3D11GeometryShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _In_reads_opt_(numEntries) const D3D11_SO_DECLARATION_ENTRY* streamOutDeclaration,
        _In_ uint32_t numEntries,
        _In_reads_opt_(numStrides) const uint32_t* bufferStrides,
        _In_ uint32_t numStrides,
        _In_ uint32_t rasterizedStream,
        _Out_ ID3D11GeometryShader** shader
    );

    void LoadShader(
        std::wstring const& filename,
        _Out_ ID3D11HullShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _Out_ ID3D11HullShader** shader
    );

    void LoadShader(
        std::wstring const& filename,
        _Out_ ID3D11DomainShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        std::wstring const& filename,
        _Out_ ID3D11DomainShader** shader
    );

    void LoadMesh(
        std::wstring const& filename,
        _Out_ ID3D11Buffer** vertexBuffer,
        _Out_ ID3D11Buffer** indexBuffer,
        _Out_opt_ uint32_t* vertexCount,
        _Out_opt_ uint32_t* indexCount
    );

    winrt::Windows::Foundation::IAsyncAction LoadMeshAsync(
        std::wstring const& filename,
        _Out_ ID3D11Buffer** vertexBuffer,
        _Out_ ID3D11Buffer** indexBuffer,
        _Out_opt_ uint32_t* vertexCount,
        _Out_opt_ uint32_t* indexCount
    );

private:
    winrt::com_ptr<ID3D11Device> m_d3dDevice;
    winrt::com_ptr<IWICImagingFactory2> m_wicFactory;
    std::unique_ptr<BasicReaderWriter> m_basicReaderWriter;

    template <class DeviceChildType>
    inline void SetDebugName(
        _In_ DeviceChildType* object,
        std::wstring const& name
    );

    std::wstring GetExtension(
        std::wstring const& filename
    );

    void CreateTexture(
        _In_ bool decodeAsDDS,
        _In_reads_bytes_(dataSize) byte* data,
        _In_ uint32_t dataSize,
        _Out_opt_ ID3D11Texture2D** texture,
        _Out_opt_ ID3D11ShaderResourceView** textureView,
        std::wstring const& debugName
    );

    void CreateInputLayout(
        _In_reads_bytes_(bytecodeSize) byte* bytecode,
        _In_ uint32_t bytecodeSize,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11InputLayout** layout
    );

    void CreateMesh(
        _In_ byte* meshData,
        _Out_ ID3D11Buffer** vertexBuffer,
        _Out_ ID3D11Buffer** indexBuffer,
        _Out_opt_ uint32_t* vertexCount,
        _Out_opt_ uint32_t* indexCount,
        std::wstring const& debugName
    );
};
