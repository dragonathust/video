
#define I8    signed char
#define U8  unsigned char     /* unsigned 8  bits. */
#define I16   signed short    /*   signed 16 bits. */
#define U16 unsigned short    /* unsigned 16 bits. */
#define I32   signed long   /*   signed 32 bits. */
#define U32 unsigned long   /* unsigned 32 bits. */
#define I16P I16              /*   signed 16 bits OR MORE ! */
#define U16P U16              /* unsigned 16 bits OR MORE ! */

typedef struct {
  int XSize;
  int YSize;
} GUI_BMP_INFO;

typedef struct {
  int bfSize;
  int bfReserved;
  int bfOffBits;
  int biHeaderSize;
  int biWidth;
  int biHeight;
  short biPlanes;
  short biBitCount;
  int biCompression;
  int biSizeImage;
  int biXPelsPerMeter;
  int biYPelsPerMeter;
  int biClrUsed;
  int biClrImportant;
} GUI_BMP_HEADER;
 

/*********************************************************************
*
*       GUI_BMP_GetInfo
*/
static int GUI_BMP_GetInfo(const void * pFileData, int DataSize, GUI_BMP_INFO* pInfo) {

	GUI_BMP_HEADER header;
	
	if( strncmp((const char*)pFileData,"BM",2)!=0 )
	{
		mprintf("Not a bmp file.\n");
		return -1;
	}
	
	memcpy(&header,((unsigned char*)pFileData)+2,sizeof(GUI_BMP_HEADER));
	
  if (pInfo) {
    pInfo->XSize =header.biWidth;
    pInfo->YSize =header.biHeight;
  }
  return 0;
}

/*********************************************************************
*
*       GUI_BMP_Draw
*/

static int GUI_BMP_Draw(const void * pFileData, int DataSize, int x0, int y0,void *dest) {
	
	GUI_BMP_HEADER header;
	
	if( strncmp((const char*)pFileData,"BM",2)!=0 )
	{
		mprintf("Not a bmp file.\n");
		return -1;
	}
	
	memcpy(&header,((unsigned char*)pFileData)+2,sizeof(GUI_BMP_HEADER));

	if( (header.biBitCount !=24)||(header.biCompression!=0) )
	{
		mprintf("Bmp file format not support.\n");
		return -1;		
	}

#if 1
	memcpy(dest,((unsigned char*)pFileData)+2+sizeof(GUI_BMP_HEADER),header.biWidth*header.biHeight*3);
#else
	int i;
	unsigned char *psrc;
	psrc=((unsigned char*)pFileData)+2+sizeof(GUI_BMP_HEADER);
	for(i=0;i<header.biHeight;i++)
	{
		memcpy((unsigned char*)dest+i*header.biWidth*3,psrc+(header.biHeight-i-1)*header.biWidth*3,header.biWidth*3);
	}
#endif
	
	return 0;
}

static unsigned char* GUI_BMP_GetPixel(const void * pFileData, int DataSize) {
	
	GUI_BMP_HEADER header;
	
	if( strncmp((const char*)pFileData,"BM",2)!=0 )
	{
		mprintf("Not a bmp file.\n");
		return NULL;
	}
	
	memcpy(&header,((unsigned char*)pFileData)+2,sizeof(GUI_BMP_HEADER));

	if( (header.biBitCount !=24)||(header.biCompression!=0) )
	{
		mprintf("Bmp file format not support.\n");
		return NULL;		
	}

	return (unsigned char*)(((unsigned char*)pFileData)+2+sizeof(GUI_BMP_HEADER));

}

static unsigned char *read_file_to_mem(char* fname,int *file_length)
{
	FILE *fd_in;
	
	unsigned char *buffer_in;
	int length;
	
    fd_in=fopen(fname,"rb");
	if(fd_in <=0)
	{
    		mprintf("Can't open file %s.\n",fname);
    		return NULL;
	}
	
	//mprintf("open file [%s] ok!\n",fname);
	
	fseek(fd_in,0,SEEK_END);
	length=ftell(fd_in);
	//mprintf("size=%d\n",length);	

	fseek(fd_in,0,SEEK_SET);
	
	buffer_in=(unsigned char*)malloc(length);

	if( !buffer_in )
	{
		mprintf("Out of memory.\n");
		fclose(fd_in);
		return NULL;
	}
	
	fread(buffer_in,length,1,fd_in);
	fclose(fd_in);
	*file_length=length;
	return buffer_in;
}

static void dump_mem_to_file(unsigned char *buf,int len,char *fname)
{
	FILE *file_out;	

	file_out=fopen(fname,"wb+");
	if( file_out == NULL )
	{
    		printf("Can't open file %s.\n",fname);
    		return ;
	}
	
	printf("Dump[0x%8x] size[%d] to file %s.\n",buf,len,fname);
		
	fwrite(buf,len,1,file_out);
		
	fclose(file_out);
}

static unsigned char *capture_one_frame(char *fname,int *length)
{
//  int out_length;
//  GUI_BMP_INFO Info;
	

//  GUI_BMP_GetInfo(buffer_in0, file_length0, &Info);
//	out_length=Info.XSize*Info.YSize*3;
	return 0;
}


