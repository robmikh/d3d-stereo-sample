#include "pch.h"
#include "StereoSimpleD3D.h"
#include "BasicTimer.h"

namespace winrt
{
    using namespace Windows;
    using namespace Windows::ApplicationModel;
    using namespace Windows::ApplicationModel::Activation;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics::Display;
    using namespace Windows::Storage;
    using namespace Windows::System;
    using namespace Windows::UI;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::Composition;
    using namespace Windows::UI::Input;
}

const static std::wstring s_StereoExaggerationFactor(L"StereoExaggerationFactor");

struct App : winrt::implements<App, winrt::IFrameworkViewSource, winrt::IFrameworkView>
{
    std::unique_ptr<StereoSimpleD3D> m_renderer;
    bool m_windowClosed = false;
    bool m_windowVisible = true;

    winrt::IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(winrt::CoreApplicationView const& view)
    {
        view.Activated({ this, &App::OnActivated });
        winrt::CoreApplication::Suspending({ this, &App::OnSuspending });
        winrt::CoreApplication::Resuming({ this, &App::OnResuming });
        m_renderer = std::make_unique<StereoSimpleD3D>();
    }

    void Load(winrt::hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        BasicTimer timer;
        auto window = winrt::CoreWindow::GetForCurrentThread();
        auto dispatcher = window.Dispatcher();

        while (!m_windowClosed)
        {
            if (m_windowVisible)
            {
                //timer.Update();
                dispatcher.ProcessEvents(winrt::CoreProcessEventsOption::ProcessAllIfPresent);

                // render the mono content or the left eye view of the stereo content
                m_renderer->Update(0, timer.Total(), timer.Delta());
                m_renderer->RenderEye(0);
                // render the right eye view of the stereo content
                if (m_renderer->GetStereoEnabledStatus())
                {
                    m_renderer->Update(1, timer.Total(), timer.Delta());
                    m_renderer->RenderEye(1);
                }
                m_renderer->Present(); // this call is sychronized to the display frame rate
            }
            else
            {
                dispatcher.ProcessEvents(winrt::CoreProcessEventsOption::ProcessOneAndAllPending);
            }
        }
    }

    void SetWindow(winrt::CoreWindow const& window)
    {
        window.PointerCursor(winrt::CoreCursor(winrt::CoreCursorType::Arrow, 0));
        window.SizeChanged({ this, &App::OnWindowSizeChanged });
        window.VisibilityChanged({ this, &App::OnVisibilityChanged });
        window.Closed({ this, &App::OnWindowClosed });
        window.KeyDown({ this, &App::OnKeyDown });

        auto displayInfo = winrt::DisplayInformation::GetForCurrentView();
        displayInfo.DpiChanged({ this, &App::OnDpiChanged });

        // Disable all pointer visual feedback for better performance when touching.
        auto pointerSettings = winrt::PointerVisualizationSettings::GetForCurrentView();
        pointerSettings.IsContactFeedbackEnabled(false);
        pointerSettings.IsBarrelButtonFeedbackEnabled(false);

        m_renderer->Initialize(window, displayInfo.LogicalDpi());
    }

    void OnWindowSizeChanged(winrt::CoreWindow const& sender, winrt::WindowSizeChangedEventArgs const& args)
    {
        m_renderer->UpdateForWindowSizeChange();
    }

    void OnVisibilityChanged(winrt::CoreWindow const& sender, winrt::VisibilityChangedEventArgs const& args)
    {
        m_windowVisible = args.Visible();
    }

    void OnWindowClosed(winrt::CoreWindow const& sender, winrt::CoreWindowEventArgs const& args)
    {
        m_windowClosed = true;
    }

    void OnDpiChanged(winrt::DisplayInformation const& sender, winrt::IInspectable const& args)
    {
        m_renderer->SetDpi(sender.LogicalDpi());
    }

    void OnKeyDown(winrt::CoreWindow const& sender, winrt::KeyEventArgs const& args)
    {
        auto key = args.VirtualKey();

        // if the image is in stereo, adjust for user keystrokes increasing/decreasing the stereo effect
        if (m_renderer->GetStereoEnabledStatus())
        {
            float stereoExaggeration = m_renderer->GetStereoExaggeration();
            // figure out the command from the keyboard
            if (key == winrt::VirtualKey::Up)             // increase stereo effect
            {
                stereoExaggeration += 0.1f;
            }
            if (key == winrt::VirtualKey::Down)           // descrease stereo effect
            {
                stereoExaggeration -= 0.1f;
            }
            stereoExaggeration = min(stereoExaggeration, 2.0f);
            stereoExaggeration = max(stereoExaggeration, 0.0f);
            m_renderer->SetStereoExaggeration(stereoExaggeration);
        }
    }

    void OnActivated(winrt::CoreApplicationView const& view, winrt::IActivatedEventArgs const& args)
    {
        if (args.Kind() == winrt::ActivationKind::Launch)
        {
            // Load previously saved state only if the application shut down cleanly last time.
            if (args.PreviousExecutionState() != winrt::ApplicationExecutionState::NotRunning)
            {
                // When this application is suspended, it stores the drawing state.
                // This code attempts to restore the saved state.
                auto set = winrt::ApplicationData::Current().LocalSettings().Values();
                // an int called StereoExaggerationFactor is used as a key
                if (set.HasKey(s_StereoExaggerationFactor))
                {
                    auto value = winrt::unbox_value<winrt::IPropertyValue>(set.Lookup(s_StereoExaggerationFactor));
                    float factor = value.GetSingle();
                    m_renderer->SetStereoExaggeration(factor);
                }
            }
        }
        else
        {
            throw winrt::hresult_error(E_UNEXPECTED);
        }
        winrt::CoreWindow::GetForCurrentThread().Activate();
    }

    void OnSuspending(winrt::IInspectable const& sender, winrt::SuspendingEventArgs const& args)
    {
        // This is also a good time to save your application's state in case the process gets terminated.
        // That way, when the user relaunches the application, they will return to the position they left.
        auto settingsValues = winrt::ApplicationData::Current().LocalSettings().Values();
        if (settingsValues.HasKey(s_StereoExaggerationFactor))
        {
            settingsValues.Remove(s_StereoExaggerationFactor);
        }

        float factor = m_renderer->GetStereoExaggeration();
        settingsValues.Insert(s_StereoExaggerationFactor, winrt::PropertyValue::CreateSingle(factor));

        // Hint to the driver that the app is entering an idle state and that its memory
        // can be temporarily used for other apps.
        m_renderer->Trim();
    }

    void OnResuming(winrt::IInspectable const& sender, winrt::IInspectable const& args)
    {
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::CoreApplication::Run(winrt::make<App>());
}
