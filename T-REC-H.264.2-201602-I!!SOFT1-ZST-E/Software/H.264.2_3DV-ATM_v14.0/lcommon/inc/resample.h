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


#ifndef RESAMPLE_H
#define RESAMPLE_H

#if EXT3D

typedef struct   Resize_Parameters
{
  int input_width;
  int input_height;
  int output_width;
  int output_heigh;
  int crop_x0 ;
  int crop_y0 ;
  int crop_w ;
  int crop_h ;  
  int input_chroma_phase_shift_x ;
  int input_chroma_phase_shift_y ;
  int output_chroma_phase_shift_x ;
  int output_chroma_phase_shift_y ;

  int input_stride[3];  //input_stride Y U V
  int output_stride[3]; //output_stride Y U V

  int minValue;
  int maxValue;

  int down_sample;//1:downsampling ,0:upsampling

  imgpel* img_buffer[3];

  int img_stride;

  int* resize_buffer;
  int* resize_buffer_tmp;

  //!<0:Y 1:UV
  int* pos_x[2];
  int* pos_y[2];

  int NormalizeResolutionDepth;
}ResizeParameters;


extern int init_ImageResize(ResizeParameters* ResizeParams, 
                int input_width, int intput_height, int output_width, int output_heigh );

extern void destroy_ImageResize(ResizeParameters* pResizeParams);



extern void generic_upsampler(ResizeParameters* ResizeParams,
               imgpel**low_imgY,imgpel**low_imgU,imgpel**low_imgV,
               imgpel**high_imgY,imgpel**high_imgU,imgpel**high_imgV,int component,int method);
#endif

#endif
