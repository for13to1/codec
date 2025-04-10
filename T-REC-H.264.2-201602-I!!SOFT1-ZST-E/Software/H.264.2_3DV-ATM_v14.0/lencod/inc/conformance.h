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
 * \file conformance.h
 *
 * \brief
 *   Level & Profile Related definitions  
 *
 * \author
 *    Alexis Michael Tourapis         <alexismt@ieee.org>       \n
 *
 ************************************************************************
 */

#ifndef _CONFORMANCE_H_
#define _CONFORMANCE_H_

extern void    ProfileCheck         (InputParameters *p_Inp);
extern void    LevelCheck           (VideoParameters *p_Vid, InputParameters *p_Inp);
extern void    update_mv_limits     (VideoParameters *p_Vid, byte is_field);
extern void    clip_mv_range        (VideoParameters *p_Vid, int search_range, MotionVector *mv, int res);
extern int     out_of_bounds_mvs    (VideoParameters *p_Vid, const MotionVector *mv);
extern void    test_clip_mvs        (VideoParameters *p_Vid, MotionVector *mv, Boolean write_mb);
extern Boolean CheckPredictionParams(Macroblock  *currMB, Block8x8Info *b8x8info, int mode);

extern unsigned int getMaxMBPS(unsigned int levelIdc);
extern unsigned int getMinCR  (unsigned int levelIdc);
extern unsigned int getMaxBR  (unsigned int levelIdc);
extern unsigned int getMaxCPB (unsigned int levelIdc);

#endif

