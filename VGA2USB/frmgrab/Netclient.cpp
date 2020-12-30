
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
#include "include\v2u_lib.h"

#pragma comment(lib,"ws2_32.lib")

void mprintf( char* format,... );

void to_rgb16(unsigned char *buf,int w,int h);
void from_rgb16(unsigned char *buf,int w,int h);
int rle4_read_mem(unsigned char *rle_file_buf,int rle_file_len,unsigned char* src_buf,unsigned char *pbuf);
int rle4_write_mem(unsigned char *dest, int w, int h, unsigned char *buf,int *rle_length,unsigned char *p_source);

#define RLE_HEAD_LEN 28

#define STR_TRY_CONNECT _T("连接网络设备...")
#define STR_CONNECT_FAILED _T("连接网络设备失败")

static int NetCaptureRunStatus;
static CString strIPAddress;
static int ClientPort;

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

/*******************************************************/
#include "capture.c"
/*******************************************************/

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

UINT NetCaptureThread(LPVOID p)
{
	HWND hHwnd = (HWND)p;

    struct sockaddr_in clientService; 
	SOCKET ConnectSocket;
	
	unsigned int val ;
	int val_size ;
    int iResult;
	
	unsigned char*buffer=NULL;
	
	unsigned char*pixbuf=NULL;
	unsigned char *decode_buf=NULL;
	
	unsigned char head[32];
	
	int width=0,height=0;
	int w,h;
	int RLE_size_rgb[3];
	int RLE_size_last_block;
	int rle_length;

	static CString strStatusInfo;
	V2U_TIME startTime;
	V2U_TIME stopTime;
	int frames,sec,fps;
	
    // Create a SOCKET for connecting to server
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        mprintf("Error at socket(): %ld\n", WSAGetLastError() );
        goto FAIL;
    }

	mprintf("socket created!\n");

	ViewDisplayInfo(STR_TRY_CONNECT);	

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
	startTime = v2u_time();
	frames=0;
	fps=0;
	
	while(NetCaptureRunStatus)
	{
RECEIVE:	
		if( GetData(ConnectSocket,(char*)head,RLE_HEAD_LEN)==0 )
		{
			mprintf("recv failed: %d\n", WSAGetLastError());
			break;		
		}
	
		if( strncmp((const char*)head,"RLE4",4)!=0 )
		{
			mprintf("receive: Not a valid rle file.\n");
			goto RECEIVE;
			//break;
		}
	
		memcpy(&w,head+4,4);
		memcpy(&h,head+8,4);

		memcpy(RLE_size_rgb,head+12,4);
		memcpy(RLE_size_rgb+1,head+16,4);
		memcpy(RLE_size_rgb+2,head+20,4);	
		memcpy(&RLE_size_last_block,head+24,4);	
		
		rle_length=RLE_HEAD_LEN+RLE_size_rgb[0]+RLE_size_rgb[1]+RLE_size_rgb[2]+RLE_size_last_block;
		
		if( (width!=w)||(height!=h) )
		{
		ChangeWindowSize(w,h,60000);
		width=w;
		height=h;
		
		if(buffer)
		{
			free(buffer);
		}
		buffer = (unsigned char*)malloc(w*h*3);
		
		if(!buffer)
			break;

			/*
			if(pixbuf)
			{	
				free(pixbuf);
			}
			pixbuf=(unsigned char*)malloc(w*h*7);
			if(!pixbuf)
				break;
			
			decode_buf=pixbuf+w*h*3;	
			*/
			
			if(decode_buf)
			{
			free(decode_buf);
			}
			
			decode_buf=(unsigned char*)malloc(w*h*4);
			if(!decode_buf)
				break;
			
			unsigned char *GetFrameBuffer(void);
			pixbuf=GetFrameBuffer();
			if(!pixbuf)
			break;
		}
		
		memcpy(buffer,head,RLE_HEAD_LEN);
		
		//mprintf("want to receive %d bytes\n",rle_length);
		
		if( GetData(ConnectSocket,(char*)buffer+RLE_HEAD_LEN,rle_length-RLE_HEAD_LEN)==0 )
		{
			mprintf("recv failed: %d\n", WSAGetLastError());
			break;		
		}
		
		rle4_read_mem(buffer,rle_length,pixbuf,decode_buf);
		//from_rgb16(pixbuf,w,h);
		//UpdateDisplayFrame( (void*)pixbuf, w*h*3);
		UpdateDisplayFrame( NULL, w*h*3);

		stopTime = v2u_time();
		sec = (int)((stopTime - startTime)/1000.0);
		frames++;
		if( sec )
		{
		fps=10*frames/sec;
		}
		strStatusInfo.Format("receive (%02dh:%02dm:%02ds,%d frames) %d.%01d fps",sec/3600,(sec/60)%60,sec%60,frames,fps/10,fps%10);
		SetStatusInfo(hHwnd,0,strStatusInfo.GetBuffer(strStatusInfo.GetLength()));		
	}
	
//	if(pixbuf)
//	free(pixbuf);

	if(decode_buf)
	free(decode_buf);
	
	if(buffer)
	free(buffer);

	closesocket(ConnectSocket);	

FAIL:
	SetCaptureRunStatus(0);
	ViewDisplayInfo(STR_CONNECT_FAILED);
	NetDeviceDisconnected();
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


