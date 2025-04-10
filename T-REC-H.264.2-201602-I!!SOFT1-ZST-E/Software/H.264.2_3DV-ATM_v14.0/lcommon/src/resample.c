/*
 * Disclaimer of Warranty
 *
 * Copyright 2001-2015, International Telecommunication Union, Geneva
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis. The ITU disclaims
 * any and all warranties, whether express, implied, or statutory,
 * including any implied warranties of merchantability or of fitness
 * for a particular purpose.  In no event shall the ITU be liable for
 * any incidental, punitive, or consequential damages of any kind
 * whatsoever arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and the user's customers, employees, agents, transferees,
 * successors, and assigns.
 *
 * The ITU does not represent or warrant that the programs furnished
 * hereunder are free of infringement of any third-party patents.
 * Commercial implementations of ITU-T Recommendations, including
 * shareware, may be subject to royalty fees to patent holders.
 * Information regarding the ITU-T patent policy is available from the
 * ITU web site at http://www.itu.int.
 *
 * THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
 *
 */




#include "win32.h"
#include "defines.h"
#include "typedefs.h"
#include "ifunctions.h"
#include "memalloc.h"
#include "resample.h"

#if EXT3D

#define TAP_10_FLT 1          // 1: use a set of 10-tap filters for downsampling; 0: a set of 12-taps.
#define KAISER_FLT 0          // 1: use Kaiser filter, 0: use sine-windowed filter


static  imgpel Clip( int iValue, int imin, int imax )   ;
static void ConvertImgpelToIntBuffer(ResizeParameters * pResizeParameters,int component);
static void ConvertIntBufferToImgpel(ResizeParameters* pResizeParameters,int component);
#if 0 // Outdated functionality
static void Downsampling( ResizeParameters* pResizeParameters, int component ) ;
static void img_downSample ( ResizeParameters*  ResizeParams, int component);
#endif
static void Upsampling(ResizeParameters* pResizeParameters, int component,int method )   ;
static void img_upSample(ResizeParameters*  ResizeParams, int component, int method);

static void img_bilinear_fast( imgpel** low_img,imgpel** high_img,int low_height, int low_width, int high_height, int high_width);


static imgpel Clip( int iValue, int imin, int imax )
{
  int tmp;
  imgpel rV ;
  memset(&rV,0x00,sizeof(imgpel));
  tmp= ( iValue < imin ? imin : iValue > imax ? imax : iValue );
  memcpy(&rV,&tmp,sizeof(imgpel))  ;

  return rV;
}
static void ConvertImgpelToIntBuffer(ResizeParameters * pResizeParameters,int component)
{
  int src_stride;
  int img_stride=  pResizeParameters->img_stride;

  imgpel* imgpel_Src=pResizeParameters->img_buffer[component];
  int * pDst=pResizeParameters->resize_buffer;

  int width=pResizeParameters->input_width;
  int height=pResizeParameters->input_height;

  int i,j,tmp;

  if((component<0)||(component>2))
  {
    printf("invalid imgpel component\n");
    exit(1);
  }

  if(component>0)
  {
    width=width>>1;
    height=height>>1;
  }


  if(pResizeParameters->down_sample)
    src_stride=pResizeParameters->input_stride[component];
  else
    src_stride=pResizeParameters->output_stride[component];

  for( j = 0; j < height; j++ )
  {
    for( i = 0; i < width;  i++ )
    {
      //  tmp=0x00;
      //  printf("%d\n",tmp);
      memset(&tmp,0x00,sizeof(int));
      memcpy(&tmp,&(imgpel_Src[i]),sizeof(imgpel));
      pDst[i] = tmp;
    }
    pDst+=img_stride;
    imgpel_Src+=src_stride;
  }
  return ;
}

static void ConvertIntBufferToImgpel(ResizeParameters* pResizeParameters,int component)
{

  int dst_stride;
  int img_stride=pResizeParameters->  img_stride;

  int* pSrc=   pResizeParameters->resize_buffer;
  imgpel* imgpel_dst=pResizeParameters->img_buffer[component];


  int width=pResizeParameters->output_width;
  int height=pResizeParameters->output_heigh;

  int i,j;
  int minValue=pResizeParameters->minValue;
  int maxValue=pResizeParameters->maxValue;

  if((component<0)||(component>2))
  {
    printf("invalid imgpel component\n");
    exit(1);
  }

  if(pResizeParameters->down_sample)
    dst_stride=pResizeParameters->input_stride[component];
  else
    dst_stride=pResizeParameters->output_stride[component];

  if(component>0)
  {
    width=width>>1;
    height=height>>1;
  }

  for( j = 0; j < height; j++ )
  {
    for( i = 0; i < width;  i++ )
    {
      imgpel_dst[i] = Clip( pSrc[i], minValue, maxValue );
    }
    imgpel_dst+= dst_stride;
    pSrc += img_stride;
  }
}

static int calcute_ref_pixel_pos(ResizeParameters* pResizeParameters)
{
  int input_width=pResizeParameters->input_width;
  int input_height=pResizeParameters->input_height;
  int output_width=pResizeParameters->output_width;
  int output_height=pResizeParameters->output_heigh;
  int crop_x0=pResizeParameters->crop_x0;
  int crop_y0=pResizeParameters->crop_y0;
  int crop_w=pResizeParameters->crop_w;
  int crop_h =pResizeParameters->crop_h;
  int input_chroma_phase_shift_x= 0;
  int input_chroma_phase_shift_y= 0;
  int output_chroma_phase_shift_x=0;
  int output_chroma_phase_shift_y=0;
  int i=0,j=0;
  int memory_size=0;

  if(!pResizeParameters->down_sample)
  {
    int ratio1_flag ;
    int G = 2, J, M, S = 12;
    unsigned short C, C1, D1;
    int D, E, q, w;

    for(i=0;i<2;++i)
    {
      if(i)
      {
        //!<UV
        input_width=input_width>>1;
        input_height=input_height>>1;
        output_width=output_width>>1;
        output_height=output_height>>1;
        crop_x0=crop_x0>>1;
        crop_y0=crop_y0>>1;
        crop_h=crop_h>>1;
        crop_w=crop_w>>1;
        input_chroma_phase_shift_x= pResizeParameters->input_chroma_phase_shift_x;
        input_chroma_phase_shift_y= pResizeParameters->input_chroma_phase_shift_y;
        output_chroma_phase_shift_x=pResizeParameters->output_chroma_phase_shift_x;
        output_chroma_phase_shift_y= pResizeParameters->output_chroma_phase_shift_y;
      }
      ratio1_flag= ( input_width == crop_w );

      // initialization
      pResizeParameters->pos_x[i]=(int*)malloc(output_width*sizeof(int));
      pResizeParameters->pos_y[i]=(int*)malloc(output_height*sizeof(int));
      memory_size+=(output_width+output_height)*sizeof(int);

      for(j=0; j<crop_x0; ++j)  
        pResizeParameters->pos_x[i][j] = -128;

      for(j=crop_x0+crop_w; j<output_width; ++j)  
        pResizeParameters->pos_x[i][j] = -128;

      if(ratio1_flag)
      {
        for(j = 0; j < crop_w; j++)
        {
          pResizeParameters->pos_x[i][j+crop_x0] = j*16+4*(2+output_chroma_phase_shift_x)-4*(2+input_chroma_phase_shift_x);
        }
      }
      else
      {
        J = 1;
        M = 13; 
        if(i)
        {
          J ++;
          M --;
        }

        // S = M + G +  J  - F;

        C = (unsigned short)( ((1<<(M+G))*input_width + (crop_w>>1))/crop_w );
        //D = ((-1)<<(G-1+J+M)) + (1<<(S-1)) - (input_chroma_phase_shift_x<<(G-2+J+M));
        D = ((-1)<<15) + (1<<11) - (input_chroma_phase_shift_x<<14);

        C1 = C<<J;
        E = 0;

        q = (C<<(J-1)) + D + (C<<(J-2))*output_chroma_phase_shift_x;
        w = q>>S;
        D1 = (unsigned short)(q - (w<<S));
        E += w;
        pResizeParameters->pos_x[i][0+crop_x0] = E;

        for(j = 1; j < crop_w; ++j)
        {
          q = C1 + D1;
          w = q>>S;
          D1 = (unsigned short)(q - (w<<S));
          E += w;    
          pResizeParameters->pos_x[i][j+crop_x0] = E;
        }
      }

      ratio1_flag = ( input_height == crop_h );

      for(j=0; j<crop_y0; j++)   
        pResizeParameters->pos_y[i][j] = -128;

      for(j=crop_y0+crop_h; j<output_height; j++) 
        pResizeParameters->pos_y[i][j] = -128;

      if(ratio1_flag)
      {
        for(j = 0; j < crop_h; j++)
        {
          pResizeParameters->pos_y[i][j+crop_y0] = j*16+4*(2+output_chroma_phase_shift_y)-4*(2+input_chroma_phase_shift_y);
        }
      }
      else
      {
        J = 1; M = 13; 
        if(i)
        {
          J ++;
          M --;
        };
        // S = M + G +  J  - F;

        C = (unsigned short)(((1<<(M+G))*input_height + (crop_h>>1))/crop_h);
        //D = ((-1)<<(G-1+J+M)) + (1<<(S-1)) - (input_chroma_phase_shift_y<<(G-2+J+M));
        D = ((-1)<<15) + (1<<11) - (input_chroma_phase_shift_y<<14);

        C1 = C<<J;
        E = 0;

        q = (C<<(J-1)) + D + (C<<(J-2))*output_chroma_phase_shift_y;
        w = q>>S;
        D1 = (unsigned short)(q - (w<<S));
        E += w;
        pResizeParameters->pos_y[i][0+crop_y0] = E;

        for(j = 1; j < crop_h; j++)
        {
          q = C1 + D1;
          w = q>>S;
          D1 = (unsigned short)(q - (w<<S));
          E += w;    
          pResizeParameters->pos_y[i][j+crop_y0] = E;
        }
      }

    }
  }
  else
  {
    for(i=0;i<2;++i)
    {
      if(i)
      {
        input_width=input_width>>1;
        input_height=input_height>>1;
        output_width=output_width>>1;
        output_height=output_height>>1;
        crop_x0=crop_x0>>1;
        crop_y0=crop_y0>>1;
        crop_h=crop_h>>1;
        crop_w=crop_w>>1;

        input_chroma_phase_shift_x=pResizeParameters->input_chroma_phase_shift_x;
        input_chroma_phase_shift_y =pResizeParameters->input_chroma_phase_shift_y;
        output_chroma_phase_shift_x=pResizeParameters->output_chroma_phase_shift_x;
        output_chroma_phase_shift_y=pResizeParameters->output_chroma_phase_shift_y;
      }

      // initialization
      pResizeParameters->pos_x[i]=(int*)malloc(output_width*sizeof(int));
      pResizeParameters->pos_y[i]=(int*)malloc(output_height*sizeof(int));

      memory_size+=(output_width+output_height)*sizeof(int);

      for( j = 0; j < output_width; ++j )
      {
        pResizeParameters->pos_x[i][j] = 16*crop_x0 + ( j*crop_w*16 + 4*(2+output_chroma_phase_shift_x)*crop_w - 4*(2+input_chroma_phase_shift_x)*output_width + output_width/2) / output_width;
      }


      for( j = 0; j < output_height; ++j )
      {
        pResizeParameters->pos_y[i][j] = 16*crop_y0 + ( j*crop_h*16 + 4*(2+output_chroma_phase_shift_y)*crop_h - 4*(2+input_chroma_phase_shift_y)*output_height + output_height/2 ) / output_height;
      }

    }
  }

  return memory_size;
}

static void Upsampling(ResizeParameters* pResizeParameters, int component,int method )
{
  int filter16[16][6] = 
  { // Lanczos3
    {0,0,32,0,0,0},
    {0,-2,32,2,0,0},
    {1,-3,31,4,-1,0},
    {1,-4,30,6,-1,0},
    {1,-4,28,9,-2,0},
    {1,-4,27,11,-3,0},
    {1,-5,25,14,-3,0},
    {1,-5,22,17,-4,1},
    {1,-5,20,20,-5,1},
    {1,-4,17,22,-5,1},
    {0,-3,14,25,-5,1},
    {0,-3,11,27,-4,1},
    {0,-2,9,28,-4,1},
    {0,-1,6,30,-4,1},
    {0,-1,4,31,-3,1},
    {0,0,2,32,-2,0}
  };
  int filter16_chroma[16][6] = 
  { // bilinear
    {0,0,32,0,0,0},
    {0,0,30,2,0,0},
    {0,0,28,4,0,0},
    {0,0,26,6,0,0},
    {0,0,24,8,0,0},
    {0,0,22,10,0,0},
    {0,0,20,12,0,0},
    {0,0,18,14,0,0},
    {0,0,16,16,0,0},
    {0,0,14,18,0,0},
    {0,0,12,20,0,0},
    {0,0,10,22,0,0},
    {0,0,8,24,0,0},
    {0,0,6,26,0,0},
    {0,0,4,28,0,0},
    {0,0,2,30,0,0}
  };


  int input_width=pResizeParameters->input_width;
  int input_height=pResizeParameters->input_height;
  int output_width=pResizeParameters->output_width;
  int output_height=pResizeParameters->output_heigh;
  int crop_x0=pResizeParameters->crop_x0;
  int crop_y0=pResizeParameters->crop_y0;
  int crop_w=pResizeParameters->crop_w;
  int crop_h =pResizeParameters->crop_h;

  int stride=pResizeParameters->img_stride ;

  int * resize_int_buffer=pResizeParameters->resize_buffer;
  int * resize_int_buffer_tmp=pResizeParameters->resize_buffer_tmp;

  //JVT-S067
  int i, j, k, *px, *py;
  int x16, y16, x, y, m;

  px=component?pResizeParameters->pos_x[1]:pResizeParameters->pos_x[0];
  py=component?pResizeParameters->pos_y[1]:pResizeParameters->pos_y[0];

  if(component)
  {
    input_width=input_width>>1;
    input_height=input_height>>1;
    output_width=output_width>>1;
    output_height=output_height>>1;
    crop_x0=crop_x0>>1;
    crop_y0=crop_y0>>1;
    crop_h=crop_h>>1;
    crop_w=crop_w>>1;
  }
  if(method)
  {
    for (i=0; i<16; i++)
      for (j=0; j<6; j++)
        filter16[i][j] = filter16_chroma[i][j];
  }

  //========== horizontal upsampling ===========
  for( j = 0; j < input_height; j++ ) 
  {
    int*  piSrc = &resize_int_buffer[j*stride];
    for( i = 0; i < output_width; i++ )
    {
      if( px[i]==-128 ) 
        continue;
      x16 = px[i]&0x0f;
      x = px[i]>>4;
      resize_int_buffer_tmp[i] = 0;
      for( k=0; k<6; k++) 
      {
        m = x - 2 + k;
        if( m<0 )
          m = 0;
        else 
          if( m>input_width-1) 
          m=input_width-1;

        resize_int_buffer_tmp[i] += filter16[x16][k]*piSrc[m];
      }
      resize_int_buffer_tmp[i] = resize_int_buffer_tmp[i];
    }
    //----- copy row back to image buffer -----
    memcpy( piSrc, resize_int_buffer_tmp, output_width*sizeof(int) );
  }
  //========== vertical upsampling ===========
  for( i = 0; i < output_width; i++ ) 
  {
    int*  piSrc = &resize_int_buffer[i];
    for( j = 0; j < output_height; j++ )
    {
      if( py[j]==-128 || px[i]==-128)
      {
        resize_int_buffer_tmp[j] = 128;
        continue;
      }
      y16 = py[j]&0x0f;
      y = py[j]>>4;
      resize_int_buffer_tmp[j] = 0;
      for( k=0; k<6; k++) 
      {
        m = y - 2 + k;
        if( m<0 ) 
          m = 0;
        else 
          if( m>input_height-1) 
          m=input_height-1;

        resize_int_buffer_tmp[j] += filter16[y16][k]*piSrc[m*stride];
      }
      resize_int_buffer_tmp[j] = (resize_int_buffer_tmp[j]+512)/1024;
    }
    //----- scale and copy back to image buffer -----
    for( j = 0; j < output_height; j++ )
    {
      piSrc[j*stride] = resize_int_buffer_tmp[j];
    }
  }
}

static void img_upSample(ResizeParameters* Parameters, int component, int method)
{
  if(-1==component)
  {
    //Y
    ConvertImgpelToIntBuffer(Parameters,0);
    Upsampling(Parameters,0,method);
    ConvertIntBufferToImgpel(Parameters,0);

    //U
    ConvertImgpelToIntBuffer(Parameters,1);
    Upsampling(Parameters,1,method);
    ConvertIntBufferToImgpel(Parameters,1);

    //V
    ConvertImgpelToIntBuffer(Parameters,2);
    Upsampling(Parameters,2,method);
    ConvertIntBufferToImgpel(Parameters,2);
  }
  else
  {
    ConvertImgpelToIntBuffer(Parameters,component);
    Upsampling(Parameters,component,method);
    ConvertIntBufferToImgpel(Parameters,component);
  }
}

static void img_bilinear_fast(imgpel** low_img,imgpel** high_img,int low_height, int low_width, int high_height, int high_width)
{
  int i,j,i2,j2;
  imgpel curr,right,down,down_right;

  for(j=0;j<low_height-1;++j)
  {
    for(i=0;i<low_width-1;++i)
    {
      curr=low_img[j][i];
      right=low_img[j][i+1];
      down=low_img[j+1][i];
      down_right=low_img[j+1][i+1];

      j2=j<<1;
      i2=i<<1;

      high_img[j2][i2]=curr;
      high_img[j2][i2+1]=(curr+right+1)>>1;
      high_img[j2+1][i2]=(curr+down+1)>>1;
      high_img[j2+1][i2+1]=(curr+right+down+down_right+2)>>2;
    }

    j2=j<<1;
    i2=i<<1;
    curr=low_img[j][i];
    down=low_img[j+1][i];

    high_img[j2][i2+1]=high_img[j2][i2]=curr;
    high_img[j2+1][i2+1]=high_img[j2+1][i2]=(curr+down+1)>>1;
  }
  j2=j<<1;
  for(i=0;i<low_width-1;++i)
  {
    i2=i<<1;
    curr=low_img[j][i];
    right=low_img[j][i+1];
    high_img[j2][i2]=curr;
    high_img[j2][i2+1]=(curr+right+1)>>1;
  }
  i2=i<<1;
  high_img[j2][i2+1]=high_img[j2][i2]=low_img[j][i];

  memcpy(high_img[high_height-1],high_img[high_height-2],high_width*sizeof(imgpel));
}


int init_ImageResize(ResizeParameters* ResizeParams, int width_in,
             int height_in, int width_out, int height_out)
{
  int input_luma_size,input_chroma_size;
  int memory_size=0;
  int downsampling=1;
  assert(ResizeParams);

  // @DT: Depth grid crop
  if((width_in > width_out)||(height_in > height_out))
    downsampling = 1;
  else 
    downsampling = 0;
  
  if(downsampling)
  {
    input_luma_size=width_in*height_in;
    input_chroma_size=(width_in>>1)*(height_in>>1);
    ResizeParams->down_sample=1;
  }
  else
  {

    input_luma_size=width_out*height_out;
    input_chroma_size=(width_out>>1)*(height_out>>1);
    ResizeParams->down_sample=0;
  }

  ResizeParams->input_width=width_in;
  ResizeParams->input_height=height_in;
  ResizeParams->output_width=width_out;
  ResizeParams->output_heigh=height_out;
  ResizeParams->input_chroma_phase_shift_x=-1;
  ResizeParams->input_chroma_phase_shift_y=0;
  ResizeParams->input_chroma_phase_shift_x=-1;
  ResizeParams->input_chroma_phase_shift_y=0;
  ResizeParams->output_chroma_phase_shift_x=-1;
  ResizeParams->output_chroma_phase_shift_y=0;

  ResizeParams->crop_x0=0;
  ResizeParams->crop_y0=0;
  ResizeParams->crop_w=imax(width_in,width_out);
  ResizeParams->crop_h=imax(height_in,height_out);

  ResizeParams->input_stride[0]=width_in;
  ResizeParams->input_stride[1]=ResizeParams->input_stride[2]=width_in>>1;
  ResizeParams->output_stride[0]=width_out;
  ResizeParams->output_stride[1]=ResizeParams->output_stride[2]=width_out>>1;

  ResizeParams->img_stride=  imax(width_in,width_out);
  ResizeParams->minValue=0;
  ResizeParams->maxValue=255;

  if((ResizeParams->resize_buffer=(int*)malloc(input_luma_size*sizeof(int)))==NULL)
    no_mem_exit("initSampleResize: pDownsampingParas->resize_buffer")    ;
  memory_size+=input_luma_size*sizeof(int);
  if((ResizeParams->resize_buffer_tmp=(int*)malloc(imax(width_in,width_out)*sizeof(int)))==NULL)
    no_mem_exit("initSampleResize: pDownsampingParas->resize_buffer_tmp")  ;
  memory_size+=width_in*sizeof(int);
  if((ResizeParams->img_buffer[0]=(imgpel*)malloc(input_luma_size*sizeof(imgpel)))==NULL)
    no_mem_exit("initSampleResize: pDownsampingParas->img_buffer[0]")   ;
  memory_size+=input_luma_size*sizeof(imgpel);
  if((ResizeParams->img_buffer[1]=(imgpel*)malloc(input_chroma_size*sizeof(imgpel)))==NULL)
    no_mem_exit("init_downsampling: pDownsampingParas->img_buffer[1]")   ;
  memory_size+=input_chroma_size*sizeof(imgpel);
  if((ResizeParams->img_buffer[2]=(imgpel*)malloc(input_chroma_size*sizeof(imgpel)))==NULL)
    no_mem_exit("init_downsampling: pDownsampingParas->img_buffer[2]")   ;
  memory_size+=input_chroma_size*sizeof(imgpel);

  memory_size+=calcute_ref_pixel_pos(ResizeParams);
  return memory_size;
}

void destroy_ImageResize(ResizeParameters* pResizeParams)
{

  if(pResizeParams)
  {
    if(pResizeParams->resize_buffer)
    {
      free(pResizeParams->resize_buffer);
      pResizeParams->resize_buffer=NULL;
    }
    if(pResizeParams->resize_buffer_tmp)
    {
      free(pResizeParams->resize_buffer_tmp);
      pResizeParams->resize_buffer_tmp=NULL;
    }
    if(pResizeParams->img_buffer[0])
    {
      free(pResizeParams->img_buffer[0]);
      pResizeParams->img_buffer[0]=NULL;
    }
    if(pResizeParams->img_buffer[1])
    {
      free(pResizeParams->img_buffer[1]);
      pResizeParams->img_buffer[1]=NULL;
    }
    if(pResizeParams->img_buffer[2])
    {
      free(pResizeParams->img_buffer[2]);
      pResizeParams->img_buffer[2]=NULL;
    }
    if(pResizeParams->pos_x[0])
    {
      free(pResizeParams->pos_x[0]);
      pResizeParams->pos_x[0]=NULL;
    }
    if(pResizeParams->pos_x[1])
    {
      free(pResizeParams->pos_x[1]);
      pResizeParams->pos_x[1]=NULL;
    }
    if(pResizeParams->pos_y[0])
    {
      free(pResizeParams->pos_y[0]);
      pResizeParams->pos_y[0]=NULL;
    }
    if(pResizeParams->pos_y[1])
    {
      free(pResizeParams->pos_y[1]);
      pResizeParams->pos_y[1]=NULL;
    }
    free(pResizeParams);
    pResizeParams=NULL;
  }
}


void generic_upsampler(ResizeParameters* ResizeParams,
            imgpel**low_imgY,imgpel**low_imgU,imgpel**low_imgV,
            imgpel**high_imgY,imgpel**high_imgU,imgpel**high_imgV,
            int component,int method)
{
  int width_in=ResizeParams->input_width;
  int height_in=ResizeParams->input_height;
  int width_out=ResizeParams->output_width;
  int height_out=ResizeParams->output_heigh;

  int j=0;

  //!<Just support 4:2:0 and 4:0:0
  if((method==BI_LINEAR)&&(ResizeParams->input_height*2==ResizeParams->output_heigh)&&(ResizeParams->input_width*2==ResizeParams->output_width))
  {
    if(low_imgY)
      img_bilinear_fast(low_imgY,high_imgY,ResizeParams->input_height,ResizeParams->input_width,ResizeParams->output_heigh,ResizeParams->output_width);
    if(low_imgU)
      img_bilinear_fast(low_imgU,high_imgU,(ResizeParams->input_height)>>1,(ResizeParams->input_width)>>1,(ResizeParams->output_heigh)>>1,(ResizeParams->output_width)>>1);
    if(low_imgV)
      img_bilinear_fast(low_imgV,high_imgV,(ResizeParams->input_height)>>1,(ResizeParams->input_width)>>1,(ResizeParams->output_heigh)>>1,(ResizeParams->output_width)>>1);

    return ;
  }
  if(low_imgY)
  {
    for(j=0;j<height_in;++j)
    {
      memcpy(ResizeParams->img_buffer[0]+j*width_out,low_imgY[j],width_in*sizeof(imgpel));
    }
  }
  if(low_imgU)
  {
    for(j=0;j<(height_in>>1);++j)
      memcpy(ResizeParams->img_buffer[1]+j*(width_out>>1),low_imgU[j],(width_in>>1)*sizeof(imgpel));
  }
  if(low_imgV)
  {
    for(j=0;j<(height_in>>1);++j)
      memcpy(ResizeParams->img_buffer[2]+j*(width_out>>1),low_imgV[j],(width_in>>1)*sizeof(imgpel));
  }

  img_upSample(ResizeParams,component, method);

  if(high_imgY)
  {
    for(j=0;j<height_out;++j)
    {
      memcpy(high_imgY[j],ResizeParams->img_buffer[0]+j*width_out,width_out*sizeof(imgpel));
    }
  }
  if(high_imgU)
  {
    for(j=0;j<(height_out>>1);++j)
    {
      memcpy(high_imgU[j],ResizeParams->img_buffer[1]+j*(width_out>>1),(width_out>>1)*sizeof(imgpel));
    }
  }
  if(high_imgV)
  {
    for(j=0;j<(height_out>>1);++j)
    {
      memcpy(high_imgV[j],ResizeParams->img_buffer[2]+j*(width_out>>1),(width_out>>1)*sizeof(imgpel));
    }
  }

}

#endif
