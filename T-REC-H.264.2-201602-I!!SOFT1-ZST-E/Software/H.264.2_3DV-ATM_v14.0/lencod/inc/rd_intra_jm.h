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
 *    rd_intra_jm.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    2 January 2008
 *
 * \brief
 *    Headerfile for JM rd based intra mode decision
 **************************************************************************
 */

#ifndef _RD_INTRA_JM_H_
#define _RD_INTRA_JM_H_

extern int mode_decision_for_I16x16_MB_RDO          (Macroblock *currMB, int lambda);
extern int mode_decision_for_I16x16_MB              (Macroblock *currMB, int lambda);
extern int mode_decision_for_I4x4_blocks_JM_High    (Macroblock *currMB, int  b8,  int  b4,  int  lambda,  distblk*  min_cost);
extern int mode_decision_for_I4x4_blocks_JM_Low     (Macroblock *currMB, int  b8,  int  b4,  int  lambda,  distblk*  min_cost);
extern int find_best_mode_I16x16_MB                 (Macroblock *currMB, int lambda, distblk min_cost);

#endif

