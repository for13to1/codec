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
* \file  c3d_decoder_layer_api.h
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


#ifndef _C3D_DECODER_LAYER_API_
#define _C3D_DECODER_LAYER_API_

/* User defined header files */
#include "c3d_rpu_kernel_api.h"



/** API FUNCTION DECLARATIONS */


/*!
 ************************************************************************
 * void*   allocate_mfc_decoder_layer()
 * \brief 
 *    Alloctaes memory for MFC Decoder Layer Context 
 *
 * \param[out]
 *       void pointer to the allocated structure
 ************************************************************************
 */
void* allocate_mfc_decoder_layer( );

/*!
 ************************************************************************
 * int initialize_mfc_decoder_layer(void *pv_dec_lyr_ctxt,RPUData      *p_rpu_data)
                                          
 * \brief 
 *    Initialize the MFC Decoder Layer Context , i.e setups RPU filters, demuxing filters
 *
 * \param[in]  
 *       p_rpu_data - Pointer to Initialized RPU Data
 *\param[out] 
 *       pv_enc_lyr_ctxt - void pointer to Encoder Layer Context
 *\return 
 *       0/1
 ***********************************************************************
 */
int32_t initialize_mfc_decoder_layer(void *pv_dec_lyr_ctxt,
                               RPUData      *p_init_params                 );


/*!
 ************************************************************************
 * int   free_mfc_decoder_layer(void  *pv_dec_layer_ctxt)
 * \brief 
 *    Free's up the memory for MFC Decoder Layer Context 
 *
 * \param[in]
 *       void pointer to the Decoder Layer Context
 ************************************************************************
 */
int32_t free_mfc_decoder_layer (
                            void *pv_dec_lyr_ctxt
                            );

/**
 * \fn int32_t process_dec_rpu(
 *                                     void      *pv_dec_lyr_ctxt,
 *                               const ImageData *p_dec_bl,
 *                                     ImageData *p_pred_el
 *                              )
 *
 * \brief API function to generate prediction of enhancement layer from decoded base layer.
 *
 * \details The decoded base layer is filtered by the RPU to obtain enhancement layer prediction. The RPU divides the
 * base layer into partitions(if any) and applies filters indicated in p_rpu_syntax.
 * 
 * \param[in]    pv_dec_lyr_ctxt        Pointer to decoder layer context.
 * \param[in]    p_dec_bl            Decoded base layer.
 * \param[in]    p_rpu_syntax        Rpu syntax used to process decoder layer rpu 
 * \param[out]    p_pred_el            Predicted enhancement layer output by the RPU.
 *
 * \return Success (0) or Failure (Error code)
 */
int32_t process_dec_rpu        (
                             void      *pv_dec_lyr_ctxt,
                        const ImageData *p_dec_bl,
                              ImageData *p_pred_el
                             );








/**
 * \fn int32_t process3DDeMux_OM(
 *                    const     void         *pv_dec_lyr_context,
 *                              ImageData    *p_view0_out,
 *                              ImageData    *p_view1_out,
 *                    const     ImageData    *p_recon_bl,
 *                    const     ImageData    *p_recon_el,
 *                    const     ImageData    *p_pred_el
 *                              )
 *
 * \brief API function tp perfom OM demuxing of BL & EL to View0 & View 1.
 *
 * \details The decoded BL and EL in orthogonally muxed format is used to generate full res View 0 and View 1.
 *            BL is upsampled as is done in a FC case , residue is obtained by resiude method /HPF method , upsampled and merged with BL upsampled 
 *            to get full resolution views.
 * 
 * \param[in]     pv_dec_lyr_ctxt      Pointer to decoder layer context.
 * \param[out]    p_view0_out          Pointer to output View 0 imageData.
 * \param[out]    p_view1_out          Pointer to output View 1 imageData.
 * \param[out]    p_recon_bl           Pointer to Input Recon BL imageData.
 * \param[out]    p_recon_el           Pointer to Input Recon EL imageData.
 * \param[out]    p_pred_el            Pointer to Input Pred EL imageData (used in difference method). 
 *
 * \return Success (0) or Failure (Error code)
 */
int32_t     process3DDeMux_OM(
                    const    void        *pv_dec_lyr_context,
                             ImageData    *p_view0_out,
                             ImageData    *p_view1_out,
                    const    ImageData    *p_recon_bl,
                    const    ImageData    *p_recon_el,
                    const    ImageData    *p_pred_el
                          );

/**
 * \fn int32_t     process3DDeMux_FC(
 *                    const   void        *pv_dec_lyr_context,
 *                            ImageData   *p_view0_out,
 *                            ImageData   *p_view1_out,
 *                    const   ImageData   *p_recon_bl,
 *                        )
 *
 * \brief API function tp perfom FC demuxing of BL to View0 & View 1.
 *
 * \details The decoded BL upsampled to generate full res View 0 and View 1.
 * 
 * \param[in]     pv_dec_lyr_ctxt       Pointer to decoder layer context.
 * \param[out]    p_view0_out           Pointer to output View 0 imageData.
 * \param[out]    p_view1_out           Pointer to output View 1 imageData.
 * \param[out]    p_recon_bl            Pointer to Input Recon BL imageData.
 *
 * \return Success (0) or Failure (Error code)
 */
int32_t     process3DDeMux_FC(
                    const    void        *pv_dec_lyr_context,
                            ImageData    *p_view0_out,
                            ImageData    *p_view1_out,
                    const    ImageData    *p_recon_bl    
                        );

#endif /* _C3D_DECODER_LAYER_API_ */
