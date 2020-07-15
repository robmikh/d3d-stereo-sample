#pragma once

// This class is responsible for initializing and rendering the title overlay visible
// at the top of the DirectX SDK samples.
class SampleOverlay
{
public:
    SampleOverlay();

    void Initialize(
        winrt::com_ptr<ID2D1Device> const& d2dDevice,
        winrt::com_ptr<ID2D1DeviceContext> const& d2dContext,
        winrt::com_ptr<IWICImagingFactory> const& wicFactory,
        winrt::com_ptr<IDWriteFactory> const& dwriteFactory,
        std::wstring const& caption
    );

    void ResetDirectXResources();

    void UpdateForWindowSizeChange();

    void Render();

    float GetTitleHeightInDips();

private:

    winrt::com_ptr<ID2D1Factory1>           m_d2dFactory;
    winrt::com_ptr<ID2D1Device>             m_d2dDevice;
    winrt::com_ptr<ID2D1DeviceContext>      m_d2dContext;
    winrt::com_ptr<IDWriteFactory>          m_dwriteFactory;
    winrt::com_ptr<ID2D1SolidColorBrush>    m_whiteBrush;
    winrt::com_ptr<ID2D1DrawingStateBlock>  m_stateBlock;

    winrt::com_ptr<IWICImagingFactory>      m_wicFactory;
    winrt::com_ptr<ID2D1Bitmap>             m_logoBitmap;
    winrt::com_ptr<IDWriteTextLayout>       m_textLayout;

    UINT                                    m_idIncrement;
    bool                                    m_drawOverlay;
    std::wstring                            m_sampleName;
    float                                   m_padding;
    float                                   m_textVerticalOffset;
    D2D1_SIZE_F                             m_logoSize;
    float                                   m_overlayWidth;
};
