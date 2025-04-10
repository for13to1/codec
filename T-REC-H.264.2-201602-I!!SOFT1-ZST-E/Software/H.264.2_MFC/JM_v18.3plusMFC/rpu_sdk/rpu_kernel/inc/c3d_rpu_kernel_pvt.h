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
* \file  c3d_rpu_kernel_pvt.h
*
* \brief 
*        MFC SDK  RPU internal structures
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_RPU_KERNEL_PVT_H_
#define _C3D_RPU_KERNEL_PVT_H_

/** ENUMERATIONS */

/** SBS_VIEW */
typedef enum sbs_view
{
    LEFT = 0,
    RIGHT
} SBS_VIEW;

/** OU_VIEW */
typedef enum ou_view
{
    TOP = 0,
    BOTTOM
} OU_VIEW;

/** STRUCTURES */

/**
 * RPU Handle
 * This is the handle to the RPU. It contains pointers to RPU filters.
 */
typedef struct rpu_handle
{
    /*
     * Array of filters
     * Depending upon the RPU process format, the corresponding SBS or OU implicit filters 
     * are initialized at init.  Used by OM Up Filters .
     */
    RPUFilter    rpu_om_up_filter;

    /*
     * Array of filters for OM Down filters are initialized at init.
     */
    RPUFilter    rpu_om_down_filter;

    /* Image buffer pointer that is used to store the intermediate result in 2D separable filtering. */
    ImageData    *p_temp_img;

} RPUHandle;

#endif /* _C3D_RPU_KERNEL_PVT_H_ */
