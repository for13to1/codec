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
 **************************************************************************************
 * \file
 *    nalu.h
 * \brief
 *    Common NALU support functions
 *
 * \date 25 November 2002
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger        <stewe@cs.tu-berlin.de>
 ***************************************************************************************
 */


#ifndef _NALU_H_
#define _NALU_H_

#include "nalucommon.h"

typedef struct sBitsFile
{
  void (*OpenBitsFile)    (VideoParameters *p_Vid, char *filename);
  void (*CloseBitsFile)   (VideoParameters *p_Vid);
  int  (*GetNALU)         (VideoParameters *p_Vid, NALU_t *nalu);
} BitsFile;

extern void initBitsFile (VideoParameters *p_Vid, int filemode);
extern void CheckZeroByteNonVCL(VideoParameters *p_Vid, NALU_t *nalu);
extern void CheckZeroByteVCL   (VideoParameters *p_Vid, NALU_t *nalu);

extern int read_next_nalu(VideoParameters *p_Vid, NALU_t *nalu);

#endif
