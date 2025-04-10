/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2016, International Telecommunications Union, Geneva
*
* DISCLAIMER OF WARRANTY
*
* These software programs are available to the user without any
* license fee or royalty on an "as is" basis. The ITU disclaims
* any and all warranties, whether express, implied, or
* statutory, including any implied warranties of merchantability
* or of fitness for a particular purpose.  In no event shall the
* contributor or the ITU be liable for any incidental, punitive, or
* consequential damages of any kind whatsoever arising from the
* use of these programs.
*
* This disclaimer of warranty extends to the user of these programs
* and user's customers, employees, agents, transferees, successors,
* and assigns.
*
* The ITU does not represent or warrant that the programs furnished
* hereunder are free of infringement of any third-party patents.
* Commercial implementations of ITU-T Recommendations, including
* shareware, may be subject to royalty fees to patent holders.
* Information regarding the ITU-T patent policy is available from
* the ITU Web site at http://www.itu.int.
*
* THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
************************************************************************
*/


/*!
*************************************************************************************
* \file  c3d_rpu_kernel_utils.h
*
* \brief 
*
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/



#ifndef _C3D_RPU_KERNEL_UTILS_H_
#define _C3D_RPU_KERNEL_UTILS_H_

/** FUNCTION MACROS */

/** Absolute value */
#define ABS_VAL(a)                  (((a) >= 0) ? (a) : -(a))

/**
 * The function macro shifts the input right by val.
 * Right shift by a negative integer is interpreted as left shift by its absolute value.
 */
#define SHIFT_RIGHT(in, shift)      ((shift) >= 0 ? ((in) >> (shift)) : ((in) << ABS_VAL(shift)))


/** The greater of two numbers */
#define MAX_VAL(a, b)               (((a) > (b)) ? (a) : (b))

/** The smaller of two numbers */
#define MIN_VAL(a, b)               (((a) < (b)) ? (a) : (b))

/* clip 16-bit signed input to 8-bit unsigned value */
#define CLIP(x)                     (MIN_VAL((MAX_VAL((x), (0))), (255)))
/*#define CLIP(x)                   u8ClipTable[(uint16_t)x] */

/* add offset and shift right */
#define OFFSET_SHIFT_RIGHT(x, o, a) (((x) +(o))>> (a))




#endif /* _C3D_RPU_KERNEL_UTILS_H_ */
