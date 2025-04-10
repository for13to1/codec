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
* \file  c3d_rpu_kernel_om_ou_filter.c
*
* \brief 
*        MFC SDK  RPU Kernal filtering functions for OU Mux Mode
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

/** 
 * \fn void horizontal_downsampling_unpacked(    img_pel_t        *p_out,
 *                                         const    img_pel_t        *p_src,
 *                                                 img_pel_t        p_leading_pixels[16],
 *                                                uint32_t        u32_safe_pixels,
 *                                                 uint32_t        u32_unsafe_end,
 *                                         const    RPUFilter        *p_flt_1d)
 *
 * \brief horizonta downsample a row of unpacked data.(Stage 1 of TAB-  OM RPU)
 *
 *    \details horizontal downsampling is implemented in blocks of 8 pixels per iteration. For each iteration downsampler needs current 8 pixels and 
 *   next 8-pixels (filter delay line)of the same row.After each iteration the current 8-pixles are replaced by next 8-pixles and the new-8 are
 *   added to the end.'u32_unsafe_end' flag takes care of the border pixles by repeating the last pixels before filtering.
 *    
 * \param[in]    p_out                    Pointer to the output row.
 * \param[in]    p_src                    Pointer to the input row.
 * \param[in]    p_leading_pixels[16]    pointer to the first 16 pixles in the row.(Unsafe pixels are already replicated and available)
 * \param[in]    u32_safe_pixels            total number of safe pixels.
 * \param[in]    u32_unsafe_end_pixels    total number of unsafe end pixels.
 * \param[in]    u32_view_offset            view_grid offset (0/1)
 * \param[in]    p_flt_1d                Pointer to filter definition. This is typecast to 1D filter.
 *
 * \return None
 */

void horizontal_downsampling_unpacked(    
                                        img_pel_t        *p_out,
                                const    img_pel_t        *p_src,
                                        img_pel_t        p_leading_pixels[16],
                                        uint32_t        u32_safe_pixels,
                                        uint32_t        u32_unsafe_end,
                                const    RPUFilter        *p_flt_1d)
{

    uint32_t u32_loop_cnt,u32_pixel_cnt,u32_filter_cnt,u32_iloop;
    const img_pel_t    *p_src_work;
    img_pel_t *p_dst;
    img_pel_t copy_pixel;
    

    u32_loop_cnt = u32_safe_pixels >> 3;


    p_dst = p_out ;            /* Destination pointer for output pixel location */

    do{
        memcpy(&p_leading_pixels[8],p_src,sizeof(img_pel_t) * 8);
        p_src +=8;
        
        
        p_src_work = &p_leading_pixels[0];    /* Points to the first pixel in the block of 16-pixels for processing 4-pixels */
    
  
        {
            int16_t i16_mac[4];
            i16_mac[0]=0;i16_mac[1]=0;i16_mac[2]=0;i16_mac[3]=0;
            
            i16_mac[0] =    p_src_work[0] * p_flt_1d->ai16_coef1[0];
            i16_mac[1] =    p_src_work[2] * p_flt_1d->ai16_coef1[0];
            i16_mac[2] =    p_src_work[4] * p_flt_1d->ai16_coef1[0];
            i16_mac[3] =    p_src_work[6] * p_flt_1d->ai16_coef1[0];


            for(u32_filter_cnt=1;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {
                i16_mac[0] +=p_src_work[u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i16_mac[1] +=p_src_work[u32_filter_cnt+2] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i16_mac[2] +=p_src_work[u32_filter_cnt+4] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i16_mac[3] +=p_src_work[u32_filter_cnt+6] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }            
            /*Write Output and  Increment to the next to-be calculated pixel location */        
            p_dst[0]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac[0], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            p_dst[1]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac[1], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            p_dst[2]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac[2], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            p_dst[3]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac[3], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

            p_dst +=4;
            p_src_work+=8;                    /* Increment working source pointer for next pixel */        
        }


        memcpy(    &p_leading_pixels[0],&p_leading_pixels[8],8*sizeof(img_pel_t));

    }while(--u32_loop_cnt);

    
                        
    if(u32_unsafe_end)                /* handle unsafe end pixels */
    {
        u32_loop_cnt = 8-u32_unsafe_end;
        p_src -=u32_loop_cnt;
        memcpy(&p_leading_pixels[8],p_src,sizeof(img_pel_t) * 8);
        copy_pixel = p_leading_pixels[15];
                                        /* repeat the last pixel */
        while(u32_loop_cnt--)
        {
            for(u32_iloop=8;u32_iloop<15;u32_iloop++)
            {
                p_leading_pixels[u32_iloop] = p_leading_pixels[u32_iloop+1];
            }
            p_leading_pixels[15] = copy_pixel;
        }/* while(u32_loop_cnt--) */

        u32_pixel_cnt=4;
        p_src_work = &p_leading_pixels[0];
        do
        {    
            int16_t i16_mac;
            i16_mac=0;
            for(u32_filter_cnt=0;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {
                i16_mac +=p_src_work[u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }            
            *p_dst++  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            p_src_work+=2;
        }while(--u32_pixel_cnt);

    }/*if(u32_unsafe_end)*/


}/* End of horizontal_upsampling_packed() */




/**
 * \fn void horizontal_downsampling_packed(
 *                                    img_pel_t    *p_out,
 *                            const    img_pel_t    *p_src,
 *                                    img_pel_t    p_leading_pixels_uv[32],
 *                                    uint32_t    u32_safe_pixels,
 *                                    uint32_t    u32_unsafe_end,                                    
 *                            const    RPUFilter *p_flt_1d)
 *
 * \brief horizonta downsampling a row of packed data.(Stage 1 of TAB OM RPU)
 *
 * \details horizontal downsampling is implemented in blocks of 8 pixels per iteration. For each iteration downsampler needs current 8 pixels and 
 *   next 8-pixels (filter delay line)of the same row.After each iteration the current 8-pixles are replaced by next 8-pixles and the new-8 are
 *   added to the end.'u32_unsafe_end' flag takes care of the border pixles by repeating the last pixels before filtering.In each iteration a 
 *   pack of 8 U and 8 V pixels are processed.


 * \param[in]    p_out                    Pointer to the output row.
 * \param[in]    p_src                    Pointer to the input row.
 * \param[in]    p_leading_pixels[32]    pointer to the first 32 pixles in the row(16 for U & 16 for V).(Unsafe pixels are already replicated and available)
 * \param[in]    u32_safe_pixels            total number of safe pixels.
 * \param[in]    u32_unsafe_end_pixels    total number of unsafe end pixels. 
 * \param[in]    p_flt_1d                Pointer to filter definition. This is typecast to 1D filter.
 *
 * \return None
 */

void horizontal_downsampling_packed(
                                    img_pel_t    *p_out,
                            const   img_pel_t    *p_src,
                                    img_pel_t    p_leading_pixels_uv[32],
                                    uint32_t     u32_safe_pixels,
                                    uint32_t     u32_unsafe_end,                                    
                            const   RPUFilter  *p_flt_1d)
{

    uint32_t u32_loop_cnt,u32_pixel_cnt,u32_filter_cnt,u32_iloop;
    int16_t i16_mac_u,i16_mac_v;
    const img_pel_t    *p_src_work_u,*p_src_work_v;
    img_pel_t *p_dst_u,*p_dst_v;
    img_pel_t copy_pixel_u,copy_pixel_v;

    u32_loop_cnt = u32_safe_pixels >> 4;

    /* Destination pointer for U & V */    
    p_dst_u = p_out ;
    p_dst_v = p_out +1;
    
    
    do{
        memcpy(&p_leading_pixels_uv[16],p_src,sizeof(img_pel_t) * 16);
        p_src +=16;
        
        
        /* Block of 8 pixels had to be iterated 4 times to  output 4 pixels when downsampling */
        u32_pixel_cnt=4;                            /* Process Blocks fo 8-U and 8-V pixels*/
        p_src_work_u = &p_leading_pixels_uv[0];        /* Points to the first U pixel in the block of 16-U-pixels(alternatively arranged) for processing 8-U-pixels */
        p_src_work_v = &p_leading_pixels_uv[1];        /* Points to the first V pixel in the block of 16-V-pixels(alternatively arranged) for processing 8-V-pixels */
        do
        {
            i16_mac_u=0;
            i16_mac_v=0;

            for(u32_filter_cnt=0;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {
                /* Handle U & V Pixels by taking every other pixels */
                i16_mac_u +=p_src_work_u[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i16_mac_v +=p_src_work_v[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }
            
            *p_dst_u  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac_u, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            *p_dst_v  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac_v, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

            /* Increment by the Destination U & V pointers by 2 , as next iteration will write the next immediate U and V  Pixel */
            p_dst_u+=2;            
            p_dst_v+=2;            
            
            /* Increment by the source U & V pointers by 4 , as next iteration will skip a U and V pixel and read the next immediate U and V  Pixel */
            p_src_work_u+=4;            
            p_src_work_v+=4;            
        }while(--u32_pixel_cnt);

        for(u32_iloop=0;u32_iloop<16;u32_iloop++)
        {
            p_leading_pixels_uv[u32_iloop] = p_leading_pixels_uv[16+u32_iloop];
        }/* move the last 8-UV-pixels to the top 8-pixels for processing next block of 8-pixels */
    }while(--u32_loop_cnt);

    if(u32_unsafe_end)            /* handle unsafe end pixels */
    {
        u32_loop_cnt = (8-u32_unsafe_end);
        p_src -=2*u32_loop_cnt;
        memcpy(&p_leading_pixels_uv[16],p_src,sizeof(img_pel_t) * 16);
        copy_pixel_u = p_leading_pixels_uv[30];
        copy_pixel_v = p_leading_pixels_uv[31];

                                    /* repeat the last U and V pixel */
        while(u32_loop_cnt--)
        {
            for(u32_iloop=16;u32_iloop<30;u32_iloop+=2)
            {
                p_leading_pixels_uv[u32_iloop] = p_leading_pixels_uv[u32_iloop+2];
                p_leading_pixels_uv[u32_iloop+1] = p_leading_pixels_uv[u32_iloop+3];
            }
            p_leading_pixels_uv[30] = copy_pixel_u;
            p_leading_pixels_uv[31] = copy_pixel_v;
        }

        /* Block of 8 pixels had to be iterated 4 times to  output 4 pixels when downsampling */
        u32_pixel_cnt=4;
        p_src_work_u = &p_leading_pixels_uv[0];
        p_src_work_v = &p_leading_pixels_uv[1];

        do
        {
            i16_mac_u=0;
            i16_mac_v=0;
            for(u32_filter_cnt=0;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {    
                /* Handle U & V Pixels by taking every other pixels */
                i16_mac_u +=p_src_work_u[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i16_mac_v +=p_src_work_v[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }
            
            *p_dst_u  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac_u, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            *p_dst_v  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i16_mac_v, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

            /* Increment by the Destination U & V pointers by 2 , as next iteration will write the next immediate U and V  Pixel */
            p_dst_u+=2;
            p_dst_v+=2;
            
            /* Increment by the source U & V pointers by 4 , as next iteration will skip a U and V pixel and read the next immediate U and V  Pixel */
            p_src_work_u+=4;
            p_src_work_v+=4;
            
        }while(--u32_pixel_cnt);
    }/* if(u32_unsafe_end) */
}/* End of horizontal_downsampling_packed() */

void om_rpu_downsample_horizontal_filter(
                              const RPUFilter    *p_flt,
                              const RPUFilterParams  *p_filter_params,
                              const img_pel_t        *p_in,                                    
                                    uint16_t         u16_input_stride,
                                    img_pel_t        *p_out,
                                    uint16_t         u16_output_stride,
                                    uint8_t          u8_packed_UV
                             )
{

    uint32_t i_loop;
    const img_pel_t        *p_src;
    uint32_t u32_row_loop;

    /* setup the safe and un_safe pixel counts */
    uint32_t u32_unsafe_start = p_filter_params->u16_center_start_x - p_filter_params->u16_origin_x;
    uint32_t u32_safe         = p_filter_params->u16_center_end_x - p_filter_params->u16_center_start_x;
    uint32_t u32_height       = p_filter_params->u16_end_y-p_filter_params->u16_origin_y;
    uint32_t u32_src_offset   = (p_flt->u16_num_taps >> 1) ;
    uint32_t u32_unsafe_end   = p_filter_params->u16_end_x - p_filter_params->u16_center_end_x ;

    
    
    if(u8_packed_UV==0)            
    {        /* data is not packed i.e. U and V are not arranged alternatively */
        img_pel_t    u8_leading_pixels[8*2];
        img_pel_t    u8_temp_pixels[8];    

        if(u32_unsafe_start)
        {

            for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)
            {
                uint32_t i_inner_loop;

                p_src = p_in;
                memcpy(&u8_temp_pixels,p_src,sizeof(img_pel_t)*8);                
                p_src += 8-u32_unsafe_start;        /* rewind the pointer to the first safe pixel  location */
                
                                                    /* setup first 8 unsafe pixels */
                for(i_inner_loop = 0; i_inner_loop < u32_unsafe_start;i_inner_loop++)
                {                    
                    u8_leading_pixels[i_inner_loop] = u8_temp_pixels[0];
                }
                i_loop=0;
                for(i_inner_loop = u32_unsafe_start; i_inner_loop < 8;i_inner_loop++)
                {                    
                    u8_leading_pixels[i_inner_loop] = u8_temp_pixels[i_loop++];
                }

                /* downsample single row */
                horizontal_downsampling_unpacked(p_out,p_src,u8_leading_pixels,u32_unsafe_start+u32_safe,u32_unsafe_end,p_flt);

                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/*if(u32_unsafe_start)*/
        else
        {
            
            for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)
            {
                p_src = p_in -  u32_src_offset;                            /* rewind the source pointer by # of filtertaps/2 locations*/
                memcpy(&u8_leading_pixels,p_src,sizeof(img_pel_t)*8);    /* read a block of 8 safe pixels */    
                p_src+=8;
                
                /* downsample single row */
                horizontal_downsampling_unpacked(p_out,p_src,u8_leading_pixels,u32_unsafe_start+u32_safe,u32_unsafe_end,p_flt);            
                
                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/*else if(u32_unsafe_start)*/
    }/* if(u8_packed_UV==0)*/
    else
    {
        img_pel_t    u8_leading_pixels_uv[16*2];
        img_pel_t    u8_temp_pixels_uv[8*2];    

        if(u32_unsafe_start)
        {

            for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)
            {
                uint32_t i_inner_loop;

                p_src = p_in;
                memcpy(&u8_temp_pixels_uv,p_src,sizeof(img_pel_t)*16);                
                p_src += 2*(8-u32_unsafe_start);            /* rewind the pointer to the first safe pixel  location */

                                                            /* setup first 8 U and V unsafe pixels */
                for(i_inner_loop = 0; i_inner_loop < u32_unsafe_start;i_inner_loop++)
                {                    
                    u8_leading_pixels_uv[2*i_inner_loop] = u8_temp_pixels_uv[0];
                    u8_leading_pixels_uv[2*i_inner_loop+1] = u8_temp_pixels_uv[1];
                }
                i_loop=0;
                for(i_inner_loop = u32_unsafe_start; i_inner_loop < 8;i_inner_loop++)
                {                    
                    u8_leading_pixels_uv[2*i_inner_loop]  = u8_temp_pixels_uv[i_loop++];
                    u8_leading_pixels_uv[2*i_inner_loop+1] = u8_temp_pixels_uv[i_loop++];                    
                }

                /* downsample single row */
                horizontal_downsampling_packed(p_out,p_src,u8_leading_pixels_uv,u32_unsafe_start+u32_safe,u32_unsafe_end,p_flt);

                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/* if(u32_unsafe_start) */
        else
        {
            for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)
            {
                p_src = p_in -  2*u32_src_offset;
                memcpy(&u8_leading_pixels_uv,p_src,sizeof(img_pel_t)*16);            /* read a block of 8U & 8V safe pixels */    
                p_src+=16;

                /* downsample single row */
                horizontal_downsampling_packed(p_out,p_src,u8_leading_pixels_uv,u32_unsafe_start+u32_safe,u32_unsafe_end,p_flt);            

                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/* else if(u32_unsafe_start)*/    
    }/* else if(u8_packed_UV==0)*/


}/* End of om_rpu_downsample_horizontal_filter()    */





int32_t ou_om_down_filter(
                         void        *pv_rpu_handle,
                   const img_pel_t   *p_src,
                         uint16_t    u16_src_stride,
                         img_pel_t   *p_dst,
                         uint16_t    u16_dst_stride,
                         uint16_t    u16_view_delimiter_sbs,
                         uint16_t    u16_view_delimiter_ou,
                         uint8_t     u8_packed_UV
                 )
{

    int32_t i32_status = SUCCESS;

    RPUHandle *p_rpu_handle = (RPUHandle *) pv_rpu_handle;

    const RPUFilter    *p_flt = &p_rpu_handle->rpu_om_down_filter;
  
    const img_pel_t *p_in;
    img_pel_t *p_out;

    /* Get the distance between the first tap and the center of the filter */
    uint16_t u16_filter_offset;
    RPUFilterParams filter_params;

    u16_filter_offset = p_flt->u16_taps_div2;

    /* Horizontal filtering parameters */
    filter_params.u16_origin_x = 0;
    filter_params.u16_origin_y = 0;    
    filter_params.u16_center_start_x = u16_filter_offset;
    filter_params.u16_center_end_x   = 2*u16_view_delimiter_sbs - u16_filter_offset;
    filter_params.u16_end_x          = 2*u16_view_delimiter_sbs;
    filter_params.u16_end_y          = u16_view_delimiter_ou;

    
    /* Src and Dst  Pointers for Left View Downsampling*/
    p_in  = p_src ;
    p_out = p_dst ;

    
    /* Call filter function */
    om_rpu_downsample_horizontal_filter(
                          p_flt,
                          &filter_params,
                          p_in,
                          u16_src_stride,
                          p_out,
                          u16_dst_stride,
                          u8_packed_UV
                         );

    /* Horizontal filtering parameters modified for right view */    
    filter_params.u16_origin_y = u16_view_delimiter_ou;    
    filter_params.u16_end_y    = 2*u16_view_delimiter_ou;

    
     /* Src and Dst  Pointers for Right View Downsampling*/
    p_in  = p_src + (u16_view_delimiter_ou * u16_src_stride) ;    
    p_out = p_dst + u16_view_delimiter_sbs ;

    
    /* Call filter function */
    om_rpu_downsample_horizontal_filter(
                          p_flt,
                          &filter_params,
                          p_in,
                          u16_src_stride,
                          p_out,
                          u16_dst_stride,
                          u8_packed_UV
                         );


    return i32_status;
} /* End of ou_om_down_filter() function */

/**
 * \fn void vertical_upsample_row(img_pel_t    *p_dst,
 *                        const img_pel_t    **ap_Src,
 *                        uint16_t u16_width,
 *                        uint32_t u32_view_offset,
 *                        const RPUFilter *p_flt_1d)
 *
 * \brief vertical_upsample_row.(Stage 2 of OM RPU)
 *
 * \details Function does vertical upsampling of a single row. The function takes in a array of row pointers needed to upsample each row.
 *
 * \param[in]    p_dst                    Pointer to the output buffer.
 * \param[in]    ap_Src                    Array of pointer to source rows.
 * \param[in]    u16_width                number of pixels in each row 
 * \param[in]    u32_view_offset            view_grid offset (0/1)
 * \param[in]    p_flt_1d                Pointer to filter definition. This is typecast to 1D filter.
 *
 * \return None
 */
void vertical_upsample_row(img_pel_t    *p_dst,
                  const img_pel_t       **ap_Src,
                        uint16_t        u16_width,
                        uint32_t        u32_dst_stride,
                        uint32_t        u32_view_offset,
                  const RPUFilter     *p_flt_1d)
{
    const int16_t *pi_coeff;
    int16_t i16_filter_loop;
    

    
    
    uint32_t u32_src_copy_loc;
    img_pel_t *p_dst_copy,*p_dst_calc;
    const img_pel_t  *p_SrcSave[15];
    const img_pel_t  **p_SrcWork  = &p_SrcSave[0];
    const img_pel_t  **p_ref_rows = &ap_Src[0];
    int16_t num_of_8pixel_blocks;


    p_dst_copy = p_dst+ (u32_view_offset  * u32_dst_stride);    /* Destination pointer for  copy pixel location */
    p_dst_calc = p_dst+ (!u32_view_offset  * u32_dst_stride);    /* Destination pointer for  to-be-calculated pixel location */
    u32_src_copy_loc = (p_flt_1d->u16_num_taps>>1) -1+ u32_view_offset; /* offset from the first pixel used in filtering to the middle pixel */
    

    i16_filter_loop = p_flt_1d->u16_num_taps;
    /* copy the source rows pointer to a working pointer */
    while(i16_filter_loop--)
    {
        *p_SrcWork++ = *p_ref_rows++;
    }    


    assert(u16_width%8==0);
    num_of_8pixel_blocks = u16_width>>3;
    while(num_of_8pixel_blocks--)
    {
        int32_t i32_mac[8];
        memset(i32_mac,0,8*sizeof(int32_t));

        pi_coeff = p_flt_1d->ai16_coef1;        
        p_SrcWork  = p_SrcSave;        
        memcpy(p_dst_copy,p_SrcWork[u32_src_copy_loc],8*sizeof(img_pel_t));
        p_dst_copy+=8;

        i16_filter_loop = p_flt_1d->u16_num_taps;
        assert(i16_filter_loop>0);

        do
        {
            i32_mac[0] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[1] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[2] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[3] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[4] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[5] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[6] += **p_SrcWork * *pi_coeff;                
            *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

            i32_mac[7] += **p_SrcWork * *pi_coeff++;                
            *p_SrcWork++ +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/
            
            /* Increment to the next row pointer - used with next filter coefficient */
            
        }while(--i16_filter_loop);        /* i16_filter_loop        */        

        p_dst_calc[0]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[0], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[1]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[1], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[2]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[2], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[3]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[3], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[4]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[4], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[5]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[5], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[6]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[6], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc[7]    = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[7], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst_calc +=8;

        
    }/* end of while u6_width */

        
}/* end of vertical_upsample_row() */



void om_rpu_upsample_vertical_filter(
                              const RPUFilter   *p_flt,
                              const RPUFilterParams *p_filter_params,
                              const img_pel_t       *p_in,
                                    uint16_t        u16_input_stride,
                                    img_pel_t       *p_out,
                                    uint16_t        u16_output_stride
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

    
    const img_pel_t *p_src_rows[11];
    const img_pel_t **p_ref_rows = &p_src_rows[0];
    const img_pel_t **p_ref_rows_2 ;
    uint16_t u16_row;
    uint16_t u16_filter_rows,u16_num_unsafe_pixels;
    uint16_t u16_filter_num_taps = p_flt->u16_num_taps;



    /* Set up source row pointers */
    u16_row = u16_center_start_y-u16_origin_y;
    p_in -= (u16_input_stride*((u16_filter_num_taps>>1)- 1 + p_filter_params->u16_view_offset_y)* !u16_row);
    while(u16_row--)
    {
        *p_ref_rows++ = p_in;        /* unsafe start pixels - repeat the first row location*/
    }
    u16_row = u16_filter_num_taps - (u16_center_start_y-u16_origin_y);
    while(u16_row--)
    {
        *p_ref_rows++ = p_in;        /* safe row positions */
        p_in +=u16_input_stride;
    }


    u16_num_unsafe_pixels = u16_end_y - u16_center_end_y + 1;
    
    
    for(u16_row = u16_end_y - u16_origin_y;u16_row > u16_num_unsafe_pixels ; u16_row--)
    {
        /* vertically upsample each row */
        vertical_upsample_row(p_out,p_src_rows,u16_end_x-u16_origin_x,u16_output_stride,p_filter_params->u16_view_offset_y,p_flt);
        p_out += 2*u16_output_stride;
        
        /* shift the pointers up by 1 row for upsampling next row*/
        u16_filter_rows = u16_filter_num_taps-2;
        p_ref_rows  = &p_src_rows[0];
        p_ref_rows_2  = &p_src_rows[1];
        while(u16_filter_rows-- )
        {
            *p_ref_rows++ = *p_ref_rows_2++;
        }        
    
        *p_ref_rows++ = *p_ref_rows_2;
        *p_ref_rows++ = *p_ref_rows_2 + (u16_input_stride ) ;        

    }
    
    vertical_upsample_row(p_out,p_src_rows,u16_end_x-u16_origin_x,u16_output_stride,p_filter_params->u16_view_offset_y,p_flt);
    p_out += 2*u16_output_stride;
    u16_row--;

    while(u16_row--)         /* unsafe bottom positions */
    {    
        

        u16_filter_rows = u16_filter_num_taps-2;
        p_ref_rows  = &p_src_rows[0];
        p_ref_rows_2  = &p_src_rows[1];
        while(u16_filter_rows-- )
        {
            *p_ref_rows++ = *p_ref_rows_2++;
        }
        *p_ref_rows++ = *p_ref_rows_2;
        *p_ref_rows++ = *p_ref_rows_2 ;

        vertical_upsample_row(p_out,p_src_rows,u16_end_x-u16_origin_x,u16_output_stride,p_filter_params->u16_view_offset_y,p_flt);
        p_out += 2*u16_output_stride;        
    }



} /* End of om_rpu_upsample_vertical_filter() */



int32_t ou_om_up_filter(
                         void       *pv_rpu_handle,
                   const img_pel_t  *p_src,
                         uint16_t   u16_src_stride,
                         img_pel_t  *p_dst,
                         uint16_t   u16_dst_stride,
                         uint16_t   u16_view_delimiter_sbs,
                         uint16_t   u16_view_delimiter_ou,
                         uint8_t    u8_packed_UV,
                         uint8_t    u8_view_grid_offset_v0,
                         uint8_t    u8_view_grid_offset_v1
                 )
{
    
    int32_t i32_status = SUCCESS;

    RPUHandle *p_rpu_handle = (RPUHandle *) pv_rpu_handle;

    const RPUFilter    *p_flt = &p_rpu_handle->rpu_om_up_filter;
 
    const img_pel_t *p_in ;
    img_pel_t *p_out ;

    uint16_t u16_filter_offset;
    RPUFilterParams filter_params;

    UNREFERENCED_PARAMETER(u8_packed_UV);

    u16_filter_offset = p_flt->u16_taps_div2;


     /* vertical filtering parameters */    
    filter_params.u16_origin_y = 0;
    filter_params.u16_end_y    = u16_view_delimiter_ou;
   
    
    /* Process from partition starting to beginning of delimiter */
    filter_params.u16_origin_x  =   0;
    filter_params.u16_end_x     =   u16_view_delimiter_sbs;
    filter_params.u16_view_offset_y     =   u8_view_grid_offset_v0;
    filter_params.u16_center_start_y    =   u16_filter_offset - !filter_params.u16_view_offset_y;
    filter_params.u16_center_end_y      =   u16_view_delimiter_ou - (u16_filter_offset - filter_params.u16_view_offset_y);
        
    p_in  = p_src ;
    p_out = p_dst ;
        
        
    /* Call filter function */
    om_rpu_upsample_vertical_filter(
                    p_flt,
                    &filter_params,
                    p_in,
                    u16_src_stride,
                    p_out,
                    u16_dst_stride
                    );

    /* Process from  beginning of delimiter to end of partition*/
    filter_params.u16_origin_x  =   u16_view_delimiter_sbs;
    filter_params.u16_end_x     =   2*u16_view_delimiter_sbs;
    filter_params.u16_view_offset_y  = u8_view_grid_offset_v1;
    filter_params.u16_center_start_y = (u16_filter_offset - !filter_params.u16_view_offset_y);
    filter_params.u16_center_end_y   = u16_view_delimiter_ou - (u16_filter_offset - filter_params.u16_view_offset_y);
        
    p_in  = p_src + u16_view_delimiter_sbs ;
    p_out = p_dst + u16_view_delimiter_sbs;


    /* Call filter function */
    om_rpu_upsample_vertical_filter(
                    p_flt,
                    &filter_params,
                    p_in,
                    u16_src_stride,
                    p_out,
                    u16_dst_stride
                    );
    


    return i32_status;

} /* End of ou_om_up_filter() function */
    
