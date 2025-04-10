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
* \file  c3d_rpu_kernel_process.c
*
* \brief 
*        MFC SDK  om-rpu-process function
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
 * \fn int32_t om_rpu_process(
 *                                 void       *pv_rpu_handle,
 *                           const ImageData *p_bl,
 *                                 ImageData *p_pred_el,
 *                           const RPUData   *p_rpu_data
 *                          )
 *
 * \brief API function to filter the input base layer to obtain the predicted enhancement layer (OM Method).
 *
 * \details The partitions listed in the p_rpu_data structure are filtered to obtain the enhancement layer prediction.
 * 
 * \param[in]    pv_rpu_handle    Handle to the RPU context.
 * \param[in]    p_bl            Base layer picture. It can correspond to the reconstructed base layer in case of 
 *                                encoder or decoded base layer in case of decoder.
 * \param[out]    p_pred_el        Predicted enhancement layer output by the RPU.
 * \param[in]    p_rpu_data        It contains information such as number of partitions, spatial information and filter 
 *                                to be used to process each partition.
 *
 * \return Success (0) or Failure (1)
 */
int32_t om_rpu_process(
                          void      *pv_rpu_handle,
                    const ImageData *p_bl,
                          ImageData *p_pred_el,
                    const RPUData   *p_rpu_data
                   )
{
    int32_t i32_status = SUCCESS;

    RPUHandle *p_rpu_handle = (RPUHandle *) pv_rpu_handle;

    RPU_PROCESS_FORMAT e_rpu_process_format = p_rpu_data->e_rpu_process_format;
     

    int32_t (*down_filter)(void*,  const img_pel_t*, uint16_t, img_pel_t*, uint16_t,  
                      uint16_t,uint16_t,  uint8_t) = NULL;

    int32_t (*up_filter)(void*,  const img_pel_t*, uint16_t, img_pel_t*, uint16_t,  
                      uint16_t,uint16_t,  uint8_t,uint8_t,uint8_t) = NULL;

 

    
    uint32_t u32_cmp,u32_tot_cmp;
    uint8_t u8_grid_position_v0;
    uint8_t u8_grid_position_v1;
    
    if(1==p_rpu_data->u8_rpu_filter_enabled_flag)
    {
        /* RPU Filter */
        switch(e_rpu_process_format)
        {
        case SBS:
            down_filter = &sbs_om_down_filter;
            up_filter   = &sbs_om_up_filter;
            u8_grid_position_v0 = p_rpu_data->u8_view0_grid_position_x;
            u8_grid_position_v1 = p_rpu_data->u8_view1_grid_position_x;
            break;

        case OU:
            down_filter = &ou_om_down_filter;
            up_filter   = &ou_om_up_filter;
            u8_grid_position_v0 = p_rpu_data->u8_view0_grid_position_y;
            u8_grid_position_v1 = p_rpu_data->u8_view1_grid_position_y;
            break;

        default: /* Invalid RPU format */
            i32_status = FAILURE;
            return i32_status;
        } /* switch(e_rpu_process_format) */

        /* OM_DOWN_RPU   Stage 1 of OM PRU*/
        u32_tot_cmp = p_rpu_data->u8_packed_UV==0?3:2;
        for(u32_cmp = 0; u32_cmp < u32_tot_cmp ; u32_cmp++)
        {
            /* Pointer to current partition */

            i32_status = down_filter(
                                p_rpu_handle,
                                p_bl->pa_buf[u32_cmp],
                                p_bl->au16_buffer_stride[u32_cmp],
                                p_rpu_handle->p_temp_img->pa_buf[u32_cmp],
                                p_rpu_handle->p_temp_img->au16_buffer_stride[u32_cmp],                                        
                                p_bl->au16_view_delimiter_sbs[u32_cmp],
                                p_bl->au16_view_delimiter_ou[u32_cmp],
                                u32_cmp==0?0:p_rpu_data->u8_packed_UV
                                );

            if(i32_status) /* FAILURE */
            {
                return i32_status;
            } /* if(i32_status) */

        } /* for(u32_cmp = 0; u32_cmp < u8_num_components; u32_cmp++) */


        /* OM_UP_RPU   Stage 2 of OM PRU*/
        for(u32_cmp = 0; u32_cmp < u32_tot_cmp ; u32_cmp++)
        {
                    /* Pointer to current partition */
              

                    i32_status = up_filter(
                                        p_rpu_handle,
                                        p_rpu_handle->p_temp_img->pa_buf[u32_cmp],
                                        p_rpu_handle->p_temp_img->au16_buffer_stride[u32_cmp],
                                        p_pred_el->pa_buf[u32_cmp],
                                        p_pred_el->au16_buffer_stride[u32_cmp],
                                        p_bl->au16_view_delimiter_sbs[u32_cmp],
                                        p_bl->au16_view_delimiter_ou[u32_cmp],
                                        u32_cmp==0?0:p_rpu_data->u8_packed_UV,
                                        u8_grid_position_v0,
                                        u8_grid_position_v1
                                       );
                    if(i32_status) /* FAILURE */
                    {
                        return i32_status;
                    } /* if(i32_status) */

        } /* for(u32_cmp = 0; u32_cmp < u8_num_components; u32_cmp++) */



        /* Complete the Padding for Pred EL */
        pad_image_based_on_delimiter(p_pred_el);
    }
    else
    {
        /* Set RPU Output to Constant DC Value */
        set_component_to_dc(p_pred_el,0,128);
        set_component_to_dc(p_pred_el,1,128);
        set_component_to_dc(p_pred_el,2,128);
    }/*     if(1==p_rpu_data->u8_rpu_filter_enabled_flag)   */


    return i32_status;

        
} /* End of om_rpu_process() function */

