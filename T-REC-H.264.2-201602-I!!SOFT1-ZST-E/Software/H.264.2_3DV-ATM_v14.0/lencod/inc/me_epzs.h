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
************************************************************************
* \file
*     me_epzs.h
*
* \author
*    Alexis Michael Tourapis        <alexis.tourapis@dolby.com>
*
* \date
*    11. August 2006
*
* \brief
*    Headerfile for EPZS motion estimation
**************************************************************************
*/


#ifndef _ME_EPZS_H_
#define _ME_EPZS_H_
#include "me_epzs_common.h"

static const int next_start_pos[5][5] =
{
  {0,  8, 5,  6, 7},
  {8,  0, 5,  8, 8}, 
  {5,  5, 0,  6, 5},
  {6,  6, 6,  0, 7},
  {7,  8, 7,  7, 0}
};

static const int next_end_pos[5][5] =
{
  {0, 10,  7,  8, 9},
  {10, 0,  6, 10, 9},
  {7,  6,  0,  7, 7},
  {8,  8,  7,  0, 8},
  {9,  9,  9,  8, 0}
};

static const MotionVector search_point_hp[10] = {{0,0},{-2,0}, {0,2}, {2,0},  {0,-2}, {-2,2},  {2,2},  {2,-2}, {-2,-2}, {-2,2}};
static const MotionVector search_point_qp[10] = {{0,0},{-1,0}, {0,1}, {1,0},  {0,-1}, {-1,1},  {1,1},  {1,-1}, {-1,-1}, {-1,1}};

// Functions
extern distblk EPZSPelBlockMotionSearch      (Macroblock *, MotionVector *, MEBlock *, distblk, int);
extern distblk EPZSPelBlockMotionSearchSubMB (Macroblock *, MotionVector *, MEBlock *, distblk, int);
extern distblk EPZSSubPelBlockMotionSearch   (Macroblock *, MotionVector *, MEBlock *mv_block, distblk, int*);
extern distblk EPZSSubPelBlockSearchBiPred   (Macroblock *,  MEBlock *mv_block, int list, 
                                         MotionVector *pred_mv1, MotionVector *pred_mv2, MotionVector *mv1, MotionVector *mv2, distblk min_mcost, int *lambda_factor);
extern distblk EPZSBiPredBlockMotionSearch   (Macroblock *, int, MotionVector *, MotionVector *, MotionVector *, MotionVector *, MEBlock *,  int, distblk, int);


#endif

