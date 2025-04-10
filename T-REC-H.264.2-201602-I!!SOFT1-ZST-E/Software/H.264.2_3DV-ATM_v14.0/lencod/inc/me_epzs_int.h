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
 *     me_epzs_int.h
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


#ifndef _ME_EPZS_INT_H_
#define _ME_EPZS_INT_H_

#include "me_epzs.h"

// Functions
extern distblk  EPZSIntPelBlockMotionSearch     (Macroblock *, MotionVector *, MEBlock *, distblk, int);
extern distblk  EPZSIntPelBlockMotionSearchSubMB(Macroblock *, MotionVector *, MEBlock *, distblk, int);
extern distblk  EPZSIntBiPredBlockMotionSearch  (Macroblock *, int, MotionVector *, MotionVector *, MotionVector *, MotionVector *, MEBlock *, int, distblk, int);
#endif

