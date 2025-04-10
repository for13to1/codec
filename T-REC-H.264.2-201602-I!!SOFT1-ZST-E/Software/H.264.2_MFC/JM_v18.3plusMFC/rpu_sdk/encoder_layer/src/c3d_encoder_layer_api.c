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
* \file  c3d_encoder_layer_api.c
*
* \brief 
*        MFC SDK  Encoder layer API functions 
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
#include "c3d_utilities_api.h"


void* allocate_mfc_encoder_layer()
{
    EncoderLayerContext *p_enc_lyr_ctxt = NULL;

    /* Allocate memory for decoder layer context */
    p_enc_lyr_ctxt = (EncoderLayerContext *) allocate_memory(sizeof(EncoderLayerContext));
    /* If memory allocation fails, then return NULL */
    if(!p_enc_lyr_ctxt)
    {
        return NULL;
    } /* if(!p_enc_lyr_ctxt) */

    /* Initialise the RPU */
    p_enc_lyr_ctxt->pv_rpu_kernel = rpu_init();
    /* If RPU kernel initialization fails, then return NULL */
    if(!p_enc_lyr_ctxt->pv_rpu_kernel)
    {
        return NULL;
    } /* if(!p_dec_lyr_ctxt->pv_rpu_kernel) */

    p_enc_lyr_ctxt->p_om_temp_img1 = alloc_image_mem(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT,    MAX_FRAME_STRIDE, YUV_420    ,0);
    if(!p_enc_lyr_ctxt->p_om_temp_img1)
    {
        return NULL;
    }                          
    
    p_enc_lyr_ctxt->p_om_temp_img2 = alloc_image_mem(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT,    MAX_FRAME_STRIDE, YUV_420    ,0);
    if(!p_enc_lyr_ctxt->p_om_temp_img2)
    {
        return NULL;
    }  

    p_enc_lyr_ctxt->p_om_temp_img3 = alloc_image_mem(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT,    MAX_FRAME_STRIDE, YUV_420    ,0);
    if(!p_enc_lyr_ctxt->p_om_temp_img3)
    {
        return NULL;
    }  



    return (void *) p_enc_lyr_ctxt;

} /* End of allocate_encoder_layer() function */

int32_t initialize_mfc_encoder_layer(void *pv_enc_lyr_ctxt,
                               RPUData      *p_rpu_data                 )
{
    
    EncoderLayerContext *p_enc_lyr_ctxt = (EncoderLayerContext *) pv_enc_lyr_ctxt;

    /* Initialises the coefficient tables for Muxing & Demuxing Filters */
    init_muxing_filters(p_enc_lyr_ctxt);
    init_demuxing_filter(&p_enc_lyr_ctxt->demuxing_up_filter);
   
    p_enc_lyr_ctxt->p_rpu_data = p_rpu_data;
/*
    for(u16_loop=0 ; u16_loop < 256 ; u16_loop++)
    {
        p_enc_lyr_ctxt->u8_clip_table[u16_loop] = (uint8_t)u16_loop;
    }
    for(u16_loop=256 ; u16_loop < 32768 ; u16_loop++)
    {
         p_enc_lyr_ctxt->u8_clip_table[u16_loop] = 255;
    }
    for(u16_loop=32768 ; u16_loop < 65535 ; u16_loop++)
    {
         p_enc_lyr_ctxt->u8_clip_table[u16_loop] = 0;
    }
     p_enc_lyr_ctxt->u8_clip_table[65535]=0;
*/

    return SUCCESS;

} /* End of allocate_encoder_layer() function */


int32_t free_mfc_encoder_layer(
                void *pv_enc_lyr_ctxt
                )
{
    int32_t i32_status = SUCCESS;

    EncoderLayerContext *p_enc_lyr_ctxt = (EncoderLayerContext *) pv_enc_lyr_ctxt;


    /* Close the RPU kernel */
    i32_status = rpu_close(p_enc_lyr_ctxt->pv_rpu_kernel);

    if(i32_status) 
    {   /* FAILURE */
        return i32_status;
    } 


    free_image_mem(p_enc_lyr_ctxt->p_om_temp_img1);
    free_image_mem(p_enc_lyr_ctxt->p_om_temp_img2);
    free_image_mem(p_enc_lyr_ctxt->p_om_temp_img3);

    /* Encoder layer context */
    free_memory((void *) p_enc_lyr_ctxt);

    return i32_status;

} /* End of free_mfc_encoder_layer() function */



 int32_t process3DMux(
              ImageData        *p_muxed_image,
     const    ImageData        *p_view0_image,
     const    ImageData        *p_view1_image,
     const    ImageData        *p_recon_bl_image,
     const    MuxingParams     *p_muxing_params,                 
     const    void             *pv_enc_layer_context,
              uint8_t           u8_layer_id,
              uint8_t           u8_recon_bl_for_el_residue,
              uint8_t           u8_recon_bl_for_el_carrier)
 {
    int32_t i32_success=SUCCESS;
    const    ImageData         *bl_image_used_for_el_residue;
    const    ImageData         *bl_image_used_for_el_carrier;


    EncoderLayerContext *p_enc_lyr_ctxt      = (EncoderLayerContext *)pv_enc_layer_context;
    RPUData             *p_rpu_data          = p_enc_lyr_ctxt->p_rpu_data;

      

    /* p_om_temp_img has generic Image information ,
    copy the input Image Dimensions and Format to temp Images */    

    copy_image_dimensions(p_enc_lyr_ctxt->p_om_temp_img1,p_view0_image);
    copy_image_dimensions(p_enc_lyr_ctxt->p_om_temp_img2,p_view0_image);
    copy_image_dimensions(p_enc_lyr_ctxt->p_om_temp_img3,p_view0_image);



    bl_image_used_for_el_residue = u8_recon_bl_for_el_residue==1 ? p_recon_bl_image : p_muxed_image;
    bl_image_used_for_el_carrier = u8_recon_bl_for_el_carrier==1 ? p_recon_bl_image : p_muxed_image;

    if(SBS == p_rpu_data->e_rpu_process_format)
    {
        /* Creating Base Layer */
        i32_success = mux_side_by_side(p_muxed_image,
                            p_view0_image,
                            p_view1_image,
                            &p_enc_lyr_ctxt->a_muxing_down_filter[p_muxing_params->e_mux_baselayer_filter],
                            p_rpu_data->u8_view0_grid_position_x ,
                            p_rpu_data->u8_view1_grid_position_x ,
                            p_rpu_data->u8_packed_UV);

        if(1==u8_layer_id)
        {

            /* Upsample Base Layer to Create Reconstructed Image */
            i32_success = upSampleSBSImage( p_enc_lyr_ctxt->p_om_temp_img1,/* Left Reconstructed */
                                            p_enc_lyr_ctxt->p_om_temp_img2,/* Right Reconstructed */
                                            bl_image_used_for_el_residue,/* Input Base Layer */
                                            &p_enc_lyr_ctxt->demuxing_up_filter, /* Demuxing Filter ID */
                                            p_rpu_data->u8_view0_grid_position_x ,
                                            p_rpu_data->u8_view1_grid_position_x ,
                                            p_rpu_data->u8_packed_UV);

            /* Get the Residue = Input Image - Reconstructed Image */
            i32_success = add_diff_image_with_offset(p_enc_lyr_ctxt->p_om_temp_img1,
                                                    p_view0_image,
                                                    p_enc_lyr_ctxt->p_om_temp_img1,                                                      
                                                    RPU_OFFSET_VALUE,0);
            i32_success = add_diff_image_with_offset(p_enc_lyr_ctxt->p_om_temp_img2,
                                                    p_view1_image,
                                                    p_enc_lyr_ctxt->p_om_temp_img2,                                                      
                                                    RPU_OFFSET_VALUE,0);

            /* Creating Orthogonal Muxed Enhancement(Just Residues) Layer */
            i32_success = mux_over_under(p_enc_lyr_ctxt->p_om_temp_img3,        /* Output Residue in TAB Mode */
                                p_enc_lyr_ctxt->p_om_temp_img1,        /* Input Left Residue */
                                p_enc_lyr_ctxt->p_om_temp_img2,        /* Input Right Residue */
                                &p_enc_lyr_ctxt->a_muxing_down_filter[p_muxing_params->e_mux_enhlayer_filter],    /* Enhancemenet Layer Muxing Filter */                                
                                0,
                                0,
                                p_rpu_data->u8_packed_UV);

            /* Do Fixed RPU for creating Carrier Image */
            rpu_process(p_enc_lyr_ctxt->pv_rpu_kernel,
                    bl_image_used_for_el_carrier,
                    p_enc_lyr_ctxt->p_om_temp_img1,                         
                    p_rpu_data);


                
            /* Add Carrier to Residue Image */
            i32_success = add_diff_image_with_offset(p_muxed_image,                    /* Carrier + Residue */
                                                    p_enc_lyr_ctxt->p_om_temp_img1,    /* Carrier Image */
                                                    p_enc_lyr_ctxt->p_om_temp_img3,    /* Residue Image */
                                                    RPU_OFFSET_VALUE,1);

            //copy_complete_ImageData(p_muxed_image,p_enc_lyr_ctxt->p_om_temp_img1);

                
            
        }
    }/* SBS Process Format  */
    else if(OU == p_rpu_data->e_rpu_process_format)
    {
        /* Creating Base Layer */
        i32_success = mux_over_under(p_muxed_image,
                            p_view0_image,
                            p_view1_image,
                            &p_enc_lyr_ctxt->a_muxing_down_filter[p_muxing_params->e_mux_baselayer_filter],                                
                            p_rpu_data->u8_view0_grid_position_y ,
                            p_rpu_data->u8_view1_grid_position_y ,
                            p_rpu_data->u8_packed_UV);
        if(1==u8_layer_id)
        {
            /* Upsample Base Layer to Create Reconstructed Image */
            i32_success = upSampleOUImage( p_enc_lyr_ctxt->p_om_temp_img1,/* Left Reconstructed */
                                            p_enc_lyr_ctxt->p_om_temp_img2,/* Right Reconstructed */
                                            bl_image_used_for_el_residue,                    /* Input BL in TAB*/
                                            &p_enc_lyr_ctxt->demuxing_up_filter,                                                
                                            p_rpu_data->u8_view0_grid_position_y ,
                                            p_rpu_data->u8_view1_grid_position_y ,
                                            p_rpu_data->u8_packed_UV);

            /* Get the Residue = Input Image - Reconstructed Image */
            i32_success = add_diff_image_with_offset(p_enc_lyr_ctxt->p_om_temp_img1,
                                                    p_view0_image,
                                                    p_enc_lyr_ctxt->p_om_temp_img1,                                                      
                                                    RPU_OFFSET_VALUE,0);
            i32_success = add_diff_image_with_offset(p_enc_lyr_ctxt->p_om_temp_img2,
                                                    p_view1_image,
                                                    p_enc_lyr_ctxt->p_om_temp_img2,                                                      
                                                    RPU_OFFSET_VALUE,0);

            /* Creating Orthogonal Muxed Enhancement(Just Residues) Layer */
            i32_success = mux_side_by_side(p_enc_lyr_ctxt->p_om_temp_img3,        /* Output Residue in SBS Mode */
                                p_enc_lyr_ctxt->p_om_temp_img1,            /* Input Left Residue */
                                p_enc_lyr_ctxt->p_om_temp_img2,            /* Input Right Residue */
                                &p_enc_lyr_ctxt->a_muxing_down_filter[p_muxing_params->e_mux_enhlayer_filter],                                    
                                0,
                                0,
                                p_rpu_data->u8_packed_UV);

            /* Do Fixed RPU for creating Carrier signal */
            rpu_process(p_enc_lyr_ctxt->pv_rpu_kernel,
                    bl_image_used_for_el_carrier,
                    p_enc_lyr_ctxt->p_om_temp_img1,                         
                    p_rpu_data);
                                                

                
            /* Add carrier to residue image */
            i32_success = add_diff_image_with_offset(p_muxed_image,                    /* Carrier + Residue */
                                                    p_enc_lyr_ctxt->p_om_temp_img1,    /* Carrier Image  */
                                                    p_enc_lyr_ctxt->p_om_temp_img3,    /* Residue Image  */
                                                    RPU_OFFSET_VALUE,1);
        }
    }/* OU Process Format  */
    else
    {
        i32_success=FAILURE;
    }/* Unknown  Process Format  */



    return i32_success;

} /* End of process3DMux */





/**
 * int32_t process_enc_rpu(
 *                               void                *pv_enc_lyr_ctxt,
 *                         const ImageData            *p_recon_bl,
 *                               ImageData            *p_pred_el     
 *
 * \brief API function to generate prediction of enhancement layer from decoded base layer.
 *
 * \details The decoded base layer is filtered by the RPU to obtain enhancement layer prediction. The RPU divides the
 * base layer into partitions and applies filters indicated in p_rpu_syntax.
 * 
 * \param[in]    pv_enc_lyr_ctxt      void pointer to decoder layer context.
 * \param[in]    p_recon_bl           Decoded base layer.
 * \param[out]   p_pred_el            Predicted enhancement layer output by the RPU.
 * \return Success (0) or Failure (Error code)
 */
int32_t process_enc_rpu(
              void              *pv_enc_lyr_ctxt,
        const ImageData         *p_recon_bl,
              ImageData         *p_pred_el                      
                        )
{
    int32_t i32_status = SUCCESS;
    EncoderLayerContext *p_enc_lyr_ctxt        = (EncoderLayerContext *)pv_enc_lyr_ctxt;
  
    

   /* Apply RPU to the Base Layer */
    i32_status = rpu_process(p_enc_lyr_ctxt->pv_rpu_kernel,
                p_recon_bl,
                p_pred_el,
                p_enc_lyr_ctxt->p_rpu_data);


    /* print the out of the rpu_data info */
    print_rpu_header(p_enc_lyr_ctxt->p_rpu_data);
  

return i32_status;

} /* End of process_enc_rpu() */


