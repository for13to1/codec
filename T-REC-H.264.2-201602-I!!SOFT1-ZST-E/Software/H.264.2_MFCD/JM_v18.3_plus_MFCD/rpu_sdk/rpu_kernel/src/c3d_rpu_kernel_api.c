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
* \file  c3d_rpu_kernel_api.c
*
* \brief 
*        MFC SDK  RPU Kernal API functions 
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
#include "c3d_rpu_kernel_utils.h"
#include "c3d_rpu_kernel_pvt.h"
#include "c3d_rpu_kernel_filter_fn_pvt.h"


void* rpu_init()
{
    RPUHandle *p_rpu_handle = NULL;
    

    /* Allocate memory for the handle */
    p_rpu_handle = (RPUHandle *) allocate_memory(sizeof(RPUHandle));
    /* If the pointer to RPU handle is NULL, then return NULL */
    if(NULL == p_rpu_handle)
    {
        return NULL;
    } /* if(NULL == p_rpu_handle) */

    init_om_up_down_filters(p_rpu_handle);
    /* 
     * Allocate memory for the image buffer used to store intermediate result in 2D separable fitlering.
     * As there is no information about the image, memory is allocated using default values.
     */
    p_rpu_handle->p_temp_img = alloc_image_mem(
                                               MAX_FRAME_WIDTH,
                                               MAX_FRAME_HEIGHT,
                                               MAX_FRAME_STRIDE,
                                               YUV_444,                                               
                                               0
                                              );
    /* If the pointer to temp image is NULL, then return NULL */
    if(NULL == p_rpu_handle->p_temp_img)
    {
        rpu_close((void *) p_rpu_handle);
        return NULL;
    } /* if(NULL == p_rpu_handle->p_temp_img) */


    return (void *) p_rpu_handle;

} /* End of rpu_init() function */


int32_t rpu_process(
                    void            *pv_rpu_handle,
                    const ImageData *p_bl,
                          ImageData *p_pred_el,
                    const RPUData   *p_rpu_data
                   )
{
    int32_t i32_status = SUCCESS;

    i32_status = om_rpu_process(pv_rpu_handle,p_bl,p_pred_el,p_rpu_data);


    return i32_status;
}/* End of rpu_process() */



int32_t rpu_close(
                  void *pv_rpu_handle
                 )
{
    int32_t i32_status = SUCCESS;

    RPUHandle *p_rpu_handle = (RPUHandle *) pv_rpu_handle;

    if(NULL !=p_rpu_handle)
    {
        /* p_rpu_handle
         * Free the memory allocated for the temporary image buffer used to store intermediate result in 2D separable 
         * fitlering.
         */
        free_image_mem(p_rpu_handle->p_temp_img);

        /* Free the memory allocated to the RPU handle */
        free_memory((void *) p_rpu_handle);
        p_rpu_handle = NULL;
    }

    return i32_status;

} /* End of rpu_close() function */





int32_t  upSampleSBSImage( 
                        ImageData                *p_view0_image,
                        ImageData                *p_view1_image,
                const    ImageData               *p_muxed_image,
                const    RPUFilter               *p_demux_filter,
                        uint8_t                  u8_view0_offset,
                        uint8_t                  u8_view1_offset,
                        uint8_t                  u8_packed_UV            
                        )
{
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
        filter_params.u16_origin_y            = 0;
        filter_params.u16_view_offset_x       = u8_view0_offset;
        filter_params.u16_center_start_x      = p_demux_filter->u16_taps_div2 - !u8_view0_offset;
        filter_params.u16_center_end_x        = p_muxed_image->au16_view_delimiter_sbs[u32_cmp]- (p_demux_filter->u16_taps_div2 -  u8_view0_offset);
        filter_params.u16_end_x               = p_muxed_image->au16_view_delimiter_sbs[u32_cmp];
        filter_params.u16_end_y               = 2*p_muxed_image->au16_view_delimiter_ou[u32_cmp];

        
        /* Upsample Left Half */
        p_in   = p_muxed_image->pa_buf[u32_cmp];
        p_out  = p_view0_image->pa_buf[u32_cmp];
        

        om_rpu_upsample_horizontal_filter(   p_demux_filter,
                                            &filter_params,
                                            p_in,
                                            p_muxed_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_view0_image->au16_buffer_stride[u32_cmp],
                                            u32_cmp==0?0:u8_packed_UV
                                            );

        filter_params.u16_origin_x            = 0;
        filter_params.u16_origin_y            = 0;
        filter_params.u16_view_offset_x       = u8_view1_offset;
        filter_params.u16_center_start_x      = p_demux_filter->u16_taps_div2 - !u8_view1_offset;
        filter_params.u16_center_end_x        = p_view0_image->au16_view_delimiter_sbs[u32_cmp] - (p_demux_filter->u16_taps_div2 -  u8_view1_offset);
        filter_params.u16_end_x               = p_view0_image->au16_view_delimiter_sbs[u32_cmp];
        filter_params.u16_end_y               = 2*p_view0_image->au16_view_delimiter_ou[u32_cmp];

        /* Upsample Right Half */
        p_in   = p_muxed_image->pa_buf[u32_cmp] +  p_muxed_image->au16_view_delimiter_sbs[u32_cmp];
        p_out  = p_view1_image->pa_buf[u32_cmp];


        om_rpu_upsample_horizontal_filter(    p_demux_filter,
                                            &filter_params,
                                            p_in,
                                            p_muxed_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_view1_image->au16_buffer_stride[u32_cmp],
                                            u32_cmp==0?0:u8_packed_UV
                                            );


    }
    
    
    return i32_status;

}/* end of upSampleSBSImage() */


int32_t  upSampleOUImage( 
                        ImageData                *p_view0_image,
                        ImageData                *p_view1_image,
                const    ImageData               *p_muxed_image,
                const    RPUFilter               *p_demux_filter,
                        uint8_t                  u8_view0_offset,
                        uint8_t                  u8_view1_offset,
                        uint8_t                  u8_packed_UV            
                        )
{
    const img_pel_t *p_in = NULL;
    img_pel_t *p_out = NULL;
    uint8_t  u8_num_components;
    uint32_t u32_cmp;
    int32_t i32_status=SUCCESS;

    RPUFilterParams filter_params;

    u8_num_components = u8_packed_UV==0 ? 3 : 2;

    for(u32_cmp = 0; u32_cmp < u8_num_components; u32_cmp++)
    {

        
        filter_params.u16_origin_x             = 0;
        filter_params.u16_origin_y             = 0;
        filter_params.u16_view_offset_y        = u8_view0_offset;
        filter_params.u16_center_start_y       = p_demux_filter->u16_taps_div2 - !u8_view0_offset;
        filter_params.u16_center_end_y         = p_muxed_image->au16_view_delimiter_ou[u32_cmp]- (p_demux_filter->u16_taps_div2 -  u8_view0_offset);
        filter_params.u16_end_x                = 2*p_muxed_image->au16_view_delimiter_sbs[u32_cmp];
        filter_params.u16_end_y                = p_muxed_image->au16_view_delimiter_ou[u32_cmp];

        /* Upsample Top Half */
        p_in   = p_muxed_image->pa_buf[u32_cmp];
        p_out  = p_view0_image->pa_buf[u32_cmp];
        

        om_rpu_upsample_vertical_filter(    p_demux_filter,
                                            &filter_params,
                                            p_in,
                                            p_muxed_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_view0_image->au16_buffer_stride[u32_cmp]
                                            );

        /* Upsample Bottom Half */
        filter_params.u16_origin_x             = 0;
        filter_params.u16_origin_y             = 0;
        filter_params.u16_view_offset_y        = u8_view1_offset;
        filter_params.u16_center_start_y       = p_demux_filter->u16_taps_div2 - !u8_view1_offset;
        filter_params.u16_center_end_y         = p_view0_image->au16_view_delimiter_ou[u32_cmp] - (p_demux_filter->u16_taps_div2 -  u8_view1_offset);
        filter_params.u16_end_x                = 2*p_view0_image->au16_view_delimiter_sbs[u32_cmp];
        filter_params.u16_end_y                = p_view0_image->au16_view_delimiter_ou[u32_cmp];


        p_in   = p_muxed_image->pa_buf[u32_cmp] +  p_muxed_image->au16_buffer_stride[u32_cmp] * p_muxed_image->au16_view_delimiter_ou[u32_cmp];
        p_out  = p_view1_image->pa_buf[u32_cmp];


        om_rpu_upsample_vertical_filter(   p_demux_filter,
                                            &filter_params,
                                            p_in,
                                            p_muxed_image->au16_buffer_stride[u32_cmp],
                                            p_out,
                                            p_view1_image->au16_buffer_stride[u32_cmp]
                                            );


    }/* end of u32_cmp */
    
    
    return i32_status;

}/* end of upSampleOUImage() */


int32_t  add_diff_image_with_offset( 
                        ImageData              *p_dst_image,
                const   ImageData             *p_src_image1,
                const   ImageData             *p_src_image2,
                        uint8_t                u8_offset,
                        uint8_t                u8_mode
                        )
{


    int32_t u32_success=SUCCESS;
    uint16_t u16_y_loop,u16_x_loop;
    img_pel_t *pui8Src1,*pui8Src2,*pui8Dst;    
    uint32_t u32_cmp;
    
    int16_t i16_Offset;    
    int16_t i16_temp[8];

    i16_Offset  = u8_mode==0 ? (int16_t)u8_offset : ((int16_t)u8_offset)*-1;

    if(0 == u8_mode)
    {
        /* sum images */
        for(u32_cmp=0;u32_cmp<3;u32_cmp++)
        {
        
            for(u16_y_loop=0 ; u16_y_loop < p_src_image1->au16_frame_height[u32_cmp] ; u16_y_loop++)
            {
                pui8Src1 = p_src_image1->pa_buf[u32_cmp] + u16_y_loop* p_src_image1->au16_buffer_stride[u32_cmp]  ;
                pui8Src2 = p_src_image2->pa_buf[u32_cmp] + u16_y_loop* p_src_image2->au16_buffer_stride[u32_cmp]  ;
                pui8Dst  = p_dst_image->pa_buf[u32_cmp] + u16_y_loop* p_dst_image->au16_buffer_stride[u32_cmp]  ;

                assert(p_src_image1->au16_frame_width[u32_cmp]%8==0);
                //  unroll the loop for efficient processing
                u16_x_loop = p_src_image1->au16_frame_width[u32_cmp] >> 3;

                do
                {
                i16_temp[0] = (int16_t)pui8Src1[0] - (int16_t)pui8Src2[0] +i16_Offset;
                i16_temp[1] = (int16_t)pui8Src1[1] - (int16_t)pui8Src2[1] +i16_Offset;
                i16_temp[2] = (int16_t)pui8Src1[2] - (int16_t)pui8Src2[2] +i16_Offset;
                i16_temp[3] = (int16_t)pui8Src1[3] - (int16_t)pui8Src2[3] +i16_Offset;
                i16_temp[4] = (int16_t)pui8Src1[4] - (int16_t)pui8Src2[4] +i16_Offset;
                i16_temp[5] = (int16_t)pui8Src1[5] - (int16_t)pui8Src2[5] +i16_Offset;
                i16_temp[6] = (int16_t)pui8Src1[6] - (int16_t)pui8Src2[6] +i16_Offset;
                i16_temp[7] = (int16_t)pui8Src1[7] - (int16_t)pui8Src2[7] +i16_Offset;

                pui8Dst[0] =(uint8_t) CLIP(i16_temp[0]);    
                pui8Dst[1] =(uint8_t) CLIP(i16_temp[1]);             
                pui8Dst[2] =(uint8_t) CLIP(i16_temp[2]);             
                pui8Dst[3] =(uint8_t) CLIP(i16_temp[3]);             
                pui8Dst[4] =(uint8_t) CLIP(i16_temp[4]);             
                pui8Dst[5] =(uint8_t) CLIP(i16_temp[5]);             
                pui8Dst[6] =(uint8_t) CLIP(i16_temp[6]);             
                pui8Dst[7] =(uint8_t) CLIP(i16_temp[7]);             


                pui8Src1 += 8;
                pui8Src2 += 8;
                pui8Dst  += 8;

                }while(--u16_x_loop);
            }
        }
    }
    else
    {   /* subtract images */
        for(u32_cmp=0;u32_cmp<3;u32_cmp++)
        {
        
            for(u16_y_loop=0 ; u16_y_loop < p_src_image1->au16_frame_height[u32_cmp] ; u16_y_loop++)
            {
                pui8Src1 = p_src_image1->pa_buf[u32_cmp] + u16_y_loop* p_src_image1->au16_buffer_stride[u32_cmp]  ;
                pui8Src2 = p_src_image2->pa_buf[u32_cmp] + u16_y_loop* p_src_image2->au16_buffer_stride[u32_cmp]  ;
                pui8Dst  = p_dst_image->pa_buf[u32_cmp] + u16_y_loop* p_dst_image->au16_buffer_stride[u32_cmp]  ;

                assert(p_src_image1->au16_frame_width[u32_cmp]%8==0);
                //  unroll the loop for efficient processing
                u16_x_loop = p_src_image1->au16_frame_width[u32_cmp] >> 3;

                do
                {
                i16_temp[0] = (int16_t)pui8Src1[0] + (int16_t)pui8Src2[0] +i16_Offset;
                i16_temp[1] = (int16_t)pui8Src1[1] + (int16_t)pui8Src2[1] +i16_Offset;
                i16_temp[2] = (int16_t)pui8Src1[2] + (int16_t)pui8Src2[2] +i16_Offset;
                i16_temp[3] = (int16_t)pui8Src1[3] + (int16_t)pui8Src2[3] +i16_Offset;
                i16_temp[4] = (int16_t)pui8Src1[4] + (int16_t)pui8Src2[4] +i16_Offset;
                i16_temp[5] = (int16_t)pui8Src1[5] + (int16_t)pui8Src2[5] +i16_Offset;
                i16_temp[6] = (int16_t)pui8Src1[6] + (int16_t)pui8Src2[6] +i16_Offset;
                i16_temp[7] = (int16_t)pui8Src1[7] + (int16_t)pui8Src2[7] +i16_Offset;


                pui8Dst[0] =(uint8_t) CLIP(i16_temp[0]);    
                pui8Dst[1] =(uint8_t) CLIP(i16_temp[1]);             
                pui8Dst[2] =(uint8_t) CLIP(i16_temp[2]);             
                pui8Dst[3] =(uint8_t) CLIP(i16_temp[3]);             
                pui8Dst[4] =(uint8_t) CLIP(i16_temp[4]);             
                pui8Dst[5] =(uint8_t) CLIP(i16_temp[5]);             
                pui8Dst[6] =(uint8_t) CLIP(i16_temp[6]);             
                pui8Dst[7] =(uint8_t) CLIP(i16_temp[7]);                   


                pui8Src1 += 8;
                pui8Src2 += 8;
                pui8Dst  += 8;

                }while(--u16_x_loop);
            }
        }

    }
    return u32_success;
}/* End of add_diff_image_with_offset() */
