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
 *************************************************************************************
 * \file mb_prediction.h
 *
 * \brief
 *    Functions for macroblock prediction
 *
 * \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>  
 *************************************************************************************
 */

#ifndef _MB_PREDICTION_H_
#define _MB_PREDICTION_H_

extern int mb_pred_intra4x4      (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern int mb_pred_intra16x16    (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_intra8x8      (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);

extern int mb_pred_skip          (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern int mb_pred_sp_skip       (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_p_inter8x8    (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_p_inter16x16  (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_p_inter16x8   (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_p_inter8x16   (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_b_d4x4spatial (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern int mb_pred_b_d8x8spatial (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern int mb_pred_b_d4x4temporal(Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern int mb_pred_b_d8x8temporal(Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern int mb_pred_b_inter8x8    (Macroblock *currMB, ColorPlane curr_plane, StorablePicture *dec_picture);
extern int mb_pred_ipcm          (Macroblock *currMB);

#endif
