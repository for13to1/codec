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
 ***************************************************************************
 * \file
 *    wp.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    22. February 2008
 *
 * \brief
 *    Headerfile for weighted prediction support
 **************************************************************************
 */

#ifndef _WP_H_
#define _WP_H_

#include "wp_lms.h"
#include "wp_mcprec.h"
#include "wp_mciter.h"

#define DEBUG_WP  0

void InitWP              (VideoParameters *p_Vid, InputParameters *p_Inp, int force_wp_method);
void ResetWP             (VideoParameters *p_Vid, InputParameters *p_Inp);

extern void   EstimateWPBSliceAlg0   (Slice *currSlice);
extern void   EstimateWPPSliceAlg0   (Slice *currSlice, int offset);
extern int    TestWPPSliceAlg0       (Slice *currSlice, int offset);
extern int    TestWPBSliceAlg0       (Slice *currSlice, int method);
extern double ComputeImgSum          (imgpel **CurrentImage, int height, int width);
extern void   ComputeImgSumBlockBased(imgpel **CurrentImage, int height_in_blk, int width_in_blk, int blk_size_y, int blk_size_x, int start_blk, int end_blk, double *dc);
extern int64  ComputeSumBlockBased   (imgpel **CurrentImage, int height_in_blk, int width_in_blk, int blk_size_y, int blk_size_x, int start_blk, int end_blk);

extern void   ComputeImplicitWeights    (Slice *currSlice,
                                         short default_weight[3],
                                         short im_weight[6][MAX_REFERENCE_PICTURES][MAX_REFERENCE_PICTURES][3]);
extern void   ComputeExplicitWPParamsLMS(Slice *currSlice,
                                         int select_offset,
                                         int start_mb,
                                         int end_mb, 
                                         short default_weight[3],
                                         short weight[6][MAX_REFERENCE_PICTURES][3],
                                         short offset[6][MAX_REFERENCE_PICTURES][3]);
extern void   ComputeExplicitWPParamsJNT(Slice *currSlice,
                                         int start_mb,
                                         int end_mb, 
                                         short default_weight[3],
                                         short weight[6][MAX_REFERENCE_PICTURES][3],
                                         short offset[6][MAX_REFERENCE_PICTURES][3]);
#if EXT3D
extern void   ComputeExplicitWPParamsDRB(Slice* currSlice,short default_weight[3],
                                         short weight[6][MAX_REFERENCE_PICTURES][3],
                                         short offset[6][MAX_REFERENCE_PICTURES][3]);
extern void   EstimateWPPSliceAlg3(Slice* currSlice,int select_offset);
extern void   EstimateWPBSliceAlg3(Slice* currSlice);
#endif


#endif

