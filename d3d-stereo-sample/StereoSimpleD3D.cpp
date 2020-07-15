#include "pch.h"
#include "StereoSimpleD3D.h"
#include "BasicLoader.h"
#include "BasicShapes.h"
#include "Stereo3DMatrixHelper.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::ViewManagement;
    using namespace Windows::Graphics::Display;
}

StereoSimpleD3D::StereoSimpleD3D()
{
    m_stereoExaggerationFactor = 1.0f;

    // Developer decided world unit: in this case, modeled in feet.
    // One world unit equals 1 foot. Therefore, m_worldScale * inches = 1 world unit.
    m_worldScale = 12.0f;
}

void StereoSimpleD3D::CreateDeviceIndependentResources()
{
    DirectXBase::CreateDeviceIndependentResources();

    // Create a DirectWrite text format object.
    winrt::check_hresult(
        m_dwriteFactory->CreateTextFormat(
            L"Segoe UI",                    // font family name
            nullptr,                        // system font collection
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            40.0f,                          // font size
            L"en-US",                       // locale
            m_textFormat.put()
        )
    );

    // Align the text horizontally.
    winrt::check_hresult(
        m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
    );

    // Align the text vertically.
    winrt::check_hresult(
        m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
    );
}

void StereoSimpleD3D::CreateDeviceResources()
{
    DirectXBase::CreateDeviceResources();

    m_sampleOverlay = std::make_unique<SampleOverlay>();

    m_sampleOverlay->Initialize(
        m_d2dDevice,
        m_d2dContext,
        m_wicFactory,
        m_dwriteFactory,
        L"Direct3D stereoscopic 3D sample"
    );

    winrt::com_ptr<IWICImagingFactory2> wicFactory;
    auto loader = std::make_unique<BasicLoader>(m_d3dDevice, wicFactory);

    loader->LoadShader(
        L"SimpleVertexShader.cso",
        nullptr,
        0,
        m_vertexShader.put(),
        m_inputLayout.put()
    );

    // Create the vertex and index buffers for drawing the cube.
    auto shapes = std::make_unique<BasicShapes>(m_d3dDevice.get());

    shapes->CreateCube(
        m_vertexBuffer.put(),
        m_indexBuffer.put(),
        nullptr,
        &m_indexCount
    );

    // Create the constant buffer for updating model and camera data.
    CD3D11_BUFFER_DESC constantBufferDescription(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        m_d3dDevice->CreateBuffer(
            &constantBufferDescription,
            nullptr, // Leave the buffer uninitialized.
            m_constantBuffer.put()
        )
    );

    loader->LoadShader(
        L"SimplePixelShader.cso",
        m_pixelShader.put()
    );

    loader->LoadTexture(
        L"texture.dds",
        nullptr,
        m_textureShaderResourceView.put()
    );

    // Create the sampler.
    D3D11_SAMPLER_DESC samplerDescription;
    ZeroMemory(&samplerDescription, sizeof(samplerDescription));
    samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.MipLODBias = 0.0f;
    samplerDescription.MaxAnisotropy = m_featureLevel > D3D_FEATURE_LEVEL_9_1 ? 4 : 2;
    samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDescription.BorderColor[0] = 0.0f;
    samplerDescription.BorderColor[1] = 0.0f;
    samplerDescription.BorderColor[2] = 0.0f;
    samplerDescription.BorderColor[3] = 0.0f;
    samplerDescription.MinLOD = 0; // Allow use of all MIP levels.
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

    winrt::check_hresult(
        m_d3dDevice->CreateSamplerState(
            &samplerDescription,
            m_sampler.put()
        )
    );

    winrt::check_hresult(
        m_d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White, 0.5f),
            m_brush.put()
        )
    );
}

void StereoSimpleD3D::CreateWindowSizeDependentResources()
{
    DirectXBase::CreateWindowSizeDependentResources();

    if (m_stereoEnabled)
    {
        m_hintMessage = L"Press up/down arrow keys to adjust stereo 3D exaggeration effect";
    }
    else
    {
        m_hintMessage = L"Stereo 3D is not enabled on your system";
    }

    m_projAspect = static_cast<float>(m_renderTargetSize.Width) / static_cast<float>(m_renderTargetSize.Height);
    m_nearZ = 0.01f;
    m_farZ = 100.0f;

    // Initialize the view matrix.
    DirectX::XMFLOAT3 Eye = DirectX::XMFLOAT3(0.0f, 2.0f, 5.0f);
    DirectX::XMFLOAT3 At = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 Up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    DirectX::XMStoreFloat4x4(
        &m_constantBufferData.view,
        DirectX::XMMatrixLookAtRH(XMLoadFloat3(&Eye), XMLoadFloat3(&At), XMLoadFloat3(&Up))
    );

    // Set camera parameters.
    m_widthInInches = m_renderTargetSize.Width / m_dpi;
    m_heightInInches = m_renderTargetSize.Height / m_dpi;

    // Create the projection matrix parameters and set up the initial/mono projection matrix.
    StereoParameters params = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_worldScale, 0); // Mono uses zero exaggeration.
    DirectX::XMStoreFloat4x4(
        &m_constantBufferData.projection,
        StereoProjectionFieldOfViewRightHand(params, m_nearZ, m_farZ, false) // Channel parameter doesn't matter for mono.
    );

    m_sampleOverlay->UpdateForWindowSizeChange();
}

// Override the default DirectXBase Render method. This class uses
// its own RenderEye method instead.
void StereoSimpleD3D::Render()
{
}

// Render function that supports both stereo and mono rendering.
void StereoSimpleD3D::RenderEye(_In_ unsigned int eyeIndex)
{
    winrt::com_ptr<ID3D11RenderTargetView> currentRenderTargetView;

    // If eyeIndex == 1, set right render target view. Otherwise, set left render target view.
    currentRenderTargetView = eyeIndex ? m_d3dRenderTargetViewRight : m_d3dRenderTargetView;

    // Bind the render targets.
    auto pRenderTargetViews = currentRenderTargetView.get();
    m_d3dContext->OMSetRenderTargets(
        1,
        &pRenderTargetViews,
        m_d3dDepthStencilView.get()
    );

    // Clear both the render target and depth stencil to default values.
    const float ClearColor[4] = { 0.071f, 0.040f, 0.561f, 1.0f };

    m_d3dContext->ClearRenderTargetView(
        currentRenderTargetView.get(),
        ClearColor
    );

    m_d3dContext->ClearDepthStencilView(
        m_d3dDepthStencilView.get(),
        D3D11_CLEAR_DEPTH,
        1.0f,
        0
    );

    m_d3dContext->IASetInputLayout(m_inputLayout.get());

    // Set the vertex and index buffers.
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    auto pVertexBuffers = m_vertexBuffer.get();
    m_d3dContext->IASetVertexBuffers(
        0,                              // Start at the first vertex buffer slot.
        1,                              // Set one vertex buffer binding.
        &pVertexBuffers,
        &stride,                        // Specify the size in bytes of a single vertex.
        &offset                         // Specify the base vertex in the buffer.
    );

    m_d3dContext->IASetIndexBuffer(
        m_indexBuffer.get(),
        DXGI_FORMAT_R16_UINT,   // Specify unsigned short index format.
        0                       // Specify the base index in the buffer.
    );

    // Specify the way the vertex and index buffers define geometry.
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the vertex shader stage state.
    m_d3dContext->VSSetShader(
        m_vertexShader.get(),
        nullptr,                // Don't use shader linkage.
        0                       // Don't use shader linkage.
    );

    auto pConstantBuffers = m_constantBuffer.get();
    m_d3dContext->VSSetConstantBuffers(
        0,                          // Start at the first constant buffer slot.
        1,                          // Set one constant buffer binding.
        &pConstantBuffers
    );

    // Set the pixel shader stage state.
    m_d3dContext->PSSetShader(
        m_pixelShader.get(),
        nullptr,                // Don't use shader linkage.
        0                       // Don't use shader linkage.
    );

    auto pShaderResources = m_textureShaderResourceView.get();
    m_d3dContext->PSSetShaderResources(
        0,                          // Start at the first shader resource slot.
        1,                          // Set one shader resource binding.
        &pShaderResources
    );

    auto pSamplers = m_sampler.get();
    m_d3dContext->PSSetSamplers(
        0,                          // Starting at the first sampler slot.
        1,                          // Set one sampler binding.
        &pSamplers
    );

    // Draw the cube.
    m_d3dContext->DrawIndexed(
        m_indexCount,   // Draw all created vertices.
        0,              // Start with the first vertex.
        0               // Start with the first index.
    );

    // Set the left/right Direct2D target bitmap.
    if (eyeIndex == 0)
    {
        m_d2dContext->SetTarget(m_d2dTargetBitmap.get());
    }
    else
    {
        m_d2dContext->SetTarget(m_d2dTargetBitmapRight.get());
    }

    m_sampleOverlay->Render();

    // Render the hint message text with different margins depending on the window size
    D2D1_RECT_F hintMessageRect;
    if (m_windowBounds.Width <= 550.0)
    {
        hintMessageRect = D2D1::RectF(10.0f, 10.0f, m_windowBounds.Width - 10.0f, 500.0f);
    }
    else
    {
        hintMessageRect = D2D1::RectF(100.0f, 100.0f, 550.0f, 380.0f);
    }

    m_d2dContext->BeginDraw();
    m_d2dContext->DrawText(
        m_hintMessage.data(),
        m_hintMessage.length(),
        m_textFormat.get(),
        hintMessageRect,
        m_brush.get()
    );

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    HRESULT hr = m_d2dContext->EndDraw();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        winrt::check_hresult(hr);
    }
}

// Updates the stereo projection matrix and constant buffers based on the latest parameters.
void StereoSimpleD3D::Update(
    _In_ unsigned int eyeIndex,
    _In_ float timeTotal,
    _In_ float timeDelta
)
{
    // Rotate the cube.
    DirectX::XMStoreFloat4x4(
        &m_constantBufferData.model,
        DirectX::XMMatrixRotationY(timeTotal)
    );

    if (m_stereoEnabled)
    {
        StereoParameters parameters = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_worldScale, m_stereoExaggerationFactor);
        if (eyeIndex == 0)
        {
            DirectX::XMStoreFloat4x4(
                &m_constantBufferData.projection,
                StereoProjectionFieldOfViewRightHand(parameters, m_nearZ, m_farZ, false)
            );
        }
        else
        {
            DirectX::XMStoreFloat4x4(
                &m_constantBufferData.projection,
                StereoProjectionFieldOfViewRightHand(parameters, m_nearZ, m_farZ, true)
            );
        }
    }

    // Transpose the matrices in the constant buffer.
    ConstantBuffer constantBuffer;
    DirectX::XMStoreFloat4x4(
        &constantBuffer.model,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_constantBufferData.model))
    );
    DirectX::XMStoreFloat4x4(
        &constantBuffer.view,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_constantBufferData.view))
    );
    DirectX::XMStoreFloat4x4(
        &constantBuffer.projection,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_constantBufferData.projection))
    );

    m_d3dContext->UpdateSubresource(m_constantBuffer.get(), 0, nullptr, &constantBuffer, 0, 0);
}

float StereoSimpleD3D::GetStereoExaggeration()
{
    return m_stereoExaggerationFactor;
}

void StereoSimpleD3D::SetStereoExaggeration(_In_ float currentExaggeration)
{
    currentExaggeration = min(currentExaggeration, 2.0f);
    currentExaggeration = max(currentExaggeration, 0.0f);
    m_stereoExaggerationFactor = currentExaggeration;
}
