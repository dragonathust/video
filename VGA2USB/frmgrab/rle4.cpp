
#include "stdafx.h"

#include<stdlib.h>
#include<stdio.h>
#include <string.h>

void mprintf( char* format,... );

void to_rgb16(unsigned char *buf,int w,int h);
void from_rgb16(unsigned char *buf,int w,int h);
int rle4_read_mem(unsigned char *rle_file_buf,int rle_file_len,unsigned char* src_buf,unsigned char *pbuf);
int rle4_write_mem(unsigned char *dest, int w, int h, unsigned char *buf,int *rle_length,unsigned char *p_source);

#define RLE_HEAD_LEN 28

/*
无损视频压缩算法

1 帧间压缩算法:

连续采集4帧,f0 f1 f2 f3

f0做帧内压缩,剩下3帧分别与前一帧做逐点异或运算,将得到的差文件做帧内压缩。
如果后一帧与前一帧图像相同的部分很多，会产生大量的0。

f3 ^=f2
f2 ^=f1
f1 ^=f0


解压时顺序相反,先解压4帧图像数据,然后剩下3帧分别与前一帧做逐点异或运算,即可还原为原始数据。

f1 ^=f0
f2 ^=f1
f3 ^=f2

1.1
可以改为自动决定I和P帧
I帧和P帧的判决
当前帧与前一帧做逐点异或运算，统计像素结果中为0个数大于等于1/8，
则使用帧间压缩，否则使用帧内压缩。

2 帧内压缩算法
将图像数据分块,将RGB分为4个数据块，每个数据块大小为width*height，数据单元为4bit

RGB[8:8:8]->R[高4bit] G[高4bit] B[高4bit] RGB[R低1bit : G低2bit :B低1bit]

将每个数据块用RLE4压缩

RLE4压缩:
数据和像素计数值均为4bit，压缩图像的细节部分可以减少空间占用。
少量的数据重复(1~45个)用1~3个字节存放。
大量的数据重复用命令1处理，3个字节，计数值扩展为46~4095，可以压缩大的色块。
支持绝对区，存放不适合压缩的原始数据。

-> Counter(1~15) -> Pixel(0~15) ->
	\/
	0  ->  0  -> stop
		\/
	   ->  1  -> CMD 1 ->Counter Byte[7~0] -> Counter[11~8] Pixel(0~15) ->
		\/ 
		Counter(3~15) -> Pixel(0~15) -> if Counter is odd padd zero ->

读入一个字节
	if( 字节高4bit != 0 )
	字节高4bit代表计数值(1~15)，低4bit代表像素(0~15)
	else
	{
		if(低4bit == 0) 结束；
	
		if(低4bit == 1)	命令1，表示后面2个字节的含义为
		( BYTE1，计数的低8位； BYTE2的高4位为计数的8~11位，计数范围46~4095,BYTE2的低4位为像素)
		
		if(低4bit == 2)	保留命令2;

		if(低4bit >= 3 ) 后面为绝对区，个数(4bit为一个单元)为低4bit的值，如果个数为奇数，最后4bit填0
	}
*/

/********************************************************************
RLE4

-> Counter(1~15) -> Pixel(0~15) ->
	\/
	0  ->  0  -> stop
		\/
	   ->  1  -> CMD 1 ->Counter Byte[7~0] -> Counter[11~8] Pixel(0~15) ->
		\/ 
		Counter(3~15) -> Pixel(0~15) -> if Counter is odd padd zero ->
********************************************************************/

static void DecompressStreamRLE4(unsigned char *pSrcBits,unsigned char *pRLEBits, int RLE_size,
					int SRC_size)
{
	int write_pos=0,read_pos,counter;
	unsigned char Cmd,Data,Cdata,data_to_be_write,Tdata;
	unsigned char *pPixel=pRLEBits;
	unsigned char *pWrite=pSrcBits;
		
	while(1)
	{
	Cdata = *pPixel++;
    Cmd  = (Cdata&0xf0)>>4;
    Data = (Cdata&0x0f);

		if( Cdata == 0x00 )
		break;
		
	    if (Cmd) {
			      while (Cmd--) {
					if(write_pos%2 == 0)
						{
						data_to_be_write = Data<<4;
					  	}
						else
						{
						data_to_be_write |= Data;
						*pWrite++=data_to_be_write;
							if(pWrite == (pSrcBits+SRC_size))
								break;
						}
					write_pos++;
				}
		}
		else
		{
			if( Data == 0x01 )	//CMD 0x01
			{	
				counter=*pPixel++;
				Data=*pPixel++;
				counter +=(Data&0xf0)*16;
				Data &=0x0f;
				while (counter--) {
					if(write_pos%2 == 0)
						{
						data_to_be_write = Data<<4;
					  	}
						else
						{
						data_to_be_write |= Data;
						*pWrite++=data_to_be_write;
							if(pWrite == (pSrcBits+SRC_size))
								break;
						}
					write_pos++;
				}
			}
			else
			{
			read_pos=0;
			while(Data--){
					Tdata=*(pPixel+read_pos/2);

						if(write_pos%2 == 0)
						{
							if(read_pos %2 == 0){
							data_to_be_write = Tdata&0xf0;	
							}
							else{
							data_to_be_write = (Tdata&0x0f)<<4;
							}						
					  	}
						else
						{
							if(read_pos %2 == 0){
							data_to_be_write |= (Tdata&0xf0)>>4;
							}
							else{
							data_to_be_write |= Tdata&0x0f;
							}
							*pWrite++=data_to_be_write;
							if(pWrite == (pSrcBits+SRC_size))
								break;
						}
					read_pos++;
					write_pos++;	
			}
			
			pPixel+=(read_pos+1)/2;
			
			}
		}
	}	
}

#define EndOfStream(pos,size)	((pos)==((size)-1))

static void _CompressStreamRLE4(unsigned char *pSrcBits,unsigned char *pRLEBits, int *RLE_size,
					int SRC_size)
{
   int src_index = 0, dst_index = 0, counter, i;
   int end_of;
   // in RLE8 every pixel is one byte

   int buffer_size=SRC_size;

state_start:  // just at the start of a block

      if ( EndOfStream(src_index,buffer_size)) // this is the last pixel of the line
      {
         pRLEBits[dst_index++] = 1; // block of length 1
         pRLEBits[dst_index++] = pSrcBits[src_index];

         src_index++;
         goto end_of_line;
      }

      // now the block length is at least 2, look ahead to decide next state
      // next two same pixels are the same, enter compress mode
      if (pSrcBits[src_index] == pSrcBits[src_index+1])
         goto state_compress;

      if ( EndOfStream(src_index+1,buffer_size)) // the last 2 pixel of the line
      {  // these 2 pixel are not the same
         pRLEBits[dst_index++] = 1; // block of length 1
         pRLEBits[dst_index++] = pSrcBits[src_index++];
         pRLEBits[dst_index++] = 1; // block of length 1
         pRLEBits[dst_index++] = pSrcBits[src_index++];
         goto end_of_line;
      }

      // now the block length is at least 3
      // in next 3 pixels, the last 2 consecutive pixel are the same
      if (pSrcBits[src_index+1] == pSrcBits[src_index+2])
      {
         pRLEBits[dst_index++] = 1; // block of length 1
         pRLEBits[dst_index++] = pSrcBits[src_index++];
         goto state_compress;
      }
      else // in these 3 pixel, no 2 consecutive pixel are the same
         goto state_no_compress; // un-compressed block need at least 3 pixel
      
state_compress:  // compress mode
	  end_of=0;
      for ( counter = 1; counter <= /*254*/4094; counter++)
      {
         // must check this condition first, or a BUG occur!
         if ( pSrcBits[src_index+counter] != pSrcBits[src_index] )
            break;
         if ( EndOfStream(src_index+counter,buffer_size) ) // reach end of line
         {
            counter+=1; // block length is (counter+1)
			end_of=1;
			break;
         }
         
      }
		
		if( counter <= 45 )
		{
			for(i=0;i<counter/15;i++)
			{
				pRLEBits[dst_index++] =0x0f;
				pRLEBits[dst_index++] =pSrcBits[src_index]; 
			}
			if( counter%15 )
			{
				pRLEBits[dst_index++] =counter%15;
				pRLEBits[dst_index++] =pSrcBits[src_index]; 
			}
			
			src_index += counter;
		}
		else
		{	
			pRLEBits[dst_index++] =0x00;
			pRLEBits[dst_index++] =0x01; //CMD 0x01 ,Counter is byte
			
			pRLEBits[dst_index++] =(counter&0xf0)>>4;
			pRLEBits[dst_index++] =counter&0x0f;	//Counter
			
			pRLEBits[dst_index++] =(counter&0x0f00)>>8;
			pRLEBits[dst_index++] =pSrcBits[src_index];
			
			src_index += counter;
		}
		
		if(end_of)
		goto end_of_line;
#if 0	  
      // src_index is the first pixel of a compressd block
      // counter is the number of additional pixels following currrent pixel
      // (src_index+counter) point to the last pixel of current block
      for ( counter = 1; counter <= 14; counter++)
      {
         // must check this condition first, or a BUG occur!
         if ( pSrcBits[src_index+counter] != pSrcBits[src_index] )
            break;
         if ( EndOfStream(src_index+counter,buffer_size) ) // reach end of line
         {
            pRLEBits[dst_index++] = counter+1; // block length is (counter+1)
            pRLEBits[dst_index++] = pSrcBits[src_index];
            src_index += counter +1;
            goto end_of_line;
         }
         
      }
      // now pSrcBits[src_index+counter] falls out of block
      pRLEBits[dst_index++] = counter; // block length is (counter)
      pRLEBits[dst_index++] = pSrcBits[src_index];
      src_index += counter;
#endif	  
      goto state_start;
      
state_no_compress:
      // src_index is the first pixel of a un-compressd block
      // un-compressed block need at least 3 pixel
      // counter is the number of additional pixels following currrent pixel
      for (counter = 2; counter <= 14; counter++)
      {
         if ( EndOfStream(src_index+counter,buffer_size) ) // reach end of line
         {
            pRLEBits[dst_index++] = 0; // escape character for un-compress block
            pRLEBits[dst_index++] = counter + 1; // block length is (counter+1)
            for (i = counter + 1; i > 0; i--)
               pRLEBits[dst_index++] = pSrcBits[src_index++];

            if ( 0 != ((counter+1) % 2) ) // block length is odd
               pRLEBits[dst_index++]=0;     // padd with a zero byte
			   
            goto end_of_line;
         }

         if ( EndOfStream(src_index+counter+1,buffer_size)  ||
            pSrcBits[src_index+counter] != pSrcBits[src_index+counter+1] )
            continue;   // still no 2 consecutive pixel are the same, 
                        // therefore continue to extend the un-compress block
          // we found two pixels are the same
         if ( EndOfStream(src_index+counter+2,buffer_size) ||
              pSrcBits[src_index+counter+1] != pSrcBits[src_index+counter+2] )
            continue; // the third pixel is not the same, no need to exit un-compressed mode
         else
         {  // // now pSrcBits[src_index+counter] and following 2 pixel are the same
            // now we need to exit the un-compressed mode
            if ( counter > 2)  
               counter--; // we can shrink the block one pixel (go backward)
            pRLEBits[dst_index++] = 0; // escape character for un-compress block
            pRLEBits[dst_index++] = counter+1; // block length is (counter+1)
            for (i = counter+1; i > 0; i--)
               pRLEBits[dst_index++] = pSrcBits[src_index++];

            if ( 0 != ((counter+1) % 2) ) // block length is odd
               pRLEBits[dst_index++]=0;     // padd with a zero byte
			   
            goto state_compress;
         }
      } // end of for (counter = 2; counter <= 254; counter++)

      // now we have a full block of 255 pixels
      pRLEBits[dst_index++] = 0; // escape character for un-compress block
      pRLEBits[dst_index++] = counter; // block length is (counter)
      for (i = counter; i > 0; i--)
         pRLEBits[dst_index++] = pSrcBits[src_index++];

      if ( 0 != ((counter) % 2) )   // block length is odd
            pRLEBits[dst_index++]=0;  // padd with a zero byte
			
      goto state_start;
	
end_of_line:

      // src_index already point to the start of next line
      pRLEBits[dst_index++] = 0; // mark end-of-line
      pRLEBits[dst_index++] = 0;

   *RLE_size = dst_index; // image size
}

static void split_to_two_byte(unsigned char *pDest,unsigned char *pSrc,int src_size)
{
	int i;
	
	for(i=0;i<src_size;i++)
	{
		pDest[2*i] = (pSrc[i]&0xf0)>>4;
		pDest[2*i+1] = pSrc[i]&0x0f;
	}
}
 
static void merge_to_one_byte(unsigned char *pDest,unsigned char *pSrc,int dest_size)
{
	int i;

	for(i=0;i<dest_size;i++)
	{
		pDest[i] = ((pSrc[2*i]&0x0f)<<4)|(pSrc[2*i+1]&0x0f);
	}
}

static void CompressStreamRLE4(unsigned char *pSrcBits,unsigned char *pRLEBits, int *RLE_size,
					int SRC_size,unsigned char*encode_buf)
{
/* encode_buf size = 4*SRC_size */
	unsigned char *pSRC=encode_buf;
	unsigned char *pRLE=encode_buf+2*SRC_size;
	int rle4_size;

	split_to_two_byte(pSRC,pSrcBits,SRC_size);
	
	_CompressStreamRLE4(pSRC,pRLE,&rle4_size,SRC_size*2);

	merge_to_one_byte(pRLEBits,pRLE,rle4_size/2);
	
	*RLE_size=rle4_size/2;
}

static void CompressStreamRLE4Byte(unsigned char *pSrcBits,unsigned char *pRLEBits, int *RLE_size,
					int SRC_size,unsigned char*encode_buf)
{
	unsigned char *pRLE=encode_buf;
	int rle4_size;
	
	_CompressStreamRLE4(pSrcBits,pRLE,&rle4_size,SRC_size*2);

	merge_to_one_byte(pRLEBits,pRLE,rle4_size/2);
	
	*RLE_size=rle4_size/2;
}

static void to_split_rgb16(unsigned char *dest_buf,unsigned char *src_buf,int w, int h)
{
	int i;
	unsigned char r0,g0,b0;
	unsigned char r1,g1,b1;

	unsigned char *dest_buf_r=dest_buf;
	unsigned char *dest_buf_g=dest_buf+w*h;
	unsigned char *dest_buf_b=dest_buf+2*w*h;
	unsigned char *dest_buf_x=dest_buf+3*w*h;
	
	for(i=0;i<w*h/2;i++)
	{
			r0=*src_buf++;
			g0=*src_buf++;
			b0=*src_buf++;

			r1=*src_buf++;
			g1=*src_buf++;
			b1=*src_buf++;
			
			*dest_buf_r++=(r0&0xf0)|((r1&0xf0)>>4);
			*dest_buf_g++=(g0&0xf0)|((g1&0xf0)>>4);
			*dest_buf_b++=(b0&0xf0)|((b1&0xf0)>>4);
		
			*dest_buf_x++=((r0&0x08)<<4)|
			((g0&0x0c)<<3)|
			((b0&0x08)<<1)|
			((r1&0x08))|
			((g1&0x0c)>>1)|
			((b1&0x08)>>3);
	}
}

static void to_split_rgb16_byte(unsigned char *dest_buf,unsigned char *src_buf,int w, int h)
{
	int i;
	unsigned char r,g,b;
	
	unsigned char *dest_buf_r=dest_buf;
	unsigned char *dest_buf_g=dest_buf+w*h;
	unsigned char *dest_buf_b=dest_buf+2*w*h;
	unsigned char *dest_buf_x=dest_buf+3*w*h;
	
	for(i=0;i<w*h;i++)
	{
			r=*src_buf++;
			g=*src_buf++;
			b=*src_buf++;
			
			*dest_buf_r++=(r&0xf0)>>4;
			*dest_buf_g++=(g&0xf0)>>4;
			*dest_buf_b++=(b&0xf0)>>4;
			*dest_buf_x++=((r&0x08))|((g&0x0c)>>1)|((b&0x08)>>3);
	}
}

static void from_split_rgb16(unsigned char *dest_buf,unsigned char *src_buf,int w, int h)
{
	int i;
	unsigned char red,green,blue,x;
	
	unsigned char *src_buf_r=src_buf;
	unsigned char *src_buf_g=src_buf+w*h;
	unsigned char *src_buf_b=src_buf+2*w*h;
	unsigned char *src_buf_x=src_buf+3*w*h;
	
	for(i=0;i<w*h/2;i++)
	{
			red=*src_buf_r++;
			green=*src_buf_g++;
			blue=*src_buf_b++;
			x=*src_buf_x++;

			*dest_buf++=(red&0xf0)|((x&0x80)>>4);//r0
			*dest_buf++=(green&0xf0)|((x&0x60)>>3);//g0
			*dest_buf++=(blue&0xf0)|((x&0x10)>>1);//b0
			*dest_buf++=((red&0x0f)<<4)|((x&0x08));//r1
			*dest_buf++=((green&0x0f)<<4)|((x&0x06)<<1);//g1
			*dest_buf++=((blue&0x0f)<<4)|((x&0x01)<<3);//b1
	}
}

static void fprint_quartet(unsigned int i,FILE * file)
{
	fwrite(&i,4,1,file);
}

static void xor_frame(unsigned char *buf_ref,unsigned char *buf,int w,int h)
{
	int i;
	
	for(i=0;i<3*w*h;i++)
	{
		buf[i]  ^= buf_ref[i];
	}
}

void to_rgb16(unsigned char *buf,int w,int h)
{
	int i,j;
	
	for(i=0;i<w*h;i++)
	{
		j=3*i;
		buf[j]  &= 0xf8;
		buf[j+1]&= 0xfc;
		buf[j+2]&= 0xf8;
	}
}

static const unsigned char R5to8[32]={
0,9,18,27,36,45,54,63,64,73,82,91,100,109,118,127,
128,137,146,155,164,173,182,191,192,201,210,219,228,237,246,255
};

static const unsigned char G6to8[64]={
0,5,10,15,16,21,26,31,32,37,42,47,48,53,58,63,
64,69,74,79,80,85,90,95,96,101,106,111,112,117,122,127,
128,133,138,143,144,149,154,159,160,165,170,175,176,181,186,191,
192,197,202,207,208,213,218,223,224,229,234,239,240,245,250,255
};

static const unsigned char *B5to8 = R5to8;

void from_rgb16(unsigned char *buf,int w,int h)
{
	int i,j;
	
	for(i=0;i<w*h;i++)
	{
		j=3*i;
/*	
		buf[3*i]  |= (buf[3*i]>>3)&0x07;
		buf[3*i+1]|= (buf[3*i+1]>>2)&0x03;
		buf[3*i+2]|= (buf[3*i+2]>>3)&0x07;
*/		
		buf[j]  = R5to8[buf[j]>>3];
		buf[j+1]= G6to8[buf[j+1]>>2];
		buf[j+2]= B5to8[buf[j+2]>>3];
	}
}

static void from_split_rgb16_24bit(unsigned char *dest_buf,unsigned char *src_buf,int w, int h)
{
	int i;
	unsigned char red,green,blue,x;
	
	unsigned char *src_buf_r=src_buf;
	unsigned char *src_buf_g=src_buf+w*h;
	unsigned char *src_buf_b=src_buf+2*w*h;
	unsigned char *src_buf_x=src_buf+3*w*h;
	
	for(i=0;i<w*h/2;i++)
	{
			red=*src_buf_r++;
			green=*src_buf_g++;
			blue=*src_buf_b++;
			x=*src_buf_x++;

			*dest_buf++=R5to8[((red&0xf0)>>3)|((x&0x80)>>7)];//r0
			*dest_buf++=G6to8[((green&0xf0)>>2)|((x&0x60)>>5)];//g0
			*dest_buf++=B5to8[((blue&0xf0)>>3)|((x&0x10)>>4)];//b0
			*dest_buf++=R5to8[((red&0x0f)<<1)|((x&0x08)>>3)];//r1
			*dest_buf++=G6to8[((green&0x0f)<<2)|((x&0x06)>>1)];//g1
			*dest_buf++=B5to8[((blue&0x0f)<<1)|((x&0x01))];//b1
	}
}

int rle4_write_mem(unsigned char *dest, int w, int h, unsigned char *buf,int *rle_length,unsigned char *p_source)
{
	/*p_source 	size = 4*w*h + 2*w*h*/
	
	unsigned char *p_dest;
	unsigned char *encode_buf=p_source+w*h*4;
		
	int RLE_size_rgb[3];
	int RLE_size_last_block;
//	int i;
	
	memcpy(dest,"RLE4",4);
	memcpy(dest+4,&w,4);
	memcpy(dest+8,&h,4);
	
	//memset(dest+12,0,16);	

	to_split_rgb16_byte(p_source,buf,w,h);

	p_dest=dest+RLE_HEAD_LEN;

	CompressStreamRLE4Byte(p_source,p_dest,&RLE_size_rgb[0],w*h/2,encode_buf);

	p_dest+=RLE_size_rgb[0];
	CompressStreamRLE4Byte(p_source+w*h,p_dest,&RLE_size_rgb[1],w*h/2,encode_buf);
	
	p_dest+=RLE_size_rgb[1];
	CompressStreamRLE4Byte(p_source+2*w*h,p_dest,&RLE_size_rgb[2],w*h/2,encode_buf);

	p_dest+=RLE_size_rgb[2];	
	CompressStreamRLE4Byte(p_source+3*w*h,p_dest,&RLE_size_last_block,w*h/2,encode_buf);

/*	
	for(i=0;i<3;i++)
	{
	mprintf("RLE_size[%d]=%d\n",i,RLE_size_rgb[i]);
	}

	mprintf("RLE_size_last_block=%d\n",RLE_size_last_block);
*/

	memcpy(dest+12,RLE_size_rgb,4);
	memcpy(dest+16,RLE_size_rgb+1,4);
	memcpy(dest+20,RLE_size_rgb+2,4);
	memcpy(dest+24,&RLE_size_last_block,4);
	
	*rle_length=RLE_HEAD_LEN+RLE_size_rgb[0]+RLE_size_rgb[1]+RLE_size_rgb[2]+RLE_size_last_block;
	
	return 0;	
}

int rle4_read_mem(unsigned char *rle_file_buf,int rle_file_len,unsigned char* src_buf,unsigned char *pbuf)
{
	/*pbuf 	size = 4*w*h */
	int w,h;
	int RLE_size_rgb[3];
	int RLE_size_last_block;
	
	unsigned char *pRLEBits;
	
	if( strncmp((const char*)rle_file_buf,"RLE4",4)!=0 )
	{
		mprintf("Not a valid rle file.\n");
		return -1;;
	}
		
	memcpy(&w,rle_file_buf+4,4);
	memcpy(&h,rle_file_buf+8,4);

	memcpy(RLE_size_rgb,rle_file_buf+12,4);
	memcpy(RLE_size_rgb+1,rle_file_buf+16,4);
	memcpy(RLE_size_rgb+2,rle_file_buf+20,4);	
	memcpy(&RLE_size_last_block,rle_file_buf+24,4);	

/*	
	mprintf("w=%d,h=%d\n",w,h);
*/

	pRLEBits = rle_file_buf+RLE_HEAD_LEN;
	
	DecompressStreamRLE4(pbuf,pRLEBits,RLE_size_rgb[0],w*h);
	pRLEBits +=RLE_size_rgb[0];

	DecompressStreamRLE4(pbuf+w*h,pRLEBits,RLE_size_rgb[1],w*h);
	pRLEBits +=RLE_size_rgb[1];

	DecompressStreamRLE4(pbuf+2*w*h,pRLEBits,RLE_size_rgb[2],w*h);
	pRLEBits +=RLE_size_rgb[2];

	DecompressStreamRLE4(pbuf+3*w*h,pRLEBits,RLE_size_last_block,w*h);
	pRLEBits +=RLE_size_last_block;

	from_split_rgb16_24bit(src_buf,pbuf,w,h);
		
	return 0;
}



