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
* \file  mfc_common_pvt.c
*
* \brief 
*        MFC SDK  functions interface to JM 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#include "c3d_rpu_kernel_api.h"
#include "c3d_utilities_api.h"
#include "mfc_common.h"


void* alloc_mfc_rpu_data(){

    RPUData *p_rpu_data = NULL;

    p_rpu_data = (RPUData *) allocate_memory(sizeof(RPUData));
    if(!p_rpu_data)
    {
        printf("\nError allocating memory for RPUData structure");
        return NULL;
    } /* if(!p_rpu_data) */

    return (void *) p_rpu_data;
}



int set_mfc_rpu_data(MFC_RPU_DATA *mfc_static_params, 
                             void *pv_rpu_data){

    RPUData *p_rpu_data = (RPUData *)pv_rpu_data;

  


    /* Set mfc_format_idc */
    if(SBS==mfc_static_params->mfc_format_idc)
    {
        p_rpu_data->e_rpu_process_format = SBS;
    }
    else if(OU==mfc_static_params->mfc_format_idc)
    {
        p_rpu_data->e_rpu_process_format = OU;
    }
    else
    {
        printf("\nMFC HIGH PROFILE : Unsupported Value  for 'mfc_format_idc' : %d\n",mfc_static_params->mfc_format_idc);
        printf("\nMFC HIGH PROFILE : Supported   Values for 'mfc_format_idc' : %d, %d\n",SBS,OU);
        return FAILURE;
    }

    /* Rpu Filter Type DC / F0  */
    p_rpu_data->u8_rpu_filter_enabled_flag            = (uint8_t) mfc_static_params->rpu_filter_enabled_flag;

    /* Default Grid Position Flag */
    p_rpu_data->u8_default_grid_position_flag = (uint8_t) mfc_static_params->default_grid_position_flag;

    if(p_rpu_data->u8_default_grid_position_flag)
    {   
        if  (SBS==p_rpu_data->e_rpu_process_format) 
        {   /* Default Grid Posiition for SBS */
               p_rpu_data->u8_view0_grid_position_x = (uint8_t) map_spec_offset_to_mfc_format(4);
               p_rpu_data->u8_view0_grid_position_y = (uint8_t) map_spec_offset_to_mfc_format(8);
               p_rpu_data->u8_view1_grid_position_x = (uint8_t) map_spec_offset_to_mfc_format(12);
               p_rpu_data->u8_view1_grid_position_y = (uint8_t) map_spec_offset_to_mfc_format(8);
        }
        else if  (OU==p_rpu_data->e_rpu_process_format) 
        {   /* Default Grid Posiition for OU */
               p_rpu_data->u8_view0_grid_position_x = (uint8_t) map_spec_offset_to_mfc_format(8);
               p_rpu_data->u8_view0_grid_position_y = (uint8_t) map_spec_offset_to_mfc_format(4);
               p_rpu_data->u8_view1_grid_position_x = (uint8_t) map_spec_offset_to_mfc_format(8);
               p_rpu_data->u8_view1_grid_position_y = (uint8_t) map_spec_offset_to_mfc_format(12);
        }
        
    }
    else
    {   
        p_rpu_data->u8_view0_grid_position_x = (uint8_t) map_spec_offset_to_mfc_format(mfc_static_params->view0_grid_position_x);
        p_rpu_data->u8_view0_grid_position_y = (uint8_t) map_spec_offset_to_mfc_format(mfc_static_params->view0_grid_position_y);
        p_rpu_data->u8_view1_grid_position_x = (uint8_t) map_spec_offset_to_mfc_format(mfc_static_params->view1_grid_position_x);
        p_rpu_data->u8_view1_grid_position_y = (uint8_t) map_spec_offset_to_mfc_format(mfc_static_params->view1_grid_position_y);
    }
    
    p_rpu_data->u8_packed_UV             = 0 ; 
    return SUCCESS;
}


void* allocate_mfc_image_format()
{
    ImageData *p_img_data = NULL;
    p_img_data = (ImageData *) allocate_memory(sizeof(ImageData));
    if(!p_img_data)
    {
        printf("\nError allocating memory for MFC Image Data Structure");
        return NULL;
    } /* if(!p_img_data) */

    return (void *) p_img_data;
}


void free_mfc_format            (
                                void *mfc_ImageData
                                )
{
    free(mfc_ImageData);
    mfc_ImageData=NULL;
}




void reset_mfc_imgData    (
                            MFC_IMG_DATA    *src_image_data,
                            void            *pv_img_data
                            )
{
    ImageData *p_img_data;
    unsigned int i_loop;

    p_img_data = (ImageData *)pv_img_data;


      for(i_loop=0;i_loop<3;i_loop++)
      {
        p_img_data->pa_buf[i_loop]                  = (img_pel_t *) src_image_data->p_buf[i_loop];
        p_img_data->au16_frame_width[i_loop]        = src_image_data->au16_frame_width[i_loop];
        p_img_data->au16_frame_height[i_loop]       = src_image_data->au16_frame_height[i_loop];
        p_img_data->au16_buffer_stride[i_loop]      = src_image_data->au16_buffer_stride[i_loop];
        p_img_data->au16_view_delimiter_sbs[i_loop] = src_image_data->au16_view_delimiter_sbs[i_loop];
        p_img_data->au16_view_delimiter_ou[i_loop]  = src_image_data->au16_view_delimiter_ou[i_loop];
      }  

      p_img_data->e_yuv_chroma_format      =(CHROMA_FORMAT) src_image_data->chroma_format;
      p_img_data->e_picture_type           =(PICTURE_TYPE) src_image_data->picture_type;




}


int write_yuv_file_wrapper  (
                            FILE *fp,
                      const void *pv_imgData
                            )
{
    int status = 0;

    status= write_yuv_image( (ImageData *)pv_imgData,
                             NULL,
                            fp,
                            0);
    return status;    
}

void copy_mfc_ImageData    (
                            void        *pv_dst_image,
                            void        *pv_src_image
                            )
{
        //copy_complete_ImageData((ImageData *)pv_dst_image,(ImageData *)pv_src_image);
        copy_image_pixeldata_only((ImageData *)pv_dst_image,(ImageData *)pv_src_image);
}






