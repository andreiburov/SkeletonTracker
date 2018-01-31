#pragma once

#include "resource.h"
#include "KinectSkeleton.h"
#include "SimpleSkeleton.h"
#include "SimpleModel.h"

class Application
{
	static const int        cStatusMessageMaxLen = MAX_PATH * 2;

public:

	Application();
	~Application();

	/// <summary>
	/// Handles window messages, passes most to the class instance to handle
	/// </summary>
	/// <param name="hWnd">window message is for</param>
	/// <param name="uMsg">message</param>
	/// <param name="wParam">message data</param>
	/// <param name="lParam">additional message data</param>
	/// <returns>result of message processing</returns>
	static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// Handle windows messages for a class instance
	/// </summary>
	/// <param name="hWnd">window message is for</param>
	/// <param name="uMsg">message</param>
	/// <param name="wParam">message data</param>
	/// <param name="lParam">additional message data</param>
	/// <returns>result of message processing</returns>
	LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// Creates the main window and begins processing
	/// </summary>
	/// <param name="hInstance"></param>
	/// <param name="nCmdShow"></param>
	int Run(HINSTANCE hInstance, int nCmdShow);

private:
	HWND                     m_hWnd;

	bool                     m_bSeatedMode;

	// Current Kinect
	INuiSensor*              m_pNuiSensor;

	// Skeletal drawing
	ID2D1HwndRenderTarget*   m_pRenderTarget;
	ID2D1SolidColorBrush*    m_pBrushJointTracked;
	ID2D1SolidColorBrush*    m_pBrushJointInferred;
	ID2D1SolidColorBrush*    m_pBrushJointSimple;
	ID2D1SolidColorBrush*    m_pBrushBoneTracked;
	ID2D1SolidColorBrush*    m_pBrushBoneInferred;
	ID2D1SolidColorBrush*    m_pBrushBoneSimple;

	// Direct2D
	ID2D1Factory*            m_pD2DFactory;
	
	// Direct3D
	D3D_FEATURE_LEVEL        m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*            m_pd3dDevice = nullptr;
	ID3D11Device1*           m_pd3dDevice1 = nullptr;
	ID3D11DeviceContext*     m_pImmediateContext = nullptr;
	ID3D11DeviceContext1*    m_pImmediateContext1 = nullptr;
	IDXGISwapChain*          m_pSwapChain = nullptr;
	IDXGISwapChain1*         m_pSwapChain1 = nullptr;
	ID3D11RenderTargetView*  m_pRenderTargetView = nullptr;
	ID3D11DepthStencilView*  m_pDepthStencilView = nullptr;

	HANDLE                   m_hNextSkeletonEvent;

	KinectSkeleton			 m_KinectSkeleton;
	SimpleSkeleton			 m_SimpleSkeleton;
	SimpleModel				 m_SimpleModel;
	float					 m_aspectRatio; // Aspect ratio of view-space X:Y

	/// <summary>
	/// Create the first connected Kinect found 
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code</returns>
	HRESULT                 CreateFirstConnected();

	void                    CheckKinectData(SimpleRotations& rotations);

	/// <summary>
	/// Handle new skeleton data
	/// </summary>
	void                    ProcessSkeleton(SimpleRotations& rotations);
	
	// Direct3D
	HRESULT					InitDevice();
	void					CleanupDevice();
	void					RenderSimpleModel(const SimpleRotations& rotations);

	/// <summary>
	/// Ensure necessary Direct2d resources are created
	/// </summary>
	/// <returns>S_OK if successful, otherwise an error code</returns>
	HRESULT                 EnsureDirect2DResources();

	/// <summary>
	/// Dispose Direct2d resources 
	/// </summary>
	void                    DiscardDirect2DResources();

	/// <summary>
	/// Set the status bar message
	/// </summary>
	/// <param name="szMessage">message to display</param>
	void                    SetStatusMessage(const WCHAR* szMessage);
};
