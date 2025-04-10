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




/*!
 *************************************************************************************
 * \file nonlinear_depth.c
 *
 * \brief
 *    Non-linear depth representation to allow more efficient compression of depth.
 *    Internally inside codec, depth disparity is stored as non linear function of input disparity.
 *    Example of such function is:
 *      disparity_internal = 255.0 * pow(disparity_linear/255.0, DepthPower)
 *    In order to work correctly, all tools that make use of depth 
 *    (for example: view synthesis, writing decoded/reconstructed output, joint_view_filter etc. )
 *    must recalculate linear disparity from reverse formula.
 *
 *    The exact shape of curves for non-linear representation is defined by means of line-segment-approximation.
 *    The first (0,0) and the last (255,255) nodes are predefined.
 *    Additional nodeds can be transmitted in SPS in form of deviations from straight-line curve (linear representation).
 *
 *    In most of places, linear-to-non-linear conversion is done through LUT tables,
 *    and in some cases (view synthesis etc) non-linearity has been incorporated
 *    into existing LUT tables.
 *    Macro definition for this tool: 
 *      in: defines.h (x2), header.h
 *      POZNAN_NONLINEAR_DEPTH - 
 *    New config options:
 *      in 3dv_enc_###_depth_###.cfg
 *      NonlinearDepth           0 turns the tool off,   1 turns the tool on.
 *      NonlinearDepthModel      definition of the non-linear curve nodes. eg. "10;19;24;27;26;22;13" 
 *      NonlinearDepthThreshold  encoder control automatic turning off/on of the tool, basing on histogram of the depth map.
 *
 * \author
 *    Contributors:
 *    - Olgierd Stankiewicz     ostank@multimedia.edi.pl:    -Original code                             (12-March-2012)
 *                                                           -Alignment of: view synthesis, joint_view_filter, depth-based motion vector prediction
 *
 *************************************************************************************
 */
#include "nonlinear_depth.h"
#include <math.h>

double nonlinear_depth_lerp(double value, imgpel lut[NONLINEAR_DEPTH_LUT_SIZE])
{
  double frc;
  int value_int;
  
  value_int = (int)value;
  if (value_int<0) return 0.0;
  if (value_int>=255) return 255.0;

  frc = value-value_int;
  return lut[value_int]*(1-frc) + lut[value_int+1]*frc;  
}

void nonlinear_depth_draw_line( imgpel lut[NONLINEAR_DEPTH_LUT_SIZE], int x1, int y1, int x2, int y2)
{
  int x;
  int y2y1  = y2-y1;
  int x2x1  = x2-x1;
  int x2x1s = (y2y1>0) ? (x2x1>>1) : -(x2x1>>1);

  for (x=x1; x<=x2; ++x)
  {
    if ((x>=0) && (x<NONLINEAR_DEPTH_LUT_SIZE))
    {
      int y = ( (x-x1)*y2y1 + x2x1s)/x2x1+y1;
      if (y<0) y=0;
      if (y>=NONLINEAR_DEPTH_LUT_SIZE) y=NONLINEAR_DEPTH_LUT_SIZE-1;
      lut[x] = (imgpel)y;
    }
  }
}

void nonlinear_depth_init_lut ( imgpel lut[NONLINEAR_DEPTH_LUT_SIZE], int nonlinear_depth_num, char *nonlinear_depth_points, int forward )
{
  int k;
  int start_x;
  int start_d;
  int end_x;
  int end_d;

  //0, 1..nonlinear_depth_num, nonlinear_depth_num+1
    
  for (k=0; k<=nonlinear_depth_num; ++k)
  {
    start_x = ((NONLINEAR_DEPTH_LUT_SIZE-1)*k      )/(nonlinear_depth_num+1);
    start_d = nonlinear_depth_points[k]*(forward?1:-1);
    end_x   = ((NONLINEAR_DEPTH_LUT_SIZE-1)*(k+1)  )/(nonlinear_depth_num+1);
    end_d   = nonlinear_depth_points[k+1]*(forward?1:-1);
    nonlinear_depth_draw_line(lut, start_x+start_d, start_x-start_d, end_x+end_d, end_x-end_d);
  }
}
void nonlinear_depth_process_buf2d_uchar  ( unsigned char **out, unsigned char **in, int size_x, int size_y, int nonlinear_depth_num, char *nonlinear_depth_points, int forward )
{
  imgpel DepthPowerLUT[NONLINEAR_DEPTH_LUT_SIZE];
  int i,j;

  nonlinear_depth_init_lut( DepthPowerLUT, nonlinear_depth_num, nonlinear_depth_points, forward);

  for(j=0;j<size_y;++j)
  for(i=0;i<size_x;++i)
    out[j][i] = DepthPowerLUT[in[j][i]];

}

void nonlinear_depth_process_buf2d_imgpel  ( imgpel **out, imgpel **in, int size_x, int size_y, int nonlinear_depth_num, char *nonlinear_depth_points, int forward )
{
  imgpel DepthPowerLUT[NONLINEAR_DEPTH_LUT_SIZE];
  int i,j;

  nonlinear_depth_init_lut( DepthPowerLUT, nonlinear_depth_num, nonlinear_depth_points, forward);

  for(j=0;j<size_y;++j)
  for(i=0;i<size_x;++i)
    out[j][i] = DepthPowerLUT[in[j][i]];

}

void nonlinear_depth_process_buf2d_with_pad_imgpel ( imgpel **out, imgpel **in, int size_x, int size_y, int nonlinear_depth_num, char *nonlinear_depth_points, int pad_x, int pad_y, int forward )
{
  imgpel DepthPowerLUT[NONLINEAR_DEPTH_LUT_SIZE];
  int i,j;

  nonlinear_depth_init_lut( DepthPowerLUT, nonlinear_depth_num, nonlinear_depth_points, forward);
  size_x+=pad_x;
  size_y+=pad_y;

  for(j=-pad_y;j<size_y;++j)
  for(i=-pad_x;i<size_x;++i)
    out[j][i] = DepthPowerLUT[in[j][i]];
}
