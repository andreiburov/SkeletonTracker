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
	m_bOnline(false),
	m_bInterruptKinect(false),
	m_bRenderModel(true),
	m_bRenderSkeleton3D(false),
	m_bRenderKinect3D(false),
	m_bTraceable2D(false),
	m_bTraceable3D(false),
	m_pD2DFactory(NULL),
	m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
	m_pRenderTarget(NULL),
	m_pBrushJointTracked(NULL),
	m_pBrushJointInferred(NULL),
	m_pBrushBoneTracked(NULL),
	m_pBrushBoneInferred(NULL),
	m_pNuiSensor(NULL),
	m_SimpleSkeleton2D(),
	m_KinectSkeleton()
{
	m_KinectFrame = { 0 };
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
		// m_KinectRotations is updated when T-Pose is calibrated
		CheckKinectData();
		if (m_bOnline) PassRotationsFromKinectToSimple();

		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// If a dialog message will be taken care of by the dialog proc
			if ((hWndApp != NULL) && IsDialogMessageW(hWndApp, &msg))
			{
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		
		if (m_bRenderModel)	RenderSimpleModel();
		if (m_bRenderSkeleton3D) RenderSimpleSkeleton3D();
		if (m_bRenderKinect3D) RenderKinectSkeleton3D();
	}

	m_SimpleModel.Clear();
	m_SimpleSkeleton3D.Clear();
	CleanupDevice();

	return static_cast<int>(msg.wParam);
}

/// <summary>
/// Main processing function
/// </summary>
void Application::CheckKinectData()
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

		HWND hModel = GetDlgItem(m_hWnd, IDC_MODEL);
		Button_SetCheck(hModel, true);

		// Init Direct2D
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

		// Init Direct3D
		if (FAILED(InitDevice()))
		{
			CleanupDevice();
			return 0;
		}

		// Look for a connected Kinect, and create it if found
		CreateFirstConnected();

		// Create Simple Model
		m_SimpleModel.Create(m_pd3dDevice, "SimpleModel/smpl.obj", 
			"SimpleModel/smpl.posedirs", L"SimpleModelVS.cso", 
			L"SimpleModelGS.cso", L"SimpleModelPS.cso", m_aspectRatio);
		m_SimpleSkeleton3D.Create(m_pd3dDevice, L"SimpleSkeleton3DVS.cso",
			L"SimpleSkeleton3DGS.cso", L"SimpleSkeleton3DPS.cso", m_aspectRatio);
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

		// Toggle Kinect to interrupt fashion
		if (IDC_INTERRUPT == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_bInterruptKinect = !m_bInterruptKinect;
		}

		// Toggle traceable rotations to online
		if (IDC_ONLINE == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			// Toggle out
			m_bOnline = !m_bOnline;
		}

		// Radio Buttons
		if (IDC_MODEL == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_bRenderModel = true;
			m_bRenderSkeleton3D = false;
			m_bRenderKinect3D = false;
		}

		if (IDC_SKELETON == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_bRenderModel = false;
			m_bRenderSkeleton3D = true;
			m_bRenderKinect3D = false;
		}
		if (IDC_KINECT == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_bRenderModel = false;
			m_bRenderSkeleton3D = false;
			m_bRenderKinect3D = true;
		}
		// End

		// If online is false (i.e. traceable) apply rotations to the SimpleModel
		if (IDC_APPLY_ROTATIONS == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			if (!m_bOnline)
			{
				PassRotationsFromKinectToSimple();
				m_bTraceable3D = true;
			}
		}

		// Toggle LBS only
		if (IDC_LBS_ONLY == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_SimpleModel.ToggleLBSonly();
		}

		// Print the joints to the local timestamped file
		if (IDC_NEXT_FRAME == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			if (m_bInterruptKinect) m_KinectSkeleton.Update(m_KinectFrame, true);
		}

		// Get the transform and apply it to SimpleSkeleton2D (blue skeleton)
		if (IDC_APPLY_TRANSFORM == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			Vector4 scale;
			Vector4 rotate;
			scale.x = 1;
			scale.y = 1;
			scale.z = 1;
			ZeroMemory(&rotate, sizeof(Vector4));

			{
				HWND hScale = GetDlgItem(m_hWnd, IDC_SCALE_VECTOR_VALUE);
				wchar_t wcScale[EDIT_CONTROL_SIZE];
				Edit_GetText(hScale, wcScale, EDIT_CONTROL_SIZE);
				std::wstring sScale(wcScale);
				std::wistringstream wiss(sScale);
				wiss >> scale.x >> scale.y >> scale.z;
			}
			{
				HWND hRotate = GetDlgItem(m_hWnd, IDC_ROTATE_VECTOR_VALUE);
				wchar_t wcRotate[EDIT_CONTROL_SIZE];
				Edit_GetText(hRotate, wcRotate, EDIT_CONTROL_SIZE);
				std::wstring sRotate(wcRotate);
				std::wistringstream wiss(sRotate);
				wiss >> rotate.x >> rotate.y;
			}

			m_SimpleModel.ApplyTransformations(scale, rotate);
			m_SimpleSkeleton3D.ApplyTransformations(scale, rotate);
		}

		// Store kinect hierarchical orientations for the T pose
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
	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &m_KinectFrame);
	if (FAILED(hr))
	{
		return;
	}

	// smooth out the skeleton data
	m_pNuiSensor->NuiTransformSmooth(&m_KinectFrame, NULL);
	if (!m_bInterruptKinect)
	{
		m_KinectSkeleton.Update(m_KinectFrame, false);
	}
	RenderSkeletons2D();

	if (m_KinectSkeleton.isTposeCalibrated())
	{
		m_KinectSkeleton.GetSimplePose(m_KinectRotations);
	}
}

void Application::RenderSkeletons2D()
{
	RECT rct;
	GetClientRect(GetDlgItem(m_hWnd, IDC_VIEW_ONE), &rct);
	int width = rct.right;
	int height = rct.bottom;

	// Endure Direct2D is ready to draw
	HRESULT hr = EnsureDirect2DResources();
	if (FAILED(hr))
	{
		return;
	}

	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear();

	//m_SimpleSkeleton2D.Render(m_pRenderTarget, m_pBrushJointSimple, m_pBrushBoneSimple, width, height);

	KinectSkeleton::RenderHelper helper;
	helper.pRenderTarget = m_pRenderTarget;
	helper.pBrushBoneInferred = m_pBrushBoneInferred;
	helper.pBrushBoneTracked = m_pBrushBoneTracked;
	helper.pBrushJointInferred = m_pBrushJointInferred;
	helper.pBrushJointTracked = m_pBrushJointTracked;
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

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Application::InitDevice()
{
	HRESULT hr = S_OK;
	HWND hWnd = GetDlgItem(m_hWnd, IDC_VIEW_TWO);
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	m_aspectRatio = ((float)width) / height;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, hWnd, &sd, nullptr, nullptr, &m_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	V_RETURN(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())),
		L"Could not get buffer #0 for RenderTargetView");
	V_RETURN(m_pd3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_pRenderTargetView),
		L"Could not create RenderTargetView");

	// About depthStencil https://msdn.microsoft.com/en-us/library/windows/desktop/bb205074(v=vs.85).aspx
	// Create depthStencil resource

	D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
	backBuffer->GetDesc(&backBufferDesc);

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = backBufferDesc.Width;
	depthStencilDesc.Height = backBufferDesc.Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
	V_RETURN(m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()),
		L"Could not create texture for the DepthStencilView");

	// Create depthStencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	V_RETURN(m_pd3dDevice->CreateDepthStencilState(&dsDesc, depthStencilState.GetAddressOf()),
		L"Could not create DepthStencilState");

	m_pImmediateContext->OMSetDepthStencilState(depthStencilState.Get(), 1);

	// Create depthStencil view and bind it to the pipeline

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	
	V_RETURN(m_pd3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &m_pDepthStencilView),
		L"Could not create the DepthStencilView");

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Application::RenderSimpleModel()
{
	// Just clear the backbuffer
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, DirectX::Colors::MidnightBlue);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView,	D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_SimpleModel.Render(m_pImmediateContext, m_SimpleRotations, m_bTraceable3D);
	m_bTraceable3D = false;

	m_pSwapChain->Present(1, 0);
}

void Application::RenderSimpleSkeleton3D()
{
	// Just clear the backbuffer
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, DirectX::Colors::MidnightBlue);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_SimpleSkeleton3D.Render(m_pImmediateContext, m_SimpleRotations, m_bTraceable3D);
	m_bTraceable3D = false;

	m_pSwapChain->Present(1, 0);
}

void Application::RenderKinectSkeleton3D()
{
	// Just clear the backbuffer
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, DirectX::Colors::MidnightBlue);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_bTraceable3D = false;

	m_pSwapChain->Present(1, 0);
}

void Application::PassRotationsFromKinectToSimple()
{
	for (unsigned int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		m_SimpleRotations[(_SMPL_SKELETON_POSITION_INDEX)i] = m_KinectRotations[(_SMPL_SKELETON_POSITION_INDEX)i];
	}
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void Application::CleanupDevice()
{
	if (m_pImmediateContext) m_pImmediateContext->ClearState();

	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pSwapChain1) m_pSwapChain1->Release();
	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pImmediateContext1) m_pImmediateContext1->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if (m_pd3dDevice1) m_pd3dDevice1->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
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