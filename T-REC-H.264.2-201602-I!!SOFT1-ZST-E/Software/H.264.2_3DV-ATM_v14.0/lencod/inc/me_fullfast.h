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
 *     me_fullfast.h
 *
 * \author
 *    Alexis Michael Tourapis        <alexis.tourapis@dolby.com>
 *
 * \date
 *    9 September 2006
 *
 * \brief
 *    Headerfile for Fast Full Search motion estimation
 **************************************************************************
 */


#ifndef _ME_FULLFAST_H_
#define _ME_FULLFAST_H_
typedef struct me_full_fast
{
  int          **search_setup_done;     //!< flag if all block SAD's have been calculated yet
  MotionVector **search_center; //!< absolute search center for fast full motion search
  MotionVector **search_center_padded; //!< absolute search center for fast full motion search
  int          **pos_00;             //!< position of (0,0) vector
  distpel   *****BlockSAD;        //!< SAD for all blocksize, ref. frames and motion vectors
  int          **max_search_range;
} MEFullFast;

extern distblk FastFullPelBlockMotionSearch (Macroblock *currMB, MotionVector *pred_mv, MEBlock *mv_block, distblk min_mcost, int lambda_factor);
extern void InitializeFastFullIntegerSearch (VideoParameters *p_Vid, InputParameters *p_Inp);
extern void ResetFastFullIntegerSearch      (VideoParameters *p_Vid);
extern void ClearFastFullIntegerSearch      (VideoParameters *p_Vid);


#endif

