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
 *    mc_prediction.h
 *
 * \brief
 *    motion compensation header
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */

#ifndef _MC_PREDICTION_H_
#define _MC_PREDICTION_H_
#include "mbuffer.h"

extern void luma_prediction       ( Macroblock* currMB, int, int, int, int, int, int[2], char *, short );
extern void luma_prediction_bi    ( Macroblock* currMB, int, int, int, int, int, int, short, short, int );
extern void chroma_prediction     ( Macroblock* currMB, int, int, int, int, int, int, int, int, short, short, short );
extern void chroma_prediction_4x4 ( Macroblock* currMB, int, int, int, int, int, int, short, short, short);   

extern void rdo_low_intra_chroma_decision              (Macroblock *currMB, int mb_available_up, int mb_available_left[2], int mb_available_up_left);
extern void rdo_low_intra_chroma_decision_mbaff        (Macroblock *currMB, int mb_available_up, int mb_available_left[2], int mb_available_up_left);
extern void OneComponentChromaPrediction4x4_regenerate (Macroblock *currMB, imgpel* , int , int , MotionVector ** , StorablePicture *listX, int );
extern void OneComponentChromaPrediction4x4_retrieve   (Macroblock *currMB, imgpel* , int , int , MotionVector ** , StorablePicture *listX, int );

extern void intra_chroma_prediction_mbaff(Macroblock *currMB, int*, int*, int*);
extern void intra_chroma_prediction      (Macroblock *currMB, int*, int*, int*);
extern void IntraChromaPrediction4x4     (Macroblock* currMB, int uv, int block_x, int  block_y);


#endif

