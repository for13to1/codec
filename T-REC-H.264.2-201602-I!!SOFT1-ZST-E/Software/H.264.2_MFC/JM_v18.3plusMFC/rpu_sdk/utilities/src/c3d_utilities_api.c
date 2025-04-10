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
* \file  c3d_utilities_api.c
*
* \brief 
*        MFC SDK  Utilities layer API functions
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
#include "c3d_utilities_api.h"
#include <time.h>





int32_t read_yuv_image(ImageData *p_img ,ImageData *p_img_tmp, FILE *pf_input,uint8_t u8_packedUV)
{
    int32_t i32_i;
    img_pel_t *ptr1,*ptr2;
    ImageData *p_workImg;
    uint32_t u32_bytes_read;

    ptr1 = p_img->pa_buf[Y];
    for(i32_i = 0; i32_i < p_img->au16_frame_height[Y]; i32_i++)
    {
        
        ptr2 = ptr1 + (i32_i *  p_img->au16_buffer_stride[Y]);
        u32_bytes_read =(uint32_t) fread(ptr2, 1, p_img->au16_frame_width[Y], pf_input )        ;

        if(u32_bytes_read != p_img->au16_frame_width[Y])
        {
            return FAILURE;
        }
    } /* read Y rows */

    p_workImg = u8_packedUV==0 ? p_img :p_img_tmp;

    ptr1 = p_workImg->pa_buf[U];
    for(i32_i = 0; i32_i < p_workImg->au16_frame_height[U]; i32_i++)
    {
        
        ptr2 = ptr1 + (i32_i * p_img->au16_buffer_stride[U]);
        u32_bytes_read =(uint32_t) fread(ptr2, 1, p_workImg->au16_frame_width[U], pf_input )        ;

        if(u32_bytes_read != p_img->au16_frame_width[U])
        {
            return FAILURE;
        }
    } /* read U rows*/

    ptr1 = p_workImg->pa_buf[V];
    for(i32_i = 0; i32_i < p_workImg->au16_frame_height[V]; i32_i++)
    {
        
        ptr2 = ptr1 + (i32_i * p_img->au16_buffer_stride[V]);
        u32_bytes_read =(uint32_t) fread(ptr2, 1, p_workImg->au16_frame_width[V], pf_input )        ;

        if(u32_bytes_read != p_img->au16_frame_width[V])
        {
            return FAILURE;
        }

    } /*read V rows */
    

    if(u8_packedUV)
    {
        img_pel_t *ptr3,*ptr4,*ptr5,*ptr6;
        int32_t i32_j;
        
        /* Pack the UV data */
        ptr1 = p_img_tmp->pa_buf[U];
        ptr2 = p_img_tmp->pa_buf[V];        
        ptr3 = p_img->pa_buf[U];        
        for(i32_i = 0; i32_i < p_img_tmp->au16_frame_height[U]; i32_i++)
        {
            ptr4 = ptr1 + (i32_i * p_img_tmp->au16_buffer_stride[U]);
            ptr5 = ptr2 + (i32_i * p_img_tmp->au16_buffer_stride[V]);
            ptr6 = ptr3 + (i32_i * p_img_tmp->au16_buffer_stride[U]);

            i32_j = p_img_tmp->au16_frame_width[U];
            while(i32_j--)
            {
                *ptr6++ = *ptr4++;
                *ptr6++ = *ptr5++;
            }
        }/* i_loop p_img_tmp->au16_frame_height[U] */
    }/* u8_packedUV==1 */
    
    return SUCCESS;
} /* End of read_yuv_image() function */


int32_t write_yuv_image(ImageData *p_img,ImageData *p_img_tmp, FILE *pf_output,uint8_t u8_packedUV)
{
    int32_t i32_i;
    img_pel_t *ptr1,*ptr2;
    ImageData *p_workImg;
    uint32_t u32_bytes_written;

    

    ptr1 = p_img->pa_buf[Y];
    for(i32_i = 0; i32_i < p_img->au16_frame_height[Y]; i32_i++)
    {
        
        ptr2 = ptr1 + (i32_i * p_img->au16_buffer_stride[Y]);
        u32_bytes_written =(uint32_t) fwrite(ptr2, 1, p_img->au16_frame_width[Y], pf_output )        ;

        if(u32_bytes_written != p_img->au16_frame_width[Y])
        {
            return FAILURE;
        }
    } /* write Y rows */

    if(u8_packedUV)
    {
        img_pel_t *ptr3,*ptr4,*ptr5,*ptr6;
        int32_t i32_j;
        
        /* Unpack the UV data */
        ptr1 = p_img_tmp->pa_buf[U];
        ptr2 = p_img_tmp->pa_buf[V];        
        ptr3 = p_img->pa_buf[U];        
        for(i32_i = 0; i32_i < p_img_tmp->au16_frame_height[U]; i32_i++)
        {
            ptr4 = ptr1 + (i32_i * p_img->au16_buffer_stride[U]);
            ptr5 = ptr2 + (i32_i * p_img->au16_buffer_stride[V]);
            ptr6 = ptr3 + (i32_i * p_img->au16_buffer_stride[U]);

            i32_j = p_img_tmp->au16_frame_width[U];
            while(i32_j--)
            {
                *ptr4++ = *ptr6++;
                *ptr5++ = *ptr6++;
            }
        }/* i_loop p_img_tmp->au16_frame_height[U] */
    }/* u8_packedUV==1 */
    
    p_workImg = u8_packedUV==0 ? p_img :p_img_tmp;

    ptr1 = p_workImg->pa_buf[U];
    for(i32_i = 0; i32_i < p_workImg->au16_frame_height[U]; i32_i++)
    {
        
        ptr2 = ptr1 + (i32_i * p_img->au16_buffer_stride[U]);
        u32_bytes_written =(uint32_t) fwrite(ptr2, 1, p_workImg->au16_frame_width[U], pf_output )        ;
        if(u32_bytes_written != p_img->au16_frame_width[U])
        {
            return FAILURE;
        }
    } /* write U rows */

    ptr1 = p_workImg->pa_buf[V];
    for(i32_i = 0; i32_i < p_workImg->au16_frame_height[V]; i32_i++)
    {
        
        ptr2 = ptr1 + (i32_i * p_img->au16_buffer_stride[V]);
        u32_bytes_written = (uint32_t) fwrite(ptr2, 1, p_workImg->au16_frame_width[V], pf_output )        ;
        if(u32_bytes_written != p_img->au16_frame_width[V])
        {
            return FAILURE;
        }
    } /* write V rows */    

    return SUCCESS;
} /* End of write_yuv_image() function */



int32_t allocate_image(
                        ImageData **p_img1,
                        ImageData *p_ref_image,                 
                        uint8_t    u8_packed_UV
                       )
{
    int32_t i32_status = SUCCESS;

    /* Extract the data from the reference image data structure */
    int16_t i16_frame_width       = p_ref_image->au16_frame_width[Y];
    int16_t i16_frame_height      = p_ref_image->au16_frame_height[Y];
    int16_t i16_frame_stride      = p_ref_image->au16_buffer_stride[Y];
    int16_t i16_view_delimiter_sbs = p_ref_image->au16_view_delimiter_sbs[Y];
    int16_t i16_view_delimiter_ou = p_ref_image->au16_view_delimiter_ou[Y];
    CHROMA_FORMAT e_chroma_format = p_ref_image->e_yuv_chroma_format;

    /* Allocate image data buffers */
    *p_img1 = alloc_image_mem(i16_frame_width,
                              i16_frame_height,    
                              i16_frame_stride,    
                              e_chroma_format,                              
                              u8_packed_UV);
    if(!(p_img1))
    {
        printf("\nError: In initialising image memory");
        i32_status = FAILURE;
        return i32_status;
    } /* if(!(*p_img1)) */

    
    (*p_img1)->au16_view_delimiter_sbs[0] = i16_view_delimiter_sbs;    
    (*p_img1)->au16_view_delimiter_sbs[1] = (i16_view_delimiter_sbs >> !u8_packed_UV);    
    (*p_img1)->au16_view_delimiter_sbs[2] = (i16_view_delimiter_sbs >> !u8_packed_UV);

    (*p_img1)->au16_view_delimiter_ou[0] = i16_view_delimiter_ou;    
    (*p_img1)->au16_view_delimiter_ou[1] = (i16_view_delimiter_ou >> 1);    
    (*p_img1)->au16_view_delimiter_ou[2] = (i16_view_delimiter_ou >> 1);

    
    return i32_status;

} /* End of allocate_images() */



int32_t generate_random_image(ImageData *p_img)
{
    time_t now;

    /* Frame dimensions */
    int16_t i16_frame_width  = p_img->au16_frame_width[Y];
    int16_t i16_frame_height = p_img->au16_frame_height[Y];
    int16_t i16_frame_stride = p_img->au16_buffer_stride[Y];

    int32_t i32_i, i32_j;

    time(&now);
    srand((unsigned int) now);

    for(i32_i = 0; i32_i < i16_frame_height; i32_i++)
    {
        for(i32_j = 0; i32_j < i16_frame_width; i32_j++)
        {
            p_img->pa_buf[Y][(i32_i * i16_frame_stride) + i32_j] = (img_pel_t ) (rand() % MAX_PIXEL_VALUE);
        } /* for(i32_j = 0; i32_j < i16_frame_width; i32_j++) */
    } /* for(i32_i = 0; i32_i < i16_frame_height; i32_i++) */

    for(i32_i = 0; i32_i < (i16_frame_height >> 1); i32_i++)
    {
        for(i32_j = 0; i32_j < p_img->au16_frame_width[U]; i32_j++)
        {
            p_img->pa_buf[U][(i32_i * p_img->au16_buffer_stride[U]) + i32_j] = (img_pel_t ) (rand() % MAX_PIXEL_VALUE);            
        } 
        for(i32_j = 0; i32_j < p_img->au16_frame_width[V]; i32_j++)
        {
            p_img->pa_buf[V][(i32_i * p_img->au16_buffer_stride[V]) + i32_j] = (img_pel_t ) (rand() % MAX_PIXEL_VALUE);            
        } /* for(i32_j = 0; i32_j < (i16_frame_height >> 1); i32_j++) */
    } /* for(i32_i = 0; i32_i < (i16_frame_width >> 1); i32_i++) */

    return SUCCESS;

} /* End of generate_random_image() function */



int32_t copy_image_pixeldata_only(
                      ImageData *p_dst,
                const ImageData *p_src
               )
{
    uint32_t u32_cmp;
    uint32_t u32_i;

    img_pel_t *p_in, *p_out;

    uint16_t u16_src_width;
    uint16_t u16_src_height;
    uint16_t u16_src_stride;

    uint16_t u16_dst_width;
    uint16_t u16_dst_height;
    uint16_t u16_dst_stride;


    for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++)
    {
        p_in  = p_src->pa_buf[u32_cmp];
        p_out = p_dst->pa_buf[u32_cmp];

        u16_src_width  = p_src->au16_frame_width[u32_cmp];
        u16_src_height = p_src->au16_frame_height[u32_cmp];
        u16_src_stride = p_src->au16_buffer_stride[u32_cmp];

        u16_dst_width  = p_dst->au16_frame_width[u32_cmp];
        u16_dst_height = p_dst->au16_frame_height[u32_cmp];
        u16_dst_stride = p_dst->au16_buffer_stride[u32_cmp];

        /* Copy Y, U and V buffers */
        for(u32_i = 0; u32_i <(uint32_t) MIN_VAL(u16_src_height,u16_dst_height); u32_i++)
        {
            copy_memory(p_out, p_in, MIN_VAL(u16_src_width,u16_dst_width));

            p_in  += u16_src_stride;
            p_out += u16_dst_stride;
        } /* for(u32_i = 0; u32_i < u16_height; u32_i++) */

        
    } /* for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++) */
    
    return SUCCESS;
    
} /* End of copy_image_pixeldata_only() function */

/**
 * \fn int32_t copy_complete_imageData( 
 *                             ImageData* p_dst,
 *                       const ImageData* p_src
 *                      )  
 *
 * \brief Copies the source image to destination.
 *
 * \details Copies the Y,U and V buffers as well as other parameters to destination buffers. It is assumed that the 
 * memory has been allocated.
 *
 * \param[out]    p_dst    Pointer to destination image data structure.
 * \param[in]    p_src    Pointer to source image data structure. 
 *
 * \return None
 */
int32_t copy_complete_ImageData(
                      ImageData *p_dst,
                const ImageData *p_src
               )
{
    uint32_t u32_cmp;
    uint32_t u32_i;

    img_pel_t *p_in, *p_out;

    uint16_t u16_width;
    uint16_t u16_height;
    uint16_t u16_stride;

    for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++)
    {
        p_in  = p_src->pa_buf[u32_cmp];
        p_out = p_dst->pa_buf[u32_cmp];

        u16_width  = p_src->au16_frame_width[u32_cmp];
        u16_height = p_src->au16_frame_height[u32_cmp];
        u16_stride = p_src->au16_buffer_stride[u32_cmp];

    
        /* Copy Y, U and V buffers */
        for(u32_i = 0; u32_i < u16_height; u32_i++)
        {
            copy_memory(p_out, p_in, u16_width);

            p_in  += u16_stride;
            p_out += u16_stride;
        } /* for(u32_i = 0; u32_i < u16_height; u32_i++) */

        /* Copy other parameters */
        p_dst->au16_frame_width[u32_cmp]   = u16_width;
        p_dst->au16_frame_height[u32_cmp]  = u16_height;
        p_dst->au16_buffer_stride[u32_cmp] = u16_stride;

        p_dst->au16_view_delimiter_sbs[u32_cmp] = p_src->au16_view_delimiter_sbs[u32_cmp];    
        p_dst->au16_view_delimiter_ou[u32_cmp] = p_src->au16_view_delimiter_ou[u32_cmp];    

    } /* for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++) */

    p_dst->e_picture_type      = p_src->e_picture_type;
    p_dst->e_yuv_chroma_format = p_src->e_yuv_chroma_format;

    return SUCCESS;

} /* End of copy_image() function */


int32_t pad_ImageData(
                      ImageData *p_dst,
                      uint16_t u16_orig_width,    
                      uint16_t u16_orig_height    
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

    au16_original_width[0] = u16_orig_width;
    au16_original_width[1] = u16_orig_width >> 1;
    au16_original_width[2] = u16_orig_width >> 1;

    au16_original_height[0] = u16_orig_height;
    au16_original_height[1] = u16_orig_height >> 1;
    au16_original_height[2] = u16_orig_height >> 1;


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



int32_t change_image_format(
                      ImageData *p_dst,
                const ImageData *p_src,
                      PICTURE_TYPE e_dst_picture_type 
               )
{
    uint32_t u32_cmp;
    

    p_dst->e_picture_type = e_dst_picture_type;
    p_dst->e_yuv_chroma_format = p_src->e_yuv_chroma_format;
    

    for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++)
    {
        if(FRAME==e_dst_picture_type)
        {
            
             p_dst->pa_buf[u32_cmp]                =  p_src->pa_buf[u32_cmp];
             p_dst->au16_buffer_stride[u32_cmp] =  p_src->au16_buffer_stride[u32_cmp];
             p_dst->au16_frame_width[u32_cmp]   =  p_src->au16_frame_width[u32_cmp]   ;
             p_dst->au16_frame_height[u32_cmp]   =  p_src->au16_frame_height[u32_cmp]   ;
             p_dst->au16_view_delimiter_sbs[u32_cmp]   =  p_src->au16_view_delimiter_sbs[u32_cmp]   ;
             p_dst->au16_view_delimiter_ou[u32_cmp]   =  p_src->au16_view_delimiter_ou[u32_cmp]   ;


        }
        else if(TOP_FIELD==e_dst_picture_type)
        {
             p_dst->pa_buf[u32_cmp] =  p_src->pa_buf[u32_cmp];
             p_dst->au16_buffer_stride[u32_cmp] =  p_src->au16_buffer_stride[u32_cmp] << 1;
             p_dst->au16_frame_width[u32_cmp]   =  p_src->au16_frame_width[u32_cmp]   ;
             p_dst->au16_frame_height[u32_cmp]   =  p_src->au16_frame_height[u32_cmp] >> 1  ;
             p_dst->au16_view_delimiter_sbs[u32_cmp]   =  p_src->au16_view_delimiter_sbs[u32_cmp]   ;
             p_dst->au16_view_delimiter_ou[u32_cmp]   =  p_src->au16_view_delimiter_ou[u32_cmp] >> 1  ;

        }
        else if(BOTTOM_FIELD==e_dst_picture_type)
        {
             p_dst->pa_buf[u32_cmp] =  p_src->pa_buf[u32_cmp] + p_src->au16_buffer_stride[u32_cmp];

             p_dst->au16_buffer_stride[u32_cmp] =  p_src->au16_buffer_stride[u32_cmp] << 1;
             p_dst->au16_frame_width[u32_cmp]   =  p_src->au16_frame_width[u32_cmp]   ;
             p_dst->au16_frame_height[u32_cmp]   =  p_src->au16_frame_height[u32_cmp] >> 1  ;
             p_dst->au16_view_delimiter_sbs[u32_cmp]   =  p_src->au16_view_delimiter_sbs[u32_cmp]   ;
             p_dst->au16_view_delimiter_ou[u32_cmp]   =  p_src->au16_view_delimiter_ou[u32_cmp] >> 1  ;

        }
        else
        {
            printf("\n\nError:Unknown Picture Type\n\n");
            exit(-1);
        }
    }

    return SUCCESS;
} /* End of change_image_format() function */



int32_t copy_image_dimensions(
                      ImageData *p_dst,
                const ImageData *p_src                      
               )
{
    uint32_t u32_cmp;
    

    p_dst->e_picture_type        = p_src->e_picture_type;
    p_dst->e_yuv_chroma_format  = p_src->e_yuv_chroma_format;
    

    for(u32_cmp = 0; u32_cmp < MAX_NUM_COMPONENTS; u32_cmp++)
    {
         p_dst->au16_buffer_stride[u32_cmp] =  p_src->au16_buffer_stride[u32_cmp];
         p_dst->au16_frame_width[u32_cmp]   =  p_src->au16_frame_width[u32_cmp]   ;
         p_dst->au16_frame_height[u32_cmp]   =  p_src->au16_frame_height[u32_cmp]   ;
         p_dst->au16_view_delimiter_sbs[u32_cmp]   =  p_src->au16_view_delimiter_sbs[u32_cmp]   ;
         p_dst->au16_view_delimiter_ou[u32_cmp]   =  p_src->au16_view_delimiter_ou[u32_cmp]   ;
    }

    return SUCCESS;

} /* End of Change_Image_DimensionsOnly() function */



int32_t print_rpu_header(RPUData *p_rpu_data)
{
    PRINT_RPU_DATA(("\n\nRPU Header Info:"));
    PRINT_RPU_DATA(("\n-----------------------------------------------------------------------------"));
    PRINT_RPU_DATA(("\n\tRPU PROCESS FORMAT          : %d    (%s)",p_rpu_data->e_rpu_process_format ,        p_rpu_data->e_rpu_process_format==SBS ? "SBS":p_rpu_data->e_rpu_process_format==OU ? "OU":"UNKNOWN" ));
    PRINT_RPU_DATA(("\n\tRPU FILTER ENABLED FLAG     : %d",p_rpu_data->u8_rpu_filter_enabled_flag));
    PRINT_RPU_DATA(("\n\tDEFAULT GRID POSITION       : %d",p_rpu_data->u8_default_grid_position_flag));
    PRINT_RPU_DATA(("\n\tVIEW0_GRID_POSITION_X       : %d",p_rpu_data->u8_view0_grid_position_x));
    PRINT_RPU_DATA(("\n\tVIEW0_GRID_POSITION_Y       : %d",p_rpu_data->u8_view0_grid_position_y));
    PRINT_RPU_DATA(("\n\tVIEW1_GRID_POSITION_X       : %d",p_rpu_data->u8_view1_grid_position_x));
    PRINT_RPU_DATA(("\n\tVIEW1_GRID_POSITION_Y       : %d",p_rpu_data->u8_view1_grid_position_y));           
    PRINT_RPU_DATA(("\n-----------------------------------------------------------------------------\n"));

    return SUCCESS;
} /* End of print_rpu_header() */


/**
 * \fn uint32_t GetNumberYUVFrames(
 *                    FILE                *pf_yuv_file,
 *                    int                 iWidth,
 *                    int                 iHeight,
 *                    int                 iYUVFormat
 *                    );
 * \brief Find the number of frames in a given yuv file pointer.
 *
 * \param[in]    pf_yuv_file             Point to the start of the YUV file.
 * \param[in]    iWidth                 Width  of the input YUV file. 
 * \param[in]    iHeight                Height of the input YUV file.
 * \param[in]    iYUVFormat             YUV420 - 0 (Only supports YUV420)   
 *
 * \return        None
 */
uint32_t GetNumberYUVFrames(
                            FILE    *pf_yuv_file,
                            int     iWidth,
                            int     iHeight,
                            int     iYUVFormat
                              )
{

    long long sz;

    fseek(pf_yuv_file, 0L, SEEK_END);
    sz = ftell(pf_yuv_file);
    fseek(pf_yuv_file, 0L, SEEK_SET);
    return((uint32_t) (sz / ((iWidth*iHeight*3)>>1)));
}





