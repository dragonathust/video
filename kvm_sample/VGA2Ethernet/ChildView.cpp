// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "VGA2USB.h"
#include "ChildView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void mprintf( char* format,... );
void HIDSendKeyMsg(char key, int state);
void HIDSendMouseMsg(int x, int y, unsigned int nFlags);

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
	FrameBuffer=NULL;
}

CChildView::~CChildView()
{
	if(FrameBuffer)
	free(FrameBuffer);
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	//cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::UpdateDestWindowSize(int x,int y)
 {
	if( (FrameWidth<=ScreenWidth)&&(FrameHeight<=ScreenHeight) )
		{
		x_offset=(ScreenWidth-FrameWidth)/2;
		y_offset=(ScreenHeight-FrameHeight)/2;
		}
	else
		{
		x_offset=0;
		y_offset=0;
		}
 }

//#define STR_TEXT_FONT "Arial Black"
#define STR_TEXT_FONT "宋体"

void CChildView::OnPaint() 
{
	BOOL bIsFullScreen;
	bIsFullScreen = ((CMainFrame* )GetParent())->IsFullScreen();

	int scaling = ((CMainFrame* )GetParent())->scaling;
		
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rcClient;
	GetClientRect(&rcClient);

	CBitmap Bitmap;
	CBitmap* pbmOld = NULL;

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	Bitmap.CreateCompatibleBitmap(&dc,rcClient.right,rcClient.bottom);
	pbmOld = dcMem.SelectObject(&Bitmap);

	dcMem.PatBlt(0, 0,rcClient.right, rcClient.bottom, bIsFullScreen?BLACKNESS:WHITENESS);
	
  if( CaptureRunStatus  )
  	{

  	if( FrameBuffer )
	{

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFO);
	bmi.bmiHeader.biWidth       = FrameWidth;
	bmi.bmiHeader.biHeight      = -FrameHeight; // top-down image
//	bmi.bmiHeader.biHeight      = FrameHeight; // down-top image
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = FrameWidth*FrameHeight*4;

	if(scaling)
	{
	int dest_width = bIsFullScreen?ScreenWidth:rcClient.right;
	int dest_height = bIsFullScreen?ScreenHeight:rcClient.bottom;
	
	SetStretchBltMode(dcMem.m_hDC, /*COLORONCOLOR*/ HALFTONE);
	StretchDIBits( dcMem.m_hDC,
                         // destination rectangle
                         0, 0,
                         //FrameWidth, FrameHeight,
			   //rcClient.right,rcClient.bottom,
			   dest_width,dest_height,
                         // source rectangle
                         0, 0, FrameWidth, FrameHeight,
                         ( void *)FrameBuffer,
                         &bmi,
                         DIB_RGB_COLORS,
                         SRCCOPY);
	}
	else
	{
	int x=bIsFullScreen?x_offset:0;
	int y=bIsFullScreen?y_offset:0;
		
	SetDIBitsToDevice(dcMem.m_hDC,
	                 // destination rectangle
			x,y,
			FrameWidth,FrameHeight,
			//rcClient.right,rcClient.bottom,
			//SrcX,SrcY,nStartScan,nNumScans
			0,0,0,FrameHeight,
		         ( void *)FrameBuffer,
		         &bmi,
		         DIB_RGB_COLORS);
	}
	
//	mprintf("rcClient.right=%d,rcClient.bottom=%d\n",rcClient.right,rcClient.bottom);
  	}
	
  	}
  else
  	{
  	COLORREF	colorRef;	
	
	if(bIsFullScreen)
	{
	colorRef = dcMem.SetTextColor(RGB( 0, 255 ,0 ));
	dcMem.SetBkColor(RGB( 0, 0 ,0 ));
	}

	CFont font;
	font.CreateFont(	 -32,           //字体字符的逻辑高度
			             0,                //字符平均宽度取默认值
						 0,                //文本行角度为0，水平
						 0,                //字符角度为0，正立
						 FW_NORMAL,        //正常字体
						 TRUE,            //不倾斜 
						 FALSE,            //不加下划线
						 FALSE,            //不加删除线
						 ANSI_CHARSET,       
						 OUT_DEFAULT_PRECIS,
						 CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,
						 DEFAULT_PITCH|FF_MODERN,
						 STR_TEXT_FONT);

	CFont *oldFont=dcMem.SelectObject (&font);

	dcMem.SetTextAlign(TA_BASELINE|TA_CENTER);
	dcMem.TextOut(rcClient.right/2,rcClient.bottom/2, strViewInfo);	

	dcMem.SelectObject(oldFont);

	if(bIsFullScreen)
	{
	dcMem.SetTextColor( colorRef );
	}

  	}

	dc.BitBlt(0,0,rcClient.right,rcClient.bottom,&dcMem, 0, 0, /*SRCINVERT*/SRCCOPY);

	dcMem.SelectObject(pbmOld);
	dcMem.DeleteDC();
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( nChar == VK_ESCAPE )
	{
		((CMainFrame* )AfxGetApp()->m_pMainWnd)->EndFullScreen();
	}

	if( nChar == VK_F1 )
	{
		((CMainFrame* )AfxGetApp()->m_pMainWnd)->OnFullScreen();	
	}

	mprintf("key down[0x%x,0x%x,0x%x]\n",nChar,nRepCnt,nFlags);
	HIDSendKeyMsg(nChar,1);

	CWnd::OnKeyDown(nChar,nRepCnt,nFlags);	
}

void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	mprintf("key up[0x%x,0x%x,0x%x]\n",nChar,nRepCnt,nFlags);
	HIDSendKeyMsg(nChar,0);

	CWnd::OnKeyUp(nChar,nRepCnt,nFlags);	
}

void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
		((CMainFrame* )AfxGetApp()->m_pMainWnd)->OnFullScreen();

	CWnd::OnLButtonDblClk(nFlags,point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	SetCursor(LoadCursor(NULL,NULL));

    //mprintf("0x%x,%d,%d\n",nFlags,point.x,point.y);
    HIDSendMouseMsg(point.x,point.y,nFlags);

	CWnd::OnMouseMove(nFlags,point);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCursor(LoadCursor(NULL,NULL));

    //mprintf("0x%x,%d,%d\n",nFlags,point.x,point.y);
    HIDSendMouseMsg(point.x,point.y,nFlags);

	CWnd::OnLButtonDown(nFlags,point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	SetCursor(LoadCursor(NULL,NULL));

	//mprintf("0x%x,%d,%d\n",nFlags,point.x,point.y);
    HIDSendMouseMsg(point.x,point.y,nFlags);

	CWnd::OnLButtonUp(nFlags,point);
}
