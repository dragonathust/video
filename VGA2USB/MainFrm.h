// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__55504317_3413_4969_A187_A2538A45D752__INCLUDED_)
#define AFX_MAINFRM_H__55504317_3413_4969_A187_A2538A45D752__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildView.h"
#include "otherlib\TrueColorToolBar.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	void UpdateWindowSize(int width, int height,int vfreq);
	void UpdateViewInfo(char * pInfo);
	void UpdateFrame(void * pBuffer,int buffer_len);
	void UpdateRunStatus(int status );
	void SetButtonState(int id,int status );
	void SetStatusText(int id,char * text );
	unsigned char *GetFrameBuffer();
	int GetRunStatus();

	//	CToolBar * m_pwndFullScrnBar;
    BOOL IsFullScreen();
	void OnFullScreen();
	void EndFullScreen();
	WINDOWPLACEMENT m_wpPrev;
	CRect m_FullScreenWindowRect;

	CString strStatusInfo;
	
	bool bDeviceReady;
	bool bReEnumerate;

/*
	int gain_red;
	int gain_green;
	int gain_blue;

	int offset_red;
	int offset_green;
	int offset_blue;
*/

	int scaling;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CTrueColorToolBar    m_wndToolBar;
	CChildView    m_wndView;

    BOOL m_bFullScreen;
	LRESULT CMainFrame::DeviceStatusChange(char *dbcc_name, WPARAM wParam);

	// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnViewFullScreen();
	afx_msg void OnDeviceConfig();
	afx_msg void OnNetConfig();
	afx_msg void OnDisplayConfig();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnUpdateViewFullScreen(CCmdUI* pCmdUI);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	afx_msg LRESULT OnPnpEvent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStatusInfo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__55504317_3413_4969_A187_A2538A45D752__INCLUDED_)
