#pragma once

class DirectXBase
{
public:
    DirectXBase();

    virtual void Initialize(winrt::Windows::UI::Core::CoreWindow const& windows, float dpi);
    virtual void HandleDeviceLost();
    virtual void CreateDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void SetDpi(float dpi);
    virtual void UpdateForWindowSizeChange();
    virtual void CreateWindowSizeDependentResources();
    virtual void Render() = 0;
    virtual void Present();
    void Trim();

    bool GetStereoEnabledStatus();

protected:
    void OnStereoEnabledChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);
    void UpdateStereoEnabledStatus();

    winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> m_window;

    // DirectWrite & Windows Imaging Component Objects.
    winrt::com_ptr<IDWriteFactory2>         m_dwriteFactory;
    winrt::com_ptr<IWICImagingFactory2>     m_wicFactory;

    // DirectX Core Objects. Required for 2D and 3D.
    winrt::com_ptr<ID3D11Device2>           m_d3dDevice;
    winrt::com_ptr<ID3D11DeviceContext2>    m_d3dContext;
    winrt::com_ptr<IDXGISwapChain1>         m_swapChain;
    winrt::com_ptr<ID3D11RenderTargetView>  m_d3dRenderTargetView;
    winrt::com_ptr<ID3D11RenderTargetView>  m_d3dRenderTargetViewRight;

    // Direct2D Rendering Objects. Required for 2D.
    winrt::com_ptr<ID2D1Factory2>           m_d2dFactory;
    winrt::com_ptr<ID2D1Device1>            m_d2dDevice;
    winrt::com_ptr<ID2D1DeviceContext1>     m_d2dContext;
    winrt::com_ptr<ID2D1Bitmap1>            m_d2dTargetBitmap;
    winrt::com_ptr<ID2D1Bitmap1>            m_d2dTargetBitmapRight;

    // Direct3D Rendering Objects. Required for 3D.
    winrt::com_ptr<ID3D11DepthStencilView>  m_d3dDepthStencilView;

    // Cached renderer properties.
    D3D_FEATURE_LEVEL                       m_featureLevel;
    winrt::Windows::Foundation::Size        m_renderTargetSize;
    winrt::Windows::Foundation::Rect        m_windowBounds;
    float                                   m_dpi;
    bool                                    m_windowSizeChangeInProgress;
    bool                                    m_stereoEnabled;
};