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
* \file  c3d_encoder_layer_api.h
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

#ifndef _C3D_ENCODER_LAYER_API_
#define _C3D_ENCODER_LAYER_API_

/* User defined header files */
#include "c3d_rpu_kernel_api.h"

/* ENUMERATIONS */


/* This enumeration list the supported downsample muxing filters */
typedef enum downsample_mux_filter
{
    SVC3D =0,
    OM_MUX_FC_P44,
    TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER    
} DOWNSAMPLE_MUX_FILTER;

/* STRUCTURES */

/*
 * Muxing Parameters structure
 * This structure contains parameters that are needed in creating base layer and enhancement layer
 */

typedef struct muxing_params
{
    /*
     * Used for Muxing Base Layer 
     */
    DOWNSAMPLE_MUX_FILTER    e_mux_baselayer_filter;                

    /* 
     * Used for Muxing Enhancement Layer 
     */
    DOWNSAMPLE_MUX_FILTER   e_mux_enhlayer_filter;            

} MuxingParams;


/** API FUNCTION DECLARATIONS */


/*!
 ************************************************************************
 * void*   allocate_mfc_encoder_layer()
 * \brief 
 *    Alloctaes memory for MFC Encoder Layer Context 
 *
 * \param[out]
 *       void pointer to the allocated structure
 ************************************************************************
 */

void* allocate_mfc_encoder_layer();

/*!
 ************************************************************************
 * int initialize_mfc_encoder_layer(void *pv_enc_lyr_ctxt,RPUData      *p_rpu_data)
                                          
 * \brief 
 *    Initialize the MFC Encoder Layer Context , i.e setups RPU filters, muxing/demuxing filters
 *
 * \param[in]  
 *       p_rpu_data - Pointer to Initialized RPU Data
 *\param[out] 
 *       pv_enc_lyr_ctxt - void pointer to Encoder Layer Context
 *\return 
 *       0/1
 ***********************************************************************
 */
int32_t initialize_mfc_encoder_layer(void *pv_enc_lyr_ctxt,
                               RPUData      *p_rpu_data                 );

/*!
 ************************************************************************
 * int   free_mfc_encoder_layer(void  *pv_enc_layer_ctxt)
 * \brief 
 *    Free's up the memory for MFC Encoder Layer Context 
 *
 * \param[in]
 *       void pointer to the Encoder Layer Context
 ************************************************************************
 */
int32_t free_mfc_encoder_layer(
                      void *pv_enc_lyr_ctxt                     
                     );






/** 
 *\fn int32_t process3DMux(
 *                         ImageData        *p_muxed_image,
 *                 const   ImageData        *p_view0_image,
 *                 const   ImageData        *p_view1_image,
 *                 const   ImageData        *p_recon_bl_image,
 *                 const   MuxingParams     *p_muxing_params,                 
 *                 const   void             *pv_enc_layer_context,
 *                         uint8_t          u8_layer_id,
 *                         uint8_t          u8_recon_bl_for_el_residue,
 *                         uint8_t          u8_recon_bl_for_el_carrier)
 *
 * \brief API function to create muxed base layer and enhancement from source view0 and view1 Imagedata.
 *
 * 
 * \param[out]   p_muxed_image            Pointer to output Muxed ImageData.
 * \param[in]    p_view0_image            Pointer to View0 ImageData.
 * \param[in]    p_view1_image            Pointer to View1 ImageData.
 * \param[in]    p_recon_bl_image         Pointer to Recon BL Image.(Used only for EL Generation).
 * \param[out]   p_muxing_params          Pointer to Muxing Parameters
 * \param[in]    pv_enc_layer_context     void Pointer to Encoder Layer Context .
 * \param[in]    u8_layerId                     0=BaseLayer,1=enhancement layer.
 * \param[in]    u8_recon_bl_for_el_residue     0 = Src BL is Used to crate EL residue ,
 *                                              1 = Recon BL is Used to crate EL residue.
 *                                             (Used only for EL Generation)
 * \param[in]    u8_recon_bl_for_el_carrier     0 = Src BL is Used to crate EL carrier 
 *                                              1 = Recon BL is Used to crate EL carrier.
 *                                             (Used only for EL Generation)
 *
 * \return Success (0) or Failure (Error code)
 */
 int32_t process3DMux(
              ImageData        *p_muxed_image,
     const    ImageData        *p_view0_image,
     const    ImageData        *p_view1_image,
     const    ImageData        *p_recon_bl_image,
     const    MuxingParams     *p_muxing_params,                 
     const    void             *pv_enc_layer_context,
              uint8_t           u8_layer_id,
              uint8_t           u8_recon_bl_for_el_residue,
              uint8_t           u8_recon_bl_for_el_carrier);


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
                               void                *pv_enc_lyr_ctxt,
                         const ImageData            *p_recon_bl,
                               ImageData            *p_pred_el                         
                        );





#endif /* _C3D_ENCODER_LAYER_API_ */
