/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2016, International Telecommunications Union, Geneva
*
* DISCLAIMER OF WARRANTY
*
* These software programs are available to the user without any
* license fee or royalty on an "as is" basis. The ITU disclaims
* any and all warranties, whether express, implied, or
* statutory, including any implied warranties of merchantability
* or of fitness for a particular purpose.  In no event shall the
* contributor or the ITU be liable for any incidental, punitive, or
* consequential damages of any kind whatsoever arising from the
* use of these programs.
*
* This disclaimer of warranty extends to the user of these programs
* and user's customers, employees, agents, transferees, successors,
* and assigns.
*
* The ITU does not represent or warrant that the programs furnished
* hereunder are free of infringement of any third-party patents.
* Commercial implementations of ITU-T Recommendations, including
* shareware, may be subject to royalty fees to patent holders.
* Information regarding the ITU-T patent policy is available from
* the ITU Web site at http://www.itu.int.
*
* THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
************************************************************************
*/


/*!
 ************************************************************************
 * \file output.c
 *
 * \brief
 *    Output an image and Trance support
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Karsten Suehring
 ************************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "mbuffer.h"
#include "image.h"
#include "memalloc.h"
#include "sei.h"
#include "input.h"
#include "fast_memory.h"

static void write_out_picture(VideoParameters *p_Vid, StorablePicture *p, int p_out);
static void img2buf_byte   (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int iOutStride);
static void img2buf_normal (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int iOutStride);
static void img2buf_endian (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int iOutStride);


/*!
 ************************************************************************
 * \brief
 *      selects appropriate output function given system arch. and data
 * \return
 *
 ************************************************************************
 */
void init_output(CodingParameters *p_cps, int symbol_size_in_bytes)
{
  if (( sizeof(char) == sizeof (imgpel)))
  {
    if ( sizeof(char) == symbol_size_in_bytes)
      p_cps->img2buf = img2buf_byte;
    else
      p_cps->img2buf = img2buf_normal;
  }
  else
  {
    if (testEndian())
      p_cps->img2buf = img2buf_endian;
    else
      p_cps->img2buf = img2buf_normal;
  }    
}

/*!
 ************************************************************************
 * \brief
 *    Convert image plane to temporary buffer for file writing
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 ************************************************************************
 */
static void img2buf_normal (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int iOutStride)
{
  int i,j;

  int twidth  = size_x - crop_left - crop_right;
  int theight = size_y - crop_top - crop_bottom;

  int size = 0;

  // sizeof (imgpel) > sizeof(char)
  // little endian
  if (sizeof (imgpel) < symbol_size_in_bytes)
  {
    // this should not happen. we should not have smaller imgpel than our source material.
    size = sizeof (imgpel);
    // clear buffer
    for(j=0; j<theight; j++)
      memset (buf+j*iOutStride, 0, (twidth * symbol_size_in_bytes));
  }
  else
  {
    size = symbol_size_in_bytes;
  }

  if ((crop_top || crop_bottom || crop_left || crop_right) || (size != 1))
  {
    for(i=crop_top; i<size_y-crop_bottom; i++)
    {
      int ipos = (i - crop_top) * iOutStride;
      for(j=crop_left; j<size_x-crop_right; j++)
      {
        memcpy(buf+(ipos+(j-crop_left)*symbol_size_in_bytes),&(imgX[i][j]), size);
      }
    }
  }
  else
  {
#if (IMGTYPE == 0)
    //if (sizeof(imgpel) == sizeof(char))
    {
      //memcpy(buf, &(imgX[0][0]), size_y * size_x * sizeof(imgpel));
      for(j=0; j<size_y; j++)
        memcpy(buf+j*iOutStride, imgX[j], size_x*sizeof(imgpel));
    }
    //else
#else
    {
      imgpel *cur_pixel;
      unsigned char *pDst; 
      for(j = 0; j < size_y; j++)
      {  
        cur_pixel = imgX[j];
        pDst = buf +j*iOutStride;
        for(i=0; i < size_x; i++)
          *(pDst++)=(unsigned char)*(cur_pixel++);
      }
    }
#endif
  }
}

/*!
 ************************************************************************
 * \brief
 *    Convert image plane to temporary buffer for file writing
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 ************************************************************************
 */
static void img2buf_byte (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int iOutStride)
{
  int twidth  = size_x - crop_left - crop_right;
  int theight = size_y - crop_top - crop_bottom;
  imgpel **img = &imgX[crop_top];
  int i;
  for(i = 0; i < theight; i++) 
  {
    memcpy(buf, *img++ + crop_left, twidth);
    buf += iOutStride;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Convert image plane to temporary buffer for file writing
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 ************************************************************************
 */
static void img2buf_endian (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int iOutStride)
{
  int i,j;
  unsigned char  ui8;
  uint16 tmp16, ui16;
  unsigned long  tmp32, ui32;

  //int twidth  = size_x - crop_left - crop_right;

  // big endian
  switch (symbol_size_in_bytes)
  {
  case 1:
    {
      for(i=crop_top;i<size_y-crop_bottom;i++)
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          ui8 = (unsigned char) (imgX[i][j]);
          buf[(j-crop_left+((i-crop_top)*iOutStride))] = ui8;
        }
        break;
    }
  case 2:
    {
      for(i=crop_top;i<size_y-crop_bottom;i++)
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          tmp16 = (uint16) (imgX[i][j]);
          ui16  = (uint16) ((tmp16 >> 8) | ((tmp16&0xFF)<<8));
          memcpy(buf+((j-crop_left+((i-crop_top)*iOutStride))*2),&(ui16), 2);
        }
        break;
    }
  case 4:
    {
      for(i=crop_top;i<size_y-crop_bottom;i++)
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          tmp32 = (unsigned long) (imgX[i][j]);
          ui32  = (unsigned long) (((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24));
          memcpy(buf+((j-crop_left+((i-crop_top)*iOutStride))*4),&(ui32), 4);
        }
        break;
    }
  default:
    {
      error ("writing only to formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
      break;
    }
  }  
}

#if 0//MFCD_POST

#ifndef CLIP
  #define CLIP(x,min,max) ( (x)<(min)?(min):((x)>(max)?(max):(x)) )
#endif

void post_dilation_filter(imgpel **high_imgY, imgpel *buffer_imgY, int width, int height)
{
#if (WRITE_OUT_PIC_BUG_FIX)
  imgpel *buffer =malloc(width*height*sizeof(imgpel));
#else
  imgpel *buffer = buffer_imgY;
#endif
  int width_out= width; 
  int height_out= height; 

  int width_out_minus_1 = width_out-1;
  int height_out_minus_1 = height_out-1;

  int i, j;
  int idx_x1, idx_x2, idx_x3;
  int idx_y1, idx_y2, idx_y3;
  imgpel *high1, *high2, *high3;
  imgpel value;

  for(j = 0; j < height_out; j++) {
    idx_y1 = CLIP(j-1, 0, height_out_minus_1);
    idx_y2 = j;
    idx_y3 = CLIP(j+1, 0, height_out_minus_1);

    high1 = high_imgY[idx_y1];
    high2 = high_imgY[idx_y2];
    high3 = high_imgY[idx_y3];

    for(i = 0; i < width_out; i++) {
      idx_x1 = CLIP(i-1, 0, width_out_minus_1);
      idx_x2 = i;
      idx_x3 = CLIP(i+1, 0, width_out_minus_1);

      value = high1[idx_x1];
      if(value < high1[idx_x2]) value = high1[idx_x2];
      if(value < high1[idx_x3]) value = high1[idx_x3];
      if(value < high2[idx_x1]) value = high2[idx_x1];
      if(value < high2[idx_x2]) value = high2[idx_x2];
      if(value < high2[idx_x3]) value = high2[idx_x3];
      if(value < high3[idx_x1]) value = high3[idx_x1];
      if(value < high3[idx_x2]) value = high3[idx_x2];
      if(value < high3[idx_x3]) value = high3[idx_x3];

      buffer[j*width_out + i] = value;
    }
  }

  for(j = 0; j < height_out; j++) {
    memcpy(high_imgY[j], &buffer[j*width_out], sizeof(imgpel)*width_out);
  }
#if (WRITE_OUT_PIC_BUG_FIX)
  free(buffer);
#endif
}
#endif
#if (MFCD_REDUCED_RES)
/*!
 ************************************************************************
 * \brief
 *    Convert image from coding resolution to original resolution to support reduced_resolution coding in 3dv
 * \param imgX_low_res
 *    Pointer to low resolution picture
 * \param imgX_ori_res
 *    Pointer to original resolution picture
 * \param component
 *    specify the image component(Y,U,V)
 * \param upsampling_params
*     parameters for upsampling
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 * \param size_x_ori
 *    the original width
 * \param size_y_ori
 *    the original height
 ************************************************************************
 */
void img_upsampling(imgpel**imgX_low_res,imgpel**imgX_ori_res, int component, int method, ResizeParameters* upsampling_params, 
          int size_x, int size_y,int crop_left, int crop_right, int crop_top, int crop_bottom)
{
  int h;
  int width  = size_x - crop_left - crop_right;
  int height = size_y - crop_top - crop_bottom;
  int cropped=crop_left||crop_right||crop_top||crop_bottom;

  imgpel** cropped_low_img;

  if(cropped)
  {
    get_mem2Dpel(&cropped_low_img,height,width);
    for(h=0;h<height;++h)
      memcpy(cropped_low_img[h],&(imgX_low_res[h+crop_top][crop_left]),width*sizeof(imgpel));
  }
  else

    cropped_low_img=imgX_low_res;

  if(0==component)
  {
    generic_upsampler(upsampling_params,cropped_low_img,NULL,NULL,imgX_ori_res,NULL,NULL,component,method);
  }
  else if(1==component)
  {
    generic_upsampler(upsampling_params,NULL,cropped_low_img,NULL,NULL,imgX_ori_res,NULL,component,method);
  }
  else
  {
    generic_upsampler(upsampling_params,NULL,NULL,cropped_low_img,NULL,NULL,imgX_ori_res,component,method);
  }


  if(cropped)
    free_mem2Dpel(cropped_low_img);

}
#endif

#if (PAIR_FIELDS_IN_OUTPUT)

void clear_picture(VideoParameters *p_Vid, StorablePicture *p);

/*!
 ************************************************************************
 * \brief
 *    output the pending frame buffer
 * \param p_out
 *    Output file
 ************************************************************************
 */
void flush_pending_output(VideoParameters *p_Vid, int p_out)
{
  if (p_Vid->pending_output_state != FRAME)
  {
    write_out_picture(p_Vid, p_Vid->pending_output, p_out);
  }

  if (p_Vid->pending_output->imgY)
  {
    free_mem2Dpel (p_Vid->pending_output->imgY);
    p_Vid->pending_output->imgY=NULL;
  }
  if (p_Vid->pending_output->imgUV)
  {
    free_mem3Dpel (p_Vid->pending_output->imgUV);
    p_Vid->pending_output->imgUV=NULL;
  }

  p_Vid->pending_output_state = FRAME;
}


/*!
 ************************************************************************
 * \brief
 *    Writes out a storable picture
 *    If the picture is a field, the output buffers the picture and tries
 *    to pair it with the next field.
 * \param p
 *    Picture to be written
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_picture(VideoParameters *p_Vid, StorablePicture *p, int p_out, int real_structure)
{
   int i, add;

  if (real_structure==FRAME)
  {
    
    flush_pending_output(p_Vid, p_out);
    write_out_picture(p_Vid, p, p_out);
    return;
  }
  if (real_structure == p_Vid->pending_output_state)
  {
    flush_pending_output(p_Vid, p_out);
    write_picture(p_Vid, p, p_out, real_structure);
    return;
  }

  if (p_Vid->pending_output_state == FRAME)
  {
    p_Vid->pending_output->size_x = p->size_x;
    p_Vid->pending_output->size_y = p->size_y;
    p_Vid->pending_output->size_x_cr = p->size_x_cr;
    p_Vid->pending_output->size_y_cr = p->size_y_cr;
    p_Vid->pending_output->chroma_format_idc = p->chroma_format_idc;

    p_Vid->pending_output->frame_mbs_only_flag = p->frame_mbs_only_flag;
    p_Vid->pending_output->frame_cropping_flag = p->frame_cropping_flag;
    if (p_Vid->pending_output->frame_cropping_flag)
    {
      p_Vid->pending_output->frame_crop_left_offset = p->frame_crop_left_offset;
      p_Vid->pending_output->frame_crop_right_offset = p->frame_crop_right_offset;
      p_Vid->pending_output->frame_crop_top_offset = p->frame_crop_top_offset;
      p_Vid->pending_output->frame_crop_bottom_offset = p->frame_crop_bottom_offset;
    }

    get_mem2Dpel (&(p_Vid->pending_output->imgY), p_Vid->pending_output->size_y, p_Vid->pending_output->size_x);
    get_mem3Dpel (&(p_Vid->pending_output->imgUV), 2, p_Vid->pending_output->size_y_cr, p_Vid->pending_output->size_x_cr);

    clear_picture(p_Vid, p_Vid->pending_output);

    // copy first field
    if (real_structure == TOP_FIELD)
    {
      add = 0;
    }
    else
    {
      add = 1;
    }

    for (i=0; i<p_Vid->pending_output->size_y; i+=2)
    {
      memcpy(p_Vid->pending_output->imgY[(i+add)], p->imgY[(i+add)], p->size_x * sizeof(imgpel));
    }
    for (i=0; i<p_Vid->pending_output->size_y_cr; i+=2)
    {
      memcpy(p_Vid->pending_output->imgUV[0][(i+add)], p->imgUV[0][(i+add)], p->size_x_cr * sizeof(imgpel));
      memcpy(p_Vid->pending_output->imgUV[1][(i+add)], p->imgUV[1][(i+add)], p->size_x_cr * sizeof(imgpel));
    }
    p_Vid->pending_output_state = real_structure;
  }
  else
  {
    if (  (p_Vid->pending_output->size_x!=p->size_x) || (p_Vid->pending_output->size_y!= p->size_y)
       || (p_Vid->pending_output->frame_mbs_only_flag != p->frame_mbs_only_flag)
       || (p_Vid->pending_output->frame_cropping_flag != p->frame_cropping_flag)
       || ( p_Vid->pending_output->frame_cropping_flag &&
            (  (p_Vid->pending_output->frame_crop_left_offset   != p->frame_crop_left_offset)
             ||(p_Vid->pending_output->frame_crop_right_offset  != p->frame_crop_right_offset)
             ||(p_Vid->pending_output->frame_crop_top_offset    != p->frame_crop_top_offset)
             ||(p_Vid->pending_output->frame_crop_bottom_offset != p->frame_crop_bottom_offset)
            )
          )
       )
    {
      flush_pending_output(p_Vid, p_out);
      write_picture (p_Vid, p, p_out, real_structure);
      return;
    }
    // copy second field
    if (real_structure == TOP_FIELD)
    {
      add = 0;
    }
    else
    {
      add = 1;
    }

    for (i=0; i<p_Vid->pending_output->size_y; i+=2)
    {
      memcpy(p_Vid->pending_output->imgY[(i+add)], p->imgY[(i+add)], p->size_x * sizeof(imgpel));
    }
    for (i=0; i<p_Vid->pending_output->size_y_cr; i+=2)
    {
      memcpy(p_Vid->pending_output->imgUV[0][(i+add)], p->imgUV[0][(i+add)], p->size_x_cr * sizeof(imgpel));
      memcpy(p_Vid->pending_output->imgUV[1][(i+add)], p->imgUV[1][(i+add)], p->size_x_cr * sizeof(imgpel));
    }

    flush_pending_output(p_Vid, p_out);
  }
}

#else

/*!
 ************************************************************************
 * \brief
 *    Writes out a storable picture without doing any output modifications
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param p
 *    Picture to be written
 * \param p_out
 *    Output file
 * \param real_structure
 *    real picture structure
 ************************************************************************
 */
void write_picture(VideoParameters *p_Vid, StorablePicture *p, int p_out, int real_structure)
{    
    write_out_picture(p_Vid, p, p_out);
}


#endif

static void allocate_p_dec_pic(VideoParameters *p_Vid, DecodedPicList *pDecPic, StorablePicture *p, int iLumaSize, int iFrameSize, int iLumaSizeX, int iLumaSizeY, int iChromaSizeX, int iChromaSizeY)
{
  int symbol_size_in_bytes = ((p_Vid->pic_unit_bitsize_on_disk+7) >> 3);
  
  if(pDecPic->pY)
    mem_free(pDecPic->pY);
  pDecPic->iBufSize = iFrameSize;
  pDecPic->pY = mem_malloc(pDecPic->iBufSize);
  pDecPic->pU = pDecPic->pY+iLumaSize;
  pDecPic->pV = pDecPic->pU + ((iFrameSize-iLumaSize)>>1);
  //init;
  pDecPic->iYUVFormat = p->chroma_format_idc;
  pDecPic->iYUVStorageFormat = 0;
  pDecPic->iBitDepth = p_Vid->pic_unit_bitsize_on_disk;
  pDecPic->iWidth = iLumaSizeX; //p->size_x;
  pDecPic->iHeight = iLumaSizeY; //p->size_y;
  pDecPic->iYBufStride = iLumaSizeX*symbol_size_in_bytes; //p->size_x *symbol_size_in_bytes;
  pDecPic->iUVBufStride = iChromaSizeX*symbol_size_in_bytes; //p->size_x_cr*symbol_size_in_bytes;
}

/*!
************************************************************************
* \brief
*    Writes out a storable picture
*
* \param p_Vid
*      image decoding parameters for current picture
* \param p
*    Picture to be written
* \param p_out
*    Output file
************************************************************************
*/
static void write_out_picture(VideoParameters *p_Vid, StorablePicture *p, int p_out)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  DecodedPicList *pDecPic;

  static const int SubWidthC  [4]= { 1, 2, 2, 1};
  static const int SubHeightC [4]= { 1, 2, 1, 1};
#if (MFC_DEPTH_DEC)
  int is_depth = p->is_depth;
#endif 

  int crop_left, crop_right, crop_top, crop_bottom;
  int symbol_size_in_bytes = ((p_Vid->pic_unit_bitsize_on_disk+7) >> 3);
#if (MFC_DEPTH_DEC)
  int rgb_output =  p_Vid->p_EncodePar[is_depth][p->layer_id]->rgb_output; //(p_Vid->active_sps->vui_seq_parameters.matrix_coefficients==0);
#else
  int rgb_output =  p_Vid->p_EncodePar[p->layer_id]->rgb_output; //(p_Vid->active_sps->vui_seq_parameters.matrix_coefficients==0);
#endif
  unsigned char *buf;
  //int iPicSizeTab[4] = {2, 3, 4, 6};
  int iLumaSize, iFrameSize;
  int iLumaSizeX, iLumaSizeY;
  int iChromaSizeX, iChromaSizeY;
#if (MFCD_REDUCED_RES)
  imgpel** imgX_linear_Y;  
  imgpel** imgX_ori_res=NULL;
#if MFCD_INTERLACE
  imgpel** imgX_linear_Y_top;
  imgpel** imgX_linear_Y_bottom;
  imgpel** imgX_ori_res_top   = NULL;
  imgpel** imgX_ori_res_bottom= NULL;
#endif
#endif

  int ret;

#if (MFC_DEPTH_DEC)
  if(!p_Vid->p_Inp->OutputYUVRecFile)
	  return ;
#endif
  if (p->non_existing)
    return;

#if (MFCD_REDUCED_RES)
 

#if MFCD_INTERLACE
  if((p->reduced_res)&&((!p->frame_mbs_only_flag)||(!p_Vid->is_texture_frame)))
    p->size_y_ori=p->size_y_ori*2;
#endif
  if(p->reduced_res)
  {
    get_mem2Dpel(&imgX_ori_res,p->size_y_ori,p->size_x_ori);
#if MFCD_INTERLACE
    get_mem2Dpel(&imgX_ori_res_top,   p->size_y_ori/2,p->size_x_ori);
    get_mem2Dpel(&imgX_ori_res_bottom,p->size_y_ori/2,p->size_x_ori);
#endif
  }

  imgX_linear_Y = p->imgY;

#if MFCD_INTERLACE 
  if(p->is_depth && (p->frame_mbs_only_flag!=1))
  {
    get_mem2Dpel(&imgX_linear_Y_top,   p->size_y/2,p->size_x);
    get_mem2Dpel(&imgX_linear_Y_bottom,p->size_y/2,p->size_x);
  }
#endif
#endif

#if (ENABLE_OUTPUT_TONEMAPPING)
  // note: this tone-mapping is working for RGB format only. Sharp
  if (p->seiHasTone_mapping && rgb_output)
  {
    //printf("output frame %d with tone model id %d\n",  p->frame_num, p->tone_mapping_model_id);
    symbol_size_in_bytes = (p->tonemapped_bit_depth>8)? 2 : 1;
    tone_map(p->imgY, p->tone_mapping_lut, p->size_x, p->size_y);
    tone_map(p->imgUV[0], p->tone_mapping_lut, p->size_x_cr, p->size_y_cr);
    tone_map(p->imgUV[1], p->tone_mapping_lut, p->size_x_cr, p->size_y_cr);
  }
#endif

  // should this be done only once?
  if (p->frame_cropping_flag)
  {
    crop_left   = SubWidthC [p->chroma_format_idc] * p->frame_crop_left_offset;
    crop_right  = SubWidthC [p->chroma_format_idc] * p->frame_crop_right_offset;
    crop_top    = SubHeightC[p->chroma_format_idc] * ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset;
    crop_bottom = SubHeightC[p->chroma_format_idc] * ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;
  }
  else
  {
    crop_left = crop_right = crop_top = crop_bottom = 0;
  }
#if (MFCD_REDUCED_RES)
  if((p->reduced_res) && (p_Vid->p_Inp->NormalizeResolutionDepth))
  {
    iChromaSizeX =  p->size_x_cr_ori;
    iChromaSizeY = p->size_y_cr_ori;
    iLumaSizeX = p->size_x_ori;
    iLumaSizeY = p->size_y_ori;
  }
  else
#endif
  {
	  iChromaSizeX =  p->size_x_cr- p->frame_crop_left_offset -p->frame_crop_right_offset;
	  iChromaSizeY = p->size_y_cr - ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset -( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;

	  iLumaSizeX = p->size_x - crop_left-crop_right;
	  iLumaSizeY = p->size_y - crop_top - crop_bottom;
  }
  iLumaSize  = iLumaSizeX * iLumaSizeY * symbol_size_in_bytes;
  iFrameSize = (iLumaSizeX * iLumaSizeY + 2 * (iChromaSizeX * iChromaSizeY)) * symbol_size_in_bytes; //iLumaSize*iPicSizeTab[p->chroma_format_idc]/2;

  //printf ("write frame size: %dx%d\n", p->size_x-crop_left-crop_right,p->size_y-crop_top-crop_bottom );

  // We need to further cleanup this function
  if (p_out == -1)
    return;



  // KS: this buffer should actually be allocated only once, but this is still much faster than the previous version
#if (MFC_DEPTH_DEC)
  if(p->is_depth)
	  p_Vid->pDecOuputPic=p_Vid->pDepthDecOuputPic;
  else
	  p_Vid->pDecOuputPic=p_Vid->pTextDecOuputPic;
#endif
  pDecPic = get_one_avail_dec_pic_from_list(p_Vid->pDecOuputPic, 0, 0);
  if( (pDecPic->pY == NULL)
	  || (pDecPic->iBufSize < iFrameSize)
	  )
    allocate_p_dec_pic(p_Vid, pDecPic, p, iLumaSize, iFrameSize, iLumaSizeX, iLumaSizeY, iChromaSizeX, iChromaSizeY);
#if (MVC_EXTENSION_ENABLE)
  {
    pDecPic->bValid = 1;
    pDecPic->iViewId = p->view_id >=0 ? p->view_id : -1;
  }
#else
  pDecPic->bValid = 1;
#endif
  
  pDecPic->iPOC = p->frame_poc;
#if (MFC_DEPTH_DEC)
	pDecPic->is_depth = is_depth;
#endif
  if (NULL==pDecPic->pY)
  {
    no_mem_exit("write_out_picture: buf");
  }

  
  if(rgb_output)
  {
    buf = malloc (p->size_x * p->size_y * symbol_size_in_bytes);
    crop_left   = p->frame_crop_left_offset;
    crop_right  = p->frame_crop_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;

    #if MFC_JM_CODE_CHANGE
    p_Vid->img2buf (p->outUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iYBufStride);
    #else
    p_Vid->img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iYBufStride);
    #endif

    if (p_out >= 0)
    {
      ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
      if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to RGB file", 500);
      }
    }

    if (p->frame_cropping_flag)
    {
      crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_crop_left_offset;
      crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_crop_right_offset;
      crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset;
      crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;
    }
    else
    {
      crop_left = crop_right = crop_top = crop_bottom = 0;
    }
    if(buf) 
      free(buf);
  }

  buf = (pDecPic->bValid==1)? pDecPic->pY: pDecPic->pY+iLumaSizeX*symbol_size_in_bytes;
#if (MFCD_REDUCED_RES)
  if((p->reduced_res) && (p_Vid->p_Inp->NormalizeResolutionDepth) && p->is_depth)
  {
#if MFCD_INTERLACE
	  if(p->frame_mbs_only_flag==1)
#endif
		  img_upsampling(imgX_linear_Y,imgX_ori_res,IMG_Y,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
		  p->size_x, p->size_y,crop_left, crop_right, crop_top, crop_bottom);
#if MFCD_INTERLACE
	  else
	  {
		  int i;
		  for (i=0; i<(p->size_y>>1); i++)//Extract Top and Bottom field from a frame
		  {
			  memcpy(imgX_linear_Y_top[i],    imgX_linear_Y[i*2],   p->size_x*sizeof(imgpel));
			  memcpy(imgX_linear_Y_bottom[i], imgX_linear_Y[i*2+1], p->size_x*sizeof(imgpel));
		  }

		  p->upsampling_params->input_height = p->upsampling_params->input_height >> 1;
		  p->upsampling_params->output_heigh = p->upsampling_params->output_heigh >> 1;
		  p->upsampling_params->crop_h = p->upsampling_params->crop_h >> 1;
		  img_upsampling(imgX_linear_Y_top,imgX_ori_res_top,IMG_Y,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
			  p->size_x, p->size_y/2,crop_left, crop_right, crop_top, crop_bottom);
		  img_upsampling(imgX_linear_Y_bottom,imgX_ori_res_bottom,IMG_Y,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
			  p->size_x, p->size_y/2,crop_left, crop_right, crop_top, crop_bottom);

		  p->upsampling_params->input_height = p->upsampling_params->input_height << 1;
		  p->upsampling_params->output_heigh = p->upsampling_params->output_heigh << 1;
		  p->upsampling_params->crop_h = p->upsampling_params->crop_h << 1;
		  for (i=0; i<(p->size_y_ori>>1); i++)
		  {
			  memcpy(imgX_ori_res[i*2],     imgX_ori_res_top[i]   , p->size_x_ori*sizeof(imgpel));
			  memcpy(imgX_ori_res[i*2 + 1], imgX_ori_res_bottom[i], p->size_x_ori*sizeof(imgpel));
		  }
	  }  
#endif
	  p_Vid->img2buf (imgX_ori_res, buf, p->size_x_ori, p->size_y_ori, symbol_size_in_bytes, 0, 0, 0, 0, pDecPic->iYBufStride);

	  if(p_out >=0)
	  {
		  ret = write(p_out, buf, (p->size_y_ori)*(p->size_x_ori)*symbol_size_in_bytes);
		  if (ret != ((p->size_y_ori)*(p->size_x_ori)*symbol_size_in_bytes))
		  {
			  error ("write_out_picture: error writing to YUV file", 500);
		  }
	  }
  }
  else
#endif
  {
#if MFC_JM_CODE_CHANGE 
      p_Vid->img2buf (p->outY, buf, p->size_x, p->size_y, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iYBufStride);
#else
      p_Vid->img2buf (p->imgY, buf, p->size_x, p->size_y, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iYBufStride);
#endif

      if(p_out >=0)
      {
          ret = write(p_out, buf, (p->size_y-crop_bottom-crop_top)*(p->size_x-crop_right-crop_left)*symbol_size_in_bytes);
          if (ret != ((p->size_y-crop_bottom-crop_top)*(p->size_x-crop_right-crop_left)*symbol_size_in_bytes))
          {
              error ("write_out_picture: error writing to YUV file", 500);
          }
      }
  }

  if (p->chroma_format_idc!=YUV400)
  {
    crop_left   = p->frame_crop_left_offset;
    crop_right  = p->frame_crop_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;
    buf = (pDecPic->bValid==1)? pDecPic->pU : pDecPic->pU + iChromaSizeX*symbol_size_in_bytes;
#if (MFCD_REDUCED_RES)
    if((p->reduced_res) && (p_Vid->p_Inp->NormalizeResolutionDepth) && p->is_depth)
    {
#if MFCD_INTERLACE
      if(p->frame_mbs_only_flag==1)
#endif
        img_upsampling(p->imgUV[0],imgX_ori_res,IMG_U,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
          p->size_x_cr, p->size_y_cr,crop_left, crop_right, crop_top, crop_bottom);
#if MFCD_INTERLACE
      else
      {
        int i;
        for (i=0; i<(p->size_y_cr>>1); i++)//Extract Top and Bottom field from a frame
        {
          memcpy(imgX_linear_Y_top[i],    p->imgUV[0][i*2],   p->size_x_cr*sizeof(imgpel));
          memcpy(imgX_linear_Y_bottom[i], p->imgUV[0][i*2+1], p->size_x_cr*sizeof(imgpel));
        }

        p->upsampling_params->input_height = p->upsampling_params->input_height >> 1;
        p->upsampling_params->output_heigh = p->upsampling_params->output_heigh >> 1;
        p->upsampling_params->crop_h = p->upsampling_params->crop_h >> 1;
        img_upsampling(imgX_linear_Y_top,imgX_ori_res_top,IMG_U,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
          p->size_x_cr, p->size_y_cr/2,crop_left, crop_right, crop_top, crop_bottom);
        img_upsampling(imgX_linear_Y_bottom,imgX_ori_res_bottom,IMG_U,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
          p->size_x_cr, p->size_y_cr/2,crop_left, crop_right, crop_top, crop_bottom);

        p->upsampling_params->input_height = p->upsampling_params->input_height << 1;
        p->upsampling_params->output_heigh = p->upsampling_params->output_heigh << 1;
        p->upsampling_params->crop_h = p->upsampling_params->crop_h << 1;
        for (i=0; i<(p->size_y_cr_ori>>1); i++)
        {
          memcpy(imgX_ori_res[i*2],     imgX_ori_res_top[i]   , p->size_x_cr_ori*sizeof(imgpel));
          memcpy(imgX_ori_res[i*2 + 1], imgX_ori_res_bottom[i], p->size_x_cr_ori*sizeof(imgpel));
        }
      }
#endif
	  p_Vid->img2buf (imgX_ori_res, buf, p->size_x_cr_ori, p->size_y_cr_ori, symbol_size_in_bytes, 0, 0, 0, 0, pDecPic->iUVBufStride);
	  if(p_out >= 0)
	  {
		  ret = write(p_out, buf, (p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes);
		  if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes))
		  {
			  error ("write_out_picture: error writing to YUV file", 500);
		  }
	  }
	}
	else
#endif
	{
    #if MFC_JM_CODE_CHANGE
    p_Vid->img2buf (p->outUV[0], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iUVBufStride);
    #else
    p_Vid->img2buf (p->imgUV[0], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iUVBufStride);
    #endif

    if(p_out >= 0)
    {
      ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes);
      if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV file", 500);
      }
    }
	}

    if (!rgb_output)
    {
      buf = (pDecPic->bValid==1)? pDecPic->pV : pDecPic->pV + iChromaSizeX*symbol_size_in_bytes;
#if (MFCD_REDUCED_RES)
	  if((p->reduced_res) && (p_Vid->p_Inp->NormalizeResolutionDepth) && p->is_depth)
      {
#if MFCD_INTERLACE
        if(p->frame_mbs_only_flag==1)
#endif
          img_upsampling(p->imgUV[1],imgX_ori_res,IMG_V,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
            p->size_x_cr, p->size_y_cr,crop_left, crop_right, crop_top, crop_bottom);
#if MFCD_INTERLACE
        else
        {
          int i;
          for (i=0; i<(p->size_y_cr>>1); i++)//Extract Top and Bottom field from a frame
          {
            memcpy(imgX_linear_Y_top[i],    p->imgUV[1][i*2],   p->size_x_cr*sizeof(imgpel));
            memcpy(imgX_linear_Y_bottom[i], p->imgUV[1][i*2+1], p->size_x_cr*sizeof(imgpel));
          }

          p->upsampling_params->input_height = p->upsampling_params->input_height >> 1;
          p->upsampling_params->output_heigh = p->upsampling_params->output_heigh >> 1;
          p->upsampling_params->crop_h = p->upsampling_params->crop_h >> 1;
          img_upsampling(imgX_linear_Y_top,imgX_ori_res_top,IMG_V,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
                         p->size_x_cr, p->size_y_cr/2,crop_left, crop_right, crop_top, crop_bottom);
          img_upsampling(imgX_linear_Y_bottom,imgX_ori_res_bottom,IMG_V,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
                         p->size_x_cr, p->size_y_cr/2,crop_left, crop_right, crop_top, crop_bottom);

          p->upsampling_params->input_height = p->upsampling_params->input_height << 1;
          p->upsampling_params->output_heigh = p->upsampling_params->output_heigh << 1;
          p->upsampling_params->crop_h = p->upsampling_params->crop_h << 1;
          for (i=0; i<(p->size_y_cr_ori>>1); i++)
          {
            memcpy(imgX_ori_res[i*2],     imgX_ori_res_top[i]   , p->size_x_cr_ori*sizeof(imgpel));
            memcpy(imgX_ori_res[i*2 + 1], imgX_ori_res_bottom[i], p->size_x_cr_ori*sizeof(imgpel));
          }
        }
#endif
        p_Vid->img2buf (imgX_ori_res, buf, p->size_x_cr_ori, p->size_y_cr_ori, symbol_size_in_bytes, 0, 0, 0, 0, pDecPic->iUVBufStride);
		  if(p_out >= 0)
		  {
			  ret = write(p_out, buf, (p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes);
			  if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes))
			  {
				  error ("write_out_picture: error writing to YUV file", 500);
			  }
		  }
	  }
	  else
#endif
	  {
      #if MFC_JM_CODE_CHANGE
      p_Vid->img2buf (p->outUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iUVBufStride);
      #else
      p_Vid->img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, pDecPic->iUVBufStride);
      #endif
      if(p_out >= 0)
      {
        ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
        if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
        {
          error ("write_out_picture: error writing to YUV file", 500);
        }
      }
	  }
    }
  }
  else
  {
#if (MFC_DEPTH_DEC)
    if (p_Inp->write_uv[is_depth])
#else
    if (p_Inp->write_uv)
#endif
    {
      int i,j;
      imgpel cr_val = (imgpel) (1<<(p_Vid->bitdepth_luma - 1));

#if (MFCD_REDUCED_RES)
      if((p->reduced_res) && (p_Vid->p_Inp->NormalizeResolutionDepth) && p->is_depth)
      {
        get_mem3Dpel (&(p->imgUV), 1, p->size_y_ori/2, p->size_x_ori/2);

        for (j=0; j<p->size_y_ori/2; j++)
        {
          for (i=0; i<p->size_x_ori/2; i++)
          {
            p->imgUV[0][j][i]=cr_val;
          }
        }

        // fake out U=V=128 to make a YUV 4:2:0 stream
        buf = malloc (p->size_x_ori*p->size_y_ori*symbol_size_in_bytes);
        p_Vid->img2buf (p->imgUV[0], buf, p->size_x_ori/2, p->size_y_ori/2, symbol_size_in_bytes, 0, 0, 0, 0, pDecPic->iYBufStride/2);

        ret = write(p_out, buf, symbol_size_in_bytes * (p->size_y_ori)/2 * (p->size_x_ori)/2 );
        if (ret != (symbol_size_in_bytes * (p->size_y_ori)/2 * (p->size_x_ori)/2))
        {
          error ("write_out_picture: error writing to YUV file", 500);
        }
        ret = write(p_out, buf, symbol_size_in_bytes * (p->size_y_ori)/2 * (p->size_x_ori)/2 );
        if (ret != (symbol_size_in_bytes * (p->size_y_ori)/2 * (p->size_x_ori)/2))
        {
          error ("write_out_picture: error writing to YUV file", 500);
        }

      }
      else
#endif
      {
      get_mem3Dpel (&(p->imgUV), 1, p->size_y/2, p->size_x/2);
      
      for (j=0; j<p->size_y/2; j++)
      {
        for (i=0; i<p->size_x/2; i++)
        {
          p->imgUV[0][j][i]=cr_val;
        }
      }
      // fake out U=V=128 to make a YUV 4:2:0 stream
      buf = malloc (p->size_x*p->size_y*symbol_size_in_bytes);
      p_Vid->img2buf (p->imgUV[0], buf, p->size_x/2, p->size_y/2, symbol_size_in_bytes, crop_left/2, crop_right/2, crop_top/2, crop_bottom/2, pDecPic->iYBufStride/2);

      ret = write(p_out, buf, symbol_size_in_bytes * (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left)/2 );
      if (ret != (symbol_size_in_bytes * (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left)/2))
      {
        error ("write_out_picture: error writing to YUV file", 500);
      }
      ret = write(p_out, buf, symbol_size_in_bytes * (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left)/2 );
      if (ret != (symbol_size_in_bytes * (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left)/2))
      {
        error ("write_out_picture: error writing to YUV file", 500);
      }
      }
      free(buf);
      free_mem3Dpel(p->imgUV);
      p->imgUV=NULL;
    }
  }

#if MFCD_INTERLACE
  if(p->is_depth && (p->frame_mbs_only_flag!=1))
  {
    free_mem2Dpel(imgX_linear_Y_top);
    free_mem2Dpel(imgX_linear_Y_bottom);
  }
#endif
#if (MFCD_REDUCED_RES)
  
  if(p->reduced_res)
  {
    free_mem2Dpel(imgX_ori_res);
#if MFCD_INTERLACE
    free_mem2Dpel(imgX_ori_res_top);
    free_mem2Dpel(imgX_ori_res_bottom);
#endif
  }
#endif
  //free(buf);
 if(p_out >=0)
   pDecPic->bValid = 0;

  //  fsync(p_out);
}

/*!
 ************************************************************************
 * \brief
 *    Initialize output buffer for direct output
 ************************************************************************
 */
void init_out_buffer(VideoParameters *p_Vid)
{
  p_Vid->out_buffer = alloc_frame_store();  

#if (PAIR_FIELDS_IN_OUTPUT)
  p_Vid->pending_output = calloc (sizeof(StorablePicture), 1);
  if (NULL==p_Vid->pending_output) no_mem_exit("init_out_buffer");
  p_Vid->pending_output->imgUV = NULL;
  p_Vid->pending_output->imgY  = NULL;
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Uninitialize output buffer for direct output
 ************************************************************************
 */
void uninit_out_buffer(VideoParameters *p_Vid)
{
  free_frame_store(p_Vid->out_buffer);
  p_Vid->out_buffer=NULL;
#if (PAIR_FIELDS_IN_OUTPUT)
  flush_pending_output(p_Vid, p_Vid->p_out);
  free (p_Vid->pending_output);
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Initialize picture memory with (Y:0,U:128,V:128)
 ************************************************************************
 */
void clear_picture(VideoParameters *p_Vid, StorablePicture *p)
{
  int i,j;

  for(i=0;i<p->size_y;i++)
  {
    for (j=0; j<p->size_x; j++)
      p->imgY[i][j] = (imgpel) p_Vid->dc_pred_value_comp[0];
  }
  for(i=0;i<p->size_y_cr;i++)
  {
    for (j=0; j<p->size_x_cr; j++)
      p->imgUV[0][i][j] = (imgpel) p_Vid->dc_pred_value_comp[1];
  }
  for(i=0;i<p->size_y_cr;i++)
  {
    for (j=0; j<p->size_x_cr; j++)
      p->imgUV[1][i][j] = (imgpel) p_Vid->dc_pred_value_comp[2];
  }
}

/*!
 ************************************************************************
 * \brief
 *    Write out not paired direct output fields. A second empty field is generated
 *    and combined into the frame buffer.
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param fs
 *    FrameStore that contains a single field
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_unpaired_field(VideoParameters *p_Vid, FrameStore* fs, int p_out)
{
  StorablePicture *p;
  assert (fs->is_used<3);

  if(fs->is_used & 0x01)
  {
    // we have a top field
    // construct an empty bottom field
    p = fs->top_field;
    fs->bottom_field = alloc_storable_picture(p_Vid, BOTTOM_FIELD, p->size_x, 2*p->size_y, p->size_x_cr, 2*p->size_y_cr, 1);
    fs->bottom_field->chroma_format_idc = p->chroma_format_idc;
    clear_picture(p_Vid, fs->bottom_field);
    dpb_combine_field_yuv(p_Vid, fs);
#if (MVC_EXTENSION_ENABLE)
    fs->frame->view_id = fs->view_id;
#endif
    write_picture (p_Vid, fs->frame, p_out, TOP_FIELD);
  }

  if(fs->is_used & 0x02)
  {
    // we have a bottom field
    // construct an empty top field
    p = fs->bottom_field;
    fs->top_field = alloc_storable_picture(p_Vid, TOP_FIELD, p->size_x, 2*p->size_y, p->size_x_cr, 2*p->size_y_cr, 1);
    fs->top_field->chroma_format_idc = p->chroma_format_idc;
    clear_picture(p_Vid, fs->top_field);
    fs ->top_field->frame_cropping_flag = fs->bottom_field->frame_cropping_flag;
    if(fs ->top_field->frame_cropping_flag)
    {
      fs ->top_field->frame_crop_top_offset = fs->bottom_field->frame_crop_top_offset;
      fs ->top_field->frame_crop_bottom_offset = fs->bottom_field->frame_crop_bottom_offset;
      fs ->top_field->frame_crop_left_offset = fs->bottom_field->frame_crop_left_offset;
      fs ->top_field->frame_crop_right_offset = fs->bottom_field->frame_crop_right_offset;
    }
    dpb_combine_field_yuv(p_Vid, fs);
#if (MVC_EXTENSION_ENABLE)
    fs->frame->view_id = fs->view_id;
#endif
    write_picture (p_Vid, fs->frame, p_out, BOTTOM_FIELD);
  }

  fs->is_used = 3;
}

/*!
 ************************************************************************
 * \brief
 *    Write out unpaired fields from output buffer.
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param p_out
 *    Output file
 ************************************************************************
 */
void flush_direct_output(VideoParameters *p_Vid, int p_out)
{
  write_unpaired_field(p_Vid, p_Vid->out_buffer, p_out);

  free_storable_picture(p_Vid->out_buffer->frame);
  p_Vid->out_buffer->frame = NULL;
  free_storable_picture(p_Vid->out_buffer->top_field);
  p_Vid->out_buffer->top_field = NULL;
  free_storable_picture(p_Vid->out_buffer->bottom_field);
  p_Vid->out_buffer->bottom_field = NULL;
  p_Vid->out_buffer->is_used = 0;
}


/*!
 ************************************************************************
 * \brief
 *    Write a frame (from FrameStore)
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param fs
 *    FrameStore containing the frame
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_stored_frame( VideoParameters *p_Vid, FrameStore *fs, int p_out)
{
  // make sure no direct output field is pending
  flush_direct_output(p_Vid, p_out);

  if (fs->is_used<3)
  {
    write_unpaired_field(p_Vid, fs, p_out);
  }
  else
  {
    if (fs->recovery_frame)
      p_Vid->recovery_flag = 1;
    if ((!p_Vid->non_conforming_stream) || p_Vid->recovery_flag)
      write_picture(p_Vid, fs->frame, p_out, FRAME);
  }

  fs->is_output = 1;
}

/*!
 ************************************************************************
 * \brief
 *    Directly output a picture without storing it in the DPB. Fields
 *    are buffered before they are written to the file.
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param p
 *    Picture for output
 * \param p_out
 *    Output file
 ************************************************************************
 */
void direct_output(VideoParameters *p_Vid, StorablePicture *p, int p_out)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
#if (MFC_DEPTH_DEC)
	int  voidx=GetVOIdx(p_Vid,p->view_id)   ;
#endif
  if (p->structure==FRAME)
  {
    // we have a frame (or complementary field pair)
    // so output it directly
    flush_direct_output(p_Vid, p_out);
    write_picture (p_Vid, p, p_out, FRAME);
    calculate_frame_no(p_Vid, p);
#if (MFC_DEPTH_DEC)
		if (-1 != p_Vid->p_ref_3dv[p->is_depth][voidx] && !p_Inp->silent)
			find_snr(p_Vid, p, &p_Vid->p_ref_3dv[p->is_depth][voidx]);
#else
    if (-1 != p_Vid->p_ref && !p_Inp->silent)
      find_snr(p_Vid, p, &p_Vid->p_ref);
#endif
    free_storable_picture(p);
    return;
  }

  if (p->structure == TOP_FIELD)
  {
    if (p_Vid->out_buffer->is_used &1)
      flush_direct_output(p_Vid, p_out);
    p_Vid->out_buffer->top_field = p;
    p_Vid->out_buffer->is_used |= 1;
  }

  if (p->structure == BOTTOM_FIELD)
  {
    if (p_Vid->out_buffer->is_used &2)
      flush_direct_output(p_Vid, p_out);
    p_Vid->out_buffer->bottom_field = p;
    p_Vid->out_buffer->is_used |= 2;
  }

  if (p_Vid->out_buffer->is_used == 3)
  {
    // we have both fields, so output them
    dpb_combine_field_yuv(p_Vid, p_Vid->out_buffer);
#if (MVC_EXTENSION_ENABLE)
    p_Vid->out_buffer->frame->view_id = p_Vid->out_buffer->view_id;
#endif
    write_picture (p_Vid, p_Vid->out_buffer->frame, p_out, FRAME);

    calculate_frame_no(p_Vid, p);
#if (MFC_DEPTH_DEC)
	if (-1 != p_Vid->p_ref_3dv[p->is_depth][voidx] && !p_Inp->silent)
      find_snr(p_Vid, p_Vid->out_buffer->frame, &p_Vid->p_ref_3dv[p->is_depth][voidx]);
#else
    if (-1 != p_Vid->p_ref && !p_Inp->silent)
      find_snr(p_Vid, p_Vid->out_buffer->frame, &p_Vid->p_ref);
#endif
    free_storable_picture(p_Vid->out_buffer->frame);
    p_Vid->out_buffer->frame = NULL;
    free_storable_picture(p_Vid->out_buffer->top_field);
    p_Vid->out_buffer->top_field = NULL;
    free_storable_picture(p_Vid->out_buffer->bottom_field);
    p_Vid->out_buffer->bottom_field = NULL;
    p_Vid->out_buffer->is_used = 0;
  }
}

