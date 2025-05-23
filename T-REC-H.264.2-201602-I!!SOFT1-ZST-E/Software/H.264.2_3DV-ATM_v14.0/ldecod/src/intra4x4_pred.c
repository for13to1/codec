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
 * \file intra4x4_pred.c
 *
 * \brief
 *    Functions for intra 4x4 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "intra4x4_pred.h"
#include "mb_access.h"
#include "image.h"


extern int intra4x4_pred_mbaff (Macroblock *currMB, ColorPlane pl, int ioff, int joff, int img_block_x, int img_block_y);
extern int intra4x4_pred_normal(Macroblock *currMB, ColorPlane pl, int ioff, int joff, int img_block_x, int img_block_y);

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 intra prediction blocks 
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *    SEARCH_SYNC   search next sync element as errors while decoding occured
 ***********************************************************************
 */
int intrapred(Macroblock *currMB,    //!< current macroblock
              ColorPlane pl,         //!< current image plane
              int ioff,              //!< pixel offset X within MB
              int joff,              //!< pixel offset Y within MB
              int img_block_x,       //!< location of block X, multiples of 4
              int img_block_y)       //!< location of block Y, multiples of 4
{
  if (currMB->p_Slice->mb_aff_frame_flag)
  {
    return intra4x4_pred_mbaff(currMB, pl, ioff, joff, img_block_x, img_block_y);   
  }
  else
  {
    return intra4x4_pred_normal(currMB, pl, ioff, joff, img_block_x, img_block_y);      
  }
}
