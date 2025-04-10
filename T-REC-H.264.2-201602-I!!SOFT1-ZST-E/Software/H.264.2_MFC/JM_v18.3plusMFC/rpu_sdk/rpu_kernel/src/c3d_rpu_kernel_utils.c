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
* \file  c3d_rpu_kernel_utils.c
*
* \brief 
*        MFC SDK  RPU utilities
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


ImageData* alloc_image_mem(
                           uint16_t      u16_frame_width_y,
                           uint16_t      u16_frame_height_y,
                           uint16_t      u16_frame_stride_y,
                           CHROMA_FORMAT e_yuv_chroma_format,                           
                           uint8_t       u8_packed_UV
                          )
{
    ImageData *p_temp = NULL;

    /* Shift factors for all color components */
    int8_t    ai8_shift_cr_x[MAX_NUM_COMPONENTS];
    int8_t    ai8_shift_cr_y[MAX_NUM_COMPONENTS];

    uint16_t u16_frame_width_u;
    uint16_t u16_frame_height_u;
    uint16_t u16_frame_stride_u;

    uint16_t u16_frame_width_v;
    uint16_t u16_frame_height_v;
    uint16_t u16_frame_stride_v;

    uint32_t u32_frame_size_y;
    uint32_t u32_frame_size_u;
    uint32_t u32_frame_size_v;
    uint32_t u32_frame_size_uv;

    p_temp = (ImageData *) allocate_memory(sizeof(ImageData));

    ai8_shift_cr_x[Y] = ai8_shift_cr_y[Y] = 0;

    switch(e_yuv_chroma_format)
    {
    case YUV_400:
        ai8_shift_cr_x[U] = ai8_shift_cr_x[V] = 0;
        ai8_shift_cr_y[U] = ai8_shift_cr_y[V] = 0;
        break; /* YUV_400 */

    case YUV_420:
        ai8_shift_cr_x[U] = ai8_shift_cr_x[V] = 1;
        ai8_shift_cr_y[U] = ai8_shift_cr_y[V] = 1;
        break; /* YUV_420 */

    case YUV_422:
        ai8_shift_cr_x[U] = ai8_shift_cr_x[V] = 1;
        ai8_shift_cr_y[U] = ai8_shift_cr_y[V] = 0;
        break; /* YUV_422 */

    case YUV_444:
        ai8_shift_cr_x[U] = ai8_shift_cr_x[V] = 0;
        ai8_shift_cr_y[U] = ai8_shift_cr_y[V] = 0;
        break; /* YUV_444 */
     
    default: /* Invalid chroma format */
        free_memory((void *) p_temp);
        p_temp = NULL;
        return p_temp;

    } /* switch(e_yuv_chroma_format) */

    /* In case U and V are packed into a single buffer, the width is doubled */
    if(u8_packed_UV)
    {
        /* There will be only two buffers (Y and UV). Hence, shift factors of only U are modified */
        ai8_shift_cr_x[U] -= 1;
    } /* if(u8_packed_UV) */
    
    u16_frame_width_u  = SHIFT_RIGHT(u16_frame_width_y, ai8_shift_cr_x[U]);
    u16_frame_height_u = SHIFT_RIGHT(u16_frame_height_y, ai8_shift_cr_y[U]);
    u16_frame_stride_u = SHIFT_RIGHT(u16_frame_stride_y, ai8_shift_cr_x[U]);
    
    u16_frame_width_v  = SHIFT_RIGHT(u16_frame_width_y, ai8_shift_cr_x[V]);
    u16_frame_height_v = SHIFT_RIGHT(u16_frame_height_y, ai8_shift_cr_y[V]);
    u16_frame_stride_v = SHIFT_RIGHT(u16_frame_stride_y, ai8_shift_cr_x[V]);

    u32_frame_size_y = (u16_frame_stride_y * u16_frame_height_y);
    u32_frame_size_u = (u16_frame_stride_u * u16_frame_height_u);
    u32_frame_size_v = (u16_frame_stride_v * u16_frame_height_v * !u8_packed_UV);
    
    
    u32_frame_size_uv = u32_frame_size_u + u32_frame_size_v;
     



    /* Allocate memory for Y,U,V buffers */
    p_temp->e_picture_type = FRAME;    

    /* Y */
    p_temp->pa_buf[Y] = (img_pel_t *) allocate_memory(u32_frame_size_y * sizeof(img_pel_t));    
    p_temp->au16_frame_width[Y]   = u16_frame_width_y;
    p_temp->au16_frame_height[Y]  = u16_frame_height_y;
    p_temp->au16_buffer_stride[Y] = u16_frame_stride_y;

    /* allocate U and V contiguously */
    p_temp->pa_buf[U] = (img_pel_t *) allocate_memory(u32_frame_size_uv * sizeof(img_pel_t));
    

    p_temp->au16_frame_width[U]  = u16_frame_width_u;
    p_temp->au16_frame_height[U] = u16_frame_height_u;
    p_temp->au16_buffer_stride[U] = u16_frame_stride_u;

    

    
    /* In case of Packed UV assign the V pointer to NULL     
     */
    if(u8_packed_UV)
    {
        p_temp->pa_buf[V] = NULL;
        p_temp->au16_frame_width[V]  = 0;
        p_temp->au16_frame_height[V] = 0;
        p_temp->au16_buffer_stride[V] = 0;
    }
    else
    {
        p_temp->pa_buf[V] =  p_temp->pa_buf[U] + u32_frame_size_u ;
        p_temp->au16_frame_width[V]  = u16_frame_width_v;
        p_temp->au16_frame_height[V] = u16_frame_height_v;
        p_temp->au16_buffer_stride[V] = u16_frame_stride_v;
    }
    
    
    p_temp->e_yuv_chroma_format = e_yuv_chroma_format;

    return p_temp;

} /* End of alloc_image_mem() function */




int32_t compare_images( 
                        const ImageData  *p_img1,
                        const ImageData  *p_img2,
                              uint32_t   *pu32_mismatch_x,
                              uint32_t   *pu32_mismatch_y,
                              uint8_t     *pu8_component
                       )
{
    uint16_t u16_frame_width;
    uint16_t u16_frame_height;
    uint16_t u16_buf1_stride;
    uint16_t u16_buf2_stride;

    img_pel_t *p_src1;
    img_pel_t *p_src2;

    uint32_t u32_i;

    int32_t i32_result;

    /* Y */
    u16_frame_width  = p_img1->au16_frame_width[Y];
    u16_frame_height = p_img1->au16_frame_height[Y];
    u16_buf1_stride  = p_img1->au16_buffer_stride[Y];
    u16_buf2_stride  = p_img2->au16_buffer_stride[Y];

    p_src1 = p_img1->pa_buf[Y];
    p_src2 = p_img2->pa_buf[Y];

    for(u32_i = 0; u32_i < u16_frame_height; u32_i++)
    {
        i32_result = compare_memory(p_src1, 
                                    p_src2, 
                                    u16_frame_width);
        /* The function returns -1 if buffers match */
        if(i32_result != -1)
        {
            *pu32_mismatch_x = (uint32_t ) i32_result;
            *pu32_mismatch_y = u32_i;
            *pu8_component     = Y;

            return FAILURE;
        } /* Mismatch */

        p_src1 += u16_buf1_stride;
        p_src2 += u16_buf2_stride;

    } /* for(u32_i = 0; u32_i < u16_frame_height; u32_i++) */

    /* U */
    u16_frame_width  = p_img1->au16_frame_width[U];
    u16_frame_height = p_img1->au16_frame_height[U];
    u16_buf1_stride  = p_img1->au16_buffer_stride[U];
    u16_buf2_stride  = p_img2->au16_buffer_stride[U];

    p_src1 = p_img1->pa_buf[U];
    p_src2 = p_img2->pa_buf[U];

    for(u32_i = 0; u32_i < u16_frame_height; u32_i++)
    {
        i32_result = compare_memory(p_src1, 
                                    p_src2, 
                                    u16_frame_width);
        /* The function returns -1 if buffers match */
        if(i32_result != -1)
        {
            *pu32_mismatch_x = (uint32_t ) i32_result;
            *pu32_mismatch_y = u32_i;
            *pu8_component     = U;

            return FAILURE;
        } /* Mismatch */

        p_src1 += u16_buf1_stride;
        p_src2 += u16_buf2_stride;

    } /* for(u32_i = 0; u32_i < u16_frame_height; u32_i++) */

    /* V */
    u16_frame_width  = p_img1->au16_frame_width[V];
    u16_frame_height = p_img1->au16_frame_height[V];
    u16_buf1_stride  = p_img1->au16_buffer_stride[V];
    u16_buf2_stride  = p_img2->au16_buffer_stride[V];

    p_src1 = p_img1->pa_buf[V];
    p_src2 = p_img2->pa_buf[V];

    for(u32_i = 0; u32_i < u16_frame_height; u32_i++)
    {
        i32_result = compare_memory(p_src1, 
                                    p_src2, 
                                    u16_frame_width);
        /* The function returns -1 if buffers match */
        if(i32_result != -1)
        {
            *pu32_mismatch_x = (uint32_t ) i32_result;
            *pu32_mismatch_y = u32_i;
            *pu8_component     = V;

            return FAILURE;
        } /* Mismatch */

        p_src1 += u16_buf1_stride;
        p_src2 += u16_buf2_stride;

    } /* for(u32_i = 0; u32_i < u16_frame_height; u32_i++) */

    return SUCCESS;
} /* End of compare_images() function */


void free_image_mem(
                    ImageData *p_img
                   )
{
    if(NULL != p_img)
    {
    free_memory((void *) p_img->pa_buf[Y]);
    free_memory((void *) p_img->pa_buf[U]);    

    free_memory((void *) p_img);
    }

} /* End of free_image_mem() */


void reset_image_mem(
                    ImageData *p_img
                   )
{
    int16_t i16_loop;

    for(i16_loop=0;i16_loop<p_img->au16_frame_height[Y];i16_loop++)
    {
        memset(p_img->pa_buf[Y] + (i16_loop * p_img->au16_buffer_stride[Y]) ,0,sizeof(img_pel_t)*p_img->au16_frame_width[Y]);
    }
    for(i16_loop=0;i16_loop<p_img->au16_frame_height[U];i16_loop++)
    {
        memset(p_img->pa_buf[U] +  (i16_loop * p_img->au16_buffer_stride[U]) ,0,sizeof(img_pel_t)*p_img->au16_frame_width[U]);
    }
    for(i16_loop=0;i16_loop<p_img->au16_frame_height[V];i16_loop++)
    {
        memset(p_img->pa_buf[V] + + (i16_loop * p_img->au16_buffer_stride[V]) ,0,sizeof(img_pel_t)*p_img->au16_frame_width[V]);
    }
    

} /* End of reset_image_mem() */



void pack_frame(uint8_t *dst, const uint8_t *src, uint16_t width, uint16_t height, uint16_t stride)
{
    int x, y;
    const uint8_t *u = src;
    const uint8_t *v = src + height * stride;
    uint8_t *d = dst;

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            d[x*2] = u[x];
            d[x*2+1] = v[x];
        }

        u += stride;
        v += stride;
        d += stride * 2;
    }
} /* pack_frame */


void unpack_frame(uint8_t *dst, const uint8_t *src, uint16_t width, uint16_t height, uint16_t stride)
{
    int x, y;
    uint8_t *u = dst;
    uint8_t *v = dst + height * stride / 2;
    const uint8_t *s = src;

    width /= 2;
    for (y = 0; y < height; ++y)
    {
        /*
         * Process pels beyond begin/end of line. These are artificial
         * test cases. Real filter input should not access invalid pels.
         */
        for (x = 0; x < width; ++x)
        {
            u[x] = s[x*2];
            v[x] = s[x*2+1];
        }

        u += stride/2;
        v += stride/2;
        s += stride;
    }
} /* unpack_frame */



int32_t pad_image_based_on_delimiter(
                      ImageData *p_dst
               )
{
    uint32_t u32_cmp;
    uint32_t u32_i;

    img_pel_t *p_in, *p_out;
    int16_t i16_width_difference,i16_height_difference;

    uint16_t u16_width;
    uint16_t u16_height;
    uint16_t u16_stride;

    uint16_t au16_original_width[MAX_NUM_COMPONENTS];
    uint16_t au16_original_height[MAX_NUM_COMPONENTS];

    img_pel_t u8_fill_pixel;

    au16_original_width[0] = p_dst->au16_view_delimiter_sbs[0] * 2;
    au16_original_width[1] = p_dst->au16_view_delimiter_sbs[1] * 2;
    au16_original_width[2] = p_dst->au16_view_delimiter_sbs[2] * 2;

    au16_original_height[0] = p_dst->au16_view_delimiter_ou[0] * 2;
    au16_original_height[1] = p_dst->au16_view_delimiter_ou[1] * 2;
    au16_original_height[2] = p_dst->au16_view_delimiter_ou[1] * 2;


    for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++)
    {
        


        u16_width  = p_dst->au16_frame_width[u32_cmp];
        u16_height = p_dst->au16_frame_height[u32_cmp];
        u16_stride = p_dst->au16_buffer_stride[u32_cmp];

        p_in = p_dst->pa_buf[u32_cmp];        

        i16_width_difference  = u16_width - au16_original_width[u32_cmp];    
        if( i16_width_difference > 0)
        {
            p_out = p_in + au16_original_width[u32_cmp];
            for(u32_i = 0; u32_i < au16_original_height[u32_cmp]; u32_i++)
             {
                u8_fill_pixel = p_out[-1];
                memset( p_out,u8_fill_pixel,sizeof(img_pel_t) * i16_width_difference);
                p_out += u16_stride;
            } /* for(u32_i = 0; u32_i < u16_height; u32_i++) */
        }

        i16_height_difference  = u16_height - au16_original_height[u32_cmp];    
        if( i16_height_difference > 0)
        {
            p_out = p_in ;
            for(u32_i = 0; u32_i < (uint32_t)(au16_original_height[u32_cmp]-1); u32_i++)
            {
                p_out +=u16_stride;
                p_in +=u16_stride;
            } 
            p_out +=u16_stride;

            for(u32_i =0; u32_i < (uint32_t)i16_height_difference; u32_i++)
            {
                memcpy(p_out,p_in,sizeof(img_pel_t) * u16_width);
                p_out +=u16_stride;
            } 

        }


    } /* for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++) */

    return SUCCESS;


} /* End of pad_ImageData() function */


int32_t set_component_to_dc(
                        ImageData    *p_dst,
                        uint16_t     u16_component_id,
                        uint16_t     u16_dc_value)

{
    uint32_t u32_i;

    img_pel_t *p_in;

    uint16_t u16_width;
    uint16_t u16_height;
    uint16_t u16_stride;

        p_in  = p_dst->pa_buf[u16_component_id];    

        u16_width  = p_dst->au16_frame_width[u16_component_id];
        u16_height = p_dst->au16_frame_height[u16_component_id];
        u16_stride = p_dst->au16_buffer_stride[u16_component_id];

        /* Copy Y, U and V buffers */
        for(u32_i = 0; u32_i < u16_height; u32_i++)
        {
            memset(p_in,u16_dc_value,u16_width * sizeof(img_pel_t));
            p_in  += u16_stride;
        } /* for(u32_i = 0; u32_i < u16_height; u32_i++) */    
        return SUCCESS;
} /* End of set_component_to_dc() function */



