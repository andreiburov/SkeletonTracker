// The Application Code is based on "SkeletonTracker.cpp" from Microsoft Kinect v1.8 SDK

#include <strsafe.h>
#include "stdafx.h"
#include "Application.h"
#include "Utils.h"

#define EDIT_CONTROL_SIZE 30

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	Application application;
	application.Run(hInstance, nCmdShow);
}

Application::Application() :
	m_pD2DFactory(NULL),
	m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
	m_bSeatedMode(false),
	m_pRenderTarget(NULL),
	m_pBrushJointTracked(NULL),
	m_pBrushJointInferred(NULL),
	m_pBrushBoneTracked(NULL),
	m_pBrushBoneInferred(NULL),
	m_pNuiSensor(NULL),
	m_SimpleSkeleton(),
	m_KinectSkeleton()
{
}

Application::~Application()
{
	if (m_pNuiSensor)
	{
		m_pNuiSensor->NuiShutdown();
	}

	if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonEvent);
	}

	// clean up Direct2D objects
	DiscardDirect2DResources();

	// clean up Direct2D
	SafeRelease(m_pD2DFactory);
	SafeRelease(m_pNuiSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int Application::Run(HINSTANCE hInstance, int nCmdShow)
{
	MSG       msg = { 0 };
	WNDCLASS  wc = { 0 };

	// Dialog custom window class
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SKELETONTRACKER));
	wc.lpfnWndProc = DefDlgProcW;
	wc.lpszClassName = L"SkeletonTrackerAppDlgWndClass";

	if (!RegisterClassW(&wc))
	{
		return 0;
	}

	// Create main application window
	HWND hWndApp = CreateDialogParamW(
		hInstance,
		MAKEINTRESOURCE(IDD_APP),
		NULL,
		(DLGPROC)Application::MessageRouter,
		reinterpret_cast<LPARAM>(this));

	// Show window
	ShowWindow(hWndApp, nCmdShow);

	const int eventCount = 1;
	HANDLE hEvents[eventCount];

	// Main message loop
	while (WM_QUIT != msg.message)
	{
		hEvents[0] = m_hNextSkeletonEvent;

		// Check to see if we have either a message (by passing in QS_ALLEVENTS)
		// Or a Kinect event (hEvents)
		// Update() will check for Kinect events individually, in case more than one are signalled
		MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);

		// Explicitly check the Kinect frame event since MsgWaitForMultipleObjects
		// can return for other reasons even though it is signaled.
		Update();

		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// If a dialog message will be taken care of by the dialog proc
			if ((hWndApp != NULL) && IsDialogMessageW(hWndApp, &msg))
			{
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return static_cast<int>(msg.wParam);
}

/// <summary>
/// Main processing function
/// </summary>
void Application::Update()
{
	if (NULL == m_pNuiSensor)
	{
		return;
	}

	// Wait for 0ms, just quickly test if it is time to process a skeleton
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0))
	{
		ProcessSkeleton();
	}
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK Application::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Application* pThis = NULL;

	if (WM_INITDIALOG == uMsg)
	{
		pThis = reinterpret_cast<Application*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else
	{
		pThis = reinterpret_cast<Application*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pThis)
	{
		return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK Application::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Bind application window handle
		m_hWnd = hWnd;

		// Init Direct2D
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

		// Look for a connected Kinect, and create it if found
		CreateFirstConnected();
	}
	break;

	// If the titlebar X is clicked, destroy app
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		// Quit the main message pump
		PostQuitMessage(0);
		break;

		// Handle button press
	case WM_COMMAND:
		// If it was for the near mode control and a clicked event, change near mode
		if (IDC_CHECK_SEATED == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			// Toggle out internal state for near mode
			m_bSeatedMode = !m_bSeatedMode;

			if (NULL != m_pNuiSensor)
			{
				// Set near mode for sensor based on our internal state
				m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, m_bSeatedMode ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0);
			}
		}

		// If the button was pressed print the joints to the local timestamped file
		if (IDC_PRINT_JOINTS == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_KinectSkeleton.PrintJoints();
		}

		// If the button was pressed, get the transform and apply it to SimpleSkeleton
		if (IDC_APPLY_TRANSFORM == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			Vector4 scale;
			Vector4 translate;
			ZeroMemory(&scale, sizeof(Vector4));
			ZeroMemory(&translate, sizeof(Vector4));

			{
				HWND hScale = GetDlgItem(m_hWnd, IDC_SCALE_VECTOR_VALUE);
				wchar_t wcScale[EDIT_CONTROL_SIZE];
				Edit_GetText(hScale, wcScale, EDIT_CONTROL_SIZE);
				std::wstring sScale(wcScale);
				std::wistringstream wiss(sScale);
				wiss >> scale.x >> scale.y >> scale.z;
			}
			{
				HWND hTranslate = GetDlgItem(m_hWnd, IDC_TRANSLATE_VECTOR_VALUE);
				wchar_t wcTranslate[EDIT_CONTROL_SIZE];
				Edit_GetText(hTranslate, wcTranslate, EDIT_CONTROL_SIZE);
				std::wstring sTranslate(wcTranslate);
				std::wistringstream wiss(sTranslate);
				wiss >> translate.x >> translate.y >> translate.z;
			}

			m_SimpleSkeleton.ApplyTransformations(scale, translate);
		}

		// If the button was pressed, convert hierarchical quaternions to axis-angle vectors
		if (IDC_PRINT_ROTATIONS == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_KinectSkeleton.PrintSimplePose();
		}

		// If the button was pressed, keep kinect orientations for the T pose
		if (IDC_FIX_MAPPING == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_KinectSkeleton.FixTpose();
		}
		break;
	}

	return FALSE;
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT Application::CreateFirstConnected()
{
	INuiSensor * pNuiSensor;

	int iSensorCount = 0;
	HRESULT hr = NuiGetSensorCount(&iSensorCount);
	if (FAILED(hr))
	{
		return hr;
	}

	// Look at each Kinect sensor
	for (int i = 0; i < iSensorCount; ++i)
	{
		// Create the sensor so we can check status, if we can't create it, move on to the next
		hr = NuiCreateSensorByIndex(i, &pNuiSensor);
		if (FAILED(hr))
		{
			continue;
		}

		// Get the status of the sensor, and if connected, then we can initialize it
		hr = pNuiSensor->NuiStatus();
		if (S_OK == hr)
		{
			m_pNuiSensor = pNuiSensor;
			break;
		}

		// This sensor wasn't OK, so release it since we're not using it
		pNuiSensor->Release();
	}

	if (NULL != m_pNuiSensor)
	{
		// Initialize the Kinect and specify that we'll be using skeleton
		hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
		if (SUCCEEDED(hr))
		{
			// Create an event that will be signaled when skeleton data is available
			m_hNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

			// Open a skeleton stream to receive skeleton data
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
		}
	}

	if (NULL == m_pNuiSensor || FAILED(hr))
	{
		SetStatusMessage(L"No ready Kinect found!");
		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Handle new skeleton data
/// </summary>
void Application::ProcessSkeleton()
{
	RECT rct;
	GetClientRect(GetDlgItem(m_hWnd, IDC_VIEW_ONE), &rct);
	int width = rct.right;
	int height = rct.bottom;

	NUI_SKELETON_FRAME skeletonFrame = { 0 };
	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if (FAILED(hr))
	{
		return;
	}

	// smooth out the skeleton data
	m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

	// Endure Direct2D is ready to draw
	hr = EnsureDirect2DResources();
	if (FAILED(hr))
	{
		return;
	}

	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear();

	m_SimpleSkeleton.Render(m_pRenderTarget, m_pBrushJointSimple, m_pBrushBoneSimple, width, height);

	KinectSkeleton::RenderHelper helper;
	helper.pRenderTarget = m_pRenderTarget;
	helper.pBrushBoneInferred = m_pBrushBoneInferred;
	helper.pBrushBoneTracked = m_pBrushBoneTracked;
	helper.pBrushJointInferred = m_pBrushJointInferred;
	helper.pBrushJointTracked = m_pBrushJointTracked;
	helper.pSkeletonFrame = &skeletonFrame;
	helper.windowWidth = width;
	helper.windowHeight = height;
	m_KinectSkeleton.Render(helper);

	hr = m_pRenderTarget->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
		DiscardDirect2DResources();
	}
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT Application::EnsureDirect2DResources()
{
	HRESULT hr = S_OK;

	// If there isn't currently a render target, we need to create one
	if (NULL == m_pRenderTarget)
	{
		RECT rc;
		GetWindowRect(GetDlgItem(m_hWnd, IDC_VIEW_ONE), &rc);

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;
		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		// Create a Hwnd render target, in order to render to the window set in initialize
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(GetDlgItem(m_hWnd, IDC_VIEW_ONE), size),
			&m_pRenderTarget
		);
		if (FAILED(hr))
		{
			SetStatusMessage(L"Couldn't create Direct2D render target!");
			return hr;
		}

		//light green
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &m_pBrushJointInferred);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aquamarine, 1.0f), &m_pBrushJointSimple);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &m_pBrushBoneTracked);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f), &m_pBrushBoneInferred);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f), &m_pBrushBoneSimple);
	}

	return hr;
}

/// <summary>
/// Dispose Direct2d resources 
/// </summary>
void Application::DiscardDirect2DResources()
{
	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pBrushJointTracked);
	SafeRelease(m_pBrushJointInferred);
	SafeRelease(m_pBrushJointSimple);
	SafeRelease(m_pBrushBoneTracked);
	SafeRelease(m_pBrushBoneInferred);
	SafeRelease(m_pBrushBoneSimple);
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void Application::SetStatusMessage(const WCHAR * szMessage)
{
	SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}