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
#include "image.h"
#include "input.h"
#include "output.h"
#if (MFCD_REDUCED_RES)
#include "resample.h"
#include "memalloc.h"
#endif

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
void img2buf (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom)
{
  int i,j;

  int twidth  = size_x - crop_left - crop_right;
  int theight = size_y - crop_top - crop_bottom;
  int size = 0;
  unsigned char  ui8;
  uint16 tmp16, ui16;
  unsigned long  tmp32, ui32;

  if (( sizeof(char) == sizeof (imgpel)) && ( sizeof(char) == symbol_size_in_bytes))
  {
    // imgpel == pixel_in_file == 1 byte -> simple copy
    if (crop_left == 0 && crop_top == 0 && crop_right == 0 && crop_bottom == 0)
    {
      //memcpy(buf,&(imgX[0][0]), twidth * theight);
      for(j=0; j<theight; j++)
        memcpy(buf+j*twidth, imgX[j], twidth);
    }
    else
    {
      for(i=0;i<theight;i++)
        //memcpy(buf + crop_left + (i * twidth),&(imgX[i + crop_top][crop_left]), twidth); //? bug???
        memcpy(buf + (i*twidth), &(imgX[i + crop_top][crop_left]), twidth);
    }
  }
  else
  {
    // sizeof (imgpel) > sizeof(char)
    if (testEndian())
    {
      // big endian
      switch (symbol_size_in_bytes)
      {
      case 1:
        {
          for(i=crop_top;i<size_y-crop_bottom;i++)
            for(j=crop_left;j<size_x-crop_right;j++)
            {
              ui8 = (unsigned char) (imgX[i][j]);
              buf[(j-crop_left+((i-crop_top)*(twidth)))] = ui8;
            }
          break;
        }
      case 2:
        {
          for(i=crop_top;i<size_y-crop_bottom;i++)
            for(j=crop_left;j<size_x-crop_right;j++)
            {
              tmp16 = (uint16) (imgX[i][j]);
              ui16  = (tmp16 >> 8) | ((tmp16&0xFF)<<8);
              memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*2),&(ui16), 2);
            }
          break;
        }
      case 4:
        {
          for(i=crop_top;i<size_y-crop_bottom;i++)
            for(j=crop_left;j<size_x-crop_right;j++)
            {
              tmp32 = (unsigned long) (imgX[i][j]);
              ui32  = ((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24);
              memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*4),&(ui32), 4);
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
    else
    {
      // little endian
      if (sizeof (imgpel) < symbol_size_in_bytes)
      {
        // this should not happen. we should not have smaller imgpel than our source material.
        size = sizeof (imgpel);
        // clear buffer
        memset (buf, 0, (twidth*theight*symbol_size_in_bytes));
      }
      else
      {
        size = symbol_size_in_bytes;
      }

      for(i=crop_top;i<size_y-crop_bottom;i++)
      {
        int i_pos = (i-crop_top)*(twidth) - crop_left;
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          memcpy(buf+((j + i_pos)*symbol_size_in_bytes),&(imgX[i][j]), size);
        }
      }
    }
  }
}

#if (MFCD_REDUCED_RES)

#ifndef CLIP
  #define CLIP(x,min,max) ( (x)<(min)?(min):((x)>(max)?(max):(x)) )
#endif

void post_dilation_filter(imgpel **high_imgY, imgpel *buffer_imgY, int width, int height)
{
  imgpel *buffer = buffer_imgY;

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
}

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
/*!
 ************************************************************************
 * \brief
 *    Writes out a storable picture without doing any output modifications
 * \param p
 *    Picture to be written
 * \param output
 *    picture structure for output
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_picture(StorablePicture *p, FrameFormat *output, int p_out)
{
  write_out_picture(p, output, p_out);
}

/*!
 ************************************************************************
 * \brief
 *    Writes out a storable picture
 * \param p
 *    Picture to be written
 * \param output
 *    Output format
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_out_picture(StorablePicture *p, FrameFormat *output, int p_out)
{
  int SubWidthC  [4]= { 1, 2, 2, 1};
  int SubHeightC [4]= { 1, 2, 1, 1};

  int ret;

  int crop_left, crop_right, crop_top, crop_bottom;
  int symbol_size_in_bytes = output->pic_unit_size_shift3;
  Boolean rgb_output = (Boolean) (output->color_model != CM_YUV && output->yuv_format == YUV444);
  unsigned char *buf;
#if (MFCD_REDUCED_RES)
  imgpel** imgX_ori_res=NULL;
  imgpel** imgX_linear_Y;
#if MFCD_INTERLACE
  imgpel** imgX_ori_res_top    = NULL;
  imgpel** imgX_ori_res_bottom = NULL;
  imgpel** imgX_linear_Y_top;
  imgpel** imgX_linear_Y_bottom;
#endif
#endif

  if (p->non_existing)
    return;

  if (p_out == -1)
    return;

#if (MFCD_REDUCED_RES)
  if(p->reduced_res&&p->upsampling_params)
#if MFCD_INTERLACE
  {
#endif
    get_mem2Dpel(&imgX_ori_res,p->size_y_ori,p->size_x_ori);
#if MFCD_INTERLACE
    get_mem2Dpel(&imgX_ori_res_top,p->size_y_ori/2,p->size_x_ori);
    get_mem2Dpel(&imgX_ori_res_bottom,p->size_y_ori/2,p->size_x_ori);
  }
#endif
  imgX_linear_Y = p->imgY;

#if (MFCD_INTERLACE)
  if(p->frame_mbs_only_flag!=1)
  {
    get_mem2Dpel(&imgX_linear_Y_top,   p->size_y/2,p->size_x);
    get_mem2Dpel(&imgX_linear_Y_bottom,p->size_y/2,p->size_x);
  }
#endif
#endif
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

  //printf ("write frame size: %dx%d\n", p->size_x-crop_left-crop_right,p->size_y-crop_top-crop_bottom );

  // KS: this buffer should actually be allocated only once, but this is still much faster than the previous version
#if (MFCD_REDUCED_RES)
  if(p->reduced_res&&p->upsampling_params)
  {
    buf = malloc (p->size_x_ori * p->size_y_ori * symbol_size_in_bytes);
  }
  else
  {
    buf = malloc (p->size_x * p->size_y * symbol_size_in_bytes);
  }
#else
  buf = malloc (p->size_x * p->size_y * symbol_size_in_bytes);
#endif
  if (NULL==buf)
  {
    no_mem_exit("write_out_picture: buf");
  }

  if(rgb_output)
  {
    crop_left   = p->frame_crop_left_offset;
    crop_right  = p->frame_crop_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;

#if (MFCD_REDUCED_RES)
  if(p->reduced_res&&p->upsampling_params && p->upsampling_params->NormalizeResolutionDepth && p->is_depth )
  {
    img_upsampling(p->imgUV[1],imgX_ori_res,IMG_V,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
      p->size_x_cr, p->size_y_cr, crop_left, crop_right, crop_top, crop_bottom);
    img2buf(imgX_ori_res,buf,p->size_x_cr_ori,p->size_y_cr_ori,symbol_size_in_bytes, 0, 0, 0, 0);
    ret = write(p_out, buf, p->size_y_cr_ori*p->size_x_cr_ori*symbol_size_in_bytes);
    if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes))
    {
      error ("write_out_picture: error writing to RGB output file.", 500);
    }
  }
  else
  {
    img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
    ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
    if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
    {
      error ("write_out_picture: error writing to RGB output file.", 500);
    }
  }
#else
    img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
  ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
  if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
  {
    error ("write_out_picture: error writing to RGB output file.", 500);
  }
#endif

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
  }

#if (MFCD_REDUCED_RES)
  if(p->reduced_res&&p->upsampling_params&& p->upsampling_params->NormalizeResolutionDepth && p->is_depth )
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
    img2buf (imgX_ori_res, buf, p->size_x_ori, p->size_y_ori, symbol_size_in_bytes, 0, 0, 0, 0);
    ret = write(p_out, buf, (p->size_y_ori)*(p->size_x_ori)*symbol_size_in_bytes);
    if (ret != ((p->size_y_ori)*(p->size_x_ori)*symbol_size_in_bytes))
    {
      error ("write_out_picture: error writing to YUV output file.", 500);
    }
  }
  else
#endif
  {
  img2buf (p->imgY, buf, p->size_x, p->size_y, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
  ret = write(p_out, buf, (p->size_y-crop_bottom-crop_top)*(p->size_x-crop_right-crop_left)*symbol_size_in_bytes);
  if (ret != ((p->size_y-crop_bottom-crop_top)*(p->size_x-crop_right-crop_left)*symbol_size_in_bytes))
  {
    error ("write_out_picture: error writing to YUV output file.", 500);
  }
  }
  if (p->chroma_format_idc != YUV400)
  {
    crop_left   = p->frame_crop_left_offset;
    crop_right  = p->frame_crop_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_crop_bottom_offset;

#if (MFCD_REDUCED_RES)
    if(p->reduced_res&&p->upsampling_params&& p->upsampling_params->NormalizeResolutionDepth && p->is_depth )
    {
#if MFCD_INTERLACE
      if(p->frame_mbs_only_flag==1)
#endif 
        img_upsampling(p->imgUV[0],imgX_ori_res,IMG_U,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
          p->size_x_cr, p->size_y_cr, crop_left, crop_right, crop_top, crop_bottom);
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
      img2buf (imgX_ori_res, buf, p->size_x_cr_ori, p->size_y_cr_ori, symbol_size_in_bytes, 0, 0, 0, 0);
      ret = write(p_out, buf, (p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes);
      if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
    }
    else
    {
      img2buf (p->imgUV[0], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
      ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes);
      if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
    }
#else
    img2buf (p->imgUV[0], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
    ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes);
    if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes))
    {
      error ("write_out_picture: error writing to YUV output file.", 500);
    }
#endif

    if (!rgb_output)
    {
#if (MFCD_REDUCED_RES)
    if(p->reduced_res&&p->upsampling_params && p->upsampling_params->NormalizeResolutionDepth && p->is_depth )
    {
#if MFCD_INTERLACE
      if(p->frame_mbs_only_flag==1)
#endif
        img_upsampling(p->imgUV[1],imgX_ori_res,IMG_V,p->is_depth?BI_LINEAR:LANCZOS3,p->upsampling_params,
          p->size_x_cr, p->size_y_cr, crop_left, crop_right, crop_top, crop_bottom);
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
      img2buf (imgX_ori_res, buf, p->size_x_cr_ori, p->size_y_cr_ori, symbol_size_in_bytes, 0, 0, 0, 0);
      ret = write(p_out, buf, (p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes);
      if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
    }
    else
    {
      img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
      ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
      if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
    }
#else
      img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
      ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
      if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
#endif
    }
  }

#if (MFCD_FORCE_YUV_400)
  if(p->force_yuv400 == 1)
  {
    crop_left   >>= 1;
    crop_right  >>= 1;
    crop_top    >>= 1;
    crop_bottom >>= 1;
#if (MFCD_REDUCED_RES)
    if(p->reduced_res&&p->upsampling_params&& p->upsampling_params->NormalizeResolutionDepth && p->is_depth )
    {
      memset (buf, 128, ((p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes));
      ret = write(p_out, buf, (p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes);
      if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
    }
    else
#endif
    {
      memset (buf, 128, ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes));
      ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes);
      if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes))
      {
        error ("write_out_picture: error writing to YUV output file.", 500);
      }
    }

    if (!rgb_output)
    {
      //img2buf (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
#if (MFCD_REDUCED_RES)
      if(p->reduced_res&&p->upsampling_params&& p->upsampling_params->NormalizeResolutionDepth && p->is_depth )
      {
        memset (buf, 128, ((p->size_y_cr_ori)*(p->size_x_cr_ori)* symbol_size_in_bytes));
        ret = write(p_out, buf, (p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes);
        if (ret != ((p->size_y_cr_ori)*(p->size_x_cr_ori)*symbol_size_in_bytes))
        {
          error ("write_out_picture: error writing to YUV output file.", 500);
        }
      }
      else
#endif
      {
        memset (buf, 128, ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes));
        ret = write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
        if (ret != ((p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes))
        {
          error ("write_out_picture: error writing to YUV output file.", 500);
        }
      }
    }
  }

#if (MFCD_REDUCED_RES)
  if(imgX_ori_res)
    free_mem2Dpel(imgX_ori_res);
#if MFCD_INTERLACE
  if(imgX_ori_res_top)
    free_mem2Dpel(imgX_ori_res_top);
  if(imgX_ori_res_bottom)
    free_mem2Dpel(imgX_ori_res_bottom);
#endif
#endif

#if MFCD_INTERLACE
  if(p->frame_mbs_only_flag!=1)
  {
    free_mem2Dpel(imgX_linear_Y_top);
    free_mem2Dpel(imgX_linear_Y_bottom);
  }
#endif
#endif
  free(buf);

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
}

/*!
 ************************************************************************
 * \brief
 *    Uninitialize output buffer for direct output
 ************************************************************************
 */
void uninit_out_buffer(VideoParameters *p_Vid)
{
  free_frame_store(p_Vid, p_Vid->out_buffer);
  p_Vid->out_buffer=NULL;
}

/*!
 ************************************************************************
 * \brief
 *    Initialize picture memory with (Y:0,U:128,V:128)
 ************************************************************************
 */
void clear_picture(VideoParameters *p_Vid, StorablePicture *p)
{
  int i;

  if (p_Vid->bitdepth_luma == 8)
  {
    // this does not seem to be right since it seems to work only for imgpel == byte
    // should be fixed
    for(i=0;i<p->size_y;i++)
      memset(p->imgY[i], p_Vid->dc_pred_value_comp[0], p->size_x*sizeof(imgpel));
  }
  else
  {
    int j;
    for(i=0;i < p->size_y; i++)
    for(j=0;j < p->size_x; j++)
      p->imgY[i][j] = (imgpel) p_Vid->dc_pred_value_comp[0];
  }

  if (p_Vid->bitdepth_chroma == 8)
  {
    for(i=0;i<p->size_y_cr;i++)
      memset(p->imgUV[0][i], p_Vid->dc_pred_value_comp[1], p->size_x_cr*sizeof(imgpel));
    for(i=0;i<p->size_y_cr;i++)
      memset(p->imgUV[1][i], p_Vid->dc_pred_value_comp[2], p->size_x_cr*sizeof(imgpel));
  }
  else
  {  
   int k, j;
   for (k = 0; k < 2; k++)
   {
      for(i=0;i<p->size_y_cr;i++)
      for(j=0;j < p->size_x_cr; j++)
        p->imgUV[k][i][j] = (imgpel) p_Vid->dc_pred_value_comp[1];
   }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Write out not paired direct output fields. A second empty field is generated
 *    and combined into the frame buffer.
 * \param p_Vid
 *    point to VideoParameters
 * \param fs
 *    FrameStore that contains a single field
 * \param output
 *    Frame format to 
 * \param p_out
 *    Output file 
 ************************************************************************
 */
void write_unpaired_field(VideoParameters *p_Vid, FrameStore* fs, FrameFormat *output, int p_out)
{
  StorablePicture *p;
  assert (fs->is_used<3);
  if(fs->is_used &1)
  {
    // we have a top field
    // construct an empty bottom field
    p = fs->top_field;
    fs->bottom_field = alloc_storable_picture(p_Vid, BOTTOM_FIELD, p->size_x, p->size_y, p->size_x_cr, p->size_y_cr);
    fs->bottom_field->chroma_format_idc = p->chroma_format_idc;
    fs->bottom_field->chroma_mask_mv_x  = p->chroma_mask_mv_x;
    fs->bottom_field->chroma_mask_mv_y  = p->chroma_mask_mv_y;
    fs->bottom_field->chroma_shift_x    = p->chroma_shift_x;
    fs->bottom_field->chroma_shift_y    = p->chroma_shift_y;

    clear_picture(p_Vid, fs->bottom_field);
    dpb_combine_field_yuv(p_Vid, fs);
    write_picture (fs->frame, output, p_out);
  }

  if(fs->is_used &2)
  {
    // we have a bottom field
    // construct an empty top field
    p = fs->bottom_field;
    fs->top_field = alloc_storable_picture(p_Vid, TOP_FIELD, p->size_x, p->size_y, p->size_x_cr, p->size_y_cr);
    clear_picture(p_Vid, fs->top_field);
    fs->top_field->chroma_format_idc = p->chroma_format_idc;
    fs->top_field->chroma_mask_mv_x  = p->chroma_mask_mv_x;
    fs->top_field->chroma_mask_mv_y  = p->chroma_mask_mv_y;
    fs->top_field->chroma_shift_x    = p->chroma_shift_x;
    fs->top_field->chroma_shift_y    = p->chroma_shift_y;

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
    write_picture (fs->frame, output, p_out);
  }

  fs->is_used=3;
}

/*!
 ************************************************************************
 * \brief
 *    Write out unpaired fields from output buffer.
 * \param p_Vid
 *    VideoParameters structure
 * \param output
 *    FrameFormat structure for output
 * \param p_out
 *    Output file
 ************************************************************************
 */
void flush_direct_output(VideoParameters *p_Vid, FrameFormat *output, int p_out)
{
  write_unpaired_field(p_Vid, p_Vid->out_buffer, output, p_out);

  free_storable_picture(p_Vid, p_Vid->out_buffer->frame);
  p_Vid->out_buffer->frame = NULL;
  free_storable_picture(p_Vid, p_Vid->out_buffer->top_field);
  p_Vid->out_buffer->top_field = NULL;
  free_storable_picture(p_Vid, p_Vid->out_buffer->bottom_field);
  p_Vid->out_buffer->bottom_field = NULL;
  p_Vid->out_buffer->is_used = 0;
}


/*!
 ************************************************************************
 * \brief
 *    Write a frame (from FrameStore)
 * \param p_Vid
 *    VideoParameters structure
 * \param fs
 *    FrameStore containing the frame
 * \param output
 *    Frame format for output
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_stored_frame( VideoParameters *p_Vid, FrameStore *fs, FrameFormat *output, int p_out)
{
  // make sure no direct output field is pending
  flush_direct_output(p_Vid, output, p_out);

  if (fs->is_used<3)
  {
    write_unpaired_field(p_Vid, fs, output, p_out);
  }
  else
  {
    write_picture(fs->frame, output, p_out);
  }

  fs->is_output = 1;
}


/*!
 ************************************************************************
 * \brief
 *    Directly output a picture without storing it in the DPB. Fields
 *    are buffered before they are written to the file.
 * \param p_Vid
 *    VideoParameters structure
 * \param p
 *    Picture for output
 * \param output
 *    Frame format for output
 * \param p_out
 *    Output file
 ************************************************************************
 */
void direct_output(VideoParameters *p_Vid, StorablePicture *p, FrameFormat *output, int p_out)
{
  switch ( p->structure )
  {
  case FRAME:
    // we have a frame (or complementary field pair)
    // so output it directly
    flush_direct_output(p_Vid, output, p_out);
    write_picture (p, output, p_out);
    free_storable_picture(p_Vid, p);
    return;
    break;
  case TOP_FIELD:
    if (p_Vid->out_buffer->is_used &1)
      flush_direct_output(p_Vid, output, p_out);
    p_Vid->out_buffer->top_field = p;
    p_Vid->out_buffer->is_used |= 1;
    break;
  case BOTTOM_FIELD:
    if (p_Vid->out_buffer->is_used &2)
      flush_direct_output(p_Vid, output, p_out);
    p_Vid->out_buffer->bottom_field = p;
    p_Vid->out_buffer->is_used |= 2;
    break;
  default:
    printf("invalid picture type\n");
    break;
  }

  if (p_Vid->out_buffer->is_used == 3)
  {
    // we have both fields, so output them
    dpb_combine_field_yuv(p_Vid, p_Vid->out_buffer);
    write_picture (p_Vid->out_buffer->frame, output, p_out);
    free_storable_picture(p_Vid, p_Vid->out_buffer->frame);
    p_Vid->out_buffer->frame = NULL;
    free_storable_picture(p_Vid, p_Vid->out_buffer->top_field);
    p_Vid->out_buffer->top_field = NULL;
    free_storable_picture(p_Vid, p_Vid->out_buffer->bottom_field);
    p_Vid->out_buffer->bottom_field = NULL;
    p_Vid->out_buffer->is_used = 0;
  }
}


/*!
************************************************************************
* \brief
*    For adaptive frame/field coding remove dangling top field from direct
*    output frame version instead.
* \param p_Vid
*    Picture for output
* \param p
*    Picture for output
* \param output
*    Frame format for output
* \param p_out
*    Output file
************************************************************************
*/
void direct_output_paff(VideoParameters *p_Vid, StorablePicture *p, FrameFormat *output, int p_out)
{
  printf("Warning!!! Frame can't fit in DPB. Displayed out of sequence.\n");
  free_storable_picture(p_Vid, p_Vid->out_buffer->frame);
  p_Vid->out_buffer->frame = NULL;
  free_storable_picture(p_Vid, p_Vid->out_buffer->top_field);
  p_Vid->out_buffer->top_field = NULL;
  free_storable_picture(p_Vid, p_Vid->out_buffer->bottom_field);
  p_Vid->out_buffer->bottom_field = NULL;
  p_Vid->out_buffer->is_used = 0;

  direct_output(p_Vid, p, output, p_out);
}
