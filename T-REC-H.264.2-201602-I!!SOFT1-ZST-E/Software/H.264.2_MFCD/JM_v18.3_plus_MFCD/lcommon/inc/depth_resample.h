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

#ifndef DEPTH_RESAMPLE_H
#define DEPTH_RESAMPLE_H

/*!
***************************************************************************
* \file
*    cabac.h
*
* \brief
*    Headerfile for depth map resampling
*
* \author
* suwenyi      <suwy@mail.ustc.edu.cn>                                                   \n
*
* \date
*    17. 6. 2011 
***************************************************************************
*/

extern void upsample_depth(imgpel**input_depth,imgpel**output_depth,int height,int width,int scale);
extern void do_upsample(imgpel** input_depth, imgpel** output, int height, int width);
extern void downsample_depth(imgpel**input_depth,imgpel**output_depth,int height,int width,int scale);
extern void do_downsample(imgpel**input_depth,imgpel**output_depth,int height,int width);

#endif

