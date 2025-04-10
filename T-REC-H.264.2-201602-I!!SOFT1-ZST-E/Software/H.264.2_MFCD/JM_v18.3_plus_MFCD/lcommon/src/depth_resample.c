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

#include "global.h"
#include "ifunctions.h"
#include "memalloc.h"

#include "depth_resample.h"

/*
* \brief
*    Upsample one depth map
* 
* \input
* \param input_depth
*    The input depth map.  Resolution is height,width
* \param height,width
*    The resolution of the input picture
*
* \output
* \param output_depth
*    The output image. Resolution is height,width*2
*
* \return
*    None
*/
void do_upsample(imgpel** input_depth, imgpel** output_depth, int height, int width)
{
  int i, j, pel[6];
  int WidthMinus1;
  imgpel* out;
  imgpel* in;

  WidthMinus1 = width-1;

  for (j = 0; j < height; j++)
  {
    out = output_depth[j];
    in  = input_depth[j];
    for(i = 0; i < width; i++)
    {
      pel[0]=iClip3(0, WidthMinus1,i-2);
      pel[1]=iClip3( 0, WidthMinus1,i-1);
      pel[2]=iClip3( 0, WidthMinus1,i  );
      pel[3]=iClip3( 0, WidthMinus1,i+1);
      pel[4]=iClip3( 0, WidthMinus1,i+2);
      pel[5]=iClip3( 0, WidthMinus1,i+3);

      out[ (i)<<1   ] = in[pel[2]];
      //!<6-tap FIR filter used in AVC
      out[((i)<<1)+1] = (imgpel)iClip3(  0, 255,(20*(in[pel[2]]+in[pel[3]]) - 5*(in[pel[1]]+in[pel[4]]) + (in[pel[0]]+in[pel[5]]) +16)>>5 );
    }
  }
}

/*
* \brief
*    Downsample the depth map
* 
* \input
* \param input
*    The input depth map.  Resolution is height,width
* \param height, width
*    The resolution of the input depth map
*
* \output
* \param output_depth
*    The output depth map. Resolution is height,width/2
*
* \return
*    None
*/
void do_downsample(imgpel**input_depth,imgpel**output_depth,int height,int width)
{

  int i, j, pel[3];
  int WidthMinus1;
  imgpel* out;
  imgpel* in;

  WidthMinus1 = width-1;

  for (j = 0; j < height; j++)
  {
    out = output_depth[j];
    in  = input_depth[j];
    for(i = 0; i < width; i+=2)
    {
      pel[0] = iClip3( 0, WidthMinus1,i-1);
      pel[1] = iClip3( 0, WidthMinus1,i);
      pel[2] = iClip3(0, WidthMinus1,i+1 );

      out[(i >> 1)] = (imgpel)iClip3(0, 255,(in[pel[0]] + 2*in[pel[1]] + in[pel[2]]) >> 2);
    }
  }
}
void upsample_depth(imgpel**input_depth,imgpel**output_depth,int height,int width,int scale)
{
  imgpel** tmp=NULL;

  if (scale == 2)
    do_upsample(input_depth,output_depth, height,width);
  else
  {
    get_mem2Dpel(&tmp,height,width*2);
    do_upsample(input_depth, tmp, height, width);
    do_upsample(tmp,output_depth,height,width*2);
    free_mem2Dpel(tmp);
  }
}
void downsample_depth(imgpel**input_depth,imgpel**output_depth,int height,int width,int scale)
{
  imgpel** tmp=NULL;

  if (scale == 2)
    do_downsample(input_depth,output_depth,height,width);
  else
  {
    get_mem2Dpel(&tmp,height,width/2);
    do_downsample(input_depth,tmp,height,width);
    do_downsample(tmp,output_depth,height,width/2) ;
    free_mem2Dpel(tmp);
  }
}