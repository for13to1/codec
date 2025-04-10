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
 *     me_fullsearch.h
 *
 * \author
 *    Alexis Michael Tourapis        <alexis.tourapis@dolby.com>
 *
 * \date
 *    9 September 2006
 *
 * \brief
 *    Headerfile for Full Search motion estimation
 **************************************************************************
 */


#ifndef _ME_FULLSEARCH_H_
#define _ME_FULLSEARCH_H_
extern distblk FullPelBlockMotionSearch (Macroblock *currMB, MotionVector *pred_mv, MEBlock *mv_block,
                                     distblk min_mcost, int lambda_factor);
extern distblk FullPelBlockMotionBiPred (Macroblock *currMB, int list, 
                                     MotionVector *pred_mv1, MotionVector *pred_mv2, MotionVector *mv1, MotionVector *mv2, MEBlock *,
                                     int search_range, distblk min_mcost, int lambda_factor);
extern distblk SubPelBlockMotionSearch  (Macroblock *currMB, MotionVector *pred_mv, MEBlock *mv_block, 
                                     distblk min_mcost, int* lambda_factor);
extern distblk SubPelBlockSearchBiPred  (Macroblock *currMB, MEBlock *mv_block, int list, 
                                     MotionVector *pred_mv1, MotionVector *pred_mv2, MotionVector *mv1, MotionVector *mv2, 
                                     distblk min_mcost, int* lambda_factor);
#endif

