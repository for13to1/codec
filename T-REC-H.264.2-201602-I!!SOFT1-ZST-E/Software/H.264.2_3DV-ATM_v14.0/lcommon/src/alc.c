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


#include "alc.h"
#include "../inc/memalloc.h"
#include <stddef.h>


#if EXT3D // @DT: Enclose all code under this file

// Internal variables
  int ref_sum;//!< sum of pixels of reference template + 1
  int dec_sum;//!< sum of pixels of reference template + 1
  static int TMIsFilled = 0;//!<<is used for fixing fact of template filling (and => als is enabled for this block)

//arrays for horizontal and vertical sub-parts of template:
/*
* Example: TM with size 2 around block 4x4
*         HH HH HH HH
*         HH HH HH HH
*      VV
*      VV
*      VV
*      VV
*/
  static XPel** ppTM_buf;      //!<< extended buffer for qpel interpolation
  XPel* pTM_buf44;            //!<< poits to begining of block in the extended buffer
  static int** ppTM_tempbuf;  //!<< temporary extended buffer for qpel interpolation

/*!
***************************************************************************
* \brief
*    Initialises alc components.
***************************************************************************
*/
void ALC_Create()
{
  // memory allocations:
  get_mem2Dpel(&ppTM_buf, 16 + MAX_TEMPLATE_SIZE, 16 + MAX_TEMPLATE_SIZE);
  pTM_buf44 = &(ppTM_buf[MAX_TEMPLATE_SIZE][MAX_TEMPLATE_SIZE]);
  get_mem2Dint(&ppTM_tempbuf, 16 + MAX_TEMPLATE_SIZE + 5, 16 + MAX_TEMPLATE_SIZE + 5); //temporal buffer for in-place interpolation inside AVC. +5 is needed.

  //Additional initial clean 
  TMIsFilled = TM_EMPTY;
}

/*!
***************************************************************************
* \brief
*    Deinitialises alc components.
***************************************************************************
*/
void ALC_Destroy()
{
  free_mem2Dpel(ppTM_buf);
  free_mem2Dint(ppTM_tempbuf);
}

/*!
***************************************************************************
* \brief
*  This function takes pixels from reference and decoded template areas in respect of pixel positions of left-top corners:
*
* \param
* RefTM_startY, RefTM_startX : left-top corner of a block nearby whom template can be taken
* DecTM_startY, DecTM_startX : left-top corner of a block nearby whom template can be taken
* Blk_height - total height of the block inside a template
* Blk_width - total width of the block inside a template
*
* \return 
* 0 if procedure was succeed.
* Non-zero value in case of any error.
***************************************************************************
*/
int ALC_GetTM(  XPel **ppRef, int RefTM_startY, int RefTM_startX, 
        XPel **ppDec, int DecTM_startY, int DecTM_startX, 
        int Blk_height, int Blk_width)
{
  int i1,j1;
  ptrdiff_t stride_ref, stride_dec;
  XPel** ppCurRef;
  XPel** ppCurDec;

  XPel* pCurRefTemp;
  XPel* pCurDecTemp;

  short temp, temp1, temp2;
  // for calculating abs below ( see iabs for details)  
  short x, y, z;
  static const short INT_BITS = (sizeof(short) * CHAR_BIT) - 1;
  
  TMIsFilled = TM_EMPTY;

  // upper block of the template
  ppCurRef = &ppRef[ RefTM_startY];
  ppCurDec = &ppDec[ DecTM_startY];

  ref_sum = dec_sum = 1;

  stride_ref = ppCurRef[ -MAX_TEMPLATE_SIZE + 1] - ppCurRef[ -MAX_TEMPLATE_SIZE] - Blk_width;
  stride_dec = ppCurDec[ -MAX_TEMPLATE_SIZE + 1 ] - ppCurDec[ -MAX_TEMPLATE_SIZE ] - Blk_width;

  pCurRefTemp = ppCurRef[ -MAX_TEMPLATE_SIZE ] + RefTM_startX;
  pCurDecTemp = ppCurDec[ -MAX_TEMPLATE_SIZE ] + DecTM_startX;

 // upper block of the template
  for(i1 = -MAX_TEMPLATE_SIZE; i1 < 0; ++i1)
  { 
    for(j1 = 0; j1 < Blk_width; ++j1)
    {            
      //take values
      temp2 = temp = *pCurRefTemp++;
      temp1 = *pCurDecTemp++;      
#if ( REF_DEC_LUMA_DIF_THRESHOLD < 256)  // in this way we can turn-off ref_dec thresholding
      //if ( iabs(temp - temp1) <= REF_DEC_LUMA_DIF_THRESHOLD )
      x = (temp - temp1);
      y = x >> INT_BITS;
      z = (x ^ y) - y;
      if ( z <= REF_DEC_LUMA_DIF_THRESHOLD )
#endif
      {
        ref_sum += temp2;
        dec_sum += temp1;
      }
    }
    pCurRefTemp += stride_ref;
    pCurDecTemp += stride_dec;
  }

// left block of the template
  RefTM_startX -=MAX_TEMPLATE_SIZE;
  DecTM_startX -= MAX_TEMPLATE_SIZE;


  pCurRefTemp = &(ppCurRef[0][RefTM_startX]);
  pCurDecTemp = &(ppCurDec[0][DecTM_startX]);

  stride_ref = ppCurRef[ -MAX_TEMPLATE_SIZE + 1]  - ppCurRef[ -MAX_TEMPLATE_SIZE] - MAX_TEMPLATE_SIZE;
  stride_dec =  ppCurDec[ -MAX_TEMPLATE_SIZE + 1 ] - ppCurDec[ -MAX_TEMPLATE_SIZE ] - MAX_TEMPLATE_SIZE;

  for(i1 = 0; i1 < Blk_height; ++i1)  
  {
    for(j1 = -MAX_TEMPLATE_SIZE; j1 < 0; ++j1)
    {
      temp2 = temp = *pCurRefTemp++;
      temp1 = *pCurDecTemp++;    
#if ( REF_DEC_LUMA_DIF_THRESHOLD < 256)
//      if ( iabs(temp - temp1) <= REF_DEC_LUMA_DIF_THRESHOLD )
      x = (temp - temp1);
      y = x >> INT_BITS;
      z = (x ^ y) - y;
      if ( z <= REF_DEC_LUMA_DIF_THRESHOLD )
#endif
      {
        ref_sum += temp2;
        dec_sum += temp1;
      }
    }

    pCurRefTemp += stride_ref;
    pCurDecTemp += stride_dec;
  }  

  TMIsFilled = TM_FILLED;
  return ALCOK;
}

/*!
***************************************************************************
* \brief
*    Performs alc (decoder)
***************************************************************************
*/
void PiLC_TMPelCorHist_2DArray( XPel **ppOut, int XOut, int h, int w)
{
  int mult_factor;
  int k, m;
  int rec_vl, ReferenceVl;
  XPel* pTempIn;
  XPel* pTempOut;
  int stride_in, stride_out;

  mult_factor = ((1 << SINGL_MULT_FACT_PREC) * (unsigned int)dec_sum + (unsigned int)(ref_sum>>1))/(unsigned int)ref_sum;  

  //stride between two adjacent lines:
  stride_out = MB_BLOCK_SIZE - w;
  pTempIn = pTM_buf44;
  stride_in =16 + MAX_TEMPLATE_SIZE - w;
  pTempOut = *ppOut+XOut;

#if (EARLY_ALC_OFF_DETECTION)
  ref_sum >>=EARLY_ALC_OFF_DETECTION_THRESHOLD;
  dec_sum >>=EARLY_ALC_OFF_DETECTION_THRESHOLD;

  if (ref_sum == dec_sum)
  {
     for (k = 0; k < h; ++k)
    {
      memcpy(pTempOut, pTempIn, sizeof(XPel) * w);
      pTempIn += (stride_in + w);
      pTempOut += (stride_out + w);
     }
     return;
  }
#endif

  for (k = 0; k < h; ++k)
  {
    for (m = XOut; m < (XOut + w); ++m)
    {
      ReferenceVl = *pTempIn++;
      rec_vl = (int)( (ReferenceVl * mult_factor + (1<<(SINGL_MULT_FACT_PREC-1))) >> SINGL_MULT_FACT_PREC);
        if (rec_vl > 255) rec_vl = 255;
        *pTempOut++ = (XPel)rec_vl;
    }  
    pTempIn += stride_in;
    pTempOut += stride_out;
  }
}

/*!
***************************************************************************
* \brief
*    Performs alc (coder)
***************************************************************************
*/
void PiLC_TMPelCorHist_1DArray(  XPel *pIn, XPel *pOut, int StrideIn, int StrideOut, int h, int w)
{
  int mult_factor;
  int k, m;
  int rec_vl, ReferenceVl;

  mult_factor = ((1 << SINGL_MULT_FACT_PREC) * (unsigned int)dec_sum + (unsigned int)(ref_sum>>1))/(unsigned int)ref_sum;  

#if (EARLY_ALC_OFF_DETECTION)
  ref_sum >>=EARLY_ALC_OFF_DETECTION_THRESHOLD;
  dec_sum >>=EARLY_ALC_OFF_DETECTION_THRESHOLD;

  if (ref_sum == dec_sum)
  {
    for (k = 0; k < h; ++k)
    {
      memcpy(pOut, pIn, sizeof(XPel) * w);
      pOut += StrideOut;
      pIn += StrideIn;
    }
    return;
  }
#endif

  for (k = 0; k < h; ++k)
  {
    for (m = 0; m < w; ++m)
    {
      ReferenceVl = pIn[m];
      rec_vl = (int)( (ReferenceVl * mult_factor + (1<<(SINGL_MULT_FACT_PREC-1))) >> SINGL_MULT_FACT_PREC);
      if (rec_vl > 255) rec_vl = 255;
      pOut[m] = (XPel)rec_vl;
    }
    pOut += StrideOut;
    pIn += StrideIn;
  }
}

/*!
***************************************************************************
* \brief
*    enables alc (when templates are availible)
***************************************************************************
*/
Boolean IsTMFilled()
{
  if (TM_FILLED == TMIsFilled)
    return TRUE;
  else
    return FALSE;
}

/*!
***************************************************************************
* \brief
*    disables alc (after compensation)
***************************************************************************
*/
void ALC_TurnOFF()
{
  TMIsFilled = TM_EMPTY;
}

/*!
***************************************************************************
* \brief
*    Get extended buffer for interpolation (because templates are nedeed)
***************************************************************************
*/
XPel** ALC_RetTM_buf()
{
  return ppTM_buf;
}

/*!
***************************************************************************
* \brief
*    Get temporary extended buffer for interpolation (because templates are nedeed)
***************************************************************************
*/
int** ALC_RetTM_tempbuf()
{
  return ppTM_tempbuf;
}

#endif