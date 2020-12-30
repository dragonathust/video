/////////////////////////////////////////////////////////////////////////////
// Config dialog

class CDisplayConfigDlg : public CDialog
{
public:
	CDisplayConfigDlg();

	void CDisplayConfigDlg::WriteConfigInfo();
	void CDisplayConfigDlg::GetConfigInfo();
			
// Dialog Data
	int scaling;

	CButton 	m_scalingCtrl;
	
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_DISPLAY_DIALOG };
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

CDisplayConfigDlg::CDisplayConfigDlg() : CDialog(CDisplayConfigDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)

	//}}AFX_DATA_INIT
}

void CDisplayConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_CHECK_SCALING, m_scalingCtrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDisplayConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDisplayConfigDlg::WriteConfigInfo()
{
	CString strInfo;
	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;

	strInfo.Format("%d",scaling);
	::WritePrivateProfileString("DISPLAY CONFIG","scaling",strInfo,strFileName);
	
}

void CDisplayConfigDlg::GetConfigInfo()
{
	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;
	
	scaling=::GetPrivateProfileInt("DISPLAY CONFIG","scaling",1,strFileName); 

}

BOOL CDisplayConfigDlg::OnInitDialog() 
{
	CString strInfo;
	
	CDialog::OnInitDialog();

	GetConfigInfo();

	if(scaling)
	m_scalingCtrl.SetCheck(TRUE);
	else
	m_scalingCtrl.SetCheck(FALSE);
		
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisplayConfigDlg::OnOK() 
{
	// TODO: Add your message handler code here

	scaling=m_scalingCtrl.GetCheck();

	CDialog::OnOK();		
}

