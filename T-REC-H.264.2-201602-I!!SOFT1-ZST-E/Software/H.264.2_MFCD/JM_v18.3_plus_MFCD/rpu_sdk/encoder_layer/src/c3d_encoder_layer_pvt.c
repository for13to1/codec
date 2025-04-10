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
* \file  c3d_encoder_layer_pvt.c
*
* \brief 
*        MFC SDK  Encoder layer internal functions 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/

/* User defined header files */

#include "c3d_encoder_layer_api.h"
#include "c3d_encoder_layer_pvt.h"
#include "c3d_generic_filters.h"



int32_t mux_side_by_side(
              ImageData       *p_muxed_image,
    const     ImageData       *p_view0_image,
    const     ImageData       *p_view1_image,
    const     RPUFilter       *p_mux_filter,
              uint8_t          u8_view0_offset,
              uint8_t          u8_view1_offset,
              uint8_t          u8_packed_UV            
            ){

const img_pel_t *p_in = NULL;
img_pel_t *p_out = NULL;
uint8_t  u8_num_components;
uint32_t u32_cmp;
int32_t i32_status=SUCCESS;



RPUFilterParams filter_params;

    u8_num_components = u8_packed_UV==0 ? 3 : 2;

    for(u32_cmp = 0; u32_cmp < u8_num_components; u32_cmp++)
    {

        
        filter_params.u16_origin_x            = u8_view0_offset;
        filter_params.u16_origin_y            = 0;
        filter_params.u16_center_start_x      = p_mux_filter->u16_taps_div2;
        filter_params.u16_center_end_x        = 2*p_view0_image->au16_view_delimiter_sbs[u32_cmp] - p_mux_filter->u16_taps_div2;
        filter_params.u16_end_x               = 2*p_view0_image->au16_view_delimiter_sbs[u32_cmp];
        filter_params.u16_end_y               = 2*p_view0_image->au16_view_delimiter_ou[u32_cmp];

        p_in  = p_view0_image->pa_buf[u32_cmp]  + u8_view0_offset;
        p_out = p_muxed_image->pa_buf[u32_cmp];

        /* Filter and DownSample View 0 */
        horizontal_filter_and_downsample(
                                            p_mux_filter,
                                            &filter_params,
                                            p_in,
                                            p_view0_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_muxed_image->au16_buffer_stride[u32_cmp],
                                            u32_cmp==0?0:u8_packed_UV
                                            );
        
        filter_params.u16_origin_x            = u8_view1_offset;
        p_in  = p_view1_image->pa_buf[u32_cmp]  + u8_view1_offset;
        p_out = p_muxed_image->pa_buf[u32_cmp] + p_muxed_image->au16_view_delimiter_sbs[u32_cmp];


        /* Filter and DownSample View 1 */
        horizontal_filter_and_downsample(
                                            p_mux_filter,
                                            &filter_params,
                                            p_in,
                                            p_view1_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_muxed_image->au16_buffer_stride[u32_cmp],
                                            u32_cmp==0?0:u8_packed_UV
                                            );


    }
    
    
    return i32_status;

}/* end of mux_side_by_side() */


int32_t mux_over_under(
            ImageData                *p_muxed_image,
    const    ImageData                *p_view0_image,
    const    ImageData                *p_view1_image,
    const    RPUFilter                *p_mux_filter,            
            uint8_t                    u8_view0_offset,
            uint8_t                    u8_view1_offset,
            uint8_t                    u8_packed_UV            
            ){

const img_pel_t *p_in = NULL;
img_pel_t *p_out = NULL;
uint8_t  u8_num_components;
uint32_t u32_cmp;
int32_t i32_status=SUCCESS;
RPUFilterParams filter_params;

    u8_num_components = u8_packed_UV==0 ? 3 : 2;

    for(u32_cmp = 0; u32_cmp < u8_num_components; u32_cmp++)
    {

        
        filter_params.u16_origin_x            = 0;
        filter_params.u16_origin_y            = u8_view0_offset;
        filter_params.u16_center_start_y      = p_mux_filter->u16_taps_div2;
        filter_params.u16_center_end_y        = 2*p_view0_image->au16_view_delimiter_ou[u32_cmp] - p_mux_filter->u16_taps_div2;
        filter_params.u16_end_x               = 2*p_view0_image->au16_view_delimiter_sbs[u32_cmp];
        filter_params.u16_end_y               = 2*p_view0_image->au16_view_delimiter_ou[u32_cmp];

        p_in  = p_view0_image->pa_buf[u32_cmp]  +(u8_view0_offset * p_view0_image->au16_buffer_stride[u32_cmp]) ;
        p_out = p_muxed_image->pa_buf[u32_cmp];

        /* Filter and DownSample View 1 */
        vertical_filter_and_downsample(
                                            p_mux_filter,
                                            &filter_params,
                                            p_in,
                                            p_view0_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_muxed_image->au16_buffer_stride[u32_cmp]
                                            );
        filter_params.u16_origin_y            = u8_view1_offset;
        p_in  = p_view1_image->pa_buf[u32_cmp]  + (u8_view1_offset * p_view0_image->au16_buffer_stride[u32_cmp]) ;
        p_out = p_muxed_image->pa_buf[u32_cmp] +  (p_muxed_image->au16_view_delimiter_ou[u32_cmp] * p_muxed_image->au16_buffer_stride[u32_cmp]) ;

        /* Filter and DownSample View 1 */
        vertical_filter_and_downsample(
                                            p_mux_filter,
                                            &filter_params,
                                            p_in,
                                            p_view1_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_muxed_image->au16_buffer_stride[u32_cmp]
                                            );


    }
    
    
    return i32_status;

}/* End of mux_over_under() */

