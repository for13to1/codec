/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2009-2012, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/

#ifndef __IMG_PROCESS_MUX_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __IMG_PROCESS_MUX_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79

#include "H264AVCEncoderLibTest.h"

#if DOLBY_ENCMUX_ENABLE
typedef UChar imgpel;
typedef struct img_process_filter_1D
{
  short coef1[16];
  short *c_coef1;
  int c_tap;
  int c_normal1;
  int c_taps_div2;
  int max_pel_value;
  int c_offset;
  int symmetric;
} ImgProcessFilter1D;

static inline int iClip1(int high, int x)
{
  x = gMax(x, 0);
  x = gMin(x, high);

  return x;
}

static inline int shift_off_sf(int x, int o, int a)
{
  return ((x + o) >> a);
}

ImgProcessFilter1D * create_img_process_filter_1D( int filter_type);
void destroy_img_process_filter_1D( ImgProcessFilter1D * ptr );

#endif

#endif //__H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
