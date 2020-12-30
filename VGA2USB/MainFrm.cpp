// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "VGA2USB.h"

#include "MainFrm.h"
#include "DBT.h"
#include "setupapi.h"

#include "frmgrab\include\frmgrab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void mprintf( char* format,... );

//#define STR_VENDOR _T("空军雷达学院")
#define STR_VENDOR _T("")

//#define STR_DEVCIE_CONNECTED _T("Capture device connected")
//#define STR_DEVICE_NOT_FOUND  _T("Capture device not found")

#define STR_DEVCIE_CONNECTED _T("设备已连接")
#define STR_DEVICE_NOT_FOUND  _T("未找到设备")


// Copy from HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
static const GUID GUID_DEVINTERFACE_LIST[] = 
{
	// GUID_DEVINTERFACE_USB_DEVICE
	{ 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },

	// GUID_DEVINTERFACE_DISK
	//{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },

	// GUID_DEVINTERFACE_HID, 
	//{ 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
			 
	// GUID_NDIS_LAN_CLASS
	//{ 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }

	//// GUID_DEVINTERFACE_COMPORT
	//{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },

	//// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
	//{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },

	//// GUID_DEVINTERFACE_PARALLEL
	//{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },

	//// GUID_DEVINTERFACE_PARCLASS
	//{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(ID_FULL_SCREEN, OnViewFullScreen)
	ON_COMMAND(ID_DEVICE_CONFIG, OnDeviceConfig)
	ON_COMMAND(ID_NET_CONFIG, OnNetConfig)
	ON_COMMAND(ID_DISPLAY_CONFIG, OnDisplayConfig)
	ON_WM_GETMINMAXINFO()
	ON_UPDATE_COMMAND_UI(ID_FULL_SCREEN, OnUpdateViewFullScreen)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DEVICECHANGE,OnPnpEvent)
	ON_MESSAGE(WM_STATUS_MESSAGE, OnStatusInfo)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	 m_bFullScreen=FALSE;
}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::OnClose()
{
	CFrameWnd::OnClose();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}
	
/*	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

*/

	if (!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
		return -1;      // fail to create
	m_wndToolBar.LoadTrueColorToolBar (32, IDB_TOOLBAR_ENABLE,
									   IDB_TOOLBAR_ENABLE,
									   IDB_TOOLBAR_DISABLE) ;
	m_wndToolBar.SetBarStyle (m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC) ;
	m_wndToolBar.SetWindowText (TEXT("shortcut toolbar")) ;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);

	HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	for(int i=0; i<sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID); i++) {
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
		hDevNotify = RegisterDeviceNotification(this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if( !hDevNotify ) {
			AfxMessageBox(CString("Can't register device notification."));
			return -1;
		}
	}

	m_wndView.CaptureRunStatus=0;
	m_wndView.ScreenWidth=GetSystemMetrics(SM_CXSCREEN);
	m_wndView.ScreenHeight=GetSystemMetrics(SM_CYSCREEN);
		
	if(FrmGrabLocal_Count()>0)
	{
		bDeviceReady = true;
		bReEnumerate = true;	
		m_wndView.strViewInfo.Format(STR_DEVCIE_CONNECTED);
		//void V2U_ListCards(void);
		//V2U_ListCards();		

		HWND hHwnd = GetSafeHwnd();
		void V2U_CaptureStart(HWND hHwnd);
		V2U_CaptureStart(hHwnd);		
	}
	else
	{
		bDeviceReady = false;
		bReEnumerate = false;	
		m_wndView.strViewInfo.Format(STR_DEVICE_NOT_FOUND);
	}

	CString strTitle;
	//strTitle.Format(STR_VENDOR" - VGA2USB");
	strTitle.Format(" VGA2USB ");
	this->SetWindowText(strTitle);

	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;
	
	scaling=::GetPrivateProfileInt("DISPLAY CONFIG","scaling",1,strFileName); 
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.x=((CVGA2USBApp*)AfxGetApp())->left;
	cs.y=((CVGA2USBApp*)AfxGetApp())->top;
	cs.cx=((CVGA2USBApp*)AfxGetApp())->width;
	cs.cy=((CVGA2USBApp*)AfxGetApp())->height;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
/*	
	cs.lpszClass = AfxRegisterWndClass(0);
*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


/**********************************************************************/

#include "DeviceConfigDlg.h"

void CMainFrame::OnDeviceConfig() 
{
	CConfigDlg cfgDlg;

	int nResponse =	cfgDlg.DoModal();

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
int V2U_Set_Params(int gain_red, int gain_green,int gain_blue,
			int offset_red,int offset_green,int offset_blue);

	V2U_Set_Params(cfgDlg.gain_red, cfgDlg.gain_green, cfgDlg.gain_blue,
			cfgDlg.offset_red, cfgDlg.offset_green, cfgDlg.offset_blue);
	
	cfgDlg.WriteConfigInfo();	
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

}

#include "NetConfigDlg.h"

void CMainFrame::OnNetConfig() 
{
	CNetConfigDlg cfgDlg;

	int nResponse =	cfgDlg.DoModal();

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	((CVGA2USBApp*)AfxGetApp())->ip0=cfgDlg.ip0;
	((CVGA2USBApp*)AfxGetApp())->ip1=cfgDlg.ip1;
	((CVGA2USBApp*)AfxGetApp())->ip2=cfgDlg.ip2;
	((CVGA2USBApp*)AfxGetApp())->ip3=cfgDlg.ip3;
	((CVGA2USBApp*)AfxGetApp())->port=cfgDlg.port;

	cfgDlg.WriteConfigInfo();	
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

}

#include "DisplayConfigDlg.h"

void CMainFrame::OnDisplayConfig() 
{
	CDisplayConfigDlg cfgDlg;

	int nResponse =	cfgDlg.DoModal();

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	scaling=cfgDlg.scaling;

	cfgDlg.WriteConfigInfo();	
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

}

void CMainFrame::OnViewFullScreen() 
{
    RECT rectDesktop;
    WINDOWPLACEMENT wpNew;

    if (!IsFullScreen())
    {
        // need to hide all status bars
		m_wndStatusBar.ShowWindow(SW_HIDE);
		m_wndToolBar.ShowWindow(SW_HIDE);

	    // We'll need these to restore the original state.
	    GetWindowPlacement (&m_wpPrev);

	    m_wpPrev.length = sizeof m_wpPrev;

        //Adjust RECT to new size of window
	    ::GetWindowRect ( ::GetDesktopWindow(), &rectDesktop );
	    ::AdjustWindowRectEx(&rectDesktop, GetStyle(), TRUE, GetExStyle());

	    // Remember this for OnGetMinMaxInfo()
	    m_FullScreenWindowRect = rectDesktop;
        
	wpNew = m_wpPrev;
	wpNew.showCmd =  SW_SHOWNORMAL;
	wpNew.rcNormalPosition = rectDesktop;

#if 0	    
         m_pwndFullScrnBar=new CToolBar;

         if (!m_pwndFullScrnBar->Create(this,CBRS_SIZE_DYNAMIC|CBRS_FLOATING) ||
		    !m_pwndFullScrnBar->LoadToolBar(IDR_FULLSCREEN))
    	    {
	    	    TRACE0("Failed to create toolbar\n");
			    return;      // fail to create
	        }
        
         //don't allow the toolbar to dock
         m_pwndFullScrnBar->EnableDocking(0);
		 m_pwndFullScrnBar->SetWindowPos(0, 100,100, 0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);   
		 m_pwndFullScrnBar->SetWindowText(_T("Full Screen"));
	     FloatControlBar(m_pwndFullScrnBar, CPoint(100,100));
#endif		 
	m_bFullScreen=TRUE;

	HWND hWnd = ::FindWindow(_T("Shell_TrayWnd"),NULL);
	::ShowWindow(hWnd,SW_HIDE);
    }
    else
    {
#if 0	
         m_pwndFullScrnBar->DestroyWindow();
		 delete m_pwndFullScrnBar;
#endif

         m_bFullScreen=FALSE;

	m_wndStatusBar.ShowWindow(SW_SHOWNORMAL);
	m_wndToolBar.ShowWindow(SW_SHOWNORMAL);
         wpNew = m_wpPrev;

	HWND hWnd = ::FindWindow(_T("Shell_TrayWnd"),NULL);
	::ShowWindow(hWnd,SW_SHOWNOACTIVATE);
     }
    
     SetWindowPlacement ( &wpNew );

}

void CMainFrame::OnFullScreen() 
{
	OnViewFullScreen();
}

void CMainFrame::EndFullScreen()
{
    if (IsFullScreen())
    {
	m_bFullScreen=FALSE;

	m_wndStatusBar.ShowWindow(SW_SHOWNORMAL);
	m_wndToolBar.ShowWindow(SW_SHOWNORMAL);

	HWND hWnd = ::FindWindow(_T("Shell_TrayWnd"),NULL);
	::ShowWindow(hWnd,SW_SHOWNOACTIVATE);
		
	SetWindowPlacement ( &m_wpPrev );
    }
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if (IsFullScreen())
    {
        lpMMI->ptMaxSize.y = m_FullScreenWindowRect.Height();
	    lpMMI->ptMaxTrackSize.y = lpMMI->ptMaxSize.y;
	    lpMMI->ptMaxSize.x = m_FullScreenWindowRect.Width();
	    lpMMI->ptMaxTrackSize.x = lpMMI->ptMaxSize.x;
    }
   
}

BOOL CMainFrame::IsFullScreen()
{
     return m_bFullScreen;
}

void CMainFrame::OnUpdateViewFullScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();

	if (IsFullScreen())
		pCmdUI->SetCheck();
	else
        pCmdUI->SetCheck(0);

}

LRESULT CMainFrame::OnPnpEvent(WPARAM wParam, LPARAM lParam) 
{
	if ( DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam ) {
		PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
		PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
		PDEV_BROADCAST_HANDLE pDevHnd;
		PDEV_BROADCAST_OEM pDevOem;
		PDEV_BROADCAST_PORT pDevPort;
		PDEV_BROADCAST_VOLUME pDevVolume;
		switch( pHdr->dbch_devicetype ) {
			case DBT_DEVTYP_DEVICEINTERFACE:
				pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
				DeviceStatusChange(pDevInf->dbcc_name, wParam);
				break;

			case DBT_DEVTYP_HANDLE:
				pDevHnd = (PDEV_BROADCAST_HANDLE)pHdr;
				break;

			case DBT_DEVTYP_OEM:
				pDevOem = (PDEV_BROADCAST_OEM)pHdr;
				break;

			case DBT_DEVTYP_PORT:
				pDevPort = (PDEV_BROADCAST_PORT)pHdr;
				break;

			case DBT_DEVTYP_VOLUME:
				pDevVolume = (PDEV_BROADCAST_VOLUME)pHdr;
				break;
		}
	}
	return 0;
}

//#define VGA2USB_VENDOR_NAME "VID_1366&PID_0101"
#define VGA2USB_VENDOR_NAME "VID_5555&PID_1110"

LRESULT CMainFrame::DeviceStatusChange(char *dbcc_name, WPARAM wParam)
{
	ASSERT(lstrlen(dbcc_name) > 4);
	CString szDevId = dbcc_name+4;
	szDevId.Replace(_T('#'), _T('\\'));
	szDevId.MakeUpper();

/*
	CString szClass;
	int idx = szDevId.Find(_T('\\'));
	ASSERT(-1 != idx );
	szClass = szDevId.Left(idx);
*/

	if( strncmp(szDevId.GetBuffer(szDevId.GetLength())+4,VGA2USB_VENDOR_NAME,strlen(VGA2USB_VENDOR_NAME))!=0 )
	return -1;
	
	if ( DBT_DEVICEARRIVAL == wParam ) {
	mprintf("OnPnpEvent(),Adding %s\r\n",szDevId.GetBuffer(szDevId.GetLength()));
	} else {
	mprintf("OnPnpEvent(),Removing %s\r\n",szDevId.GetBuffer(szDevId.GetLength()));		
	}

	switch(wParam)
	{
		case DBT_DEVICEARRIVAL:
		if( bReEnumerate )
		{
		bDeviceReady	= true;
		//device inserted
		m_wndView.strViewInfo.Format(STR_DEVCIE_CONNECTED);	
		mprintf("device inserted\n");

		HWND hHwnd = GetSafeHwnd();
		void V2U_CaptureStart(HWND hHwnd);
		V2U_CaptureStart(hHwnd);	
		}
		else
		{
		bReEnumerate=true;
		}
			break;
	
		case DBT_DEVICEREMOVECOMPLETE:
		if (bDeviceReady) {
			bDeviceReady = false;
			bReEnumerate = false;
		//device removed
			m_wndView.strViewInfo.Format(STR_DEVICE_NOT_FOUND);
			mprintf("device removed\n");

			void V2U_CaptureStop(void);
			V2U_CaptureStop();

			UpdateWindowSize(480,320,0);
		}		
			break;
	
		case DBT_DEVNODES_CHANGED:
	
			break;
		
		default:
			break;
	}
	
	m_wndView.Invalidate(FALSE);			

	return 0;
}

LRESULT CMainFrame::OnStatusInfo(WPARAM wParam, LPARAM lParam) 
{
	SetStatusText((int)wParam,(char*)lParam);
	return 0;	
}

void CMainFrame::UpdateWindowSize(int width, int height,int vfreq)
{
	RECT rect;
	RECT toolbar_rect;
	RECT statusbar_rect;
	
	WINDOWPLACEMENT wpOld;
	WINDOWPLACEMENT wpNew;
	
	m_wndView.FrameWidth=width;
	m_wndView.FrameHeight=height;
	m_wndView.UpdateDestWindowSize(width,height);

	if(m_wndView.FrameBuffer)
	{
	 free(m_wndView.FrameBuffer);
	}

	m_wndView.FrameBuffer=(unsigned int *)malloc(width*height*4);
	
    if (!IsFullScreen())
    	{
//	GetWindowRect(&rect);
//	SetWindowPos(NULL,rect.left,rect.top,width,height,SWP_NOACTIVATE|SWP_DRAWFRAME);	

	m_wndView.GetWindowRect(&rect);
	m_wndToolBar.GetWindowRect(&toolbar_rect);
	m_wndStatusBar.GetWindowRect(&statusbar_rect);

	rect.right=rect.left+width;
	rect.bottom=rect.top+height+(toolbar_rect.bottom-toolbar_rect.top)
		+(statusbar_rect.bottom-statusbar_rect.top);
	
	::AdjustWindowRectEx(&rect, GetStyle(), TRUE, GetExStyle());

	GetWindowPlacement (&wpOld);

	wpNew=wpOld;
	wpNew.showCmd =  SW_SHOWNORMAL;
	wpNew.rcNormalPosition = rect;

	SetWindowPlacement ( &wpNew );

	if( vfreq !=0 )
		{
		CString strTitle;
		//strTitle.Format(STR_VENDOR" - VGA2USB - %dx%d %d.%03d Hz",width,height,vfreq/1000,vfreq%1000);
		strTitle.Format(" VGA2USB - %dx%d %d.%03d Hz",width,height,vfreq/1000,vfreq%1000);
		this->SetWindowText(strTitle);
		}
    	}
	
}

void CMainFrame::UpdateViewInfo(char * pInfo)
{
	m_wndView.strViewInfo.Format("%s",pInfo);
	m_wndView.Invalidate(FALSE);
}

void CMainFrame::UpdateFrame(void * pBuffer,int buffer_len)
{
#if 0
#define BGR24

//RGB16
#define COLOR_RGB16TO32(a)\
	                  ( ((a<<3)&0x000000ff)|((a<<5)&0x0000ff00)|((a<<8)&0x00ff0000) )

//BGR16
#define COLOR_BGR16TO32(a)\
	                  ( ((a>>8)&0x000000ff)|((a<<5)&0x0000ff00)|((a<<19)&0x00ff0000) )

#if defined(RGB16)||defined(BGR16)
	unsigned short usColor;
#endif
	
	unsigned int *capture_buffer=(unsigned int *)m_wndView.FrameBuffer;
	int width=m_wndView.FrameWidth;
	int height=m_wndView.FrameHeight;
	
	int i,j;
	unsigned char r,g,b,*p;
	
	for ( i = 0;i < height;i++)
	{
		for ( j = 0;j < width;j++)
		{  
		#ifdef RGB16
		usColor=*( (unsigned short*)pBuffer+i*width +j );
		*( (unsigned int*)capture_buffer+i* width +j )=COLOR_RGB16TO32(usColor);
		#endif

		#ifdef BGR16
		usColor=*( (unsigned short*)pBuffer+i*width +j );
		*( (unsigned int*)capture_buffer+i* width +j )=COLOR_BGR16TO32(usColor);
		#endif

		#ifdef BGR24
		p=(unsigned char*)pBuffer+(i*width +j)*3;
		b=*p++;
		g=*p++;
		r=*p++;
		//*( (unsigned int*)capture_buffer+(height-i-1)* width +j )=(r<<16)|(g<<8)|b;//top-down image
		*( (unsigned int*)capture_buffer+i* width +j )=(r<<16)|(g<<8)|b;// down-top image
		#endif
        }
	}
#else
	if(pBuffer)
	memcpy((unsigned char*)m_wndView.FrameBuffer,pBuffer,buffer_len);
#endif
		
	m_wndView.Invalidate(FALSE);
}

void CMainFrame::UpdateRunStatus(int status )
{
	m_wndView.CaptureRunStatus=status;
}

int CMainFrame::GetRunStatus()
{
	return m_wndView.CaptureRunStatus;
} 

void CMainFrame::SetButtonState(int id,int status )
{
	m_wndToolBar.GetToolBarCtrl().SetState(id,   status);
}

void CMainFrame::SetStatusText(int id,char * text )
{
	strStatusInfo.Format("%s",text);
	m_wndStatusBar.SetPaneText(id,strStatusInfo.GetBuffer(strStatusInfo.GetLength()),true);
}

unsigned char *CMainFrame::GetFrameBuffer()
{
	return (unsigned char*)m_wndView.FrameBuffer;
}
	
/*****************************************************************/

void ChangeWindowSize(int width, int height,int vfreq)
{
	mprintf("ChangeWindowSize width=%d,height=%d,vfreq=%d.%03d Hz\n",width,height,vfreq/1000,vfreq%1000);
	((CMainFrame* )AfxGetApp()->m_pMainWnd)->UpdateWindowSize(width,height,vfreq);
}

void ViewDisplayInfo(char * pInfo)
{
	((CMainFrame* )AfxGetApp()->m_pMainWnd)->UpdateViewInfo(pInfo);
}

void UpdateDisplayFrame(void * pBuffer,int buffer_len)
{
	((CMainFrame* )AfxGetApp()->m_pMainWnd)->UpdateFrame(pBuffer,buffer_len);	

}

void SetCaptureRunStatus(int status)
{
	((CMainFrame* )AfxGetApp()->m_pMainWnd)->UpdateRunStatus(status);	

}

int GetCaptureRunStatus()
{
	return ((CMainFrame* )AfxGetApp()->m_pMainWnd)->GetRunStatus();	
}

void SetStatusInfo(HWND hHwnd,int id,char * info )
{
	::PostMessage(hHwnd, WM_STATUS_MESSAGE, (WPARAM)id, (LPARAM )info);   
}

void GetVideoConfigInfo()
{
	int gain_red,gain_green,gain_blue;
	int offset_red,offset_green,offset_blue;

	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;
	
	gain_red=::GetPrivateProfileInt("GAIN CONFIG","red",128,strFileName); 
	gain_green=::GetPrivateProfileInt("GAIN CONFIG","green",128,strFileName);
	gain_blue=::GetPrivateProfileInt("GAIN CONFIG","blue",128,strFileName);

	offset_red=::GetPrivateProfileInt("OFFSET CONFIG","red",32,strFileName);
	offset_green=::GetPrivateProfileInt("OFFSET CONFIG","green",32,strFileName);
	offset_blue=::GetPrivateProfileInt("OFFSET CONFIG","blue",32,strFileName);

	int V2U_Set_Params(int gain_red, int gain_green,int gain_blue,
			int offset_red,int offset_green,int offset_blue);

	V2U_Set_Params(gain_red, gain_green, gain_blue,
			offset_red, offset_green, offset_blue);
}

int GetNetAddress(unsigned char *p)
{
	*p++=((CVGA2USBApp*)AfxGetApp())->ip0;
	*p++=((CVGA2USBApp*)AfxGetApp())->ip1;
	*p++=((CVGA2USBApp*)AfxGetApp())->ip2;
	*p++=((CVGA2USBApp*)AfxGetApp())->ip3;
	
	return ((CVGA2USBApp*)AfxGetApp())->port;
}

void NetDeviceDisconnected(void)
{
	mprintf("NetDeviceDisconnected()\n");
	((CMainFrame* )AfxGetApp()->m_pMainWnd)->SetButtonState(ID_CONNECT_NETDEVICE,TBSTATE_ENABLED);
	((CVGA2USBApp*)AfxGetApp())->m_bNetDeviceConnected=false;
}

unsigned char *GetFrameBuffer(void)
{
	return ((CMainFrame* )AfxGetApp()->m_pMainWnd)->GetFrameBuffer();
}



