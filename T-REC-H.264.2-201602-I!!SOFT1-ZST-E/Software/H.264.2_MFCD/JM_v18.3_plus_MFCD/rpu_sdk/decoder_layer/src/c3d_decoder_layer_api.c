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
* \file  c3d_decoder_layer_api.c
*
* \brief 
*        MFC SDK  Decoder layer API functions 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


/* User defined header files */
#include "c3d_decoder_layer_api.h"
#include "c3d_decoder_layer_pvt.h"
#include "c3d_utilities_api.h"


void* allocate_mfc_decoder_layer()
{
  DecoderLayerContext *p_dec_lyr_ctxt = NULL;    

  /* Allocate memory for decoder layer context */
  p_dec_lyr_ctxt = (DecoderLayerContext *) allocate_memory(sizeof(DecoderLayerContext));
  
  /* If memory allocation fails, then return NULL */
  if(!p_dec_lyr_ctxt)
  {
    return NULL;
  } /* if(!p_dec_lyr_ctxt) */

  /* Initialise the RPU Kernel */
  p_dec_lyr_ctxt->pv_rpu_kernel = rpu_init();
  
  /* If RPU kernel initialization fails, then return NULL */
  if(!p_dec_lyr_ctxt->pv_rpu_kernel)
  {
    return NULL;
  } /* if(!p_dec_lyr_ctxt->pv_rpu_kernel) */

  /* Allocate temporary images used in various stages */

  p_dec_lyr_ctxt->p_om_dec_temp_img1 = alloc_image_mem(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT,    MAX_FRAME_STRIDE, YUV_420    ,0);
  if(!p_dec_lyr_ctxt->p_om_dec_temp_img1)
  {
    return NULL;
  }
                
  p_dec_lyr_ctxt->p_om_dec_temp_img2 = alloc_image_mem(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT,    MAX_FRAME_STRIDE, YUV_420    ,0);
  if(!p_dec_lyr_ctxt->p_om_dec_temp_img2)
  {
    return NULL;
  }
  
  
  p_dec_lyr_ctxt->p_om_dec_temp_img3 = alloc_image_mem(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT,    MAX_FRAME_STRIDE, YUV_420    ,0);
  if(!p_dec_lyr_ctxt->p_om_dec_temp_img3)
  {
    return NULL;
  }

  /* Initialize Demuxing Filters */
  init_demuxing_filter(&p_dec_lyr_ctxt->demuxing_up_filter);


  return((void *)p_dec_lyr_ctxt);
} /* End of allocate_mfc_decoder_layer()*/

int32_t initialize_mfc_decoder_layer(void *pv_dec_lyr_ctxt,
                               RPUData      *p_init_params                 )
{
    
    DecoderLayerContext *p_dec_lyr_ctxt = (DecoderLayerContext *) pv_dec_lyr_ctxt;
    

    /* Initialises the coefficient tables for Muxing & Demuxing Filters */    
    init_demuxing_filter(&p_dec_lyr_ctxt->demuxing_up_filter);
    
    p_dec_lyr_ctxt->p_rpu_data = p_init_params;
/*
    for(u16_loop=0 ; u16_loop < 256 ; u16_loop++)
    {
        p_dec_lyr_ctxt->u8_clip_table[u16_loop] = (uint8_t)u16_loop;
    }
    for(u16_loop=256 ; u16_loop < 32768 ; u16_loop++)
    {
         p_dec_lyr_ctxt->u8_clip_table[u16_loop] = 255;
    }
    for(u16_loop=32768 ; u16_loop < 65535 ; u16_loop++)
    {
         p_dec_lyr_ctxt->u8_clip_table[u16_loop] = 0;
    }
     p_dec_lyr_ctxt->u8_clip_table[65535]=0;
*/

    return SUCCESS;

} /* End of initialize_mfc_decoder_layer() function */


int32_t free_mfc_decoder_layer (
              void *pv_dec_lyr_ctxt
              )
{
  int32_t i32_status = SUCCESS;

  DecoderLayerContext *p_dec_lyr_ctxt = (DecoderLayerContext *) pv_dec_lyr_ctxt;

   /* Close the RPU kernel */
  i32_status = rpu_close(p_dec_lyr_ctxt->pv_rpu_kernel);

  if(i32_status) /* FAILURE */
  {
    return i32_status;
  } /* FAILURE */

  free_image_mem(p_dec_lyr_ctxt->p_om_dec_temp_img1);
  free_image_mem(p_dec_lyr_ctxt->p_om_dec_temp_img2);
  free_image_mem(p_dec_lyr_ctxt->p_om_dec_temp_img3);
  

  /* Decoder layer context */
  free_memory((void *) p_dec_lyr_ctxt);

  return i32_status;

} /* End of free_mfc_decoder_layer() function */



int32_t process_dec_rpu(
              void          *pv_dec_lyr_ctxt,
        const ImageData     *p_dec_bl,
              ImageData     *p_pred_el
              )
{
  int32_t i32_status = SUCCESS;

  DecoderLayerContext *p_dec_lyr_ctxt = (DecoderLayerContext *) pv_dec_lyr_ctxt;

 
  print_rpu_header(p_dec_lyr_ctxt->p_rpu_data);
  
    

  /* Run conformance test on RPU header info */
  i32_status = rpu_header_info_conformance_test(p_dec_lyr_ctxt->p_rpu_data);
  /* In case of error, return the error code */
  if(i32_status)
  {
    return i32_status;
  } /* if(i32_status) */
    
  /* Run conformance test on decoded base layer */
  i32_status = image_data_conformance_test((const ImageData *) p_dec_bl,
                       p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV,
                       p_dec_lyr_ctxt->p_rpu_data->e_rpu_process_format);
  /* In case of error, return the error code */
  if(i32_status)
  {
    return i32_status;
  } /* if(i32_status) */

  /* Run conformance test on pred enhancement layer */
  i32_status = image_data_conformance_test((const ImageData *) p_pred_el,
                        p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV,
                       p_dec_lyr_ctxt->p_rpu_data->e_rpu_process_format);
  /* In case of error, return the error code */
  if(i32_status)
  {
    return i32_status;
  } /* if(i32_status) */



 
  /* Call RPU kernel to filter the decoded BL to obtain pred EL */
  i32_status = rpu_process(p_dec_lyr_ctxt->pv_rpu_kernel,
               p_dec_bl,
               p_pred_el,
               p_dec_lyr_ctxt->p_rpu_data);


  /* In case of error, return the error code */
  if(i32_status)
  {
    return ERROR_IN_RPU_KERNEL;
  } /* if(i32_status) */



  return i32_status;

} /* End of process_dec_rpu() function */



int32_t     process3DDeMux_OM(
          const    void        *pv_dec_lyr_context,
                   ImageData    *p_view0_out,
                   ImageData    *p_view1_out,
          const    ImageData    *p_recon_bl,
          const    ImageData    *p_recon_el,
          const    ImageData    *p_pred_el
              )
{
  int32_t i32_success = SUCCESS;

  DecoderLayerContext *p_dec_lyr_ctxt = (DecoderLayerContext *)pv_dec_lyr_context;
  
  
  /* Reset the temporary image dimensions to match input image */
  copy_image_dimensions(p_dec_lyr_ctxt->p_om_dec_temp_img1,p_recon_bl);
  copy_image_dimensions(p_dec_lyr_ctxt->p_om_dec_temp_img2,p_recon_bl);
  copy_image_dimensions(p_dec_lyr_ctxt->p_om_dec_temp_img3,p_recon_bl);
  

  /* Get the residue by difference method Residue = Recon_EL - Predicted_EL */
  add_diff_image_with_offset(p_dec_lyr_ctxt->p_om_dec_temp_img3,
              p_recon_el,
              p_pred_el,                            
              RPU_OFFSET_VALUE,0);

  
  if(SBS == p_dec_lyr_ctxt->p_rpu_data->e_rpu_process_format)
  {

      /* Upsample Base Layer */
      i32_success = upSampleSBSImage( p_dec_lyr_ctxt->p_om_dec_temp_img1,            /* Output - FC View 0 */
                  p_dec_lyr_ctxt->p_om_dec_temp_img2,                    /* Output - FC View 1 */
                  p_recon_bl,                                            /* Input  - SBS Base Layer*/
                  &p_dec_lyr_ctxt->demuxing_up_filter,    /* Base Layer Demuxing Filter */
                  p_dec_lyr_ctxt->p_rpu_data->u8_view0_grid_position_x,                    /* Base Layer View 0 Grid Position*/
                  p_dec_lyr_ctxt->p_rpu_data->u8_view1_grid_position_x,                    /* Base Layer View 1 Grid Position*/
                  p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV);                                    
      
      /* Upsample Residue Enhancement Layer */
      i32_success = upSampleOUImage( p_view0_out,                                    /* Output - Residue View 0 */
                  p_view1_out,                                        /* Output - Residue View 0 */
                  p_dec_lyr_ctxt->p_om_dec_temp_img3,                    /* Input - Residue in TAB Mode*/ 
                  &p_dec_lyr_ctxt->demuxing_up_filter,    /* Enh Layer Demuxing Filter */
                  0,                                                                                        /* Enh Layer Grid Positions are always 0 */
                  0,
                  p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV);


  }
  else if(OU == p_dec_lyr_ctxt->p_rpu_data->e_rpu_process_format)
  {    
      /* Upsample Base Layer */
      i32_success = upSampleOUImage( 
                p_dec_lyr_ctxt->p_om_dec_temp_img1,                                                         /* Output - FC View 0 */
                p_dec_lyr_ctxt->p_om_dec_temp_img2,                                                         /* Output - FC View 1 */
                p_recon_bl,                                                                                 /* Input  - TAB Base Layer*/ 
                &p_dec_lyr_ctxt->demuxing_up_filter,       /* Base Layer Demuxing Filter */
                p_dec_lyr_ctxt->p_rpu_data->u8_view0_grid_position_y ,                     /* Base Layer View 0 Grid Position*/
                p_dec_lyr_ctxt->p_rpu_data->u8_view1_grid_position_y ,                     /* Base Layer View 1 Grid Position*/
                p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV);
      
      /* Upsample Residue Enhancement Layer */
      i32_success = upSampleSBSImage( 
                p_view0_out,                                                                                /* Output - Residue View 0 */
                p_view1_out,                                                                                /* Output - Residue View 1 */
                p_dec_lyr_ctxt->p_om_dec_temp_img3,                                                         /* Input - Residue in SBS Mode*/  
                &p_dec_lyr_ctxt->demuxing_up_filter,        /* Enh Layer Demuxing Filter */
                0,                                                                                          /* Enh Layer Grid Positions are always 0 */
                0,
                p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV);
  }
  else
  {
    return ERROR_UNKNOWN_RPU_PROCESS_FORMAT;
  }

  /* Add Residue with FC to construct Full Res View 0 */
  add_diff_image_with_offset(p_view0_out,
                p_view0_out,
                p_dec_lyr_ctxt->p_om_dec_temp_img1,                            
                RPU_OFFSET_VALUE,1);

  /* Add Residue with FC to construct Full Res View 1 */
  add_diff_image_with_offset(p_view1_out,
                p_view1_out,
                p_dec_lyr_ctxt->p_om_dec_temp_img2,                            
                RPU_OFFSET_VALUE,1);
  
  /* dbg    copy_complete_ImageData(p_view0_out,p_recon_el);*/


  return i32_success;
}/* End of process3DDeMux_OM() */


int32_t     process3DDeMux_FC(
          const    void         *pv_dec_lyr_context,
                   ImageData    *p_view0_out,
                   ImageData    *p_view1_out,
          const    ImageData    *p_recon_bl
            )
{
  int32_t i32_success = SUCCESS;

  DecoderLayerContext *p_dec_lyr_ctxt = (DecoderLayerContext *)pv_dec_lyr_context;

  

  if(SBS == p_dec_lyr_ctxt->p_rpu_data->e_rpu_process_format)
  {
      /* Upsample Base Layer */
      i32_success = upSampleSBSImage(
                  p_view0_out,                                                                            /* Output - FC View 0 */
                  p_view1_out,                                                                            /* Output - FC View 1 */
                  p_recon_bl,                                                                             /* Input  - SBS Base Layer*/
                  &p_dec_lyr_ctxt->demuxing_up_filter,   /* Base Layer Demuxing Filter */
                  p_dec_lyr_ctxt->p_rpu_data->u8_view0_grid_position_x ,                 /* Base Layer View 0 Grid Position*/
                  p_dec_lyr_ctxt->p_rpu_data->u8_view1_grid_position_x ,                 /* Base Layer View 1 Grid Position*/
                  p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV);
  }
  else if(OU == p_dec_lyr_ctxt->p_rpu_data->e_rpu_process_format)
  {
      /* Upsample Base Layer */
      i32_success = upSampleOUImage(
                p_view0_out,                                                                              /* Output - FC View 0 */
                p_view1_out,                                                                              /* Output - FC View 1 */
                p_recon_bl,                                                                               /* Input  - TAB Base Layer*/
                &p_dec_lyr_ctxt->demuxing_up_filter,     /* Base Layer Demuxing Filter */
                p_dec_lyr_ctxt->p_rpu_data->u8_view0_grid_position_y ,                   /* Base Layer View 0 Grid Position*/
                p_dec_lyr_ctxt->p_rpu_data->u8_view1_grid_position_y ,                   /* Base Layer View 1 Grid Position*/
                p_dec_lyr_ctxt->p_rpu_data->u8_packed_UV);

  }
  else
  {
    return ERROR_UNKNOWN_RPU_PROCESS_FORMAT;
  }
  

    return i32_success;
}/* End of process3DDeMux_FC() */






