
#include "stdafx.h"

#include<stdlib.h>
#include<iostream.h>
#include<conio.h>
#include<stdio.h>
#include<winsock2.h>
#include<windows.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

void mprintf( char* format,... );

void to_rgb16(unsigned char *buf,int w,int h);
void from_rgb16(unsigned char *buf,int w,int h);
int rle4_read_mem(unsigned char *rle_file_buf,int rle_file_len,unsigned char* src_buf,unsigned char *pbuf);
int rle4_write_mem(unsigned char *dest, int w, int h, unsigned char *buf,int *rle_length,unsigned char *p_source);

#define RLE_HEAD_LEN 28

#define STR_TRY_CONNECT _T("连接网络设备...")
#define STR_TRY_CONNECT_SUCCESSFUL _T("设备连接成功，同步中...")
#define STR_CONNECT_FAILED _T("连接网络设备失败或终止连接")
#define STR_DEVICE_DISCONNECTED _T("设备未连接，服务不能启动")

static int NetCaptureRunStatus;
static CString strIPAddress;
static int ClientPort;
static SOCKET HIDSocket;

void ChangeWindowSize(int width, int height,int vfreq);
void ViewDisplayInfo(char * pInfo);
void UpdateDisplayFrame(void * pBuffer,int buffer_len);
void SetCaptureRunStatus(int status);
void SetStatusInfo(HWND hHwnd,int id,char * info );
void GetVideoConfigInfo();
int GetNetAddress(unsigned char *p);
void NetDeviceDisconnected(void);

UINT NetCaptureThread(LPVOID p);

int GetData(SOCKET s,char *buf,int num_bytes);

int VideoEngineOpen(void);
void VideoEngineClose(void);
void VideoEngineDecode(SOCKET video_sock,HWND hWnd);
int keyboard_fill_report(char report[8], char key);
int mouse_fill_report(char report[8], int x, int y, unsigned int nFlags);

typedef enum {
		KEYBOARD	= 0x01,
		MOUSE		= 0x02,
}HIDDevice;

typedef struct  {
unsigned int type;
char report[8];
} hid_msg_s ;


void V2U_NetInit(void)
{
    // Initialize Winsock.
    WSADATA wsaData;
	
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void V2U_NetCleanup(void)
{
	WSACleanup();
}

int V2U_NetServerStart(HWND hHwnd)
{
	ViewDisplayInfo(STR_DEVICE_DISCONNECTED);
	return 0;
}

void V2U_NetServerStop(void)
{

}

void V2U_NetCaptureStart(HWND hHwnd)
{
	unsigned char ip[4];

	void V2U_CaptureStop(void);
	V2U_CaptureStop();
	
	ClientPort=GetNetAddress(ip);
	
	strIPAddress.Format("%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);

	NetCaptureRunStatus=1;
	::AfxBeginThread(NetCaptureThread,(LPVOID)hHwnd);
}

void V2U_NetCaptureStop(void)
{
	NetCaptureRunStatus=0;
	//Wait NetCaptureThread to stop
	::Sleep(200);
}

void HIDSendKeyMsg(char key, int state)
{
 struct sockaddr_in server;
 int len = sizeof(server);
 hid_msg_s msg;
 
 if( state ) {
 server.sin_addr.s_addr = inet_addr(strIPAddress.GetBuffer(strIPAddress.GetLength()));
 server.sin_port = htons(4000);
 server.sin_family = AF_INET;
 
 msg.type = KEYBOARD;
 keyboard_fill_report(msg.report,key);

   if(SOCKET_ERROR != sendto(HIDSocket, (char*)&msg, sizeof(hid_msg_s),
   0, (sockaddr *)&server, len)) {
	   mprintf("send HID key msg[0x%x]\n",key);
   }

 memset(&msg, 0x0, sizeof(msg));
 msg.type = KEYBOARD;
   if(SOCKET_ERROR != sendto(HIDSocket, (char*)&msg, sizeof(hid_msg_s),
   0, (sockaddr *)&server, len)) {
   }
 }
}

void HIDSendMouseMsg(int x, int y, unsigned int nFlags)
{
 struct sockaddr_in server;
 int len = sizeof(server);
 hid_msg_s msg;
 
 server.sin_addr.s_addr = inet_addr(strIPAddress.GetBuffer(strIPAddress.GetLength()));
 server.sin_port = htons(4000);
 server.sin_family = AF_INET;
 
 msg.type = MOUSE;
 mouse_fill_report(msg.report,x,y,nFlags);

   if(SOCKET_ERROR != sendto(HIDSocket, (char*)&msg, sizeof(hid_msg_s),
   0, (sockaddr *)&server, len)) {
	   //mprintf("send HID mouse msg[%d,%d,0x%x]\n",x,y,nFlags);
   }

}

UINT NetCaptureThread(LPVOID p)
{
	HWND hHwnd = (HWND)p;

    struct sockaddr_in clientService; 
	SOCKET ConnectSocket;
	
	unsigned int val ;
	int val_size ;
    int iResult;
	
	int width=0,height=0;

	static CString strStatusInfo;
	DWORD startTime;
	DWORD stopTime;
	int frames,sec,fps;
	
	iResult = VideoEngineOpen();
    if ( iResult ) {
        goto FAIL;
    }
	
	HIDSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (HIDSocket == INVALID_SOCKET) {
        mprintf("Error at socket(): %ld\n", WSAGetLastError() );
        goto FAIL;
    }
	
    // Create a SOCKET for connecting to server
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        mprintf("Error at socket(): %ld\n", WSAGetLastError() );
        goto FAIL;
    }

	mprintf("socket created!\n");

	ViewDisplayInfo(STR_TRY_CONNECT_SUCCESSFUL);	

	// The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
	memset(&clientService,0,sizeof(clientService));
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(strIPAddress.GetBuffer(strIPAddress.GetLength()));//inet_addr( "127.0.0.1" );
    clientService.sin_port = htons( ClientPort );
	
	mprintf("try connect to %s\n",inet_ntoa(clientService.sin_addr));
	
	// Connect to server.
    iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        mprintf("Unable to connect to server: %ld\n", WSAGetLastError());
        goto FAIL;
    }

	mprintf("connect successful !\n");
	ViewDisplayInfo(STR_TRY_CONNECT);
		
	val = 1;
	val_size = sizeof(val);
	//获取接收缓冲区大小
	iResult = getsockopt(ConnectSocket, SOL_SOCKET , SO_RCVBUF,(char*)&val,&val_size);
	
	if(!iResult)
		mprintf("The getsockopt()  SO_RCVBUF return val is %d \n",val);
	//获取本机地址
	val_size = sizeof(struct sockaddr);
	iResult = getsockname(ConnectSocket, (struct sockaddr *)&clientService,&val_size );
	
	if(!iResult)
		mprintf("The getsockname return addr is %s \n",inet_ntoa(clientService.sin_addr));
	//获取服务器地址
	iResult = getpeername(ConnectSocket, (struct sockaddr *)&clientService,&val_size );
	
	if(!iResult)
		mprintf("The getpeername return addr is %s \n",inet_ntoa(clientService.sin_addr));

	SetCaptureRunStatus(1);
	startTime = GetTickCount();
	frames=0;
	fps=0;
	
	while(NetCaptureRunStatus)
	{
	VideoEngineDecode(ConnectSocket,hHwnd);


		stopTime = GetTickCount();
		sec = (int)((stopTime - startTime)/1000.0);
		frames++;
		if( sec )
		{
		fps=10*frames/sec;
		}
		strStatusInfo.Format("receive (%02dh:%02dm:%02ds,%d frames) %d.%01d fps",sec/3600,(sec/60)%60,sec%60,frames,fps/10,fps%10);
		SetStatusInfo(hHwnd,0,strStatusInfo.GetBuffer(strStatusInfo.GetLength()));
		
	}
	
	closesocket(ConnectSocket);	

FAIL:
	SetCaptureRunStatus(0);
	ViewDisplayInfo(STR_CONNECT_FAILED);
	NetDeviceDisconnected();
	VideoEngineClose();
	closesocket(HIDSocket);
	return 0;
}

int GetData(SOCKET s,char *buf,int num_bytes)
{
	int len;
	int bytes_received=0;
		
	while(bytes_received<num_bytes)
	{
		len=recv(s,(char*)buf+bytes_received,num_bytes-bytes_received,0);
		if( len<=0 )
		{
		return 0;
		}
		bytes_received+=len;
	}
	
	return 1;
}


