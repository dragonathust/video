// VGA2USB.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VGA2USB.h"

#include "otherlib\HyperLink.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************************************************************************/
//将调试信息输出到控制台窗口
/***********************************************************************************************************/
    #ifdef _DEBUG
         FILE* _fStdOut = NULL;
         HANDLE _hStdOut = NULL;
    #endif

// width and height is the size of Console Window 
// if you give a file name the output will be writted to the file

void startConsoleWin( int width=80, int height=200, char* fname=NULL);

void startConsoleWin( int width, int height, char* fname)
{
	
	#ifdef _DEBUG
	
	AllocConsole();
	SetConsoleTitle("Debug Window");
	 _hStdOut = GetStdHandle( STD_OUTPUT_HANDLE);
	 
	 COORD co= {width,height };
	 SetConsoleScreenBufferSize( _hStdOut, co );
	 if( fname )
	 _fStdOut = fopen( fname,"w");
	 #endif
}

void mprintf( char* format,... )
{
	#ifdef _DEBUG
        char s[4096];
        va_list argptr;
        va_start( argptr,format);
        vsprintf(s,format,argptr);
        va_end(argptr);
        
        DWORD cCharsWritten;
        if( _hStdOut )
        WriteConsole( _hStdOut, s ,strlen(s), &cCharsWritten, NULL );
        if( _fStdOut )
        fprintf( _fStdOut,s );
	#endif
}

/////////////////////////////////////////////////////////////////////////////
// CVGA2USBApp

BEGIN_MESSAGE_MAP(CVGA2USBApp, CWinApp)
	//{{AFX_MSG_MAP(CVGA2USBApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_SNAPSHOT_SAVE, OnAppSave)
	ON_COMMAND(ID_VIDEO_REC, OnAppRecord)
	ON_COMMAND(ID_VIDEO_PAUSE, OnAppPause)
	ON_COMMAND(ID_FULLSCREEN,OnFullScreen)
	ON_COMMAND(ID_CONNECT_NETDEVICE, OnConnectNetdevice)	
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVGA2USBApp construction

CVGA2USBApp::CVGA2USBApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_bRecordStart=false;
	m_bRecordPause=false;
	m_bNetServiceStart=false;
	m_bNetDeviceConnected=false;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVGA2USBApp object

CVGA2USBApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVGA2USBApp initialization

BOOL CVGA2USBApp::InitInstance()
{
		#ifdef _DEBUG
	    startConsoleWin();
		#endif	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	strConfigFileName = GetAppPath()+"VGA2USB.ini";
	
	GetWindowConfig();
	
	void V2U_NetInit(void);
	V2U_NetInit();
	
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);


	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CVGA2USBApp message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	CHyperLink m_mailCtrl;
	CHyperLink m_addressCtrl;
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_mailCtrl.SetURL(_T("mailto:dragonathust@gmail.com"));
	//m_mailCtrl.SetURL(_T("http://www.google.com"));

	m_addressCtrl.SetURL(_T("http://dragonathust.taobao.com/"));
	
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
		DDX_Control(pDX, IDC_MAILTO, m_mailCtrl);
		DDX_Control(pDX, IDC_ADDRESS, m_addressCtrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_BUILD)->SetWindowText("Build "__DATE__" "__TIME__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
// App command to run the dialog
void CVGA2USBApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/****************************************************************************/

void CVGA2USBApp::OnAppSave()
{
//	((CMainFrame* )m_pMainWnd)->UpdateWindowSize(800,480);

	static CString sFileName;
	static char szFilter[] = "bmp文件(*.bmp)|*.bmp|png文件(*.png)|*.png|jpeg文件(*.jpg)|*.jpg||";

	if(sFileName.IsEmpty())
	{
	CFileDialog FileDlg( FALSE, _T("png"), NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter );
		if( FileDlg.DoModal() == IDOK )
		{
		sFileName = FileDlg.GetPathName();
		}
	}

	if(!sFileName.IsEmpty())
	{
		void SaveSnapshotOnce(CString filename);
		SaveSnapshotOnce(sFileName);
		mprintf(sFileName.GetBuffer(sFileName.GetLength()));	
		((CMainFrame* )m_pMainWnd)->SetStatusText(0,sFileName.GetBuffer(sFileName.GetLength()));
	}	
}

void CVGA2USBApp::OnAppRecord()
{
	static char szFilter[] = "avi文件(*.avi)|*.avi||";

	if( !m_bRecordStart )
	{
	CFileDialog FileDlg( FALSE, _T("avi"), NULL,	OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter );
	if( FileDlg.DoModal() == IDOK )
	{
		CString sFileName;
		sFileName = FileDlg.GetPathName();
		mprintf(sFileName.GetBuffer(sFileName.GetLength()));
		((CMainFrame* )m_pMainWnd)->SetButtonState(ID_VIDEO_REC,TBSTATE_PRESSED);
		void VideoRecordStart(CString filename);
		VideoRecordStart(sFileName);
		m_bRecordStart=true;
	}
	}
	else
	{
	((CMainFrame* )m_pMainWnd)->SetButtonState(ID_VIDEO_REC,TBSTATE_ENABLED);
		if(m_bRecordPause)
		{
		((CMainFrame* )m_pMainWnd)->SetButtonState(ID_VIDEO_PAUSE,TBSTATE_ENABLED);	
		void VideoRecordResume(void);
		VideoRecordResume();
		m_bRecordPause=false;
		}
	void VideoRecordStop(void);
	VideoRecordStop();
	m_bRecordStart=false;
	}
}

void CVGA2USBApp::OnAppPause()
{
//	void V2U_AVI_Test_Start(HWND hHwnd);
//	V2U_AVI_Test_Start(((CMainFrame* )m_pMainWnd)->GetSafeHwnd());

	if(m_bRecordStart)
	{
		if( !m_bRecordPause )
		{
		((CMainFrame* )m_pMainWnd)->SetButtonState(ID_VIDEO_PAUSE,TBSTATE_PRESSED);	
		void VideoRecordPause(void);
		VideoRecordPause();
		m_bRecordPause=true;
		}
		else
		{
		((CMainFrame* )m_pMainWnd)->SetButtonState(ID_VIDEO_PAUSE,TBSTATE_ENABLED);	
		void VideoRecordResume(void);
		VideoRecordResume();
		m_bRecordPause=false;
		}
	}
}

BOOL CVGA2USBApp::SaveAllModified()
{
	RECT rect;
	GetWindowRect(this->m_pMainWnd->GetSafeHwnd(),&rect);
	
	CString strInfo;
	
	strInfo.Format("%d",rect.left);
	::WritePrivateProfileString("WINDOW CONFIG","left",strInfo,strConfigFileName);	
	strInfo.Format("%d",rect.top);
	::WritePrivateProfileString("WINDOW CONFIG","top",strInfo,strConfigFileName);
	strInfo.Format("%d",rect.right);
	::WritePrivateProfileString("WINDOW CONFIG","right",strInfo,strConfigFileName);	
	strInfo.Format("%d",rect.bottom);
	::WritePrivateProfileString("WINDOW CONFIG","bottom",strInfo,strConfigFileName);	
	
	void V2U_NetCleanup(void);
	V2U_NetCleanup();
	
	return true;
}

CString CVGA2USBApp::GetAppPath()
{
	CString strAppPath;
	
	DWORD dwResult = ::GetModuleFileName(NULL,strAppPath.GetBuffer(MAX_PATH),MAX_PATH);
	ASSERT(dwResult);
	strAppPath.ReleaseBuffer();
	strAppPath = strAppPath.Left(strAppPath.ReverseFind(_T('\\'))+1);
	
	return strAppPath;
}

void CVGA2USBApp::GetWindowConfig()
{
	left=::GetPrivateProfileInt("WINDOW CONFIG","left",320,strConfigFileName); 
	top=::GetPrivateProfileInt("WINDOW CONFIG","top",240,strConfigFileName); 
	right=::GetPrivateProfileInt("WINDOW CONFIG","right",800,strConfigFileName); 
	bottom=::GetPrivateProfileInt("WINDOW CONFIG","bottom",600,strConfigFileName); 
	width=right-left;
	height=bottom-top;

	ip0=::GetPrivateProfileInt("NETWORK CONFIG","ip0",127,strConfigFileName); 
	ip1=::GetPrivateProfileInt("NETWORK CONFIG","ip1",0,strConfigFileName); 
	ip2=::GetPrivateProfileInt("NETWORK CONFIG","ip2",0,strConfigFileName); 
	ip3=::GetPrivateProfileInt("NETWORK CONFIG","ip3",1,strConfigFileName); 
	port=::GetPrivateProfileInt("NETWORK CONFIG","port",1234,strConfigFileName); 
	
}

/////////////////////////////////////////////////////////////////////////////
// CVGA2USBApp message handlers

void CVGA2USBApp::OnFullScreen()
{

	((CMainFrame* )m_pMainWnd)->OnFullScreen();

}

void CVGA2USBApp::OnConnectNetdevice()
{
	if(!m_bNetDeviceConnected)
	{
	((CMainFrame* )m_pMainWnd)->SetButtonState(ID_CONNECT_NETDEVICE,TBSTATE_PRESSED);	
	HWND hHwnd = ((CMainFrame* )m_pMainWnd)->GetSafeHwnd();
	void V2U_NetCaptureStart(HWND hHwnd);
	V2U_NetCaptureStart(hHwnd);
	m_bNetDeviceConnected=true;
	}
	else
	{
	void V2U_NetCaptureStop(void);
	V2U_NetCaptureStop();
	}
}

