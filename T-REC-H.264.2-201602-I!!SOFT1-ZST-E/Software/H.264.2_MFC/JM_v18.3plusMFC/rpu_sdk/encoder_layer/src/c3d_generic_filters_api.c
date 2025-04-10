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
* \file  c3d_generic_filters_api.c
*
* \brief 
*        MFC SDK  Generic filter definitions
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

/** 
 *\fn void vertical_downsample_unsafe_top_positions(img_pel_t    *p_Dst,
 *                                        const img_pel_t    *p_Src,
 *                                        uint16_t    u16_width,
 *                                        uint16_t    u16_start_y,
 *                                        uint16_t    u16_end_y,
 *                                        uint16_t    u16_input_stride,
 *                                        uint16_t    u16_output_stride,
 *                                        const RPUFilter    *p_flt_1d
 *                                        )
 * \brief Vertcial Downsampling of unsafe top pixels.
 * 
 * \details The boundary rows that needs pixels repetetion while downsampling vertically is handled here.The number of unsafe
 * top pixels depend of the filter length.(Stage 1 of OM RPU for SBS)
 *
 * \param[in]    p_Dst                    Pointer to the output buffer
 * \param[in]    p_Src                    Pointer to the input buffer
 * \param[in]    u16_width                number for horizontal pixels to be processed while downsampling vertically
 * \param[in]    u16_start_y                starting row position of unsafe top pixels. 
 * \param[in]    u16_end_y                ending row position of unsafe top pixels.
 * \param[in]    u16_input_stride        Input buffer stride.
 * \param[in]    u16_output_stride        Output buffer stride.
 * \param[in]    p_flt_1d                Pointer to the 1D filter data structure to be used for downsampling.
 *
 * \return None
 */

void vertical_downsample_unsafe_top_positions(img_pel_t    *p_Dst,
                                        const img_pel_t    *p_Src,
                                        uint16_t    u16_width,
                                        uint16_t    u16_start_y,
                                        uint16_t    u16_end_y,
                                        uint16_t    u16_input_stride,
                                        uint16_t    u16_output_stride,
                                        const RPUFilter    *p_flt_1d)
{
    const img_pel_t     *p_SrcWork , *p_SrcWork_Inner;
    const int16_t *pi_coeff;
    int16_t i16_col_loop,i16_row_loop,i16_filter_loop,i16_dstIndx,i16_pix_pos;
    int32_t i32_mac = 0 ;
    img_pel_t  *p_DstWork ;

    /* Downsample only every other row */
    for(i16_col_loop=u16_start_y,i16_dstIndx=0; i16_col_loop<u16_end_y; i16_col_loop+=2,i16_dstIndx++)            
    {
        p_DstWork = p_Dst;
        p_SrcWork  =p_Src;

        /* For each row downsample all the columns */
        for(i16_row_loop=0; i16_row_loop<u16_width; i16_row_loop++)
        {
            p_SrcWork_Inner = p_SrcWork++;
            pi_coeff = p_flt_1d->ai16_coef1;
            i32_mac=0;
            /* multiply and accumulate for filtering */
            for(i16_filter_loop =0 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
            {
                i16_pix_pos = - p_flt_1d->u16_taps_div2 + i16_filter_loop ;        
                i16_pix_pos = MAX_VAL(i16_pix_pos,-i16_col_loop);
                i32_mac += * (p_SrcWork_Inner  + (u16_input_stride *i16_pix_pos)) * *pi_coeff++;                
            }    /* i16_filter_loop        */        
            *p_DstWork++   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

        }/* i16_row_loop        */

        /* Increment the src pointer by two rows for next iteration */
        p_Dst += u16_output_stride;
        /* Increment the dst pointer by one row for next iteration */
        p_Src += 2 * u16_input_stride;
    }/* i16_col_loop    */


} /* End of vertical_downsample_unsafe_top_positions function */

/** 
 *\fn void vertical_downsample_safe_positions(img_pel_t    *p_Dst,
 *                                        const img_pel_t    *p_Src,
 *                                        uint16_t    u16_width,
 *                                        uint16_t    u16_origin_y,
 *                                        uint16_t    u16_start_y,
 *                                        uint16_t    u16_end_y,
 *                                        uint16_t    u16_input_stride,
 *                                        uint16_t    u16_output_stride,
 *                                        const RPUFilter    *p_flt_1d
 *                                        )
 * \brief Vertcial Downsampling of safe middle pixels. (Stage 1 of OM RPU for SBS)
 * 
 * \details 
 *
 * \param[in]    p_Dst                    Pointer to the output buffer
 * \param[in]    p_Src                    Pointer to the input buffer
 * \param[in]    u16_width                number for horizontal pixels to be processed while downsampling vertically
 * \param[in]    u16_origin_y            origin row  of the partition.
 * \param[in]    u16_start_y                starting row position of safe pixels. 
 * \param[in]    u16_end_y                ending row position of safe pixels.
 * \param[in]    u16_input_stride        Input buffer stride.
 * \param[in]    u16_output_stride        Output buffer stride. 
 * \param[in]    p_flt_1d                Pointer to the 1D filter data structure to be used for downsampling.
 *
 * \return None
 */
void vertical_downsample_safe_positions(img_pel_t    *p_Dst,
                                  const img_pel_t    *p_Src,
                                        uint16_t    u16_width,
                                        uint16_t    u16_origin_y,
                                        uint16_t    u16_start_y,
                                        uint16_t    u16_end_y,
                                        uint16_t    u16_input_stride,
                                        uint16_t    u16_output_stride,
                                  const RPUFilter    *p_flt_1d)
{
    const img_pel_t     *p_SrcWork , *p_SrcWork_Inner;
    const int16_t *pi_coeff;
    img_pel_t  *p_DstWork ;
    int16_t i16_col_loop,i16_row_loop,i16_filter_loop;    
    int32_t i32_mac = 0;
    
    u16_start_y = u16_start_y  + (u16_start_y - u16_origin_y)%2;

    p_Dst = p_Dst + ((u16_start_y-u16_origin_y)>>1) * (u16_output_stride);
    p_Src = p_Src +   (u16_start_y-u16_origin_y-p_flt_1d->u16_taps_div2) * u16_input_stride;

    /* Downsample only every other row */
    for(i16_col_loop=u16_start_y; i16_col_loop<u16_end_y; i16_col_loop+=2)            
    {        
        p_DstWork = p_Dst;
        p_SrcWork = p_Src;

        /* For each row downsample all the columns */
        for(i16_row_loop=0; i16_row_loop<u16_width; i16_row_loop++)
        {
            p_SrcWork_Inner = p_SrcWork++;
            pi_coeff = p_flt_1d->ai16_coef1;
            i32_mac=0;

            /* multiply and accumulate for filtering */
            for(i16_filter_loop =0 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
            {
                i32_mac += *p_SrcWork_Inner * *pi_coeff++;
                p_SrcWork_Inner +=  u16_input_stride;
            }/* i16_filter_loop */

            *p_DstWork++  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

        } /* i16_row_loop */

        /* Increment the src pointer by two rows for next iteration */
        p_Src += 2 * u16_input_stride;
        /* Increment the dst pointer by one row for next iteration */
        p_Dst += u16_output_stride;

    } /* i16_col_loop*/

}/* End of vertical_downsample_safe_positions function */

/** 
 *\fn void vertical_downsample_unsafe_bottom_positions(img_pel_t    *p_Dst,
 *                                        const img_pel_t    *p_Src,
 *                                        uint16_t    u16_width,
 *                                        uint16_t    u16_origin_y,
 *                                        uint16_t    u16_start_y,
 *                                        uint16_t    u16_end_y,
 *                                        uint16_t    u16_input_stride,
 *                                        uint16_t    u16_output_stride,
 *                                        const RPUFilter    *p_flt_1d
 *                                        )
 * \brief Vertcial Downsampling of unsafe bottom pixels. (Stage 1 of OM RPU for SBS)
 * 
 * \details 
 *
 * \param[in]    p_Dst                    Pointer to the output buffer
 * \param[in]    p_Src                    Pointer to the input buffer
 * \param[in]    u16_width                number for horizontal pixels to be processed while downsampling vertically
 * \param[in]    u16_origin_y            origin row  of the partition.
 * \param[in]    u16_start_y                starting row position of unsafe bottom, pixels. 
 * \param[in]    u16_end_y                ending row position of unsafe bottom pixels.
 * \param[in]    u16_input_stride        Input buffer stride.
 * \param[in]    u16_output_stride        Output buffer stride.
 * \param[in]    p_flt_1d                Pointer to the 1D filter data structure to be used for downsampling.
 *
 * \return None
 */
void vertical_downsample_unsafe_bottom_positions(img_pel_t    *p_Dst,
                                    const  img_pel_t    *p_Src,
                                        uint16_t    u16_width,
                                        uint16_t    u16_origin_y,
                                        uint16_t    u16_start_y,
                                        uint16_t    u16_end_y,
                                        uint16_t    u16_input_stride,
                                        uint16_t    u16_output_stride,
                                  const RPUFilter    *p_flt_1d)
{
    const img_pel_t    *p_SrcWork , *p_SrcWork_Inner;
    const int16_t *pi_coeff;
    img_pel_t  *p_DstWork ;
    int16_t i16_col_loop,i16_row_loop,i16_filter_loop,i16_dstIndx,i16_pix_pos,i16_repeat;    
    int32_t i32_mac = 0;

    u16_start_y = u16_start_y  + (u16_start_y - u16_origin_y)%2;
    
    p_Dst = p_Dst + ((u16_start_y - u16_origin_y )>>1) * (u16_output_stride);
    p_Src = p_Src +  (u16_start_y - u16_origin_y ) * u16_input_stride;

    i16_repeat = u16_end_y - u16_start_y - 1;
    for(i16_col_loop=u16_start_y,i16_dstIndx=0; i16_col_loop<u16_end_y; i16_col_loop+=2,i16_dstIndx++)            
    {
        p_DstWork = p_Dst;
        p_SrcWork  =p_Src;        
        for(i16_row_loop=0; i16_row_loop<u16_width; i16_row_loop++)
        {
            p_SrcWork_Inner = p_SrcWork++;
            pi_coeff = p_flt_1d->ai16_coef1;
            i32_mac=0;
            for(i16_filter_loop =0 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
            {
                i16_pix_pos = - p_flt_1d->u16_taps_div2 + i16_filter_loop ;    
                i16_pix_pos = MIN_VAL(i16_pix_pos,i16_repeat);
                i32_mac += *(p_SrcWork_Inner + (u16_input_stride *i16_pix_pos)) * *pi_coeff++;                
            }        /* i16_filter_loop */

            *p_DstWork++   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

        } /* i16_row_loop */
        i16_repeat-=2;
        p_Src += 2 * u16_input_stride;
        p_Dst += u16_output_stride;        
    }/* i16_col_loop*/

}/* End of vertical_downsample_unsafe_bottom_positions function */



void vertical_filter_and_downsample(                                    
                              const RPUFilter        *p_flt,
                              const RPUFilterParams  *p_filter_params,
                              const img_pel_t        *p_in,
                                    uint16_t         u16_input_stride,
                                    img_pel_t        *p_out,
                                    uint16_t         u16_output_stride
                             )
{

    /* Origin */
    uint16_t u16_origin_y = p_filter_params->u16_origin_y;
    uint16_t u16_origin_x = p_filter_params->u16_origin_x;    
    
    /* End */
    uint16_t u16_end_y    = p_filter_params->u16_end_y;
    uint16_t u16_end_x    = p_filter_params->u16_end_x;    

    /* Center region */
    uint16_t u16_center_start_y = p_filter_params->u16_center_start_y;
    uint16_t u16_center_end_y   = p_filter_params->u16_center_end_y;

    /* Top boundary pixels */
    vertical_downsample_unsafe_top_positions    (p_out,p_in,u16_end_x-u16_origin_x, u16_origin_y,u16_center_start_y,                 u16_input_stride,u16_output_stride,p_flt);

    /* Center safe pixels */
    vertical_downsample_safe_positions          (p_out,p_in,u16_end_x-u16_origin_x, u16_origin_y,u16_center_start_y,u16_center_end_y,u16_input_stride,u16_output_stride,p_flt);

    /*Bottom boundary pixels */
    vertical_downsample_unsafe_bottom_positions (p_out,p_in,u16_end_x-u16_origin_x, u16_origin_y,u16_center_end_y,  u16_end_y,       u16_input_stride,u16_output_stride,p_flt);


} /* End of om_rpu_downsample_vertical_filter() */






void horizontal_downsample_unsafe_left_positions(img_pel_t    *p_Dst,
                                  const img_pel_t    *p_Src,
                                        uint8_t        u8_packed_UV,
                                        uint16_t    u16_height,
                                        uint16_t    u16_offset,
                                        uint16_t    u16_end_x,
                                        uint16_t    u16_input_stride,
                                        uint16_t    u16_output_stride,
                                  const RPUFilter   *p_flt_1d)
{
    const img_pel_t     *p_SrcWork , *p_SrcWork_Inner;
    const int16_t *pi_coeff;
    img_pel_t  *p_DstWork ;
    uint8_t i8_uv_packed_loop;
    int16_t i16_col_loop,i16_row_loop,i16_filter_loop,i16_pix_pos;    
    int32_t i32_mac = 0;
    
    

    for(i16_row_loop=0; i16_row_loop<u16_height; i16_row_loop++)
    {        
        p_DstWork = p_Dst;
        p_SrcWork = p_Src;
        for(i16_col_loop=u16_offset; i16_col_loop<u16_end_x; i16_col_loop+=2)            
        {
            for(i8_uv_packed_loop=0; i8_uv_packed_loop < u8_packed_UV +1 ;i8_uv_packed_loop++)
            {                
                p_SrcWork_Inner = p_SrcWork + i8_uv_packed_loop;
                pi_coeff = p_flt_1d->ai16_coef1;
                i32_mac=0;
                p_SrcWork_Inner -= (i16_col_loop * (u8_packed_UV+1));
                for(i16_filter_loop =0 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
                {
                    i16_pix_pos =i16_col_loop - p_flt_1d->u16_taps_div2 + i16_filter_loop +1;
                    i32_mac += *p_SrcWork_Inner * *pi_coeff++;                    
                    p_SrcWork_Inner += (u8_packed_UV+1)*(i16_pix_pos > 0);
                }
                *p_DstWork++  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            }
            p_SrcWork += 2 * (u8_packed_UV+1);
        }                        
        p_Src += u16_input_stride;
        p_Dst += u16_output_stride;
    }

}
void horizontal_downsample_safe_positions(img_pel_t    *p_Dst,
                                        const img_pel_t    *p_Src,
                                        uint8_t        u8_packed_UV,
                                        uint16_t    u16_height,
                                        uint16_t    u16_origin_x,
                                        uint16_t    u16_start_x,
                                        uint16_t    u16_end_x,
                                        uint16_t    u16_input_stride,
                                        uint16_t    u16_output_stride,
                                        const RPUFilter    *p_flt_1d)
{
    const img_pel_t     *p_SrcWork , *p_SrcWork_Inner;
    const int16_t *pi_coeff;
    img_pel_t  *p_DstWork ;
    uint8_t i8_uv_packed_loop;
    int16_t i16_col_loop,i16_row_loop,i16_filter_loop;    
    int32_t i32_mac = 0;
    
    u16_start_x = u16_start_x  + (u16_start_x - u16_origin_x)%2;

    p_Dst = p_Dst + ((u16_start_x-u16_origin_x)>>1) ;
    p_Src = p_Src + (u16_start_x-u16_origin_x-p_flt_1d->u16_taps_div2);

    for(i16_row_loop=0; i16_row_loop<u16_height; i16_row_loop++)
    {        
        p_DstWork = p_Dst;
        p_SrcWork = p_Src;
        for(i16_col_loop=u16_start_x; i16_col_loop<u16_end_x; i16_col_loop+=2)            
        {
            for(i8_uv_packed_loop=0; i8_uv_packed_loop < u8_packed_UV +1 ;i8_uv_packed_loop++)
            {                
                p_SrcWork_Inner = p_SrcWork + i8_uv_packed_loop;
                pi_coeff = p_flt_1d->ai16_coef1;
                i32_mac=0;
                for(i16_filter_loop =0 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
                {
                    i32_mac += *p_SrcWork_Inner * *pi_coeff++;                    
                    p_SrcWork_Inner += (u8_packed_UV+1);
                }
                *p_DstWork++  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            }
            p_SrcWork += 2 * (u8_packed_UV+1);
        }                        
        p_Src += u16_input_stride;
        p_Dst += u16_output_stride;
    }
}
void horizontal_downsample_unsafe_right_positions(img_pel_t    *p_Dst,
                                        const img_pel_t    *p_Src,
                                        uint8_t        u8_packed_UV,
                                        uint16_t    u16_height,
                                        uint16_t    u16_origin_x,
                                        uint16_t    u16_start_x,
                                        uint16_t    u16_end_x,
                                        uint16_t    u16_input_stride,
                                        uint16_t    u16_output_stride,
                                        const RPUFilter    *p_flt_1d)
{

    const img_pel_t     *p_SrcWork , *p_SrcWork_Inner;
    const int16_t *pi_coeff;
    img_pel_t  *p_DstWork ;
    uint8_t i8_uv_packed_loop;
    int16_t i16_col_loop,i16_row_loop,i16_filter_loop,i16_pix_pos;    
    int32_t i32_mac = 0;
    
    p_Dst = p_Dst + ((u16_start_x)>>1) ;
    p_Src = p_Src + (u16_start_x-p_flt_1d->u16_taps_div2);    

    for(i16_row_loop=0; i16_row_loop<u16_height; i16_row_loop++)
    {        
        p_DstWork = p_Dst;
        p_SrcWork = p_Src;
        for(i16_col_loop=u16_start_x ; i16_col_loop<u16_end_x ; i16_col_loop+=2)            
        {
            for(i8_uv_packed_loop=0; i8_uv_packed_loop < u8_packed_UV +1 ;i8_uv_packed_loop++)
            {                
                p_SrcWork_Inner = p_SrcWork + i8_uv_packed_loop;
                pi_coeff = p_flt_1d->ai16_coef1;
                i32_mac=0;
                for(i16_filter_loop =0 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
                {
                    i16_pix_pos =i16_col_loop + 2*u16_origin_x - p_flt_1d->u16_taps_div2 + i16_filter_loop +1;
                    i32_mac += *p_SrcWork_Inner * *pi_coeff++;                    
                    p_SrcWork_Inner += (u8_packed_UV+1)*(i16_pix_pos < (u16_end_x+u16_origin_x));
                }
                *p_DstWork++  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            }
            p_SrcWork += 2 * (u8_packed_UV+1);
        }                        
        p_Src += u16_input_stride;
        p_Dst += u16_output_stride;
    }
}

void horizontal_filter_and_downsample(                                    
                              const RPUFilter    *p_flt,
                              const RPUFilterParams    *p_filter_params,
                              const img_pel_t        *p_in,                                    
                                    uint16_t        u16_input_stride,
                                    img_pel_t        *p_out,
                                    uint16_t        u16_output_stride,
                                    uint8_t            u8_packed_UV
                             )
{

    /* Origin */
    uint16_t u16_origin_y = p_filter_params->u16_origin_y;
    uint16_t u16_origin_x = p_filter_params->u16_origin_x;    
    
    /* End */
    uint16_t u16_end_y    = p_filter_params->u16_end_y;
    uint16_t u16_end_x    = p_filter_params->u16_end_x;    

    /* Center region */    
    uint16_t u16_center_start_x = p_filter_params->u16_center_start_x;
    uint16_t u16_center_end_x   = p_filter_params->u16_center_end_x;

    horizontal_downsample_unsafe_left_positions (p_out,p_in,u8_packed_UV,u16_end_y-u16_origin_y, u16_origin_x,u16_center_start_x,                 u16_input_stride,u16_output_stride,p_flt);
    horizontal_downsample_safe_positions        (p_out,p_in,u8_packed_UV,u16_end_y-u16_origin_y, u16_origin_x,u16_center_start_x,u16_center_end_x,u16_input_stride,u16_output_stride,p_flt);
    horizontal_downsample_unsafe_right_positions(p_out,p_in,u8_packed_UV,u16_end_y-u16_origin_y, u16_origin_x,u16_center_end_x,  u16_end_x,       u16_input_stride,u16_output_stride,p_flt);


}







    

 




    