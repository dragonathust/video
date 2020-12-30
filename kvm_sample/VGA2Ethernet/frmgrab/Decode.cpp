
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

//river
#include "def.h"
#include "jpegdec.h"
#include "JTABLES.h"
#include "vq.h"
#include <math.h>
//river
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <dos.h>

void mprintf( char* format,... );

unsigned char First_Frame;
unsigned char UV_selector, advance_selector, Old_UV_Selector, Old_advance_selector;
unsigned char RC4_Enable, RC4_Reset, Disable_VGA, VGA_DISPLAY, Auto_Mode;

unsigned char Set_Differentail_Enable, Differentail_Enable;
unsigned char Y_selector, Set_Y_selector, Old_Y_Selector;
unsigned char Mode_420, Set_Mode_420, Old_Mode_420;
unsigned char Direct_Mode, Set_Direct_Mode;
unsigned char VQ_Mode, Set_VQ_Mode, Old_VQ_Mode;

char szBuff[ 80 ];         /* Temp buffer - used to pass strings    */

#define MAX_PENDING_CONNECTS 4  /* The backlog allowed for listen() */
#define NO_FLAGS_SET         0  /* Used with recv()/send()          */
#define MY_MSG_LENGTH       80  /* msg buffer sent back and forth   */

unsigned long *Buffer=NULL;
unsigned char *Receive_Buffer=NULL;
unsigned char SendBuffer[50] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
unsigned char Mouse_SendBuffer[50] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
unsigned char Keyboard_SendBuffer[50] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
unsigned char TestBuffer[50] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
FILE   *fp;
char file_node[20];

int SCALEFACTOR, SCALEFACTORUV;
int ADVANCESCALEFACTOR, ADVANCESCALEFACTORUV;
int newbits;
unsigned long codebuf, newbuf, readbuf;

#define exit_func(err) { strcpy(error_string, err); return 0;}

static DWORD byte_pos; // current byte position
#define BYTE_p(i) bp=buf[(i)++]
#define WORD_p(i) wp=(((WORD)(buf[(i)]))<<8) + buf[(i)+1]; (i)+=2

int  m_CrToR[256], m_CbToB[256], m_CrToG[256], m_CbToG[256], m_Y[256];
long *QT[4]; // quantization tables, no more than 4 quantization tables (QT[0..3])
Huffman_table HTDC[4]; //DC huffman tables , no more than 4 (0..3)
Huffman_table HTAC[4]; //AC huffman tables                  (0..3)

BYTE YQ_nr=0,CbQ_nr=1,CrQ_nr=1; // quantization table number for Y, Cb, Cr
BYTE YDC_nr=0,CbDC_nr=1,CrDC_nr=1; // DC Huffman table number for Y,Cb, Cr
BYTE YAC_nr=0,CbAC_nr=1,CrAC_nr=1; // AC Huffman table number for Y,Cb, Cr

SWORD DCY, DCCb, DCCr; // Coeficientii DC pentru Y,Cb,Cr
SWORD DCT_coeff[384]; // Current DCT_coefficients

static DWORD wordval ; // the actual used value in Huffman decoding.
static SWORD neg_pow2[17]={0,-1,-3,-7,-15,-31,-63,-127,-255,-511,-1023,-2047,-4095,-8191,-16383,-32767};
static DWORD start_neg_pow2=(DWORD)neg_pow2;

static int shift_temp;
#define RIGHT_SHIFT(x,shft)  \
	((shift_temp = (x)) < 0 ? \
	 (shift_temp >> (shft)) | ((~(0L)) << (32-(shft))) : \
	 (shift_temp >> (shft)))
#define DESCALE(x,n)  RIGHT_SHIFT((x) + (1L << ((n)-1)), n)
BYTE *rlimit_table;

//void Init_Color_Table(PVIDEO_ENGINE_INFO VideoEngineInfo);
void Init_Color_Table();

//river
void process_Huffman_data_unit(BYTE DC_nr, BYTE AC_nr,SWORD *previous_DC, WORD position);
void IDCT_transform(SWORD *incoeff,BYTE *outcoeff,BYTE Q_nr);
int init_JPG_decoding(void);
ULONG decode(PVIDEO_ENGINE_INFO, ULONG, ULONG);

int selector;
int Mapping;
struct rc4_state s;
int SharpModeSelection;

int txb, tyb;
unsigned long pixels;
struct COMPRESSHEADER yheader, uvheader;
struct RGB *OutBuffer=NULL;
unsigned long index;
// WIDTH and HEIGHT are the modes your display used
unsigned long WIDTH, Old_WIDTH = 0, User_WIDTH;
unsigned long HEIGHT, Old_HEIGHT = 0, User_HEIGHT;
unsigned long tmp_HEIGHT, tmp_WIDTH, Change = 0;
unsigned long JPEG_no = 0;
unsigned long Frame = 0, Frame_Rate, Total_Data = 0;
clock_t start, end, duration, overhead;
char  Output_Info[100];
void init_jpg_table ();
float Data_Rate;
short Old_CursorX, Old_CursorY;
short Current_CursorX, Current_CursorY, Reset = 0;
short Differ_Y, Differ_X;
int  Key_Code, Key_State;
SCROLLINFO Xsi, Ysi;
int W_Cursor_Position = 0, Old_W_Cursor_Position = 0;
//For 2Pass
struct RGB *YUVBuffer=NULL;

void ChangeWindowSize(int width, int height,int vfreq);
void ViewDisplayInfo(char * pInfo);
void UpdateDisplayFrame(void * pBuffer,int buffer_len);
void mprintf( char* format,... );

VIDEO_ENGINE_INFO VideoEngineInfo;

int  GetINFData (PVIDEO_ENGINE_INFO VideoEngineInfo)
{
    CHAR          string[81],name[80];
    ULONG         i;
    CHAR          StringToken[256];
    FILE          *fp;

    fp = fopen("input.inf", "rb");
	if( !fp ) {
		mprintf("Can't open file input.inf!\n");
		AfxMessageBox("Can't open file input.inf!");
		return -1;
	}
    while (fgets (string, 80, fp) != NULL) {
        sscanf (string, "%[^=] = %s", name, StringToken);
        if (stricmp (name, "COMPRESS_MODE") == 0) {
            VideoEngineInfo->FrameHeader.CompressionMode = (BYTE)(atoi(StringToken));
    	}
    	if (stricmp (name, "Y_JPEG_TABLESELECTION") == 0) {
            VideoEngineInfo->FrameHeader.Y_JPEGTableSelector = (BYTE)(atoi(StringToken));
    	}
    	if (stricmp (name, "UV_JPEG_TABLESELECTION") == 0) {
            VideoEngineInfo->FrameHeader.UV_JPEGTableSelector = (BYTE)(atoi(StringToken));
    	}
    	if (stricmp (name, "JPEG_YUVTABLE_MAPPING") == 0) {
            VideoEngineInfo->FrameHeader.JPEGYUVTableMapping = (BYTE)(atoi(StringToken));
    	}
    	if (stricmp (name, "JPEG_SCALE_FACTOR") == 0) {
            VideoEngineInfo->FrameHeader.JPEGScaleFactor = (BYTE)(atoi(StringToken));
    	}
        if (stricmp (name, "DOWN_SCALING_METHOD") == 0) {
            VideoEngineInfo->INFData.DownScalingMethod = (BYTE)(atoi(StringToken));
    	}
        if (stricmp (name, "DIFFERENTIAL_SETTING") == 0) {
            VideoEngineInfo->INFData.DifferentialSetting = (BYTE)(atoi(StringToken));
    	}
        if (stricmp (name, "DESTINATION_X") == 0) {
            VideoEngineInfo->DestinationModeInfo.X = (USHORT)(atoi(StringToken));
    	}
        if (stricmp (name, "DESTINATION_Y") == 0) {
            VideoEngineInfo->DestinationModeInfo.Y = (USHORT)(atoi(StringToken));
    	}
        if (stricmp (name, "RC4_ENABLE") == 0) {
            VideoEngineInfo->FrameHeader.RC4Enable = (BYTE)(atoi(StringToken));
    	}
        if (stricmp (name, "RC4_RESET") == 0) {
            VideoEngineInfo->FrameHeader.RC4Reset = (BYTE)(atoi(StringToken));
    	}
        if (stricmp (name, "RC4_KEYS") == 0) {
            for (i = 0; i < strlen (StringToken); i++) {
                EncodeKeys[i] = StringToken[i];
            }
        }
        if (stricmp (name, "ANALOG_DIFFERENTIAL_THRESHOLD") == 0) {
            VideoEngineInfo->INFData.AnalogDifferentialThreshold = (USHORT)(atoi(StringToken));
        }
        if (stricmp (name, "DIGITAL_DIFFERENTIAL_THRESHOLD") == 0) {
            VideoEngineInfo->INFData.DigitalDifferentialThreshold = (USHORT)(atoi(StringToken));
        }
        if (stricmp (name, "EXTERNAL_SIGNAL") == 0) {
            VideoEngineInfo->INFData.ExternalSignalEnable = (BYTE)(atoi(StringToken));
        }
        if (stricmp (name, "JPEG_ADVANCE_TABLESELECTION") == 0) {
            VideoEngineInfo->FrameHeader.AdvanceTableSelector = (BYTE)(atoi(StringToken));
        }
        if (stricmp (name, "JPEG_ADVANCE_SCALE_FACTOR") == 0) {
            VideoEngineInfo->FrameHeader.AdvanceScaleFactor = (BYTE)(atoi(StringToken));
        }
        if (stricmp (name, "SHARP_MODE_SELECTION") == 0) {
            VideoEngineInfo->FrameHeader.SharpModeSelection = (BYTE)(atoi(StringToken));
        }
        if (stricmp (name, "AUTO_MODE") == 0) {
            VideoEngineInfo->INFData.AutoMode = (BYTE)(atoi(StringToken));
        }
        if (stricmp (name, "GAMMA1") == 0) {
            VideoEngineInfo->INFData.Gamma1Parameter = (atof(StringToken));
        }
        if (stricmp (name, "GAMMA2") == 0) {
            VideoEngineInfo->INFData.Gamma2Parameter = (atof(StringToken));
        }
        if (stricmp (name, "GAMMA1_GAMMA2_SEPERATE") == 0) {
            VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate = (atof(StringToken));
        }
    }
    fclose (fp);
	return 0;
}

int VideoEngineOpen(void)
{
	Buffer = (unsigned long*)malloc ((size_t)1920 * 1200 * 8);
	Receive_Buffer = (unsigned char*)malloc ((size_t)1920 * 1200 * 8);
    OutBuffer=(struct RGB*)malloc((size_t)1920 * 1200 * 8);
//For 2Pass
    YUVBuffer=(struct RGB*)malloc((size_t)1920 * 1200 * 8);
	
  
	if( GetINFData (&VideoEngineInfo) ) {
			return -1;
	}

		init_jpg_table ();

		Old_Y_Selector = 255;
		Old_advance_selector = 255;
		Y_selector = 4;
	
	return 0;
}

void VideoEngineClose(void)
{
/*	
	if(Buffer) {
		mprintf("free p=%x\n",Buffer);
		free (Buffer);
		Buffer=NULL;
	}
*/	
	if(Receive_Buffer) {
		mprintf("free p=%x\n",Receive_Buffer);
		free (Receive_Buffer);
		Receive_Buffer=NULL;		
	}
	

	if(OutBuffer) {		
		mprintf("free p=%x\n",OutBuffer);
		free (OutBuffer);
		OutBuffer=NULL;
	}


	if(YUVBuffer) {		
		mprintf("free p=%x\n",YUVBuffer);
		free (YUVBuffer);
		YUVBuffer=NULL;
	}
}

void VideoEngineDecode(SOCKET video_sock,HWND hWnd)
{
   unsigned long Length, Data_Length, Number_Of_MB, Offset, decode_result;
   
			//recv host header 
			for (Offset = 0; Offset < 29;)  {
				Length = recv (video_sock, (char*)(SendBuffer + Offset), 29, 0);
				if (Length == 0)
					break;
				if (Length != -1) {
					Offset += Length;
				}
			}

			//check revive header and set
			
			//first frame 
			Mode_420 = SendBuffer[20];
			Y_selector = SendBuffer[18];
			advance_selector = SendBuffer[28];
			tmp_WIDTH = User_WIDTH = WIDTH = (SendBuffer[9] << 8) + (SendBuffer[8]);
			tmp_HEIGHT = User_HEIGHT = HEIGHT = (SendBuffer[11] << 8) + (SendBuffer[10]);
			Differentail_Enable = SendBuffer[24];	
			Direct_Mode = SendBuffer[21];			
			VQ_Mode = SendBuffer[22];

			
			if(SendBuffer[12] == 1) {
				Old_Mode_420 = Mode_420;
				Set_Mode_420 = Mode_420;
				Old_Y_Selector = Y_selector;
				Set_Y_selector = Y_selector;
				Old_advance_selector = advance_selector;
				First_Frame = 1;
				Disable_VGA = 0;
				Old_WIDTH = WIDTH;
				Old_HEIGHT = HEIGHT;
				Set_VQ_Mode = VQ_Mode;
				Set_Differentail_Enable = Differentail_Enable;
				Set_Direct_Mode = Direct_Mode;
				
//				Auto_Mode = 1;
			} else {
				First_Frame = 0;
			}

			//format 
			if(SendBuffer[13] == 1) {
				Data_Length = ((SendBuffer[3] << 24) + (SendBuffer[2] << 16) + (SendBuffer[1] << 8) + SendBuffer[0]);
			} else {
				Data_Length = ((SendBuffer[3] << 24) + (SendBuffer[2] << 16) + (SendBuffer[1] << 8) + SendBuffer[0]) * 4;
			}

			Number_Of_MB = (SendBuffer[7] << 24) + (SendBuffer[6] << 16) + (SendBuffer[5] << 8) + SendBuffer[4];
			RC4_Enable = SendBuffer[16];
			UV_selector = Y_selector;
			VQ_Mode = SendBuffer[22];	
			Auto_Mode = SendBuffer[25];

//			sprintf(szTemp1, "Rcv first frame %d, rc4 %d, 420 %d, y %d, uv %d, adv %d, direct %d, vq %d, auto %d  ", First_Frame, RC4_Enable, Mode_420, Y_selector, UV_selector, advance_selector, Direct_Mode, VQ_Mode, Auto_Mode);
//			MessageBox(hWnd, szTemp1, "Frame header ", MB_OK);
			
//send dummy header setting
		    send (video_sock, (char*)SendBuffer, 29, NO_FLAGS_SET);

//************* 2Pass only can work under Mode444

//reveive frame 
			    for (Offset = 0; Offset < Data_Length;)  {
	                Length = recv (video_sock, (char*)Receive_Buffer + Offset, Data_Length, 0);
				    if (Length == 0)
					    break;
				    if (Length != -1) {
	   			        Offset += Length;
					}
				}			

			if(SendBuffer[13]) {
//				Data_Length = ((SendBuffer[3] << 24) + (SendBuffer[2] << 16) + (SendBuffer[1] << 8) + SendBuffer[0]);
				printf("rcv JPEG to socket size = %d\n",Data_Length);
				sprintf(file_node, "%d.jpg",JPEG_no);
			    fp = fopen(file_node, "wb");
			    for (Offset = 0; Offset < Data_Length; Offset++) {
				    fputc (Receive_Buffer[Offset], fp);
				}
			    fclose (fp);			
				JPEG_no++;
			}

			Total_Data += Data_Length;
 		    Buffer = (unsigned long *)Receive_Buffer;

			if(Set_Y_selector != Y_selector)
				SendBuffer[18] = Set_Y_selector;
			if(Set_Mode_420 != Mode_420)
				SendBuffer[20] = Set_Mode_420;
			if(Set_Direct_Mode != Direct_Mode)
				SendBuffer[21] = Set_Direct_Mode;
			if(Set_VQ_Mode != VQ_Mode)
				SendBuffer[22] = Set_VQ_Mode;

			if(Set_Differentail_Enable != Differentail_Enable)
				SendBuffer[24] = Set_Differentail_Enable;

			SendBuffer[23] = Disable_VGA;
			SendBuffer[25] = Auto_Mode;
			SendBuffer[28] = advance_selector;
			
//send set header 
		    send (video_sock, (char*)SendBuffer, 29, NO_FLAGS_SET);
			if(!SendBuffer[13]) {			
            	decode_result = decode (&VideoEngineInfo, Number_Of_MB, Data_Length);
			}
			Frame++;

			if (First_Frame) {
				Old_WIDTH = WIDTH;
				Old_HEIGHT = HEIGHT;
				ChangeWindowSize(WIDTH,HEIGHT,60000);
				//unsigned char *GetFrameBuffer(void);
				//free(OutBuffer);
				//OutBuffer=(struct RGB *)GetFrameBuffer();
			}
			UpdateDisplayFrame( OutBuffer, WIDTH*HEIGHT*4);
					

		duration = (clock() - start);
		if ((duration / CLOCKS_PER_SEC) >= 5) {
			Frame_Rate = Frame / (duration / CLOCKS_PER_SEC);
			Data_Rate = (float)Total_Data / (float)(5.0 * 1000.0);
			sprintf (Output_Info, "%d FPS,  %8f KBytes/s [First frame: %d, 420: %d, Y: %d, rc4: %d, diff : %d]", 
									Frame_Rate, Data_Rate, First_Frame, Mode_420, Y_selector, RC4_Enable, Differentail_Enable);
			SetWindowText (hWnd, Output_Info);
			Frame = 0;
			Total_Data = 0;
			start = clock();
		} else {
			sprintf (Output_Info, "%d FPS,  %8f KBytes/s [First frame: %d, 420: %d, Y: %d, rc4: %d, diff : %d]", 
								Frame_Rate, Data_Rate, First_Frame, Mode_420, Y_selector, RC4_Enable, Differentail_Enable);
			SetWindowText (hWnd, Output_Info);
		}

}


//  Key expansion function for RC4. RC4 repeat keys to 256 bytes.
void Keys_Expansion (unsigned char *key)
{
    USHORT    i, StringLength;

    StringLength = strlen ((char*)key);
    for (i = 0; i < 256; i++) {
        key[i] = key[i % StringLength];
    }
}

//  Setup function for RC4. Algorithm is as following:
/*********************************************************************
  Fill s->m[1] to s->m[255] linearly (s->m[0] = 0, s->m[1] = 1......)
  index y = 0
  for (x = 0 to x = 255) {
      y = (y + s->m[x] + k[x]) mod 256
      Swap s->m[x] and s->m[y]
  }
**********************************************************************/
//  Setup function for RC4. The same routine with encode
void DecodeRC4_setup( struct rc4_state *s, unsigned char *key, PVIDEO_ENGINE_INFO VideoEngineInfo)
{
    int i, j, k, a, *m;

    s->x = 0;
    s->y = 0;
    m = s->m;

    for( i = 0; i < 256; i++ )
    {
        m[i] = i;
    }

    j = k = 0;

    for( i = 0; i < 256; i++ )
    {
        a = m[i];
        j = (unsigned char) ( j + a + key[k] );
        m[i] = m[j]; m[j] = a;
        k++;
    }
}

//  Crypt function for RC4. Algorithm is as following:
/*********************************************************************
  x = (x + 1) mod 256
  y = (y + s->m[x]) mod 256
  Swap s->m[x] and s->m[y]
  t = (s->m[x] + s->m[y]) mod 256
  K = s->m[t]
  K is the XORed with the plaintext to produce the ciphertext or the
  ciphertext to produce the plaintext
**********************************************************************/
void RC4_crypt( struct rc4_state *s, unsigned char *data, int length )
{ 
    int i, x, y, a, b, *m;

    x = s->x;
    y = s->y;
    m = s->m;

    for( i = 0; i < length; i++ )
    {
        x = (unsigned char) ( x + 1 ); a = m[x];
        y = (unsigned char) ( y + a );
        m[x] = b = m[y];
        m[y] = a;
        data[i] ^= m[(unsigned char) ( a + b )];
    }

    s->x = x;
    s->y = y;
}

WORD WORD_hi_lo(BYTE byte_high,BYTE byte_low)
{
	_asm {
		  mov ah,byte_high
		  mov al,byte_low
		 }
}

void prepare_range_limit_table()
/* Allocate and fill in the sample_range_limit table */
{
  int j;
  rlimit_table = (BYTE *)malloc(5 * 256L + 128) ;
  /* First segment of "simple" table: limit[x] = 0 for x < 0 */
  memset((void *)rlimit_table,0,256);
  rlimit_table += 256;	/* allow negative subscripts of simple table */
  /* Main part of "simple" table: limit[x] = x */
  for(j = 0; j < 256; j++)
     rlimit_table[j] = j;
  /* End of simple table, rest of first half of post-IDCT table */
  for(j = 256; j < 640; j++)
     rlimit_table[j] = 255;

  /* Second half of post-IDCT table */
  memset((void *)(rlimit_table + 640),0,384);
  for(j = 0; j < 128 ; j++)
     rlimit_table[j+1024] = j;
}

WORD lookKbits(BYTE k)
{
  WORD revcode;

  revcode= (WORD)(codebuf>>(32-k));

  return(revcode);

}

void skipKbits(BYTE k)
{  unsigned long readbuf;

    if ((newbits-k)<=0) {
      readbuf = Buffer[index];
      index++;
      codebuf=(codebuf<<k)|((newbuf|(readbuf>>(newbits)))>>(32-k));
      newbuf=readbuf<<(k-newbits);
      newbits=32+newbits-k;
    } else {
    	codebuf=(codebuf<<k)|(newbuf>>(32-k));
    	newbuf=newbuf<<k;
    	newbits-=k;
	}
}

SWORD getKbits(BYTE k)
{
 SWORD signed_wordvalue;

 //river
 //signed_wordvalue=lookKbits(k);
 signed_wordvalue=(WORD)(codebuf>>(32-k));
 if (((1L<<(k-1))& signed_wordvalue)==0)
   signed_wordvalue=signed_wordvalue+neg_pow2[k];

 skipKbits(k);
 return signed_wordvalue;
}

void init_QT()
{
 BYTE i;

 for (i=0;i<=3;i++)
   QT[i]=(long *)malloc(sizeof(long)*64);
}

void set_quant_table(BYTE *basic_table,BYTE scale_factor,BYTE *newtable)
// Set quantization table and zigzag reorder it
{
   BYTE i;
   long temp;
   for (i = 0; i < 64; i++)
   {
      temp = ((long) (basic_table[i] * 16)/scale_factor);
	/* limit the values to the valid range */
      if (temp <= 0L)
        temp = 1L;
      if (temp > 255L)
         temp = 255L; /* limit to baseline range if requested */
      newtable[zigzag[i]] = (BYTE) temp;
   }
}

void load_quant_table(long *quant_table)
{
 float scalefactor[8]={1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
					   1.0f, 0.785694958f, 0.541196100f, 0.275899379f};
 BYTE j,row,col;
 BYTE tempQT[64];

// Load quantization coefficients from JPG file, scale them for DCT and reorder
// from zig-zag order
 switch (Y_selector) {
   case 0:
       std_luminance_qt = Tbl_000Y;
       break;
   case 1:
       std_luminance_qt = Tbl_014Y;
       break;
   case 2:
       std_luminance_qt = Tbl_029Y;
       break;
   case 3:
       std_luminance_qt = Tbl_043Y;
       break;
   case 4:
       std_luminance_qt = Tbl_057Y;
       break;
   case 5:
       std_luminance_qt = Tbl_071Y;
       break;
   case 6:
       std_luminance_qt = Tbl_086Y;
       break;
   case 7:
       std_luminance_qt = Tbl_100Y;
       break;
 }
 set_quant_table(std_luminance_qt, (BYTE)SCALEFACTOR, tempQT);

 for (j=0;j<=63;j++) quant_table[j]=tempQT[zigzag[j]];
 j=0;
 for (row=0;row<=7;row++)
   for (col=0;col<=7;col++) {
		quant_table[j] = (long)((quant_table[j] * scalefactor[row] * scalefactor[col]) * 65536);
		j++;
   }
 byte_pos+=64;
}

void load_quant_tableCb(long *quant_table)
{
 float scalefactor[8]={1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
					   1.0f, 0.785694958f, 0.541196100f, 0.275899379f};
 BYTE j,row,col;
 BYTE tempQT[64];

// Load quantization coefficients from JPG file, scale them for DCT and reorder
// from zig-zag order
 if (Mapping == 1) {
   switch (UV_selector) {
     case 0:
         std_chrominance_qt = Tbl_000Y;
         break;
     case 1:
         std_chrominance_qt = Tbl_014Y;
         break;
     case 2:
         std_chrominance_qt = Tbl_029Y;
         break;
     case 3:
         std_chrominance_qt = Tbl_043Y;
         break;
     case 4:
         std_chrominance_qt = Tbl_057Y;
         break;
     case 5:
         std_chrominance_qt = Tbl_071Y;
         break;
     case 6:
         std_chrominance_qt = Tbl_086Y;
         break;
     case 7:
         std_chrominance_qt = Tbl_100Y;
         break;
   }
 }
 else {
   switch (UV_selector) {
     case 0:
         std_chrominance_qt = Tbl_000UV;
         break;
     case 1:
         std_chrominance_qt = Tbl_014UV;
         break;
     case 2:
         std_chrominance_qt = Tbl_029UV;
         break;
     case 3:
         std_chrominance_qt = Tbl_043UV;
         break;
     case 4:
         std_chrominance_qt = Tbl_057UV;
         break;
     case 5:
         std_chrominance_qt = Tbl_071UV;
         break;
     case 6:
         std_chrominance_qt = Tbl_086UV;
         break;
     case 7:
         std_chrominance_qt = Tbl_100UV;
         break;
   }
 }
 set_quant_table(std_chrominance_qt, (BYTE)SCALEFACTORUV, tempQT);

 for (j=0;j<=63;j++) quant_table[j]=tempQT[zigzag[j]];
 j=0;
 for (row=0;row<=7;row++)
   for (col=0;col<=7;col++) {
		quant_table[j] = (long)((quant_table[j] * scalefactor[row] * scalefactor[col]) * 65536);
		j++;
   }
 byte_pos+=64;
}

//  Note: Added for Dual_JPEG
void load_advance_quant_table(long *quant_table)
{
 float scalefactor[8]={1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
					   1.0f, 0.785694958f, 0.541196100f, 0.275899379f};
 BYTE j,row,col;
 BYTE tempQT[64];

// Load quantization coefficients from JPG file, scale them for DCT and reorder
// from zig-zag order
 switch (advance_selector) {
   case 0:
       std_luminance_qt = Tbl_000Y;
       break;
   case 1:
       std_luminance_qt = Tbl_014Y;
       break;
   case 2:
       std_luminance_qt = Tbl_029Y;
       break;
   case 3:
       std_luminance_qt = Tbl_043Y;
       break;
   case 4:
       std_luminance_qt = Tbl_057Y;
       break;
   case 5:
       std_luminance_qt = Tbl_071Y;
       break;
   case 6:
       std_luminance_qt = Tbl_086Y;
       break;
   case 7:
       std_luminance_qt = Tbl_100Y;
       break;
 }
//  Note: pass ADVANCE SCALE FACTOR to sub-function in Dual-JPEG
 set_quant_table(std_luminance_qt, (BYTE)ADVANCESCALEFACTOR, tempQT);

 for (j=0;j<=63;j++) quant_table[j]=tempQT[zigzag[j]];
 j=0;
 for (row=0;row<=7;row++)
   for (col=0;col<=7;col++) {
		quant_table[j] = (long)((quant_table[j] * scalefactor[row] * scalefactor[col]) * 65536);
		j++;
   }
 byte_pos+=64;
}

//  Note: Added for Dual-JPEG
void load_advance_quant_tableCb(long *quant_table)
{
 float scalefactor[8]={1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
					   1.0f, 0.785694958f, 0.541196100f, 0.275899379f};
 BYTE j,row,col;
 BYTE tempQT[64];

// Load quantization coefficients from JPG file, scale them for DCT and reorder
// from zig-zag order
 if (Mapping == 1) {
   switch (advance_selector) {
     case 0:
         std_chrominance_qt = Tbl_000Y;
         break;
     case 1:
         std_chrominance_qt = Tbl_014Y;
         break;
     case 2:
         std_chrominance_qt = Tbl_029Y;
         break;
     case 3:
         std_chrominance_qt = Tbl_043Y;
         break;
     case 4:
         std_chrominance_qt = Tbl_057Y;
         break;
     case 5:
         std_chrominance_qt = Tbl_071Y;
         break;
     case 6:
         std_chrominance_qt = Tbl_086Y;
         break;
     case 7:
         std_chrominance_qt = Tbl_100Y;
         break;
   }
 }
 else {
   switch (advance_selector) {
     case 0:
         std_chrominance_qt = Tbl_000UV;
         break;
     case 1:
         std_chrominance_qt = Tbl_014UV;
         break;
     case 2:
         std_chrominance_qt = Tbl_029UV;
         break;
     case 3:
         std_chrominance_qt = Tbl_043UV;
         break;
     case 4:
         std_chrominance_qt = Tbl_057UV;
         break;
     case 5:
         std_chrominance_qt = Tbl_071UV;
         break;
     case 6:
         std_chrominance_qt = Tbl_086UV;
         break;
     case 7:
         std_chrominance_qt = Tbl_100UV;
         break;
   }
 }
//  Note: pass ADVANCE SCALE FACTOR to sub-function in Dual-JPEG
 set_quant_table(std_chrominance_qt, (BYTE)ADVANCESCALEFACTORUV, tempQT);

 for (j=0;j<=63;j++) quant_table[j]=tempQT[zigzag[j]];
 j=0;
 for (row=0;row<=7;row++)
   for (col=0;col<=7;col++) {
		quant_table[j] = (long)((quant_table[j] * scalefactor[row] * scalefactor[col]) * 65536);
		j++;
   }
 byte_pos+=64;
}

void load_Huffman_table(Huffman_table *HT, BYTE *nrcode, BYTE *value, WORD *Huff_code)
{
  BYTE k,j, i;
  DWORD code, code_index;
  
  for (j=1;j<=16;j++) {
	HT->Length[j]=nrcode[j];
  }
  for (i=0, k=1;k<=16;k++)
	for (j=0;j<HT->Length[k];j++) {
		HT->V[WORD_hi_lo(k,j)]=value[i];
		i++;
	}

  code=0;
  for (k=1;k<=16;k++) {
	 HT->minor_code[k] = (WORD)code;
	 for (j=1;j<=HT->Length[k];j++) code++;
	 HT->major_code[k]=(WORD)(code-1);
	 code*=2;
	 if (HT->Length[k]==0) {
		HT->minor_code[k]=0xFFFF;
		HT->major_code[k]=0;
	 }
  }

  HT->Len[0] = 2;  i = 2;

  for (code_index = 1; code_index < 65535; code_index++) {
	  if (code_index < Huff_code[i]) {
          HT->Len[code_index] = (unsigned char)Huff_code[i + 1];
	  }
	  else {
		  i = i + 2;
          HT->Len[code_index] = (unsigned char)Huff_code[i + 1];
	  }
  }
}

//river
void process_Huffman_data_unit(BYTE DC_nr, BYTE AC_nr,SWORD *previous_DC, WORD position)
{
   BYTE nr,k;
   register WORD tmp_Hcode;
   BYTE size_val,count_0;
   WORD *min_code;
   BYTE *huff_values;
   BYTE byte_temp;


   min_code=HTDC[DC_nr].minor_code;
//   maj_code=HTDC[DC_nr].major_code;
   huff_values=HTDC[DC_nr].V;

//DC
   nr=0;
   k = HTDC[DC_nr].Len[(WORD)(codebuf>>16)];
//river
//	 tmp_Hcode=lookKbits(k);
   tmp_Hcode = (WORD)(codebuf>>(32-k));
   skipKbits(k);
   size_val=huff_values[WORD_hi_lo(k,(BYTE)(tmp_Hcode-min_code[k]))];
   if (size_val==0) DCT_coeff[position + 0]=*previous_DC;
   else {
       DCT_coeff[position + 0]=*previous_DC+getKbits(size_val);
       *previous_DC=DCT_coeff[position + 0];
   }

// Second, AC coefficient decoding
   min_code=HTAC[AC_nr].minor_code;
//   maj_code=HTAC[AC_nr].major_code;
   huff_values=HTAC[AC_nr].V;

   nr=1; // AC coefficient
   do {
       k = HTAC[AC_nr].Len[(WORD)(codebuf>>16)];
	   tmp_Hcode = (WORD)(codebuf>>(32-k));
       skipKbits(k);

       byte_temp=huff_values[WORD_hi_lo(k,(BYTE)(tmp_Hcode-min_code[k]))];
       size_val=byte_temp&0xF;
       count_0=byte_temp>>4;
       if (size_val==0) {
            if (count_0 != 0xF) {
	            break;
            }
			nr+=16;
       }
	   else {
		 nr+=count_0; //skip count_0 zeroes
		 DCT_coeff[position + dezigzag[nr++]]=getKbits(size_val);
	   }
   } while (nr < 64);
}

void init_jpg_table ()
{
 init_QT();
 Init_Color_Table ();
 prepare_range_limit_table();
 load_Huffman_table(&HTDC[0], std_dc_luminance_nrcodes, std_dc_luminance_values, DC_LUMINANCE_HUFFMANCODE);
 load_Huffman_table(&HTAC[0], std_ac_luminance_nrcodes, std_ac_luminance_values, AC_LUMINANCE_HUFFMANCODE);
 load_Huffman_table(&HTDC[1], std_dc_chrominance_nrcodes, std_dc_chrominance_values, DC_CHROMINANCE_HUFFMANCODE);
 load_Huffman_table(&HTAC[1], std_ac_chrominance_nrcodes, std_ac_chrominance_values, AC_CHROMINANCE_HUFFMANCODE);
}

int init_JPG_decoding()
{
 byte_pos=0;
 load_quant_table(QT[0]);
 load_quant_tableCb(QT[1]);
//  Note: Added for Dual-JPEG
 load_advance_quant_table(QT[2]);
 load_advance_quant_tableCb(QT[3]);
 return 1;
}

void Init_Color_Table()
{
	int i, x;
	int nScale	= 1L << 16;		//equal to power(2,16)
	int nHalf	= nScale >> 1;

#define FIX(x) ((int) ((x) * nScale + 0.5))


	/* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
    /* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
    /* Cr=>R value is nearest int to 1.597656 * x */
        /* Cb=>B value is nearest int to 2.015625 * x */
        /* Cr=>G value is scaled-up -0.8125 * x */
        /* Cb=>G value is scaled-up -0.390625 * x */
	for (i = 0, x = -128; i < 256; i++, x++)
	{
                m_CrToR[i] = (int) ( FIX(1.597656) * x + nHalf ) >> 16;
                m_CbToB[i] = (int) ( FIX(2.015625) * x + nHalf ) >> 16;
                m_CrToG[i] = (int) (- FIX(0.8125) * x + nHalf) >> 16;
                m_CbToG[i] = (int) (- FIX(0.390625) * x + nHalf ) >> 16;
	}
        for (i = 0, x = -16; i < 256; i++, x++)
	{
                m_Y[i] = (int) ( FIX(1.164) * x + nHalf ) >> 16;
	}
//For Color Text Enchance Y Re-map. Recommend to disable in default
/*
        for (i = 0; i < (VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate); i++) {
                temp = (double)i / VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate;
                temp1 = 1.0 / VideoEngineInfo->INFData.Gamma1Parameter;
                m_Y[i] = (BYTE)(VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate * pow (temp, temp1));
                if (m_Y[i] > 255) m_Y[i] = 255;
        }
        for (i = (VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate); i < 256; i++) {
                m_Y[i] = (BYTE)((VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate) + (256 - VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate) * ( pow((double)((i - VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate) / (256 - (VideoEngineInfo->INFData.Gamma1_Gamma2_Seperate))), (1.0 / VideoEngineInfo->INFData.Gamma2Parameter)) ));
                if (m_Y[i] > 255) m_Y[i] = 255;
        }
*/        
}

//  added for free buffer
void  FreeQT()
{
    BYTE  i;
    for (i=0;i<=3;i++) {
        free (QT[i]);
    }
    rlimit_table -= 256;
    free (rlimit_table);

}
void IDCT_transform(short *coef, BYTE *data, BYTE nBlock)
{

#define FIX_1_082392200  ((int)277)		/* FIX(1.082392200) */
#define FIX_1_414213562  ((int)362)		/* FIX(1.414213562) */
#define FIX_1_847759065  ((int)473)		/* FIX(1.847759065) */
#define FIX_2_613125930  ((int)669)		/* FIX(2.613125930) */

#define MULTIPLY(var,cons)  ((int) ((var)*(cons))>>8 )

	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z5, z10, z11, z12, z13;
	int workspace[64];		/* buffers data between passes */

	short* inptr = coef;
    long* quantptr;
	int* wsptr = workspace;
	unsigned char* outptr;
        unsigned char* r_limit = rlimit_table+128;
	int ctr, dcval, DCTSIZE = 8;

    quantptr = (long *)QT[nBlock];

	//Pass 1: process columns from input (inptr), store into work array(wsptr)

	for (ctr = 8; ctr > 0; ctr--) {
    /* Due to quantization, we will usually find that many of the input
	* coefficients are zero, especially the AC terms.  We can exploit this
	* by short-circuiting the IDCT calculation for any column in which all
	* the AC terms are zero.  In that case each output is equal to the
	* DC coefficient (with scale factor as needed).
	* With typical images and quantization tables, half or more of the
	* column DCT calculations can be simplified this way.
	*/

		if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*2] | inptr[DCTSIZE*3] |
			inptr[DCTSIZE*4] | inptr[DCTSIZE*5] | inptr[DCTSIZE*6] |
			inptr[DCTSIZE*7]) == 0)
		{
			/* AC terms all zero */
			dcval = (int)(( inptr[DCTSIZE*0] * quantptr[DCTSIZE*0] ) >> 16);

			wsptr[DCTSIZE*0] = dcval;
			wsptr[DCTSIZE*1] = dcval;
			wsptr[DCTSIZE*2] = dcval;
			wsptr[DCTSIZE*3] = dcval;
			wsptr[DCTSIZE*4] = dcval;
			wsptr[DCTSIZE*5] = dcval;
			wsptr[DCTSIZE*6] = dcval;
			wsptr[DCTSIZE*7] = dcval;

			inptr++;			/* advance pointers to next column */
			quantptr++;
			wsptr++;
			continue;
		}

		/* Even part */

		tmp0 = (inptr[DCTSIZE*0] * quantptr[DCTSIZE*0]) >> 16;
		tmp1 = (inptr[DCTSIZE*2] * quantptr[DCTSIZE*2]) >> 16;
		tmp2 = (inptr[DCTSIZE*4] * quantptr[DCTSIZE*4]) >> 16;
		tmp3 = (inptr[DCTSIZE*6] * quantptr[DCTSIZE*6]) >> 16;

		tmp10 = tmp0 + tmp2;	/* phase 3 */
		tmp11 = tmp0 - tmp2;

		tmp13 = tmp1 + tmp3;	/* phases 5-3 */
		tmp12 = MULTIPLY(tmp1 - tmp3, FIX_1_414213562) - tmp13; /* 2*c4 */

		tmp0 = tmp10 + tmp13;	/* phase 2 */
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		/* Odd part */

		tmp4 = (inptr[DCTSIZE*1] * quantptr[DCTSIZE*1]) >> 16;
		tmp5 = (inptr[DCTSIZE*3] * quantptr[DCTSIZE*3]) >> 16;
		tmp6 = (inptr[DCTSIZE*5] * quantptr[DCTSIZE*5]) >> 16;
		tmp7 = (inptr[DCTSIZE*7] * quantptr[DCTSIZE*7]) >> 16;

		z13 = tmp6 + tmp5;		/* phase 6 */
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;

		tmp7  = z11 + z13;		/* phase 5 */
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); /* 2*c4 */

		z5	  = MULTIPLY(z10 + z12, FIX_1_847759065); /* 2*c2 */
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
		tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */

		tmp6 = tmp12 - tmp7;	/* phase 2 */
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		wsptr[DCTSIZE*0] = (int) (tmp0 + tmp7);
		wsptr[DCTSIZE*7] = (int) (tmp0 - tmp7);
		wsptr[DCTSIZE*1] = (int) (tmp1 + tmp6);
		wsptr[DCTSIZE*6] = (int) (tmp1 - tmp6);
		wsptr[DCTSIZE*2] = (int) (tmp2 + tmp5);
		wsptr[DCTSIZE*5] = (int) (tmp2 - tmp5);
		wsptr[DCTSIZE*4] = (int) (tmp3 + tmp4);
		wsptr[DCTSIZE*3] = (int) (tmp3 - tmp4);

		inptr++;			/* advance pointers to next column */
		quantptr++;
		wsptr++;
	}

	/* Pass 2: process rows from work array, store into output array. */
	/* Note that we must descale the results by a factor of 8 == 2**3, */
	/* and also undo the PASS1_BITS scaling. */

//#define RANGE_MASK 1023; //2 bits wider than legal samples
#define PASS1_BITS  0
#define IDESCALE(x,n)  ((int) ((x)>>n) )

	wsptr = workspace;
	for (ctr = 0; ctr < DCTSIZE; ctr++) {
		outptr = data + ctr * 8;

		/* Rows of zeroes can be exploited in the same way as we did with columns.
		* However, the column calculation has created many nonzero AC terms, so
		* the simplification applies less often (typically 5% to 10% of the time).
		* On machines with very fast multiplication, it's possible that the
		* test takes more time than it's worth.  In that case this section
		* may be commented out.
		*/
		/* Even part */

		tmp10 = ((int) wsptr[0] + (int) wsptr[4]);
		tmp11 = ((int) wsptr[0] - (int) wsptr[4]);

		tmp13 = ((int) wsptr[2] + (int) wsptr[6]);
		tmp12 = MULTIPLY((int) wsptr[2] - (int) wsptr[6], FIX_1_414213562)
			- tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		/* Odd part */

		z13 = (int) wsptr[5] + (int) wsptr[3];
		z10 = (int) wsptr[5] - (int) wsptr[3];
		z11 = (int) wsptr[1] + (int) wsptr[7];
		z12 = (int) wsptr[1] - (int) wsptr[7];

		tmp7 = z11 + z13;		/* phase 5 */
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); /* 2*c4 */

		z5    = MULTIPLY(z10 + z12, FIX_1_847759065); /* 2*c2 */
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
		tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */

		tmp6 = tmp12 - tmp7;	/* phase 2 */
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		/* Final output stage: scale down by a factor of 8 and range-limit */

                outptr[0] = r_limit[IDESCALE((tmp0 + tmp7), (PASS1_BITS+3))
                        & 1023L];
		outptr[7] = r_limit[IDESCALE((tmp0 - tmp7), (PASS1_BITS+3))
                        & 1023L];
		outptr[1] = r_limit[IDESCALE((tmp1 + tmp6), (PASS1_BITS+3))
                        & 1023L];
		outptr[6] = r_limit[IDESCALE((tmp1 - tmp6), (PASS1_BITS+3))
                        & 1023L];
		outptr[2] = r_limit[IDESCALE((tmp2 + tmp5), (PASS1_BITS+3))
                        & 1023L];
		outptr[5] = r_limit[IDESCALE((tmp2 - tmp5), (PASS1_BITS+3))
                        & 1023L];
		outptr[4] = r_limit[IDESCALE((tmp3 + tmp4), (PASS1_BITS+3))
                        & 1023L];
		outptr[3] = r_limit[IDESCALE((tmp3 - tmp4), (PASS1_BITS+3))
                        & 1023L];

		wsptr += DCTSIZE;		/* advance pointer to next row */
	}
}


void updatereadbuf(unsigned long *codebuf, unsigned long *newbuf, int walks, int *newbits, unsigned long *fp)
{  unsigned long readbuf;

    if ((*newbits-walks)<=0) {
       readbuf = Buffer[index];
       index++;
      *codebuf=(*codebuf<<walks)|((*newbuf|(readbuf>>(*newbits)))>>(32-walks));
      *newbuf=readbuf<<(walks-*newbits);
      *newbits=32+*newbits-walks;
    } else {
    	*codebuf=(*codebuf<<walks)|(*newbuf>>(32-walks));
    	*newbuf=*newbuf<<walks;
    	*newbits-=walks;
    }
}

void YUVToRGB  (
		int txb, int tyb,
		unsigned char * pYCbCr,	//in, Y: 256 or 64 bytes; Cb: 64 bytes; Cr: 64 bytes
		struct RGB * pYUV,	//in, Y: 256 or 64 bytes; Cb: 64 bytes; Cr: 64 bytes
		unsigned char * pBgr	//out, BGR format, 16*16*3 = 768 bytes; or 8*8*3=192 bytes
               )
{
        int i, j, pos, m, n;
        unsigned char cb, cr, *py, *pcb, *pcr, *py420[4];
        int y;
        struct RGB *pByte; 
        int nBlocksInMcu = 6;
        unsigned int pixel_x, pixel_y;

        pByte = (struct RGB *)pBgr;
		if (Mode_420 == 0) {
		    py = pYCbCr;
  	        pcb	  = pYCbCr + 64;
	        pcr   = pcb + 64;

	        pixel_x = txb*8;
            pixel_y = tyb*8;
            pos = (pixel_y*WIDTH)+pixel_x;


	        for(j = 0; j < 8; j++) {
		       for(i = 0; i < 8; i++) {
                    m = ( (j << 3 ) + i);
					y = py[m];
    		        cb = pcb[m];
    		        cr = pcr[m];
                    n = pos + i;
//For 2Pass. Save the YUV value
					pYUV[n].B = cb;
					pYUV[n].G = y;
					pYUV[n].R = cr;
                    pByte[n].B = rlimit_table[ m_Y[y] + m_CbToB[cb] ];
                    pByte[n].G = rlimit_table[ m_Y[y] + m_CbToG[cb] + m_CrToG[cr]];
                    pByte[n].R = rlimit_table[ m_Y[y] + m_CrToR[cr] ];
			   }
		       pos += WIDTH;
			}
		}
		else {
	        for(i = 0; i < nBlocksInMcu-2; i++)
		        py420[i] = pYCbCr + i * 64;
            pcb	  = pYCbCr + (nBlocksInMcu-2) * 64;
            pcr   = pcb + 64;

            pixel_x = txb*16;
            pixel_y = tyb*16;
            pos = (pixel_y*WIDTH)+pixel_x;

            for(j = 0; j < 16; j++) {
		        for(i = 0; i < 16; i++) {
			    //	block number is ((j/8) * 2 + i/8)={0, 1, 2, 3}
			        y = *( py420[(j>>3) * 2 + (i>>3)] ++ );
                    m = ( (j>>1) << 3 ) + (i>>1);
    		        cb = pcb[m];
    		        cr = pcr[m];
                    n = pos + i;
                    pByte[n].B = rlimit_table[ m_Y[y] + m_CbToB[cb] ];
                    pByte[n].G = rlimit_table[ m_Y[y] + m_CbToG[cb] + m_CrToG[cr]];
                    pByte[n].R = rlimit_table[ m_Y[y] + m_CrToR[cr] ];
				}
		        pos += WIDTH;
			}
		}
}


void YUVToBuffer  (
		int txb, int tyb,
		unsigned char * pYCbCr,	//in, Y: 256 or 64 bytes; Cb: 64 bytes; Cr: 64 bytes
		struct RGB * pYUV,	//out, BGR format, 16*16*3 = 768 bytes; or 8*8*3=192 bytes
		unsigned char * pBgr	//out, BGR format, 16*16*3 = 768 bytes; or 8*8*3=192 bytes
               )
{
        int i, j, pos, m, n;
        unsigned char cb, cr, *py, *pcb, *pcr, *py420[4];
        int y;
        struct RGB *pByte; 
        int nBlocksInMcu = 6;
        unsigned int pixel_x, pixel_y;

        pByte = (struct RGB *)pBgr;
		if (Mode_420 == 0) {
		    py = pYCbCr;
  	        pcb	  = pYCbCr + 64;
	        pcr   = pcb + 64;

	        pixel_x = txb*8;
            pixel_y = tyb*8;
            pos = (pixel_y*WIDTH)+pixel_x;

	        for(j = 0; j < 8; j++) {
		       for(i = 0; i < 8; i++) {
                    m = ( (j << 3 ) + i);
                    n = pos + i;
					y = pYUV[n].G + (py[m] - 128);
    		        cb = pYUV[n].B + (pcb[m] - 128);
    		        cr = pYUV[n].R + (pcr[m] - 128);
					pYUV[n].B = cb;
					pYUV[n].G = y;
					pYUV[n].R = cr;
                    pByte[n].B = rlimit_table[ m_Y[y] + m_CbToB[cb] ];
                    pByte[n].G = rlimit_table[ m_Y[y] + m_CbToG[cb] + m_CrToG[cr]];
                    pByte[n].R = rlimit_table[ m_Y[y] + m_CrToR[cr] ];
			   }
		       pos += WIDTH;
			}
		}
		else {
	        for(i = 0; i < nBlocksInMcu-2; i++)
		        py420[i] = pYCbCr + i * 64;
            pcb	  = pYCbCr + (nBlocksInMcu-2) * 64;
            pcr   = pcb + 64;

            pixel_x = txb*16;
            pixel_y = tyb*16;
            pos = (pixel_y*WIDTH)+pixel_x;

            for(j = 0; j < 16; j++) {
		        for(i = 0; i < 16; i++) {
			    //	block number is ((j/8) * 2 + i/8)={0, 1, 2, 3}
			        y = *( py420[(j>>3) * 2 + (i>>3)] ++ );
                    m = ( (j>>1) << 3 ) + (i>>1);
    		        cb = pcb[m];
    		        cr = pcr[m];
                    n = pos + i;
                    pByte[n].B = rlimit_table[ m_Y[y] + m_CbToB[cb] ];
                    pByte[n].G = rlimit_table[ m_Y[y] + m_CbToG[cb] + m_CrToG[cr]];
                    pByte[n].R = rlimit_table[ m_Y[y] + m_CrToR[cr] ];
				}
		        pos += WIDTH;
			}
		}
}

int Decompress (int txb, int tyb, char *outBuf, BYTE QT_TableSelection)
{
	unsigned char    *ptr;
    unsigned char    byTileYuv[768];

	memset (DCT_coeff, 0, 384 * 2);


        ptr = byTileYuv;
        process_Huffman_data_unit(YDC_nr,YAC_nr,&DCY, 0);
        IDCT_transform(DCT_coeff, ptr, QT_TableSelection);
        ptr += 64;

	    if (Mode_420 == 1) {
            process_Huffman_data_unit(YDC_nr,YAC_nr,&DCY, 64);
            IDCT_transform(DCT_coeff + 64, ptr, QT_TableSelection);
	        ptr += 64;

            process_Huffman_data_unit(YDC_nr,YAC_nr,&DCY, 128);
            IDCT_transform(DCT_coeff + 128, ptr, QT_TableSelection);
	        ptr += 64;

            process_Huffman_data_unit(YDC_nr,YAC_nr,&DCY, 192);
            IDCT_transform(DCT_coeff + 192, ptr, QT_TableSelection);
	        ptr += 64;

            process_Huffman_data_unit(CbDC_nr,CbAC_nr,&DCCb, 256);
            IDCT_transform(DCT_coeff + 256, ptr, QT_TableSelection + 1);
	        ptr += 64;

            process_Huffman_data_unit(CrDC_nr,CrAC_nr,&DCCr, 320);
            IDCT_transform(DCT_coeff + 320, ptr, QT_TableSelection + 1);
		}
	    else {
            process_Huffman_data_unit(CbDC_nr,CbAC_nr,&DCCb, 64);
            IDCT_transform(DCT_coeff + 64, ptr, QT_TableSelection + 1);
	        ptr += 64;

            process_Huffman_data_unit(CrDC_nr,CrAC_nr,&DCCr, 128);
            IDCT_transform(DCT_coeff + 128, ptr, QT_TableSelection + 1);
		}

//    YUVToRGB (txb, tyb, byTileYuv, (unsigned char *)outBuf);
//  YUVBuffer for YUV record
    YUVToRGB (txb, tyb, byTileYuv, (struct RGB *)YUVBuffer, (unsigned char *)outBuf);

    return  TRUE;
}

int Decompress_2PASS (int txb, int tyb, char *outBuf, BYTE QT_TableSelection)
{
	unsigned char    *ptr;
    unsigned char    byTileYuv[768];

	memset (DCT_coeff, 0, 384 * 2);

    ptr = byTileYuv;
    process_Huffman_data_unit(YDC_nr,YAC_nr,&DCY, 0);
    IDCT_transform(DCT_coeff, ptr, QT_TableSelection);
    ptr += 64;

    process_Huffman_data_unit(CbDC_nr,CbAC_nr,&DCCb, 64);
    IDCT_transform(DCT_coeff + 64, ptr, QT_TableSelection + 1);
    ptr += 64;

    process_Huffman_data_unit(CrDC_nr,CrAC_nr,&DCCr, 128);
    IDCT_transform(DCT_coeff + 128, ptr, QT_TableSelection + 1);

	YUVToBuffer (txb, tyb, byTileYuv, (struct RGB *)YUVBuffer, (unsigned char *)outBuf);
//    YUVToRGB (txb, tyb, byTileYuv, (unsigned char *)outBuf);

    return  TRUE;
}

int VQ_Decompress (int txb, int tyb, char *outBuf, BYTE QT_TableSelection, struct COLOR_CACHE *VQ)
{
	unsigned char    *ptr, i;
    unsigned char    byTileYuv[192];
	int    Data;

    ptr = byTileYuv;
	if (VQ->BitMapBits == 0) {
		for (i = 0; i < 64; i++) {
		    ptr[0] = (VQ->Color[VQ->Index[0]] & 0xFF0000) >> 16;
		    ptr[64] = (VQ->Color[VQ->Index[0]] & 0x00FF00) >> 8;
		    ptr[128] = VQ->Color[VQ->Index[0]] & 0x0000FF;
		    ptr += 1;
		}
	}
	else {
	    for (i = 0; i < 64; i++) {
		    Data = (int)lookKbits(VQ->BitMapBits);
		    ptr[0] = (VQ->Color[VQ->Index[Data]] & 0xFF0000) >> 16;
		    ptr[64] = (VQ->Color[VQ->Index[Data]] & 0x00FF00) >> 8;
		    ptr[128] = VQ->Color[VQ->Index[Data]] & 0x0000FF;
		    ptr += 1;
		    skipKbits(VQ->BitMapBits);
		}
	}
//    YUVToRGB (txb, tyb, byTileYuv, (unsigned char *)outBuf);
    YUVToRGB (txb, tyb, byTileYuv, (struct RGB *)YUVBuffer, (unsigned char *)outBuf);

    return  TRUE;
}

void MoveBlockIndex (void)
{
	if (Mode_420 == 0) {
	    txb++;
	    if (txb >= (int)(tmp_WIDTH/8)) {
		    tyb++;
		    if (tyb >= (int)(tmp_HEIGHT/8))
			    tyb = 0;
		    txb = 0;
		}
        pixels += 64;
	}
	else {
	    txb++;
	    if (txb >= (int)(tmp_WIDTH/16)) {
		    tyb++;
		    if (tyb >= (int)(tmp_HEIGHT/16))
			    tyb = 0;
		    txb = 0;
		}
        pixels += 256;
	}
}

void VQ_Initialize (struct COLOR_CACHE *VQ)
{
	int i;

	for (i = 0; i < 4; i++) {
	    VQ->Index[i] = i;
	}
	VQ->Color[0] = 0x008080;
	VQ->Color[1] = 0xFF8080;
	VQ->Color[2] = 0x808080;
	VQ->Color[3] = 0xC08080;
}

ULONG decode(PVIDEO_ENGINE_INFO VideoEngineInfo, ULONG MB, ULONG StringLength)
{
  ULONG  NumberOfMB, i;
  struct COLOR_CACHE    Decode_Color;

  VQ_Initialize (&Decode_Color);
//OutputDebugString  ("In decode\n");
//            GetINFData (VideoEngineInfo);
//  WIDTH = VideoEngineInfo->SourceModeInfo.X = 640;
//  HEIGHT = VideoEngineInfo->SourceModeInfo.Y = 480;
//  AST2000 JPEG block is 16x16(pixels) base
	if (Mode_420 == 1) {
		if (WIDTH % 16) {
			WIDTH = WIDTH + 16 - (WIDTH % 16);
		}
		if (HEIGHT % 16) {
			HEIGHT = HEIGHT + 16 - (HEIGHT % 16);
		}
	} else {
		if (WIDTH % 8) {
			WIDTH = WIDTH + 8 - (WIDTH % 8);
		}
		if (HEIGHT % 8) {
			HEIGHT = HEIGHT + 8 - (HEIGHT % 8);
		}
	}

//  tmp_WDITH, tmp_HEIGHT are for block position
//  tmp_WIDTH = VideoEngineInfo->DestinationModeInfo.X;
//  tmp_HEIGHT = VideoEngineInfo->DestinationModeInfo.Y;
  if (Mode_420 == 1) {
      if (tmp_WIDTH % 16) {
        tmp_WIDTH = tmp_WIDTH + 16 - (tmp_WIDTH % 16);
	  }
      if (tmp_HEIGHT % 16) {
        tmp_HEIGHT = tmp_HEIGHT + 16 - (tmp_HEIGHT % 16);
	  }
  }
  else {
      if (tmp_WIDTH % 8) {
        tmp_WIDTH = tmp_WIDTH + 8 - (tmp_WIDTH % 8);
	  }
      if (tmp_HEIGHT % 8) {
        tmp_HEIGHT = tmp_HEIGHT + 8 - (tmp_HEIGHT % 8);
	  }
  }

//  templ=(sizeof(struct RGB1)*HEIGHT*WIDTH);
//  bitmap=malloc((size_t)templ);
//  templ=(sizeof(struct RGB)*HEIGHT*WIDTH);  
//  OutBuffer=malloc(templ);
//  Get data from encode program. These data should get from network.
//  These codes are for DOS version sample only
/*
  WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, 0xF004, 0, 0x1E700000);
  WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, 0xF000, 0, 0x1);
  WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, KEY_CONTROL, 0, 0xA8);
  CompressBufferSize = Indwm (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, COMPRESS_DATA_COUNT_REGISTER);
  VideoEngineInfo->CompressData.CompressSize = CompressBufferSize * 4;
  for (index = 0; index < CompressBufferSize; index++) {
      Buffer[index] = IndwmBankMode (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, VIDEO_COMPRESS_DESTINATION_BUFFER_ADDRESS + index * 4);
  }
*/
//  RC4 decoding. Keys_Expansion, RC4_setup and RC4_crypt please reference video.c

  if (RC4_Enable == 1) {
	if(First_Frame == 1) {
          Keys_Expansion (DecodeKeys);
          DecodeRC4_setup (&s, DecodeKeys, VideoEngineInfo);
	}


      RC4_crypt (&s, (unsigned char*)Buffer, StringLength);
  }

  if (Old_Mode_420 != Mode_420) {
	  Old_Mode_420 = Mode_420;
	  return TRUE;
  }

    yheader.qfactor = 16;
//    yheader.qfactor = VideoEngineInfo->FrameHeader.JPEGScaleFactor;
 //   yheader.height = VideoEngineInfo->DestinationModeInfo.Y;
 //   yheader.width = VideoEngineInfo->DestinationModeInfo.X;
    NumberOfMB = MB;

    SCALEFACTOR=yheader.qfactor;
    SCALEFACTORUV=yheader.qfactor;
    ADVANCESCALEFACTOR= 16;
    ADVANCESCALEFACTORUV= 16;
//    advance_selector = 7;
    Mapping = 0;
    SharpModeSelection = 0;
/*
    ADVANCESCALEFACTOR= VideoEngineInfo->FrameHeader.AdvanceScaleFactor;
    ADVANCESCALEFACTORUV= VideoEngineInfo->FrameHeader.AdvanceScaleFactor;
    advance_selector = VideoEngineInfo->FrameHeader.AdvanceTableSelector;
    Mapping = VideoEngineInfo->FrameHeader.JPEGYUVTableMapping;
    SharpModeSelection = VideoEngineInfo->FrameHeader.SharpModeSelection;
*/
//    init_jpg_table ();
	if (First_Frame == 1) {
        init_JPG_decoding();
		Old_Y_Selector = Y_selector;
		Old_advance_selector = advance_selector;
//		return TRUE;
	}

    codebuf = Buffer[0];
    newbuf = Buffer[1];
    index = 2;

    txb = tyb = 0;
    newbits=32;
    DCY = DCCb = DCCr = 0;

	do {
      if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==JPEG_NO_SKIP_CODE) {
        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_START_LENGTH, &newbits, Buffer);
        Decompress (txb, tyb, (char *)OutBuffer, 0);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==FRAME_END_CODE) {
//          FreeQT ();
          return TRUE;
	  }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==JPEG_SKIP_CODE) {
        txb = (codebuf & 0x0FF00000) >> 20;
        tyb = (codebuf & 0x0FF000) >> 12;

        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_SKIP_LENGTH, &newbits, Buffer);
        Decompress (txb, tyb, (char *)OutBuffer, 0);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==VQ_NO_SKIP_1_COLOR_CODE) {
        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_START_LENGTH, &newbits, Buffer);
		Decode_Color.BitMapBits = 0;

		for (i = 0; i < 1; i++) {
		    Decode_Color.Index[i] = ((codebuf>>29)&(long)VQ_INDEX_MASK);
		    if (((codebuf>>31)&(long)VQ_HEADER_MASK)==VQ_NO_UPDATE_HEADER) {
                updatereadbuf (&codebuf, &newbuf, VQ_NO_UPDATE_LENGTH, &newbits, Buffer);
			}
		    else {
			    Decode_Color.Color[Decode_Color.Index[i]] = ((codebuf>>5)&(long)VQ_COLOR_MASK);
                updatereadbuf (&codebuf, &newbuf, VQ_UPDATE_LENGTH, &newbits, Buffer);
			}
		}
        VQ_Decompress (txb, tyb, (char *)OutBuffer, 0, &Decode_Color);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==VQ_SKIP_1_COLOR_CODE) {
        txb = (codebuf & 0x0FF00000) >> 20;
        tyb = (codebuf & 0x0FF000) >> 12;

        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_SKIP_LENGTH, &newbits, Buffer);
		Decode_Color.BitMapBits = 0;

		for (i = 0; i < 1; i++) {
		    Decode_Color.Index[i] = ((codebuf>>29)&(long)VQ_INDEX_MASK);
		    if (((codebuf>>31)&(long)VQ_HEADER_MASK)==VQ_NO_UPDATE_HEADER) {
                updatereadbuf (&codebuf, &newbuf, VQ_NO_UPDATE_LENGTH, &newbits, Buffer);
			}
		    else {
			    Decode_Color.Color[Decode_Color.Index[i]] = ((codebuf>>5)&(long)VQ_COLOR_MASK);
                updatereadbuf (&codebuf, &newbuf, VQ_UPDATE_LENGTH, &newbits, Buffer);
			}
		}
        VQ_Decompress (txb, tyb, (char *)OutBuffer, 0, &Decode_Color);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==VQ_NO_SKIP_2_COLOR_CODE) {
        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_START_LENGTH, &newbits, Buffer);
		Decode_Color.BitMapBits = 1;

		for (i = 0; i < 2; i++) {
		    Decode_Color.Index[i] = ((codebuf>>29)&(long)VQ_INDEX_MASK);
		    if (((codebuf>>31)&(long)VQ_HEADER_MASK)==VQ_NO_UPDATE_HEADER) {
                updatereadbuf (&codebuf, &newbuf, VQ_NO_UPDATE_LENGTH, &newbits, Buffer);
			}
		    else {
			    Decode_Color.Color[Decode_Color.Index[i]] = ((codebuf>>5)&(long)VQ_COLOR_MASK);
                updatereadbuf (&codebuf, &newbuf, VQ_UPDATE_LENGTH, &newbits, Buffer);
			}
		}
        VQ_Decompress (txb, tyb, (char *)OutBuffer, 0, &Decode_Color);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==VQ_SKIP_2_COLOR_CODE) {
        txb = (codebuf & 0x0FF00000) >> 20;
        tyb = (codebuf & 0x0FF000) >> 12;

        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_SKIP_LENGTH, &newbits, Buffer);
		Decode_Color.BitMapBits = 1;

		for (i = 0; i < 2; i++) {
			Decode_Color.Index[i] = ((codebuf>>29)&(long)VQ_INDEX_MASK);
		    if (((codebuf>>31)&(long)VQ_HEADER_MASK)==VQ_NO_UPDATE_HEADER) {
                updatereadbuf (&codebuf, &newbuf, VQ_NO_UPDATE_LENGTH, &newbits, Buffer);
			}
		    else {
			    Decode_Color.Color[Decode_Color.Index[i]] = ((codebuf>>5)&(long)VQ_COLOR_MASK);
                updatereadbuf (&codebuf, &newbuf, VQ_UPDATE_LENGTH, &newbits, Buffer);
			}
		}
        VQ_Decompress (txb, tyb, (char *)OutBuffer, 0, &Decode_Color);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==VQ_NO_SKIP_4_COLOR_CODE) {
        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_START_LENGTH, &newbits, Buffer);
		Decode_Color.BitMapBits = 2;

		for (i = 0; i < 4; i++) {
		    Decode_Color.Index[i] = ((codebuf>>29)&(long)VQ_INDEX_MASK);
		    if (((codebuf>>31)&(long)VQ_HEADER_MASK)==VQ_NO_UPDATE_HEADER) {
                updatereadbuf (&codebuf, &newbuf, VQ_NO_UPDATE_LENGTH, &newbits, Buffer);
			}
		    else {
			    Decode_Color.Color[Decode_Color.Index[i]] = ((codebuf>>5)&(long)VQ_COLOR_MASK);
                updatereadbuf (&codebuf, &newbuf, VQ_UPDATE_LENGTH, &newbits, Buffer);
			}
		}
        VQ_Decompress (txb, tyb, (char *)OutBuffer, 0, &Decode_Color);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==VQ_SKIP_4_COLOR_CODE) {
        txb = (codebuf & 0x0FF00000) >> 20;
        tyb = (codebuf & 0x0FF000) >> 12;

        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_SKIP_LENGTH, &newbits, Buffer);
		Decode_Color.BitMapBits = 2;

		for (i = 0; i < 4; i++) {
			Decode_Color.Index[i] = ((codebuf>>29)&(long)VQ_INDEX_MASK);
		    if (((codebuf>>31)&(long)VQ_HEADER_MASK)==VQ_NO_UPDATE_HEADER) {
                updatereadbuf (&codebuf, &newbuf, VQ_NO_UPDATE_LENGTH, &newbits, Buffer);
			}
		    else {
			    Decode_Color.Color[Decode_Color.Index[i]] = ((codebuf>>5)&(long)VQ_COLOR_MASK);
                updatereadbuf (&codebuf, &newbuf, VQ_UPDATE_LENGTH, &newbits, Buffer);
			}
		}
        VQ_Decompress (txb, tyb, (char *)OutBuffer, 0, &Decode_Color);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==JPEG_PASS2_CODE) {
        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_START_LENGTH, &newbits, Buffer);
        Decompress_2PASS (txb, tyb, (char *)OutBuffer, 2);
        MoveBlockIndex ();
      }
      else if (((codebuf>>28)&(long)BLOCK_HEADER_MASK)==JPEG_SKIP_PASS2_CODE) {
        txb = (codebuf & 0x0FF00000) >> 20;
        tyb = (codebuf & 0x0FF000) >> 12;

        updatereadbuf (&codebuf, &newbuf, BLOCK_AST2100_SKIP_LENGTH, &newbits, Buffer);
        Decompress_2PASS (txb, tyb, (char *)OutBuffer, 2);
        MoveBlockIndex ();
      }
    } while (index <= StringLength);

//    free (bitmap);
//    free (OutBuffer);
//    free (Buffer);
//    freePhysicalMapping (ulLinearBaseAddress);

//    FreeQT ();
    return    TRUE;
}

