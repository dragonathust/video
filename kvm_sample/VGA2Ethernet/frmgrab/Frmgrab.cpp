
#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

void mprintf( char* format,... );


/*****************************************************************/

int FrmGrabLocal_Count(void)
{
	return 0;
}

/*****************************************************************/
void ChangeWindowSize(int width, int height,int vfreq);
void ViewDisplayInfo(char * pInfo);
void UpdateDisplayFrame(void * pBuffer,int buffer_len);
void SetCaptureRunStatus(int status);
void SetStatusInfo(HWND hHwnd,int id,char * info );
void GetVideoConfigInfo();

UINT CaptureThread(LPVOID p);

void CopyFrameToNetserver(unsigned char*pBuf,int length);
void SetFrameSize(int w,int h);

static int CaptureDeviceExist;

static int SaveSnapshot;
static CString SnapshotFilename;

static int VideoRecord;
static CString VideoFileName;

static int VideoPause;

static int m_UpdateParameters;

static int m_gain_red = 128;
static int m_gain_green = 128;
static int m_gain_blue = 128;

static int m_offset_red = 32;
static int m_offset_green = 32;
static int m_offset_blue = 32;
	
/*****************************************************************/

UINT CaptureThread(LPVOID p)
{

	return 0;
}

void V2U_CaptureStart(HWND hHwnd)
{
	::AfxBeginThread(CaptureThread,(LPVOID)hHwnd);
}

void V2U_CaptureStop(void)
{
	CaptureDeviceExist=0;
	//Wait CaptureThread to stop
	::Sleep(200);
}

int V2U_Set_Params(int gain_red, int gain_green,int gain_blue,
			int offset_red,int offset_green,int offset_blue)
{
	int ret=0;
	
	return ret;
}

void SaveSnapshotOnce(CString filename)
{
SnapshotFilename=filename;
SaveSnapshot=1;
}

void VideoRecordStart(CString filename)
{
VideoFileName=filename;
VideoRecord=1;
}

void VideoRecordStop(void)
{
VideoRecord=0;
}

void VideoRecordPause(void)
{
VideoPause=1;
}

void VideoRecordResume(void)
{
VideoPause=0;
}

/*****************************************************************/



