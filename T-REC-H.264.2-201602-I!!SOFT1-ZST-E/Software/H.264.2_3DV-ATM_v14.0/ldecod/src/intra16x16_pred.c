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
 * \file intra16x16_pred.c
 *
 * \brief
 *    Functions for intra 16x16 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "intra16x16_pred.h"
#include "mb_access.h"
#include "image.h"


extern int intrapred_16x16_mbaff (Macroblock *currMB, ColorPlane pl, int predmode);
extern int intrapred_16x16_normal(Macroblock *currMB, ColorPlane pl, int predmode);

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 intra prediction blocks 
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *    SEARCH_SYNC   search next sync element as errors while decoding occured
 ***********************************************************************
 */
int intrapred16x16(Macroblock *currMB,  //!< Current Macroblock
                   ColorPlane pl,       //!< Current colorplane (for 4:4:4)                         
                   int predmode)        //!< prediction mode
{
  if (currMB->p_Slice->mb_aff_frame_flag)
  {
    return intrapred_16x16_mbaff(currMB, pl, predmode);
  }
  else
  {
    return intrapred_16x16_normal(currMB, pl, predmode);
  }
}

