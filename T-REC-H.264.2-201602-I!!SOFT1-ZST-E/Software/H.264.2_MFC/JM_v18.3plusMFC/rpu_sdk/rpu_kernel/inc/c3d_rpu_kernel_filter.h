/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2014, International Telecommunications Union, Geneva
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
* \file  c3d_rpu_kernel_filter.h
*
* \brief 
*        MFC SDK  RPU filters structure 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*         - Santosh Chilkunda     (santosh.chilkunda@dolby.com)
*
*************************************************************************************
*/


#ifndef _C3D_RPU_KERNEL_FILTER_H_
#define _C3D_RPU_KERNEL_FILTER_H_

/* Standard header files */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** STRUCTURES */

/** 1D filter definition structure */
typedef struct rpu_filter
{
    /* Filter Coefficient Values */
    int16_t     ai16_coef1[MAX_NUM_FILTER_TAPS];
    
    /* Length of the filter */
    uint16_t    u16_num_taps;

    /* Normalizing factor of the filter */
    uint16_t    u16_normal1;
    
    /* Half the number  of taps */
    uint16_t    u16_taps_div2;

   /* Filter offset to added before normalizing */
    uint16_t    u16_offset;
    
} RPUFilter;






/** 
 * RPU filter params 
 * The partition is divided into center and boundary regions. Boundary pixels are duplicated before filtering. This 
 * structure contains the spatial information about the center and boundary regions.
 */
typedef struct rpu_filter_params
{
    /* Origin */
    uint16_t    u16_origin_x;
    uint16_t    u16_origin_y;

    /* End */
    uint16_t    u16_end_x;    
    uint16_t    u16_end_y;

    /* Center region */
    uint16_t    u16_center_start_x;
    uint16_t    u16_center_start_y;
    uint16_t    u16_center_end_x;    
    uint16_t    u16_center_end_y;

    /* X and Y Offsets */
    uint16_t    u16_view_offset_x;
    uint16_t    u16_view_offset_y;


    
} RPUFilterParams;



#endif /* _C3D_RPU_KERNEL_FILTER_H_ */
