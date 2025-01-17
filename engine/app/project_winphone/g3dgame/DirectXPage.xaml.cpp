﻿//
//DirectXPage.xaml.cpp
//DirectXPage 类的实现。
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace g3dgame;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;
using namespace Windows::Phone::UI::Input;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	// 注册页面生命周期的事件处理程序。
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);
    HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs ^>(this, &DirectXPage::OnBackPressed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);
        
	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// 此时，我们具有访问设备的权限。
	// 我们可创建与设备相关的资源。
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	m_main = std::unique_ptr<Main>(new Main(m_deviceResources));
	m_main->StartRenderLoop();

	// 注册我们的 SwapChainPanel 以获取独立的输入指针事件
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// 对于指定的设备类型，无论它是在哪个线程上创建的，CoreIndependentInputSource 都将引发指针事件。
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// 指针事件的寄存器，将在后台线程上引发。
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// 一旦发送输入消息，即开始处理它们。
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// 在高优先级的专用后台线程上运行任务。
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

DirectXPage::~DirectXPage()
{
	// 析构时停止渲染和处理事件。
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// 针对挂起和终止事件保存应用程序的当前状态。
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// 挂起应用程序时停止渲染。
	m_main->StopRenderLoop();

	// 在此处放置代码以保存应用程序状态。
}

// 针对恢复事件加载应用程序的当前状态。
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// 在此处放置代码以加载应用程序状态。

	// 恢复应用程序时开始渲染。
	m_main->StartRenderLoop();
}

// 窗口事件处理程序。

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation 事件处理程序。

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

void DirectXPage::OnBackPressed(Platform::Object^ sender, BackPressedEventArgs^ e)
{
    //m_main->Done();
    //Application::Current->Exit();
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	m_main->OnPointerPressed(sender, e);
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	m_main->OnPointerMoved(sender, e);
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	m_main->OnPointerReleased(sender, e);
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}
