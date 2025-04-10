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
*************************************************************************************
* \file  c3d_mux_filters.h
*
* \brief 
*
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hgana@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_MUX_FILTERS_
#define _C3D_MUX_FILTERS_


#define MAX_NUM_MUX_FILTER_TAPS 25


/** 1D Muxing filter definition */
const int16_t ai16_1D_muxing_coef[TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER][MAX_NUM_MUX_FILTER_TAPS] = {
  /* SVC3D */ {2, 0,-4, -3 , 5 , 19, 26, 19, 5, -3, -4, 0, 2},
  /* FC_P44 */ {30, -4,-61,-21, 83, 71,-102,-178, 116, 638,904,638,116,-178,-102,71,83,-21,-61,-4,30}, 
};

/** 
 * Filter length, offset and dynamic range of 1D muxing horizontal filters. 
 */

const uint16_t au16_1D_muxing_filter_length[TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER] = {
  /* SVC3D */ 13,
  /* FC_P44 */ 21,
};

const uint16_t au16_1D_muxing_filter_dynamic_range[TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER] = {
  /* SVC3D */ 6,
  /* FC_P44 */ 11,
};

const uint16_t au16_1D_muxing_filter_offset[TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER] = {
  /* SVC3D */ 32,
  /* FC_P44 */ 1024,
};



#endif /* _C3D_MUX_FILTERS_ */
