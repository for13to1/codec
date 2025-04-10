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
* \file  c3d_rpu_kernel_init.c
*
* \brief 
*        MFC SDK  RPU Kernal initialize OM rpu filters
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


/* User defined header files */
#include "c3d_rpu_kernel_api.h"
#include "c3d_rpu_kernel_pvt.h"
#include "c3d_rpu_kernel_filter_fn_pvt.h"


/* OM Down Filter Definitions */
static const uint16_t u16_om_down_filter_length = 5;
static const uint16_t u16_om_down_dynamic_range = 6 ;
static const uint16_t u16_om_down_offset = 32;
static const int16_t  i16_om_down_coef1[MAX_NUM_FILTER_TAPS] = {4,   7,  10,  7,  4};

#define USE_4TAP_UPSAMPLER 0

#if (1==USE_4TAP_UPSAMPLER)
/* OM Up Filter Definitions */
static const uint16_t u16_om_up_filter_length = 4;
static const uint16_t u16_om_up_dynamic_range = 7 ;
static const uint16_t u16_om_up_offset = 64;
static const int16_t  i16_om_up_coef1[MAX_NUM_FILTER_TAPS] = {-11, 75, 75 , -11};

/* Demux Up Filter Definitions */
static const uint16_t u16_demux_filter_length = 4;
static const uint16_t u16_demux_dynamic_range = 7 ;
static const uint16_t u16_demux_offset = 64;
static const int16_t  i16_demux_coef1[MAX_NUM_FILTER_TAPS] = {-11, 75, 75 , -11};

#else
/* OM Up Filter Definitions */
static const uint16_t u16_om_up_filter_length = 6;
static const uint16_t u16_om_up_dynamic_range = 7 ;
static const uint16_t u16_om_up_offset = 64;
static const int16_t  i16_om_up_coef1[MAX_NUM_FILTER_TAPS] = {  3, -17 , 78 , 78, -17, 3};

/* Demux Up Filter Definitions */
static const uint16_t u16_demux_filter_length = 6;
static const uint16_t u16_demux_dynamic_range = 7 ;
static const uint16_t u16_demux_offset = 64;
static const int16_t  i16_demux_coef1[MAX_NUM_FILTER_TAPS] = {  3, -17 , 78 , 78, -17, 3};

#endif




int32_t init_om_up_down_filters(
                         RPUHandle *p_rpu_handle
                        )
{
    int32_t i32_status = SUCCESS;

    RPUFilter *p_rpu_om_down_filter = &p_rpu_handle->rpu_om_down_filter;
    RPUFilter *p_rpu_om_up_filter   = &p_rpu_handle->rpu_om_up_filter;

    int32_t i32_i;    

    /**************** Setup OM Down Filter ***************/

    /* Filter length */
    p_rpu_om_down_filter->u16_num_taps  = u16_om_down_filter_length;  
    p_rpu_om_down_filter->u16_taps_div2 = SHIFT_RIGHT(p_rpu_om_down_filter->u16_num_taps, 1);

    /* Filter taps */
    for (i32_i = 0; i32_i < p_rpu_om_down_filter->u16_num_taps; i32_i++)
    {
        p_rpu_om_down_filter->ai16_coef1[i32_i] = i16_om_down_coef1[i32_i];
    } /* for (i32_j = 0; i32_j < p_flt_1D->u16_num_taps; i32_j++) */
            
    /* Dynamic range of the filter */
    p_rpu_om_down_filter->u16_normal1 = u16_om_down_dynamic_range;

    /* Filter offset */
    p_rpu_om_down_filter->u16_offset = u16_om_down_offset;

    /**************** Setup OM Up Filter ***************/

    /* Filter length */
    p_rpu_om_up_filter->u16_num_taps  = u16_om_up_filter_length;  
    p_rpu_om_up_filter->u16_taps_div2 = SHIFT_RIGHT(p_rpu_om_up_filter->u16_num_taps, 1);

    /* Filter taps */
    for (i32_i = 0; i32_i < p_rpu_om_up_filter->u16_num_taps; i32_i++)
    {
        p_rpu_om_up_filter->ai16_coef1[i32_i] = i16_om_up_coef1[i32_i];
    } /* for (i32_j = 0; i32_j < p_flt_1D->u16_num_taps; i32_j++) */
            
    /* Dynamic range of the filter */
    p_rpu_om_up_filter->u16_normal1 = u16_om_up_dynamic_range;

    /* Filter offset */
    p_rpu_om_up_filter->u16_offset = u16_om_up_offset;



    return i32_status;

} /* End of init_om_up_down_filters() function */


int32_t init_demuxing_filter(RPUFilter *p_demux_filter)
{
    int32_t i32_i;  
    int32_t i32_status = SUCCESS; 
    /* Filter length */
    p_demux_filter->u16_num_taps  = u16_demux_filter_length;  
    p_demux_filter->u16_taps_div2 = SHIFT_RIGHT(p_demux_filter->u16_num_taps, 1);

    /* Filter taps */
    for (i32_i = 0; i32_i < p_demux_filter->u16_num_taps; i32_i++)
    {
        p_demux_filter->ai16_coef1[i32_i] = i16_demux_coef1[i32_i];
    } /* for (i32_j = 0; i32_j < p_flt_1D->u16_num_taps; i32_j++) */
            
    /* Dynamic range of the filter */
    p_demux_filter->u16_normal1 = u16_demux_dynamic_range;

    /* Filter offset */
    p_demux_filter->u16_offset = u16_demux_offset;

    return i32_status;


}/* End of init_demuxing_filter() */