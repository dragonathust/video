/////////////////////////////////////////////////////////////////////////////
// Config dialog

class CConfigDlg : public CDialog
{
public:
	CConfigDlg();

	void CConfigDlg::WriteConfigInfo();
	void CConfigDlg::GetConfigInfo();
			
// Dialog Data
	int gain_red,gain_green,gain_blue;
	int offset_red,offset_green,offset_blue;

	CSliderCtrl m_Slider_gain_red;
	CSliderCtrl m_Slider_gain_green;
	CSliderCtrl m_Slider_gain_blue;

	CSliderCtrl m_Slider_offset_red;
	CSliderCtrl m_Slider_offset_green;
	CSliderCtrl m_Slider_offset_blue;

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_CONFIG_DIALOG };
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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd,UINT nCtlColor);
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CConfigDlg::CConfigDlg() : CDialog(CConfigDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)

	//}}AFX_DATA_INIT
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
		DDX_Control(pDX, IDC_GAIN_RED, m_Slider_gain_red);
		DDX_Control(pDX, IDC_GAIN_GREEN, m_Slider_gain_green);
		DDX_Control(pDX, IDC_GAIN_BLUE, m_Slider_gain_blue);

		DDX_Control(pDX, IDC_OFFSET_RED, m_Slider_offset_red);
		DDX_Control(pDX, IDC_OFFSET_GREEN, m_Slider_offset_green);
		DDX_Control(pDX, IDC_OFFSET_BLUE, m_Slider_offset_blue);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR()
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CConfigDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CDialog::OnHScroll(nSBCode,nPos,pScrollBar);

	if(nSBCode==SB_THUMBPOSITION)
	{	
	gain_red = ((CSliderCtrl*)GetDlgItem(IDC_GAIN_RED))->GetPos();
	gain_green = ((CSliderCtrl*)GetDlgItem(IDC_GAIN_GREEN))->GetPos();
	gain_blue = ((CSliderCtrl*)GetDlgItem(IDC_GAIN_BLUE))->GetPos();

	offset_red = ((CSliderCtrl*)GetDlgItem(IDC_OFFSET_RED))->GetPos();
	offset_green = ((CSliderCtrl*)GetDlgItem(IDC_OFFSET_GREEN))->GetPos();
	offset_blue = ((CSliderCtrl*)GetDlgItem(IDC_OFFSET_BLUE))->GetPos();
	
	mprintf("gain R: %d G: %d B: %d\n",gain_red,gain_green,gain_blue);	
	mprintf("offset R: %d G: %d B: %d\n",offset_red,offset_green,offset_blue);
	}
	
}

HBRUSH CConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd,UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC,pWnd,nCtlColor);
	
	switch(nCtlColor)
	{	
		case CTLCOLOR_STATIC:
			switch(pWnd->GetDlgCtrlID())
			{		
				case IDC_STATIC_RED:
					pDC->SetTextColor(RGB(255,0,0));
				break;

				case IDC_STATIC_GREEN:
					pDC->SetTextColor(RGB(0,255,0));
				break;

				case IDC_STATIC_BLUE:
					pDC->SetTextColor(RGB(0,0,255));
				break;					
			}
			break;
		
		default:
			break;
	}
	
	return hbr;
}

void CConfigDlg::WriteConfigInfo()
{
	CString strInfo;
	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;
	
	strInfo.Format("%d",gain_red);
	::WritePrivateProfileString("GAIN CONFIG","red",strInfo,strFileName);
	strInfo.Format("%d",gain_green);
	::WritePrivateProfileString("GAIN CONFIG","green",strInfo,strFileName);
	strInfo.Format("%d",gain_blue);
	::WritePrivateProfileString("GAIN CONFIG","blue",strInfo,strFileName);

	strInfo.Format("%d",offset_red);
	::WritePrivateProfileString("OFFSET CONFIG","red",strInfo,strFileName);
	strInfo.Format("%d",offset_green);
	::WritePrivateProfileString("OFFSET CONFIG","green",strInfo,strFileName);
	strInfo.Format("%d",offset_blue);
	::WritePrivateProfileString("OFFSET CONFIG","blue",strInfo,strFileName);
	
}

void CConfigDlg::GetConfigInfo()
{
	CString strFileName = ((CVGA2USBApp*)AfxGetApp())->strConfigFileName;
	
	gain_red=::GetPrivateProfileInt("GAIN CONFIG","red",128,strFileName); 
	gain_green=::GetPrivateProfileInt("GAIN CONFIG","green",128,strFileName);
	gain_blue=::GetPrivateProfileInt("GAIN CONFIG","blue",128,strFileName);

	offset_red=::GetPrivateProfileInt("OFFSET CONFIG","red",32,strFileName);
	offset_green=::GetPrivateProfileInt("OFFSET CONFIG","green",32,strFileName);
	offset_blue=::GetPrivateProfileInt("OFFSET CONFIG","blue",32,strFileName);
}
			
BOOL CConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetConfigInfo();
	
/* gain */
	m_Slider_gain_red.SetRange(0,255);
	m_Slider_gain_red.SetTicFreq(16);
	m_Slider_gain_red.SetPos(gain_red);

	m_Slider_gain_green.SetRange(0,255);
	m_Slider_gain_green.SetTicFreq(16);
	m_Slider_gain_green.SetPos(gain_green);
	
	m_Slider_gain_blue.SetRange(0,255);
	m_Slider_gain_blue.SetTicFreq(16);
	m_Slider_gain_blue.SetPos(gain_blue);

/* offset */	
	m_Slider_offset_red.SetRange(0,63);
	m_Slider_offset_red.SetTicFreq(4);
	m_Slider_offset_red.SetPos(offset_red);

	m_Slider_offset_green.SetRange(0,63);
	m_Slider_offset_green.SetTicFreq(4);
	m_Slider_offset_green.SetPos(offset_green);
	
	m_Slider_offset_blue.SetRange(0,63);
	m_Slider_offset_blue.SetTicFreq(4);
	m_Slider_offset_blue.SetPos(offset_blue);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}