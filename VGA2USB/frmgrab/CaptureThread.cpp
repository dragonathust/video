

/*****************************************************************/
#include "avifmt.h"

#define SWAP2(x) (((x>>8) & 0x00ff) |\
                  ((x<<8) & 0xff00))


#define SWAP4(x) (((x>>24) & 0x000000ff) |\
                  ((x>>8)  & 0x0000ff00) |\
                  ((x<<8)  & 0x00ff0000) |\
                  ((x<<24) & 0xff000000))

# define LILEND2(a) (a)
# define LILEND4(a) (a)
# define BIGEND2(a) SWAP2((a))
# define BIGEND4(a) SWAP4((a))


//#define STR_NO_SIGNAL _T("No signal detected")
#define STR_NO_SIGNAL _T("无输入信号")

//#define STR_TUNING_PARAMETER _T("Tuning parameter...")
#define STR_TUNING_PARAMETER _T("调整中...")

#define DISCARD_FRAME_NUMBER 3
#define FAILED_FRAME_NUMBER 8

void ChangeWindowSize(int width, int height,int vfreq);
void ViewDisplayInfo(char * pInfo);
void UpdateDisplayFrame(void * pBuffer,int buffer_len);
void SetCaptureRunStatus(int status);
void SetStatusInfo(HWND hHwnd,int id,char * info );
void GetVideoConfigInfo();

UINT CaptureThread(LPVOID p);

int GetParameters(FrmGrabber* fg);
int UpdateParameters(FrmGrabber* fg);

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
#include "llist.c"

typedef struct _Jpeg_Data Jpeg_Data;

struct _Jpeg_Data
{
  DWORD size;
  DWORD offset;
};

typedef struct _MJPEG_WR{
	List *frlst;
	DWORD width;
	DWORD height;
	DWORD jpg_sz;
}MJPEG_WR;

struct avi_writer{
	AW_CTX* aw;
	MJPEG_WR *mjpeg_aw;
	FILE * out;
	V2U_UINT8* jpg_buffer;
	V2U_UINT32 jpg_size;
	V2U_TIME startTime ;
	V2U_TIME pauseTimeTotal ;
	V2U_TIME pauseTime ;
	V2U_UINT32 frames;
};

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

void V2U_ListCards(void)
{
v2u_list_cards();
}

/***************************************************************/

struct avi_writer * v2u_prepare_raw_avi(const char* fname,
    const V2U_VideoMode* vm,V2U_UINT32 format)
{
    FILE * out;
    AW_CTX* aw = NULL;

    struct avi_writer *p_writer;
	
    const V2UCaptureFormatInfo* formatInfo = v2u_get_format_info(format);

    if (formatInfo && formatInfo->fourcc) {
        out = fopen(fname, "wb");
        if (out) {
            //AW_CTX* aw = NULL;
            if (AW_FInit(&aw, out, vm->width, vm->height) == AW_OK) {
               int bpp = V2UPALETTE_2_BPP(format);
                if (V2U_GRABFRAME_FORMAT(format) ==
                    V2U_GRABFRAME_FORMAT_BGR16) {
                    /* Special case - see AW_AddVideoStream in aw_writer.c */
                    bpp = -16;
                }
                if (AW_AddVideoStream(aw, formatInfo->fourcc, bpp) == AW_OK &&
                    AW_StartStreamsData(aw) == AW_OK) {
                    	p_writer=(struct avi_writer*)malloc(sizeof(struct avi_writer));
			if(!p_writer)
				{
				AW_FDone(aw);
				fclose(out);	
				return NULL;
				}
			p_writer->aw=aw;
			p_writer->out=out;
			p_writer->startTime = v2u_time();
			p_writer->frames=0;
			 //v2u_println("initializing avi stream ok.\n");
			return p_writer;	
                	}
		else {
                    v2u_println("error initializing avi stream");
                }

                AW_FDone(aw);		
				
            	}
		else{
                v2u_println("error initializing avi output");
            }

            fclose(out);	
        	}
	else {
            v2u_println("failed to open %s",fname);
        }
 	}
	else {
        v2u_println("AVI recording is not available for %s format",
            formatInfo ? formatInfo->opt : "this");
 		}	

	return NULL;
}

void v2u_addframe_raw_avi(struct avi_writer * p_writer,V2U_GrabFrame2* frame,HWND hHwnd)
{
	static CString strStatusInfo;

	//v2u_println("%d frames\n",frames);	
	
                        if (AW_AddFrame(p_writer->aw, 0, (unsigned char*)frame->pixbuf, frame->imagelen,
                            (frame->palette & V2U_GRABFRAME_KEYFRAME_FLAG) != 0)
                            != AW_OK) {
                            v2u_println("error writing frame");
                            return;
                        }
	p_writer->frames++;	

         V2U_TIME  stopTime;
         stopTime = v2u_time();
         int sec = (int)((stopTime - p_writer->startTime)/1000.0);

	strStatusInfo.Format(" (%d sec,%d frames)",sec,p_writer->frames);
	strStatusInfo=VideoFileName+strStatusInfo;
	SetStatusInfo(hHwnd,0,strStatusInfo.GetBuffer(strStatusInfo.GetLength()));
}

void v2u_finish_raw_avi(struct avi_writer * p_writer)
{
                    const V2U_UINT32 scale = 100;
                    V2U_UINT32  rate = 0;
                    V2U_TIME  stopTime;
					
                    if (p_writer->frames > 0) {
                        double sec;

                        /* time calc and header update */
                        stopTime = v2u_time();
                        sec = (stopTime - p_writer->startTime)/1000.0;

                        /* calculate frame rate */
                        if (stopTime != p_writer->startTime) {
                            rate = (V2U_UINT32)(p_writer->frames * scale / sec);
                        }

                        AW_UpdateStreamHeaderField(p_writer->aw, 0, AW_SHDR_Scale, scale);
                        AW_UpdateStreamHeaderField(p_writer->aw, 0, AW_SHDR_Rate, rate);

                        v2u_println("frames captured: %d\n",p_writer->frames);
                        v2u_println("recording time:  %.2f sec\n",sec);
                        v2u_println("frame rate:      %.2f fps\n",p_writer->frames/sec);
                    }
						
            AW_FDone(p_writer->aw);
            fclose(p_writer->out);

	   free(p_writer);

}

/***************************************************************/
//#define AVI_TEST

#ifdef AVI_TEST
void v2u_copy_jpeg(FILE *file_out);
void v2u_copy_jpeg_mem(void* out, int out_len,int *actual_len);
#endif

void fprint_quartet(unsigned int i,FILE * file)
{
	fwrite(&i,4,1,file);
}

int write_avi_header(FILE * file_out,
  DWORD width,
  DWORD height,
  DWORD per_usec,
  DWORD frames,
  DWORD riff_sz,
  DWORD jpg_sz)
{
  struct AVI_list_hdrl hdrl = {
    /* header */
    {
      {'L', 'I', 'S', 'T'},
      LILEND4(sizeof(struct AVI_list_hdrl) - 8),
      {'h', 'd', 'r', 'l'}
    },

    /* chunk avih */
    {'a', 'v', 'i', 'h'},
    LILEND4(sizeof(struct AVI_avih)),
    {
      LILEND4(per_usec),
      LILEND4(1000000 * (jpg_sz/frames) / per_usec),
      LILEND4(0),
      LILEND4(AVIF_HASINDEX),
      LILEND4(frames),
      LILEND4(0),
      LILEND4(1),
      LILEND4(0),
      LILEND4(width),
      LILEND4(height),
      {LILEND4(0), LILEND4(0), LILEND4(0), LILEND4(0)}
    },

    /* list strl */
    {
      {
	{'L', 'I', 'S', 'T'},
	LILEND4(sizeof(struct AVI_list_strl) - 8),
	{'s', 't', 'r', 'l'}
      },

      /* chunk strh */
      {'s', 't', 'r', 'h'},
      LILEND4(sizeof(struct AVI_strh)),
      {
	{'v', 'i', 'd', 's'},
	{'M', 'J', 'P', 'G'},
	LILEND4(0),
	LILEND4(0),
	LILEND4(0),
	LILEND4(per_usec),
	LILEND4(1000000),
	LILEND4(0),
	LILEND4(frames),
	LILEND4(0),
	LILEND4(0),
	LILEND4(0)
      },
      
      /* chunk strf */
      {'s', 't', 'r', 'f'},
      sizeof(struct AVI_strf),
      {      
	LILEND4(sizeof(struct AVI_strf)),
	LILEND4(width),
	LILEND4(height),
	LILEND4(1 + 24*256*256),
	{'M', 'J', 'P', 'G'},
	LILEND4(width * height * 3),
	LILEND4(0),
	LILEND4(0),
	LILEND4(0),
	LILEND4(0)
      },

      /* list odml */
      {
	{
	  {'L', 'I', 'S', 'T'},
	  LILEND4(16),
	  {'o', 'd', 'm', 'l'}
	},
	{'d', 'm', 'l', 'h'},
	LILEND4(4),
	LILEND4(frames)
      }
    }
  };
  
  /* printing AVI riff hdr */
  fprintf(file_out,"RIFF");
  fprint_quartet(riff_sz,file_out);//fill later
  fprintf(file_out,"AVI ");

  /* list hdrl */
  hdrl.avih.us_per_frame = LILEND4(per_usec);
  hdrl.avih.max_bytes_per_sec = LILEND4(1000000 * (jpg_sz/frames)
				      / per_usec);
  hdrl.avih.tot_frames = LILEND4(frames);
  hdrl.avih.width = LILEND4(width);
  hdrl.avih.height = LILEND4(height);
  hdrl.strl.strh.scale = LILEND4(per_usec);
  hdrl.strl.strh.rate = LILEND4(1000000);
  hdrl.strl.strh.length = LILEND4(frames);
  hdrl.strl.strf.width = LILEND4(width);
  hdrl.strl.strf.height = LILEND4(height);
  hdrl.strl.strf.image_sz = LILEND4(width * height * 3);
  hdrl.strl.list_odml.frames = LILEND4(frames);	/*  */
  fwrite(&hdrl, sizeof(hdrl), 1, file_out);//fill later

  /* list movi */
  fprintf(file_out,"LIST");
  fprint_quartet(jpg_sz + 8*frames + 4,file_out);//fill later
  fprintf(file_out,"movi");
  
  return 0;
}

struct avi_writer * v2u_prepare_mjpeg_avi(const char* fname,
    const V2U_VideoMode* vm,V2U_UINT32 format)
{
  DWORD width=vm->width;
  DWORD height=vm->height;
  DWORD per_usec = 100;
  DWORD frames = 1;
  DWORD riff_sz=1;
  DWORD jpg_sz = 1;
  
  FILE *file_out;
  MJPEG_WR * mjpeg_aw;  
  V2U_UINT8 *jpg_buffer;
  V2U_UINT32 jpg_size;
  
  struct avi_writer *p_writer;

    file_out = fopen(fname, "wb");
	if( file_out == NULL )
	{
    		v2u_println("Can't open file %s.\n",fname);
    		return NULL;
	}

	jpg_size = width*height*4;
	jpg_buffer = (V2U_UINT8 *)malloc(jpg_size);
	if(!jpg_buffer)
	{
		fclose(file_out);
		return NULL;
	}
	
	mjpeg_aw = (MJPEG_WR*)malloc(sizeof(MJPEG_WR));
	if( !mjpeg_aw )
	{	
		fclose(file_out);
		free(jpg_buffer);
		return NULL;
	}
	
	p_writer = (struct avi_writer*)malloc(sizeof(struct avi_writer));
	if( !p_writer )
	{	
		fclose(file_out);
		free(jpg_buffer);
		free(mjpeg_aw);
		return NULL;
	}
	
  write_avi_header(file_out,  
   width,
   height,
   per_usec,
   frames,
   riff_sz,
   jpg_sz);

  mjpeg_aw->frlst = NULL;   
  mjpeg_aw->width = width;
  mjpeg_aw->height = height;  
  mjpeg_aw->jpg_sz=0;
  
  p_writer->startTime = v2u_time();
  p_writer->frames=0;

  p_writer->pauseTime=0;
  p_writer->pauseTimeTotal=0;

  p_writer->mjpeg_aw = mjpeg_aw;
  p_writer->out = file_out;
  p_writer->jpg_buffer = jpg_buffer;
  p_writer->jpg_size = jpg_size;	
	return p_writer;
}

void v2u_addframe_mjpeg_avi(struct avi_writer * p_writer,V2U_GrabFrame2* frame,HWND hHwnd)
{
  static CString strStatusInfo;
	
  FILE *file_out = p_writer->out;
  unsigned int mfsz, remnant;
//  unsigned int start,end;
//  unsigned char buff[4];

  List *frlst = p_writer->mjpeg_aw->frlst;
  List *f = NULL;
  Jpeg_Data *tmp;

  if(VideoPause)
  {
  	if(!p_writer->pauseTime)
  	{
  	p_writer->pauseTime = v2u_time();
	
	strStatusInfo.Format(_T(" (已暂停)"));
	strStatusInfo=VideoFileName+strStatusInfo;
	SetStatusInfo(hHwnd,0,strStatusInfo.GetBuffer(strStatusInfo.GetLength()));	
  	}
  	return;
  }

  if( ((unsigned int)p_writer->mjpeg_aw->jpg_sz) >= (2047*1024*1024) ) /* 2 GB limit */
  return;
  
  tmp = (Jpeg_Data *) malloc(sizeof(Jpeg_Data));
  if(!tmp) return;
  
	if( !frlst )
	{
	frlst = (List *) malloc(sizeof(List));
		if( !frlst )
		{
			free(tmp);
			return;
		}
	p_writer->mjpeg_aw->frlst=frlst;

	frlst->data = 0;
	frlst->prev = NULL;
	frlst->next = NULL;
	}
	else
	{
	f = list_back(frlst);
	}

#if 0
    fprintf(file_out,"00db");
    fprint_quartet(0,file_out); //fill later
	
	start = ftell(file_out);

#ifdef AVI_TEST	
	v2u_copy_jpeg(file_out);
#else
	v2u_write_jpeg(file_out, frame->crop.width, frame->crop.height,
                                frame->palette, frame->pixbuf);
#endif

	end = ftell(file_out);
	
    mfsz = end-start;
	v2u_println("start=%d,end=%d,mfsz=%d\n",start,end,mfsz);	
    remnant = (4-(mfsz%4)) % 4;								
	tmp->size = mfsz+remnant;

	if (remnant > 0) {
      fwrite(buff, remnant, 1, file_out);
    }

	fseek(file_out,start-4,SEEK_SET);
	fprint_quartet(tmp->size,file_out); //fill size
	fseek(file_out,6,SEEK_CUR);
	fwrite("AVI1", 4, 1, file_out);
	fseek(file_out,0,SEEK_END);
#else

#ifdef AVI_TEST
	v2u_copy_jpeg_mem(p_writer->jpg_buffer+8, p_writer->jpg_size-8,(int*)&mfsz);
#else
	v2u_write_jpeg_mem(p_writer->jpg_buffer+8, p_writer->jpg_size-8,(int*)&mfsz,
	frame->crop.width, frame->crop.height,frame->palette, frame->pixbuf,85);
#endif

	v2u_println("mfsz=%d\n",mfsz);
	remnant = (4-(mfsz%4)) % 4;
	tmp->size = mfsz+remnant;

	//fprintf(file_out,"00db");
	//fprint_quartet(tmp->size,file_out); //fill size
	memcpy(p_writer->jpg_buffer,"00db",4);
	memcpy(p_writer->jpg_buffer+4,&tmp->size,4);
	memcpy(p_writer->jpg_buffer+8+6,"AVI1",4);	
	fwrite(p_writer->jpg_buffer,tmp->size+8,1, file_out);
#endif

    if (!f) {
      tmp->offset = 4;

    } else {
      tmp->offset = 
	((Jpeg_Data *) f->data)->offset +
	((Jpeg_Data *) f->data)->size + 8;
    }
	
	p_writer->mjpeg_aw->jpg_sz += tmp->size;
	
    if (!f)
	{
		frlst->data = tmp;
	}
	else
	{
	list_push_back(frlst,tmp);
	}
	
	p_writer->frames++;

         V2U_TIME  stopTime;
		 
         stopTime = v2u_time();
         int sec = (int)((stopTime - p_writer->startTime)/1000.0);

  if(p_writer->pauseTime)
  {
	p_writer->pauseTimeTotal += stopTime-p_writer->pauseTime;
	p_writer->pauseTime=0;
  }
  
	strStatusInfo.Format(" (%d sec,%d frames)",sec,p_writer->frames);
	strStatusInfo=VideoFileName+strStatusInfo;
	SetStatusInfo(hHwnd,0,strStatusInfo.GetBuffer(strStatusInfo.GetLength()));	
}

void v2u_finish_mjpeg_avi(struct avi_writer * p_writer)
{
  FILE *file_out = p_writer->out;
  DWORD frames = p_writer->frames;
  DWORD width = p_writer->mjpeg_aw->width;
  DWORD height = p_writer->mjpeg_aw->height;  
  V2U_TIME  stopTime;
  DWORD per_usec;
  double sec;

  DWORD riff_sz;
  DWORD jpg_sz =p_writer->mjpeg_aw->jpg_sz;
  
  List *frlst = p_writer->mjpeg_aw->frlst;
  List *f = NULL;

 if(frames > 0)
 {
  /* indices */
  fprintf(file_out,"idx1");
  fprint_quartet(16 * frames,file_out);
  
  for (f = frlst; (f); f = f->next) {
    fprintf(file_out,"00db");
    fprint_quartet(18,file_out);
    fprint_quartet(((Jpeg_Data *) f->data)->offset,file_out);
    fprint_quartet(((Jpeg_Data *) f->data)->size,file_out);
  }

  /* time calc and header update */
  stopTime = v2u_time();
  sec = (stopTime - p_writer->startTime-p_writer->pauseTimeTotal)/1000.0;
  //per_usec=1000*(stopTime - p_writer->startTime)/frames;
  per_usec=(DWORD)(1000000.0*sec/frames);

  riff_sz = sizeof(struct AVI_list_hdrl) + 4 + 4 + jpg_sz
    + 8*frames + 8 + 8 + 16*frames;
	
  fseek(file_out,0,SEEK_SET);
  write_avi_header(file_out,  
   width,
   height,
   per_usec,
   frames,
   riff_sz,
   jpg_sz);
 
 } 
	fclose(p_writer->out);  
	free(p_writer->mjpeg_aw);
	free(p_writer->jpg_buffer);
	free(p_writer);
	list_rerased(frlst);

   v2u_println("frames captured: %d\n",frames);
   v2u_println("recording time:  %.2f sec\n",sec);
   v2u_println("frame rate:      %.2f fps\n",frames/sec);
						
}

/***************************************************************/

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

#if 0
#define v2u_prepare_avi v2u_prepare_raw_avi
#define v2u_addframe_avi v2u_addframe_raw_avi
#define v2u_finish_avi v2u_finish_raw_avi
#else
#define v2u_prepare_avi v2u_prepare_mjpeg_avi
#define v2u_addframe_avi v2u_addframe_mjpeg_avi
#define v2u_finish_avi v2u_finish_mjpeg_avi
#endif

UINT CaptureThread(LPVOID p)
{
	HWND hHwnd = (HWND)p;
	
	int FrameWidth,FrameHeight;

	V2U_VideoMode vm;
	V2U_GrabFrame2* f ;
	int SignalOK=0;
	int GrabFailedCnt;
	int discard_frm=0;

	struct avi_writer * p_avi_writer=NULL;

	FrmGrabber* fg = NULL;			
	fg = FrmGrabLocal_Open();
	if(!fg)
	{
		return 1;
	}

			//Open ok,Start Signal monitor
			CaptureDeviceExist=1;
			SaveSnapshot=0;
			VideoRecord=0;
			VideoPause=0;
			m_UpdateParameters=0;

			//GetParameters(fg);
			GetVideoConfigInfo();
			if(m_UpdateParameters)
			{
				UpdateParameters(fg);
				m_UpdateParameters=0;
			}
			
SIGNAL_DETECT:	
	while(CaptureDeviceExist)
	{
    		if (FrmGrab_DetectVideoMode(fg, &vm)) {
				if( (vm.width!=0)&&(vm.height!=0) )
				{
				ChangeWindowSize(vm.width,vm.height,vm.vfreq);
				SetFrameSize(vm.width,vm.height);
				FrameWidth=vm.width;
				FrameHeight=vm.height;
				SignalOK=1;
				discard_frm=DISCARD_FRAME_NUMBER;				
				break;
				}
				else
				ViewDisplayInfo(STR_NO_SIGNAL);
			}	
		else
		{
		ViewDisplayInfo(STR_NO_SIGNAL);
		}
		if(m_UpdateParameters)
		{
			UpdateParameters(fg);
			m_UpdateParameters=0;
		}
		::Sleep(100);
	}

	if(!SignalOK)
	{
		FrmGrab_Close(fg);
		return 1;
	}
//Signal ok,capture start
	ViewDisplayInfo(STR_TUNING_PARAMETER);

	FrmGrab_Start(fg);

	//SetCaptureRunStatus(1);	

	while(CaptureDeviceExist)
		{
		//f = FrmGrab_Frame(fg, V2U_GRABFRAME_FORMAT_BGR16, NULL);
		f = FrmGrab_Frame(fg, V2U_GRABFRAME_FORMAT_BGR24|V2U_GRABFRAME_BOTTOM_UP_FLAG, NULL);
		//f = FrmGrab_Frame(fg, V2U_GRABFRAME_FORMAT_ARGB32, NULL);
		
               if (f) {
			   	GrabFailedCnt=0;
				if( (int)f->imagelen != FrameWidth*FrameHeight*3 )
					{

					v2u_println("info: f->imagelen =%d\n",f->imagelen );

		   			if (FrmGrab_DetectVideoMode(fg, &vm)) {
						if( (vm.width!=0)&&(vm.height!=0) )
						{
							if( (vm.width !=FrameWidth)||(vm.height!=FrameHeight) )
								{
									ChangeWindowSize(vm.width,vm.height,vm.vfreq);
									SetFrameSize(vm.width,vm.height);
									FrameWidth=vm.width;
									FrameHeight=vm.height;
									discard_frm=DISCARD_FRAME_NUMBER;
									SetCaptureRunStatus(0);
									ViewDisplayInfo(STR_TUNING_PARAMETER);
								}
						}
						}
					}
			if(discard_frm)
			{
			discard_frm--;
			if(discard_frm==0)
			SetCaptureRunStatus(1);
			}
			else
			{
			if( (int)f->imagelen == FrameWidth*FrameHeight*3 )
			{
				UpdateDisplayFrame( (void*)f->pixbuf, f->imagelen);
				CopyFrameToNetserver((unsigned char*)f->pixbuf, f->imagelen);
				if(SaveSnapshot)
				{
				v2u_save_frame(f,SnapshotFilename.GetBuffer(SnapshotFilename.GetLength()));
				SaveSnapshot=0;
				}
			}
			}
				if( VideoRecord )
					{
						if( !p_avi_writer )
						{
 						p_avi_writer=v2u_prepare_avi(VideoFileName.GetBuffer(VideoFileName.GetLength()),
						&vm,V2U_GRABFRAME_FORMAT_BGR24) ;
						}
						
						if(p_avi_writer)
						v2u_addframe_avi(p_avi_writer,f,hHwnd);
					}
					else
					{
						if(p_avi_writer)
							{
							v2u_finish_avi(p_avi_writer);
							p_avi_writer=NULL;
							}
					}
					
                                FrmGrab_Release(fg, f);

                          } 
			   else {
                                v2u_println("failed to grab frame\n");
                          		GrabFailedCnt++;
				if( GrabFailedCnt>FAILED_FRAME_NUMBER )	
					{
						//Signal lost
						SetCaptureRunStatus(0);	
						FrmGrab_Stop(fg);
						SignalOK=0;
						ViewDisplayInfo(STR_NO_SIGNAL);
						break;
					}
                            }
			if(m_UpdateParameters)
			{
				UpdateParameters(fg);
				m_UpdateParameters=0;
			}				
		}

	if(!SignalOK)
	{
	goto SIGNAL_DETECT;
	}
	
	SetCaptureRunStatus(0);	

	FrmGrab_Stop(fg);
	
	FrmGrab_Close(fg);

	return 0;
}

/**************************************************************************/
#ifdef AVI_TEST
void v2u_copy_jpeg(FILE *file_out)
{
	FILE * fd;
	char buff[512];
	int nbr;
	
	fd=fopen("1.jpg", "rb");

	if(!fd)
	{
		v2u_println("Jpeg file not found.\n");
		fwrite(buff, 512, 1, file_out);
		return;
	}
	
	while ((nbr = fread(buff, 1, 512,fd)) > 0) {
      fwrite(buff, nbr, 1, file_out);
    }
	
	fclose(fd);
}

void v2u_copy_jpeg_mem(void* out, int out_len,int *actual_len)
{	
	FILE * fd;
	char buff[512];	
	int nbr;	
	int offset=0;		

	fd=fopen("1.jpg", "rb");
	
	if(!fd)	
	{		
	v2u_println("Jpeg file not found.\n");
	memcpy(out,buff, 512);
	return;
	}

	while ((nbr = fread(buff, 1, 512,fd)) > 0) 
	{
	memcpy((unsigned char*)out+offset,buff, nbr);
	offset +=nbr;
	}
	
	*actual_len=offset;

	fclose(fd);
}

UINT AVITestThread(LPVOID p)
{
	HWND hHwnd = (HWND)p;
	struct avi_writer * p_avi_writer=NULL;
	V2U_VideoMode vm;
	int i;
	
	v2u_println("AVI Test.\n");

	vm.width=800;
	vm.height=536;
	
	p_avi_writer=v2u_prepare_avi("1.avi",
						&vm,0) ;
	
	if(p_avi_writer)
	{
	  for(i=0;i<10;i++)
	  {
		v2u_addframe_avi(p_avi_writer,NULL,hHwnd);	
		::Sleep(200);
	  }
	}
	
	if(p_avi_writer)
	{
	v2u_finish_avi(p_avi_writer);
	}
	
	return 0;
}
#endif

void V2U_AVI_Test_Start(HWND hHwnd)
{
#ifdef AVI_TEST
	::AfxBeginThread(AVITestThread,(LPVOID)hHwnd);
#endif
}

int GetParameters(FrmGrabber* fg)
{
	int ret;
	V2U_GrabParameters gp;
    memset(&gp, 0, sizeof(gp));

	ret=FrmGrab_GetGrabParams(fg, &gp);

	m_gain_red = gp.gain_r;
	m_gain_green = gp.gain_g;
	m_gain_blue = gp.gain_b;
	
	m_offset_red = gp.offset_r;
	m_offset_green = gp.offset_g;
	m_offset_blue = gp.offset_b;

	return ret;
}

int UpdateParameters(FrmGrabber* fg)
{
    V2U_GrabParameters gp;
    memset(&gp, 0, sizeof(gp));

    gp.flags |= V2U_FLAG_VALID_OFFSETGAIN;

	gp.gain_r = m_gain_red;
	gp.gain_g = m_gain_green;
	gp.gain_b = m_gain_blue;
	
	gp.offset_r = m_offset_red;
	gp.offset_g = m_offset_green;
	gp.offset_b = m_offset_blue;

	v2u_println("gain   (set): R:%d G:%d B:%d\n",gp.gain_r,gp.gain_g,gp.gain_b);
	v2u_println("offset (set): R:%d G:%d B:%d\n",gp.offset_r,gp.offset_g,gp.offset_b);

	return FrmGrab_SetGrabParams(fg, &gp);
}

int V2U_Set_Params(int gain_red, int gain_green,int gain_blue,
			int offset_red,int offset_green,int offset_blue)
{
	int ret=0;

	m_gain_red = 255 - gain_blue;
	m_gain_green = 255 - gain_green;
	m_gain_blue = 255 - gain_red;
	
	m_offset_red = 63 - offset_blue;
	m_offset_green = 63 - offset_green;
	m_offset_blue = 63 - offset_red;
	
	m_UpdateParameters =1;

#if 0	
	FrmGrabber* fg = NULL;			

	fg = FrmGrabLocal_Open();
	if(!fg)
	{
		v2u_println("Open local frame grabber failed!\n");
		return -1;
	}
	
	ret = UpdateParameters(fg);
	
	if(ret)
	{
		v2u_println("Set parameters failed!\n");
	}
	
	FrmGrab_Close(fg);
#endif
	
	return ret;
}

/*****************************************************************/


