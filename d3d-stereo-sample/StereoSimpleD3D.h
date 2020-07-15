#pragma once
#include "DirectXBase.h"
#include "SampleOverlay.h"

// The constant buffer that is used with the DirectXMath library to draw the cube.
struct ConstantBuffer
{
    DirectX::XMFLOAT4X4 model;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
};

class StereoSimpleD3D : public DirectXBase
{
public:
    StereoSimpleD3D();

    virtual void CreateDeviceIndependentResources() override;
    virtual void CreateDeviceResources() override;
    virtual void CreateWindowSizeDependentResources() override;
    virtual void Render() override;

    float GetStereoExaggeration();
    void RenderEye(_In_ unsigned int eyeIndex);
    void SetStereoExaggeration(_In_ float currentExaggeration);
    void Update(unsigned int eyeIndex, float timeTotal, float timeDelta);

private:
    std::unique_ptr<SampleOverlay> m_sampleOverlay;
    winrt::com_ptr<ID3D11InputLayout>           m_inputLayout;                // cube vertex input layout
    winrt::com_ptr<ID3D11Buffer>                m_vertexBuffer;               // cube vertex buffer
    winrt::com_ptr<ID3D11Buffer>                m_indexBuffer;                // cube index buffer
    winrt::com_ptr<ID3D11VertexShader>          m_vertexShader;               // cube vertex shader
    winrt::com_ptr<ID3D11PixelShader>           m_pixelShader;                // cube pixel shader
    winrt::com_ptr<ID3D11ShaderResourceView>    m_textureShaderResourceView;  // cube texture view
    winrt::com_ptr<ID3D11SamplerState>          m_sampler;                    // cube texture sampler
    winrt::com_ptr<ID3D11Buffer>                m_constantBuffer;             // constant buffer resource
    winrt::com_ptr<ID2D1SolidColorBrush>        m_brush;                      // brush for message drawing
    winrt::com_ptr<IDWriteTextFormat>           m_textFormat;                 // text format for message drawing

    unsigned int             m_indexCount;                  // cube index count
    ConstantBuffer           m_constantBufferData;          // constant buffer resource data
    float                    m_projAspect;                  // aspect ratio for projection matrix
    float                    m_nearZ;                       // nearest Z-distance at which to draw vertices
    float                    m_farZ;                        // farthest Z-distance at which to draw vertices
    float                    m_widthInInches;               // estimated screen width in inches
    float                    m_heightInInches;              // estimated screen height in inches
    float                    m_stereoExaggerationFactor;    // stereo effect that is user adjustable
    std::wstring             m_hintMessage;                 // hint message about customer manipulation
    float                    m_worldScale;                  // developer specified world unit
};