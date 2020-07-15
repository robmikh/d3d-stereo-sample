#include "pch.h"
#include "SampleOverlay.h"

namespace winrt
{
    using namespace Windows::UI::Core;
    using namespace Windows::Foundation;
    using namespace Windows::UI::ViewManagement;
    using namespace Windows::Graphics::Display;
}

SampleOverlay::SampleOverlay() :
    m_drawOverlay(true)
{
}

void SampleOverlay::Initialize(
    winrt::com_ptr<ID2D1Device> const& d2dDevice,
    winrt::com_ptr<ID2D1DeviceContext> const& d2dContext,
    winrt::com_ptr<IWICImagingFactory> const& wicFactory,
    winrt::com_ptr<IDWriteFactory> const& dwriteFactory,
    std::wstring const& caption
)
{
    m_wicFactory = wicFactory;
    m_dwriteFactory = dwriteFactory;
    m_sampleName = caption;
    m_d2dDevice = d2dDevice;
    m_d2dContext = d2dContext;
    m_padding = 3.0f;
    m_textVerticalOffset = 5.0f;
    m_logoSize = D2D1::SizeF(0.0f, 0.0f);
    m_overlayWidth = 0.0f;

    winrt::com_ptr<ID2D1Factory> factory;
    d2dDevice->GetFactory(factory.put());

    m_d2dFactory = factory.as<ID2D1Factory1>();

    ResetDirectXResources();
}

void SampleOverlay::ResetDirectXResources()
{
    winrt::check_hresult(
        m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_whiteBrush.put())
    );

    winrt::com_ptr<IWICBitmapDecoder> wicBitmapDecoder;
    winrt::check_hresult(
        m_wicFactory->CreateDecoderFromFilename(
            L"windowsbig-sdk.png",
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            wicBitmapDecoder.put()
        )
    );

    winrt::com_ptr<IWICBitmapFrameDecode> wicBitmapFrame;
    winrt::check_hresult(
        wicBitmapDecoder->GetFrame(0, wicBitmapFrame.put())
    );

    winrt::com_ptr<IWICFormatConverter> wicFormatConverter;
    winrt::check_hresult(
        m_wicFactory->CreateFormatConverter(wicFormatConverter.put())
    );

    winrt::check_hresult(
        wicFormatConverter->Initialize(
            wicBitmapFrame.get(),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom  // the BGRA format has no palette so this value is ignored
        )
    );

    double dpiX = 96.0f;
    double dpiY = 96.0f;
    winrt::check_hresult(
        wicFormatConverter->GetResolution(&dpiX, &dpiY)
    );

    winrt::check_hresult(
        m_d2dContext->CreateBitmapFromWicBitmap(
            wicFormatConverter.get(),
            D2D1::BitmapProperties(
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                static_cast<float>(dpiX),
                static_cast<float>(dpiY)
            ),
            m_logoBitmap.put()
        )
    );

    m_logoSize = m_logoBitmap->GetSize();

    winrt::com_ptr<IDWriteTextFormat> nameTextFormat;
    winrt::check_hresult(
        m_dwriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            36.0f,
            L"en-US",
            nameTextFormat.put()
        )
    );

    winrt::check_hresult(
        nameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
    );

    winrt::check_hresult(
        nameTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
    );

    winrt::check_hresult(
        m_dwriteFactory->CreateTextLayout(
            m_sampleName.data(),
            m_sampleName.length(),
            nameTextFormat.get(),
            4096.0f,
            4096.0f,
            m_textLayout.put()
        )
    );

    DWRITE_TEXT_METRICS metrics = { 0 };
    winrt::check_hresult(
        m_textLayout->GetMetrics(&metrics)
    );

    m_overlayWidth = m_padding * 3.0f + m_logoSize.width + metrics.width;

    winrt::check_hresult(
        m_d2dFactory->CreateDrawingStateBlock(m_stateBlock.put())
    );

    UpdateForWindowSizeChange();
}

void SampleOverlay::UpdateForWindowSizeChange()
{
    if (winrt::CoreWindow::GetForCurrentThread().Bounds().Width < m_overlayWidth)
    {
        m_drawOverlay = false;
    }
    else
    {
        m_drawOverlay = true;
    }
}

void SampleOverlay::Render()
{
    if (m_drawOverlay)
    {
        m_d2dContext->SaveDrawingState(m_stateBlock.get());

        m_d2dContext->BeginDraw();
        m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
        m_d2dContext->DrawBitmap(
            m_logoBitmap.get(),
            D2D1::RectF(m_padding, 0.0f, m_logoSize.width + m_padding, m_logoSize.height)
        );

        m_d2dContext->DrawTextLayout(
            D2D1::Point2F(m_logoSize.width + 2.0f * m_padding, m_textVerticalOffset),
            m_textLayout.get(),
            m_whiteBrush.get()
        );

        // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
        // is lost. It will be handled during the next call to Present.
        HRESULT hr = m_d2dContext->EndDraw();
        if (hr != D2DERR_RECREATE_TARGET)
        {
            winrt::check_hresult(hr);
        }

        m_d2dContext->RestoreDrawingState(m_stateBlock.get());
    }
}

float SampleOverlay::GetTitleHeightInDips()
{
    return m_logoSize.height;
}