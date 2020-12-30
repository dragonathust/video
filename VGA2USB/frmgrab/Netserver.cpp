
#include "stdafx.h"

#include<stdlib.h>
#include<iostream.h>
#include<conio.h>
#include<stdio.h>
#include<winsock2.h>
#include<windows.h>
#include "afxmt.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "include\v2u_lib.h"

#pragma comment(lib,"ws2_32.lib")

void mprintf( char* format,... );

void to_rgb16(unsigned char *buf,int w,int h);
void from_rgb16(unsigned char *buf,int w,int h);
int rle4_read_mem(unsigned char *rle_file_buf,int rle_file_len,unsigned char* src_buf,unsigned char *pbuf);
int rle4_write_mem(unsigned char *dest, int w, int h, unsigned char *buf,int *rle_length,unsigned char *p_source);

#define STR_SERVER_CONNECT_LISTEN _T("网络设备服务已启动")
#define STR_SERVER_CONNECTED _T("网络客户端已连接")
#define STR_SERVER_DISCONNECTED _T("网络设备服务已停止")
#define STR_DEVICE_DISCONNECTED _T("设备未连接，服务不能启动")

static int NetServerRunStatus;
static int ServerPort;

static CSemaphore *pSemaphoreFrame;

static unsigned char*pFrameBuffer=NULL;
static int FrameWidth,FrameHeight;

void ViewDisplayInfo(char * pInfo);
void SetStatusInfo(HWND hHwnd,int id,char * info );
int GetNetAddress(unsigned char *p);

int GetCaptureRunStatus();

UINT NetServerThread(LPVOID p);

void V2U_NetCaptureStop(void);
void V2U_NetServerStop(void);

//#define TEST_MODE

/*******************************************************/
#include "capture.c"
/*******************************************************/

void V2U_NetInit(void)
{
    // Initialize Winsock.
    WSADATA wsaData;
	
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	pSemaphoreFrame=new CSemaphore(0,1);	
	NetServerRunStatus=0;
}

void V2U_NetCleanup(void)
{
	V2U_NetCaptureStop();
	V2U_NetServerStop();

	delete pSemaphoreFrame;
	
	WSACleanup();
}

int V2U_NetServerStart(HWND hHwnd)
{
	unsigned char ip[4];
	
	ServerPort=GetNetAddress(ip);

#ifndef TEST_MODE
	if( GetCaptureRunStatus() )
#endif	
	{	
	NetServerRunStatus=1;

	::AfxBeginThread(NetServerThread,(LPVOID)hHwnd);
	return 1;
	}
#ifndef TEST_MODE
	else
	{
	ViewDisplayInfo(STR_DEVICE_DISCONNECTED);
	return 0;
	}
#endif	
	
}

void V2U_NetServerStop(void)
{
	NetServerRunStatus=0;
	
	//Wait NetServerThread to stop
	::Sleep(200);
}

void SetFrameSize(int w,int h)
{
	FrameWidth=w;
	FrameHeight=h;
}

void CopyFrameToNetserver(unsigned char*pBuf,int length)
{
	if(pFrameBuffer)
	{
	memcpy(pFrameBuffer,pBuf,length);
	pSemaphoreFrame->Unlock();
	}
}

unsigned char*GetOneFrame()
{	
	if(pFrameBuffer)
	{
	pSemaphoreFrame->Lock();
	return 	pFrameBuffer;
	}
	else
		return NULL;
}

UINT NetServerThread(LPVOID p)
{
    sockaddr_in service;
	struct sockaddr_in remote_addr;
	SOCKET ListenSocket,AcceptSocket; 
	int sin_size;

	int file_length;
	unsigned char*pixbuf=NULL;
	unsigned char*pixbuf_current;

	unsigned char *rlebuf;
	unsigned char *encode_buf;

	int w,h;
	
#ifdef TEST_MODE	
	int j;
	char file_name_open[32];
	unsigned char *buffer;
	
    unsigned char *buffer_in0;
    unsigned char *buffer_in1;
	unsigned char *buffer_in2;
	unsigned char *buffer_in3;
	
    int file_length0;
    int file_length1;
    int file_length2;
    int file_length3;
	
    GUI_BMP_INFO Info;
#endif

	static CString strStatusInfo;

	V2U_TIME startTime;
	V2U_TIME stopTime;
	int frames,sec,fps;
	
	int nSendBuf;
    int iResult;	
	//char dump_name[32];
		
	HWND hHwnd = (HWND)p;

    // Create a SOCKET for listening for
    // incoming connection requests.
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
        mprintf("socket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
	memset(&service,0,sizeof(service));	
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
    service.sin_port = htons(ServerPort);

	nSendBuf=64*1024;
	setsockopt(ListenSocket,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));

    if (bind(ListenSocket,
             (SOCKADDR *) & service, sizeof (service)) == SOCKET_ERROR) {
        mprintf("bind failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        return 1;
    }

	mprintf("bind successful !\n");

    // Listen for incoming connection requests.
    // on the created socket
    if (listen(ListenSocket, 1) == SOCKET_ERROR) {
        mprintf("listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        return 1;
    }
	
	mprintf("listening...\n");

WAIT_CONNECT:	
	ViewDisplayInfo(STR_SERVER_CONNECT_LISTEN);
	
    // Accept the connection.
	sin_size = sizeof(struct sockaddr_in);
    AcceptSocket = accept(ListenSocket, (struct sockaddr*)&remote_addr,&sin_size);
    if (AcceptSocket == INVALID_SOCKET) {
        mprintf("accept failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        return 1;
    } 

	mprintf("received a connection from %s\n",inet_ntoa(remote_addr.sin_addr));

#ifdef TEST_MODE	
	sprintf(file_name_open,"d0.bmp");
	buffer_in0=read_file_to_mem(file_name_open,&file_length0);
	
	sprintf(file_name_open,"d1.bmp");
	buffer_in1=read_file_to_mem(file_name_open,&file_length1);

	sprintf(file_name_open,"d2.bmp");
	buffer_in2=read_file_to_mem(file_name_open,&file_length2);

	sprintf(file_name_open,"d3.bmp");
	buffer_in3=read_file_to_mem(file_name_open,&file_length3);
	
	if( (!buffer_in0)||(!buffer_in1)||(!buffer_in2)||(!buffer_in3) )
	{
		goto FAIL;
	}
	
	GUI_BMP_GetInfo(buffer_in0, file_length0 , &Info);
	to_rgb16(GUI_BMP_GetPixel(buffer_in0,file_length0),Info.XSize,Info.YSize);

	GUI_BMP_GetInfo(buffer_in1, file_length1 , &Info);
	to_rgb16(GUI_BMP_GetPixel(buffer_in1,file_length1),Info.XSize,Info.YSize);

	GUI_BMP_GetInfo(buffer_in2, file_length2 , &Info);
	to_rgb16(GUI_BMP_GetPixel(buffer_in2,file_length2),Info.XSize,Info.YSize);

	GUI_BMP_GetInfo(buffer_in3, file_length3 , &Info);
	to_rgb16(GUI_BMP_GetPixel(buffer_in3,file_length3),Info.XSize,Info.YSize);

	w=Info.XSize;
	h=Info.YSize;
#else
		w=FrameWidth;
		h=FrameHeight;
		pFrameBuffer=(unsigned char*)malloc(w*h*3);
		if(!pFrameBuffer)
			goto FAIL;
#endif

		if( !pixbuf )
		{
			pixbuf=(unsigned char*)malloc(w*h*9);
			if(!pixbuf)
				goto FAIL;
			
			rlebuf=pixbuf;
			encode_buf=pixbuf+w*h*3;
		}
		
	ViewDisplayInfo(STR_SERVER_CONNECTED);
	startTime = v2u_time();
	frames=0;
	fps=0;
	
	while(NetServerRunStatus)
	{

#ifdef TEST_MODE	
		for(j=0;j<4;j++)
		{
		switch(j)
		{
			case 0:
				buffer=buffer_in0;
				file_length=file_length0;
				break;
			case 1:
				buffer=buffer_in1;
				file_length=file_length1;
				break;
			case 2:
				buffer=buffer_in2;
				file_length=file_length2;
				break;				
			case 3:
				buffer=buffer_in3;
				file_length=file_length3;
				break;				
		}
		
		pixbuf_current=GUI_BMP_GetPixel(buffer,file_length);
#else
		pixbuf_current=GetOneFrame();
#endif

		rle4_write_mem(rlebuf, w, h,pixbuf_current,&file_length,encode_buf);
		
		if( send(AcceptSocket,(const char*)rlebuf,file_length,0) <=0  )
		{
			mprintf("send error, errno is %d !\n",errno);
			goto FAIL;
		}
		
		//mprintf("send %d bytes\n",file_length);
		
		stopTime = v2u_time();
		sec = (int)((stopTime - startTime)/1000.0);
		frames++;
		if( sec )
		{
		fps=10*frames/sec;
		}
		strStatusInfo.Format("send (%02dh:%02dm:%02ds,%d frames) %d.%01d fps",sec/3600,(sec/60)%60,sec%60,frames,fps/10,fps%10);
		SetStatusInfo(hHwnd,0,strStatusInfo.GetBuffer(strStatusInfo.GetLength()));
#ifdef TEST_MODE
		}
#endif	
	}

FAIL:
#ifdef TEST_MODE
		if(buffer_in0)
		free(buffer_in0);

		if(buffer_in1)
		free(buffer_in1);

		if(buffer_in2)
		free(buffer_in2);

		if(buffer_in3)
		free(buffer_in3);
#else
	if(pFrameBuffer)
	{
	free(pFrameBuffer);
	pFrameBuffer=NULL;
	}
#endif
		
	if(pixbuf)
	{
	free(pixbuf);
	pixbuf=NULL;
	}

	iResult = shutdown(AcceptSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
        mprintf("shutdown failed: %d\n", WSAGetLastError());
	}
	
	closesocket(AcceptSocket);
	
	if(NetServerRunStatus)
	{
		goto WAIT_CONNECT;
	}
	
	closesocket(ListenSocket);
			
	ViewDisplayInfo(STR_SERVER_DISCONNECTED);
		
	return 0;
}


