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
* \file  c3d_rpu_kernel_om_sbs_filter.c
*
* \brief 
*        MFC SDK  RPU Kernal filtering functions for SBS Mux Mode
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
 * \fn void vertical_downsample_row( img_pel_t    *p_dst,
 *                               const img_pel_t    **ap_Src,
 *                                     uint16_t u16_width,
 *                               const RPUFilter *p_flt_1d)
 *
 * \brief vertical_downsample_row.(Stage 1 of OM RPU)
 *
 * \details Function does vertical downsampling of a single row. The function takes in a array of row pointers need to downsample each row.
 *
 * \param[in]    p_dst                    Pointer to the output buffer.
 * \param[in]    ap_Src                    Array of pointer to source rows.
 * \param[in]    u16_width                number of pixels in each row 
 * \param[in]    p_flt_1d                Pointer to filter definition. This is typecast to 1D filter.
 *
 * \return None
 */
void vertical_downsample_row( img_pel_t        *p_dst,
                       const  img_pel_t        **ap_Src,
                                uint16_t       u16_width,
                        const RPUFilter      *p_flt_1d)
{
    const int16_t *pi_coeff;
    int16_t i16_filter_loop;

    const img_pel_t  *p_SrcSave[15];
    const img_pel_t  **p_SrcWork  = &p_SrcSave[0];
    const img_pel_t  **p_ref_rows = &ap_Src[0];
    int16_t num_of_8pixel_blocks;

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

        pi_coeff = p_flt_1d->ai16_coef1;

        p_SrcWork  = p_SrcSave;

        i32_mac[0] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[1] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[2] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[3] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[4] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[5] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[6] = **p_SrcWork * *pi_coeff;                
        *p_SrcWork +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/

        i32_mac[7] = **p_SrcWork * *pi_coeff++;                
        *p_SrcWork++ +=1;        /* Increment current row pointer to the next horizontal pixel  - used during next filtering iteration*/


        for(i16_filter_loop =1 ; i16_filter_loop < p_flt_1d->u16_num_taps ; i16_filter_loop++)
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

            
        }    /* i16_filter_loop        */                
        p_dst[0]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[0], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[1]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[1], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[2]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[2], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[3]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[3], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[4]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[4], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[5]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[5], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[6]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[6], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */
        p_dst[7]   = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[7], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));    /* Normalise and round filter output */

        p_dst +=8;
    }

        
}/* End of vertical_downsample_row() */


/**
 * \fn void horizontal_upsampling_unpacked(img_pel_t    *p_out,
 *                                    const img_pel_t    *p_src,
 *                                    img_pel_t p_leading_pixels[16],
 *                                    uint32_t u32_safe_pixels,
 *                                    uint32_t u32_unsafe_end,
 *                                    uint32_t u32_view_offset,
 *                                    const RPUFilter *p_flt_1d)
 *
 * \brief horizontal_upsampling_unpacked.(Stage 2 of OM RPU)
 *
 * \details Function does horizontal upsampling of a single row for unpacked data.
 *
 *
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

void horizontal_upsampling_unpacked(img_pel_t   *p_out,
                              const img_pel_t   *p_src,
                                    img_pel_t   p_leading_pixels[16],
                                    uint32_t    u32_safe_pixels,
                                    uint32_t    u32_unsafe_end,
                                    uint32_t    u32_view_offset,
                              const RPUFilter *p_flt_1d)
{

    uint32_t u32_loop_cnt,u32_pixel_cnt,u32_filter_cnt,u32_src_copy_loc,u32_iloop;

    const img_pel_t    *p_src_work;
    img_pel_t *p_dst_copy,*p_dst_calc;
    img_pel_t copy_pixel;

    u32_loop_cnt = u32_safe_pixels >> 3;

    p_dst_copy = p_out + u32_view_offset;            /* Destination pointer for  copy pixel location */
    p_dst_calc = p_out + !u32_view_offset;            /* Destination pointer for  to-be-calculated pixel location */
    u32_src_copy_loc = (p_flt_1d->u16_num_taps>>1) -1+ u32_view_offset; /* offset from the first pixel used in filtering to the middle pixel */

    


    do{
        
        memcpy(&p_leading_pixels[8],p_src,sizeof(img_pel_t) * 8);
        p_src +=8;

        
        p_src_work = &p_leading_pixels[0];    /* Points to the first pixel in the block of 16-pixels for processing 8-pixels */

        {
            int32_t i32_mac[8];
            const img_pel_t *p_src_work_start;
            const int16_t *pi16_coeff;
            
            assert(p_flt_1d->u16_num_taps%2==0 && p_flt_1d->u16_num_taps !=0 );
            p_src_work_start = p_src_work;
            pi16_coeff         = p_flt_1d->ai16_coef1;
            
      
            i32_mac[0] =    p_src_work_start[0] * pi16_coeff[0];
            i32_mac[1] =    p_src_work_start[1] * pi16_coeff[0];
            i32_mac[2] =    p_src_work_start[2] * pi16_coeff[0];
            i32_mac[3] =    p_src_work_start[3] * pi16_coeff[0];
            i32_mac[4] =    p_src_work_start[4] * pi16_coeff[0];
            i32_mac[5] =    p_src_work_start[5] * pi16_coeff[0];
            i32_mac[6] =    p_src_work_start[6] * pi16_coeff[0];
            i32_mac[7] =    p_src_work_start[7] * pi16_coeff[0];
            p_src_work_start++;
            pi16_coeff++;    

            u32_filter_cnt = p_flt_1d->u16_num_taps-1;
            do
            {
                i32_mac[0] +=    p_src_work_start[0] * pi16_coeff[0];
                i32_mac[1] +=    p_src_work_start[1] * pi16_coeff[0];
                i32_mac[2] +=    p_src_work_start[2] * pi16_coeff[0];
                i32_mac[3] +=    p_src_work_start[3] * pi16_coeff[0];
                i32_mac[4] +=    p_src_work_start[4] * pi16_coeff[0];
                i32_mac[5] +=    p_src_work_start[5] * pi16_coeff[0];
                i32_mac[6] +=    p_src_work_start[6] * pi16_coeff[0];
                i32_mac[7] +=    p_src_work_start[7] * pi16_coeff[0];
                p_src_work_start++;
                pi16_coeff++;
            }while(--u32_filter_cnt);

            p_dst_copy[0]  = p_src_work[u32_src_copy_loc];    /* Get the pixel to be copied */
            p_dst_calc[0]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[0], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[2]  = p_src_work[u32_src_copy_loc+1];    /* Get the pixel to be copied */
            p_dst_calc[2]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[1], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[4]  = p_src_work[u32_src_copy_loc+2];    /* Get the pixel to be copied */
            p_dst_calc[4]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[2], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[6]  = p_src_work[u32_src_copy_loc+3];    /* Get the pixel to be copied */
            p_dst_calc[6]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[3], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[8]  = p_src_work[u32_src_copy_loc+4];    /* Get the pixel to be copied */
            p_dst_calc[8]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[4], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[10]  = p_src_work[u32_src_copy_loc+5];    /* Get the pixel to be copied */
            p_dst_calc[10]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[5], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[12]  = p_src_work[u32_src_copy_loc+6];    /* Get the pixel to be copied */
            p_dst_calc[12]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[6], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            

            p_dst_copy[14]  = p_src_work[u32_src_copy_loc+7];    /* Get the pixel to be copied */
            p_dst_calc[14]  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac[7], p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            
            
            p_dst_copy  +=16;                    /* Increment to the next copy pixel location */
            p_dst_calc  +=16;                    /* Increment to the next to-be calculated pixel location */        
            p_src_work  +=8;                    /* Increment working source pointer for next pixel */        

        }
        memcpy(&p_leading_pixels[0],&p_leading_pixels[8],sizeof(img_pel_t) * 8);

    }while(--u32_loop_cnt);

                            
    if(u32_safe_pixels%8)                /* handle unsafe end pixels */
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

        u32_pixel_cnt=8;
        p_src_work = &p_leading_pixels[0];
        do
        {
            int32_t i32_mac;
            i32_mac=0;
            for(u32_filter_cnt=0;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {
                i32_mac +=p_src_work[u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }
            *p_dst_copy  = p_src_work[u32_src_copy_loc];
            *p_dst_calc  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            p_dst_copy+=2;
            p_dst_calc+=2;
            
            p_src_work++;
        }while(--u32_pixel_cnt);

    }/*if(u32_safe_pixels%8)*/
}/* End of horizontal_upsampling_packed() */



/**
 * \fn void horizontal_upsampling_packed(img_pel_t    *p_out,
 *                                    const img_pel_t    *p_src,
 *                                    img_pel_t p_leading_pixels[32],
 *                                    uint32_t u32_safe_pixels,
 *                                    uint32_t u32_unsafe_end,
 *                                    uint32_t u32_view_offset,
 *                                    const RPUFilter *p_flt_1d)
 *
 * \brief horizonta upsample a row of packed data.(Stage 2 of OM RPU)
 *
 * \details Function does horizontal upsampling of a single row for packed data.
 *
 *
 * \param[in]    p_out                    Pointer to the output row.
 * \param[in]    p_src                    Pointer to the input row.
 * \param[in]    p_leading_pixels[32]    pointer to the first 32 pixles in the row(16 for U & 16 for V).(Unsafe pixels are already replicated and available)
 * \param[in]    u32_safe_pixels            total number of safe pixels.
 * \param[in]    u32_unsafe_end_pixels    total number of unsafe end pixels.
 * \param[in]    u32_view_offset            view_grid offset (0/1)
 * \param[in]    p_flt_1d                Pointer to filter definition. This is typecast to 1D filter.
 *
 * \return None
 */

void horizontal_upsampling_packed(img_pel_t        *p_out,
                              const img_pel_t    *p_src,
                                    img_pel_t   p_leading_pixels_uv[32],
                                    uint32_t    u32_safe_pixels,
                                    uint32_t    u32_unsafe_end,
                                    uint32_t    u32_view_offset,
                              const RPUFilter *p_flt_1d)
{

    uint32_t u32_loop_cnt,u32_pixel_cnt,u32_filter_cnt,u32_src_copy_loc,u32_iloop;
    int32_t i32_mac_u,i32_mac_v;
    const img_pel_t    *p_src_work_u,*p_src_work_v;
    img_pel_t *p_dst_copy_u,*p_dst_calc_u,*p_dst_copy_v,*p_dst_calc_v;
    img_pel_t copy_pixel_u,copy_pixel_v;

    u32_loop_cnt = u32_safe_pixels >> 4;

    /* Destination pointer for  copy pixel location */
    /* Destination pointer for  to-be-calculated pixel location */
    if(u32_view_offset==0)
    {
    p_dst_copy_u = p_out ;
    p_dst_copy_v = p_out +1;
    p_dst_calc_u = p_out +2;
    p_dst_calc_v = p_out +3;
    }
    else
    {
    p_dst_copy_u = p_out +2;
    p_dst_copy_v = p_out +3;
    p_dst_calc_u = p_out ;
    p_dst_calc_v = p_out +1;
    }

    u32_src_copy_loc = 2*((p_flt_1d->u16_num_taps>>1) -1+ u32_view_offset);        /* offset from the first pixel used in filtering to the middle pixel */
    
    do{
        memcpy(&p_leading_pixels_uv[16],p_src,sizeof(img_pel_t) * 16);
        p_src +=16;
        
        u32_pixel_cnt=8;                            /* Process Blocks fo 8-U and 8-V pixels*/
        p_src_work_u = &p_leading_pixels_uv[0];        /* Points to the first U pixel in the block of 16-U-pixels(alternatively arranged) for processing 8-U-pixels */
        p_src_work_v = &p_leading_pixels_uv[1];        /* Points to the first V pixel in the block of 16-V-pixels(alternatively arranged) for processing 8-V-pixels */
        do
        {
            i32_mac_u=0;
            i32_mac_v=0;

            for(u32_filter_cnt=0;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {
                i32_mac_u +=p_src_work_u[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i32_mac_v +=p_src_work_v[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }
            *p_dst_copy_u  = p_src_work_u[u32_src_copy_loc];/* Get the U-pixel to be copied */
            *p_dst_copy_v  = p_src_work_v[u32_src_copy_loc];/* Get the V-pixel to be copied */

            *p_dst_calc_u  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac_u, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            *p_dst_calc_v  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac_v, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            p_dst_copy_u+=4;            /* Increment to the next U copy pixel location */
            p_dst_calc_u+=4;            /* Increment to the next U to-be-calculated pixel location */
            p_dst_copy_v+=4;            /* Increment to the next V copy pixel location */
            p_dst_calc_v+=4;            /* Increment to the next V to-be-calculated pixel location */

            p_src_work_u+=2;            /* Increment working source pointer for next U pixel */        
            p_src_work_v+=2;            /* Increment working source pointer for next V pixel */        
        }while(--u32_pixel_cnt);

        for(u32_iloop=0;u32_iloop<16;u32_iloop++)
        {
            p_leading_pixels_uv[u32_iloop] = p_leading_pixels_uv[16+u32_iloop];
        }/* move the last 8-UV-pixels to the top 8-pixels for processing next block of 8-pixels */
    }while(--u32_loop_cnt);


    if(u32_safe_pixels%16)                /* handle unsafe end pixels */
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

        u32_pixel_cnt=8;
        p_src_work_u = &p_leading_pixels_uv[0];
        p_src_work_v = &p_leading_pixels_uv[1];

        do
        {
            i32_mac_u=0;
            i32_mac_v=0;
            for(u32_filter_cnt=0;u32_filter_cnt<p_flt_1d->u16_num_taps;u32_filter_cnt++)
            {
                i32_mac_u +=p_src_work_u[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
                i32_mac_v +=p_src_work_v[2*u32_filter_cnt] * p_flt_1d->ai16_coef1[u32_filter_cnt];
            }
            *p_dst_copy_u  = p_src_work_u[u32_src_copy_loc];
            *p_dst_copy_v  = p_src_work_v[u32_src_copy_loc];

            *p_dst_calc_u  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac_u, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));
            *p_dst_calc_v  = (img_pel_t ) CLIP(OFFSET_SHIFT_RIGHT(i32_mac_v, p_flt_1d->u16_offset, p_flt_1d->u16_normal1));

            p_dst_copy_u+=4;
            p_dst_copy_v+=4;
            p_dst_calc_u+=4;
            p_dst_calc_v+=4;
            
            p_src_work_u+=2;
            p_src_work_v+=2;
            
        }while(--u32_pixel_cnt);
    }/* if(u32_safe_pixels%16) */
}/* End of horizontal_upsampling_packed() */



int32_t sbs_om_down_filter(
                         void        *pv_rpu_handle,
                   const img_pel_t    *p_src,
                         uint16_t    u16_src_stride,
                         img_pel_t    *p_dst,
                         uint16_t    u16_dst_stride,
                         uint16_t    u16_view_delimiter_sbs,
                         uint16_t    u16_view_delimiter_ou,
                         uint8_t    u8_packed_UV
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


    /* Vertical filtering parameters */
    filter_params.u16_origin_x       = 0;
    filter_params.u16_origin_y       = 0;      
    filter_params.u16_center_start_y = u16_filter_offset;
    filter_params.u16_center_end_y   = 2 * u16_view_delimiter_ou - u16_filter_offset;    
    filter_params.u16_end_x          = u16_view_delimiter_sbs;
    filter_params.u16_end_y          = 2 * u16_view_delimiter_ou;
    


    /* Src and Dst  Pointers for Left View Downsampling*/
    p_in  = p_src ;    
    p_out = p_dst ;

    
    /* Call filter function */
    om_rpu_downsample_vertical_filter(p_flt,
                          &filter_params,
                          p_in,
                          u16_src_stride,
                          p_out,
                          u16_dst_stride,
                          u8_packed_UV
                         );

    

    /* Vertical filtering parameters modified for right view*/
    filter_params.u16_origin_x = u16_view_delimiter_sbs;
    filter_params.u16_end_x    = 2*u16_view_delimiter_sbs;

    /* Src and Dst  Pointers for Right View Downsampling*/
    p_in  = p_src +  u16_view_delimiter_sbs;    
    p_out = p_dst + (u16_view_delimiter_ou * u16_dst_stride) ;

    
    /* Call filter function */
    om_rpu_downsample_vertical_filter(p_flt,
                          &filter_params,
                          p_in,
                          u16_src_stride,
                          p_out,
                          u16_dst_stride,
                          u8_packed_UV
                         ); 

    return i32_status;

} /* End of sbs_om_down_filter() function */


void om_rpu_downsample_vertical_filter(                                    
                              const RPUFilter   *p_flt,
                              const RPUFilterParams *p_filter_params,
                              const img_pel_t       *p_in,
                                    uint16_t        u16_input_stride,
                                    img_pel_t       *p_out,
                                    uint16_t        u16_output_stride,
                                    uint8_t         u8_packed_UV
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

    int16_t i16_available_rows;
    const img_pel_t *p_src_rows[11];
    const img_pel_t **p_ref_rows = &p_src_rows[0];
    const img_pel_t **p_ref_rows_2 ;
    uint16_t u16_row;
    uint16_t u16_filter_rows,u16_num_unsafe_pixels;
    uint16_t u16_filter_num_taps = p_flt->u16_num_taps;


    UNREFERENCED_PARAMETER(u8_packed_UV);

    /* Set up source row pointers */
    u16_row = u16_center_start_y-u16_origin_y;
    p_in -= (u16_input_stride*(u16_filter_num_taps>>1) * !u16_row);
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


    u16_num_unsafe_pixels = u16_end_y - u16_center_end_y;
    
    
    for(u16_row = u16_end_y - u16_origin_y;u16_row > u16_num_unsafe_pixels+2 ; u16_row-=2)
    {
        /* vertically downsample each row */
        vertical_downsample_row(p_out,p_src_rows,u16_end_x-u16_origin_x,p_flt);
        p_out += u16_output_stride;
        
        /* shift the pointers up by 2 rows for downsampling - and add last two row pointers */
        u16_filter_rows = u16_filter_num_taps-3;
        p_ref_rows  = &p_src_rows[0];
        p_ref_rows_2  = &p_src_rows[2];
        while(u16_filter_rows-- )
        {
            *p_ref_rows++ = *p_ref_rows_2++;
        }        
    
        *p_ref_rows++ = *p_ref_rows_2;
        *p_ref_rows++ = *p_ref_rows_2 + (u16_input_stride ) ;
        *p_ref_rows++ = *p_ref_rows_2 +(2*u16_input_stride) ;

    }

    vertical_downsample_row(p_out,p_src_rows,u16_end_x-u16_origin_x,p_flt);
    p_out += u16_output_stride;
    u16_row -=2;
    i16_available_rows = u16_row+1-u16_num_unsafe_pixels;

    while(u16_row)        /* unsafe bottom positions */
    {    
        uint16_t u16_offset1;


        u16_filter_rows = u16_filter_num_taps-3;
        p_ref_rows  = &p_src_rows[0];
        p_ref_rows_2  = &p_src_rows[2];
        while(u16_filter_rows-- )
        {
            *p_ref_rows++ = *p_ref_rows_2++;
        }
        *p_ref_rows++ = *p_ref_rows_2;
        

        u16_offset1 = ((i16_available_rows--)>0) *  (u16_input_stride );
        *p_ref_rows_2 += u16_offset1;
        *p_ref_rows++ = *p_ref_rows_2 ;

        u16_offset1 = ((i16_available_rows--)>0) *  (u16_input_stride );        
        *p_ref_rows_2 += u16_offset1;
        *p_ref_rows++ = *p_ref_rows_2 ;

        u16_row-=2;
        
        vertical_downsample_row(p_out,p_src_rows,u16_end_x-u16_origin_x,p_flt);
        p_out += u16_output_stride;

    }


} /* End of om_rpu_downsample_vertical_filter() */



void om_rpu_upsample_horizontal_filter(
                              const RPUFilter   *p_flt,
                              const RPUFilterParams *p_filter_params,
                              const img_pel_t       *p_in,
                                    uint16_t        u16_input_stride,
                                    img_pel_t       *p_out,
                                    uint16_t        u16_output_stride,
                                    uint8_t         u8_packed_UV
                             )
{
    
    uint32_t i_loop;
    const img_pel_t        *p_src;
    uint32_t u32_row_loop;
    
    /* setup the safe and un_safe pixel counts */
    uint32_t u32_unsafe_start = p_filter_params->u16_center_start_x - p_filter_params->u16_origin_x;
    uint32_t u32_safe            = p_filter_params->u16_center_end_x - p_filter_params->u16_center_start_x;
    uint32_t u32_height       = p_filter_params->u16_end_y-p_filter_params->u16_origin_y;
    uint32_t u32_unsafe_end   = ((p_flt->u16_num_taps >> 1) - 1 + p_filter_params->u16_view_offset_x );
        
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

                /* upsample single row */
                horizontal_upsampling_unpacked(p_out,p_src,u8_leading_pixels,u32_unsafe_start+u32_safe,u32_unsafe_end,p_filter_params->u16_view_offset_x,p_flt);

                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/*if(u32_unsafe_start)*/
        else
        {
            
            for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)
            {
                p_src = p_in -  u32_unsafe_end;        
                memcpy(&u8_leading_pixels,p_src,sizeof(img_pel_t)*8);    /* read a block of 8 safe pixels */    
                p_src+=8;
                
                /* upsample single row */
                horizontal_upsampling_unpacked(p_out,p_src,u8_leading_pixels,u32_unsafe_start+u32_safe,u32_unsafe_end,p_filter_params->u16_view_offset_x,p_flt);            
                
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

                /* upsample single row */
                horizontal_upsampling_packed(p_out,p_src,u8_leading_pixels_uv,u32_unsafe_start+u32_safe,u32_unsafe_end,p_filter_params->u16_view_offset_x,p_flt);

                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/* if(u32_unsafe_start) */
        else
        {
            for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)
            {
                p_src = p_in -  2*u32_unsafe_end;
                memcpy(&u8_leading_pixels_uv,p_src,sizeof(img_pel_t)*16);            /* read a block of 8U & 8V safe pixels */    
                p_src+=16;

                /* upsample single row */
                horizontal_upsampling_packed(p_out,p_src,u8_leading_pixels_uv,u32_unsafe_start+u32_safe,u32_unsafe_end,p_filter_params->u16_view_offset_x,p_flt);            

                p_in +=u16_input_stride;
                p_out +=u16_output_stride;
            }/*for(u32_row_loop=u32_height;u32_row_loop!=0;u32_row_loop--)*/
        }/* else if(u32_unsafe_start)*/    
    }/* else if(u8_packed_UV==0)*/

} /* End of om_rpu_upsample_horizontal_filter() */
 

 

int32_t sbs_om_up_filter(
                         void        *pv_rpu_handle,
                   const img_pel_t   *p_src,
                         uint16_t    u16_src_stride,
                         img_pel_t   *p_dst,
                         uint16_t    u16_dst_stride,
                         uint16_t    u16_view_delimiter_sbs,
                         uint16_t    u16_view_delimiter_ou,
                         uint8_t     u8_packed_UV,
                         uint8_t     u8_view_grid_offset_v0,
                         uint8_t     u8_view_grid_offset_v1
                  )
{
    int32_t i32_status = SUCCESS;

    RPUHandle *p_rpu_handle = (RPUHandle *) pv_rpu_handle;

    const RPUFilter    *p_flt = &p_rpu_handle->rpu_om_up_filter;


    const img_pel_t *p_in = NULL;
    img_pel_t *p_out = NULL;

    uint16_t u16_filter_offset;
    RPUFilterParams filter_params;

        
    
   
    u16_filter_offset = p_flt->u16_taps_div2;

    /* Horizontal filtering parameters */
    filter_params.u16_origin_x = 0;
    filter_params.u16_end_x    = u16_view_delimiter_sbs;

    
    /* Process from partition starting to beginning of delimiter */
    filter_params.u16_origin_y =    0;
    filter_params.u16_end_y    =    u16_view_delimiter_ou;
    filter_params.u16_view_offset_x  = u8_view_grid_offset_v0;
    filter_params.u16_center_start_x = u16_filter_offset - !filter_params.u16_view_offset_x;
    filter_params.u16_center_end_x   = u16_view_delimiter_sbs - (u16_filter_offset - filter_params.u16_view_offset_x);
        
    p_in  = p_src ;
    p_out = p_dst ;
        
        
    /* Call filter function */
    om_rpu_upsample_horizontal_filter(p_flt,
                    &filter_params,
                    p_in,
                    u16_src_stride,
                    p_out,
                    u16_dst_stride,
                    u8_packed_UV
                    );

    /* Process from  beginning of delimiter to end of partition*/
    filter_params.u16_origin_y  = u16_view_delimiter_ou;
    filter_params.u16_end_y     = 2*u16_view_delimiter_ou;
    filter_params.u16_view_offset_x  = u8_view_grid_offset_v1;
    filter_params.u16_center_start_x = u16_filter_offset - !filter_params.u16_view_offset_x;
    filter_params.u16_center_end_x   = u16_view_delimiter_sbs - (u16_filter_offset - filter_params.u16_view_offset_x);
        
    p_in  = p_src + (u16_view_delimiter_ou * u16_src_stride)  ;
    p_out = p_dst + (u16_view_delimiter_ou * u16_dst_stride)  ;


    /* Call filter function */
    om_rpu_upsample_horizontal_filter(p_flt,
                    &filter_params,
                    p_in,
                    u16_src_stride,
                    p_out,
                    u16_dst_stride,
                    u8_packed_UV
                    );



    return i32_status;

} /* End of sbs_om_up_filter() function */



    
