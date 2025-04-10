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


#ifndef __ALC_H__
#define __ALC_H__

#include "defines.h"
#include "global.h"

#if EXT3D

#ifndef CLIP
#define CLIP(x,min,max) ( (x)<(min)?(min):((x)>(max)?(max):(x)) )
#endif

typedef imgpel XPel;

//thickness of template. Is expressed in pixels
#define MAX_TEMPLATE_SIZE                                       (1)

// max MB Size 16
#define MAX_TEMPLATE_VOLUME                                     ((MAX_TEMPLATE_SIZE * 16) * 2 + (MAX_TEMPLATE_SIZE * MAX_TEMPLATE_SIZE))

//Threshold of difference between template values in reference and decoded frames
#define REF_DEC_LUMA_DIF_THRESHOLD                (30)          // if REF_DEC_LUMA_DIF_THRESHOLD == 256, then no comparison between ref and dec to be applied at all

//  EARLY_ALC_OFF_DETECTION -this define allows to compare sums of templates and exclude ALC_Correction if templates differencies are not crucial
//  EARLY_ALC_OFF_DETECTION_THRESHOLD defines how strong ref_sum/dec_sum will be quantizaed before comparison; it uses as follows:
//  ref_sum >> = EARLY_ALC_OFF_DETECTION_THRESHOLD;
//  dec_sum >> = EARLY_ALC_OFF_DETECTION_THRESHOLD;
#define EARLY_ALC_OFF_DETECTION                                 (1)
#define EARLY_ALC_OFF_DETECTION_THRESHOLD                       (4)

// fixed point macroses
#define SINGL_MULT_FACT_PREC                                    (15)
#define MULT_FACTOR_X_255                                       (255 << SINGL_MULT_FACT_PREC)

#define ALCOK                                                   (0)

#define TM_EMPTY                                                (0)
#define TM_FILLED                                               (1)

void ALC_Create();
void ALC_Destroy();
int  ALC_GetTM(  XPel **ppRef, int RefTM_startY, int RefTM_startX, 
                XPel **ppDec, int DecTM_startY, int DecTM_startX, int Blk_height, int Blk_width);
Boolean IsTMFilled();
void    ALC_TurnOFF();
XPel**  ALC_RetTM_buf();
int**   ALC_RetTM_tempbuf();
void    PiLC_TMPelCorHist_2DArray( XPel **ppOut, int   XOut,                              int h, int w );
void    PiLC_TMPelCorHist_1DArray( XPel  *pIn,   XPel *pOut, int StrideIn, int StrideOut, int h, int w );

#endif

#endif //__ALC_H__
