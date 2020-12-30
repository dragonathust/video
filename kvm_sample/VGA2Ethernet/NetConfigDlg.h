/////////////////////////////////////////////////////////////////////////////
// Config dialog

class CNetConfigDlg : public CDialog
{
public:
	CNetConfigDlg();

	void CNetConfigDlg::WriteConfigInfo();
	void CNetConfigDlg::GetConfigInfo();
			
// Dialog Data
	BYTE ip0,ip1,ip2,ip3;
	int port;

	CIPAddressCtrl	m_IPCtrl;
	CEdit	m_portCtrl;
	
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_NETCFG_DIALOG };
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
	virtual void OnOK();	
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CNetConfigDlg::CNetConfigDlg() : CDialog(CNetConfigDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)

	//}}AFX_DATA_INIT
}

void CNetConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_IPADDRESS, m_IPCtrl);
	DDX_Control(pDX, IDC_EDIT_PORT, m_portCtrl);	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNetConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNetConfigDlg::WriteConfigInfo()
{
	CString strInfo;
	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;

	strInfo.Format("%d",ip0);
	::WritePrivateProfileString("NETWORK CONFIG","ip0",strInfo,strFileName);
	strInfo.Format("%d",ip1);
	::WritePrivateProfileString("NETWORK CONFIG","ip1",strInfo,strFileName);
	strInfo.Format("%d",ip2);
	::WritePrivateProfileString("NETWORK CONFIG","ip2",strInfo,strFileName);
	strInfo.Format("%d",ip3);
	::WritePrivateProfileString("NETWORK CONFIG","ip3",strInfo,strFileName);
	strInfo.Format("%d",port);
	::WritePrivateProfileString("NETWORK CONFIG","port",strInfo,strFileName);
	
}

void CNetConfigDlg::GetConfigInfo()
{
	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;
	
	ip0=::GetPrivateProfileInt("NETWORK CONFIG","ip0",127,strFileName); 
	ip1=::GetPrivateProfileInt("NETWORK CONFIG","ip1",0,strFileName); 
	ip2=::GetPrivateProfileInt("NETWORK CONFIG","ip2",0,strFileName); 
	ip3=::GetPrivateProfileInt("NETWORK CONFIG","ip3",1,strFileName); 
	port=::GetPrivateProfileInt("NETWORK CONFIG","port",1234,strFileName); 
}

BOOL CNetConfigDlg::OnInitDialog() 
{
	CString strInfo;
	
	CDialog::OnInitDialog();

	GetConfigInfo();

	m_IPCtrl.SetAddress(ip0,ip1,ip2,ip3);
	strInfo.Format("%d",port);
	m_portCtrl.SetWindowText(strInfo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNetConfigDlg::OnOK() 
{
	// TODO: Add your message handler code here
	m_IPCtrl.GetAddress(ip0,ip1,ip2,ip3);
	
	char s[12];
	ZeroMemory(s,12);
	m_portCtrl.GetLine(0,s,8);
	port = atol(s);
	if(port>65535)
	port=65535;

	CDialog::OnOK();		
}
