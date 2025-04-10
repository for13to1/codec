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
* \file  mfc_dec.h
*
* \brief 
*        MFC SDK  decoder layer interface to JM decoder
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hgana@dolby.com)
*
*
*************************************************************************************
*/



#ifndef _MFC_DEC_INC_
#define _MFC_DEC_INC_


#include <stdio.h>
#include "mfc_common.h"

/*!
 ************************************************************************
 * void*   allocate_mfc_decoder_wrapper()
 * \brief 
 *    Alloctaes memory for MFC Decoder Layer Context 
 *
 * \param[out]
 *       void pointer to the allocated structure
 ************************************************************************
 */
void* allocate_mfc_decoder_wrapper();

/*!
 ************************************************************************
 * int initialize_mfc_decoder_wrapper(void *pv_dec_lyr_ctxt,void  *init_params)
                                          
 * \brief 
 *    Initialize the MFC Decoder Layer Context , i.e setups RPU filters, muxing/demuxing filters
 *
 * \param[in]  
 *       pv_rpu_data - void pointer to Initialized RPU Data
 *\param[out] 
 *       pv_dec_lyr_ctxt - void pointer to Decoder Layer Context
 *\return 
 *       0/1
 ***********************************************************************
 */
int initialize_mfc_decoder_wrapper(void *pv_dec_lyr_ctxt,void  *init_params);



/*!
 ************************************************************************
 *  int processDecRpuWrapper(
 *                        void   *pv_dec_layer_context,
 *                const   void   *pv_recon_bl,                
 *                        void   *pv_pred_el                        
 *                )
 *                                          
 * \brief 
 *    Process Decoder RPU Wrapper Fnction
 *
 * \param[out]  
 *       pv_pred_el - void pointer to RPU'ed Image
 *\param[in] 
 *       pv_recon_bl    - void pointer to input reconstruced Base Layer Image
 *       pv_dec_layer_context  - void pointer to Decoder Layer Context
 *\return 
 *       0/1
 ***********************************************************************
 */
int processDecRpuWrapper(
                        void    *pv_enc_layer_context,
                const   void    *pv_recon_bl,                
                        void    *pv_pred_el                        
                        );


/*!
 ************************************************************************
 * int   process3DDeMuxWrapper(void    *pv_dec_layer_context,
 *                        void    *pv_view0_image,
 *                        void    *pv_view1_image,
 *               const    void    *pv_bl_image,
 *               const    void    *pv_el_image,                        
 *               const    void    *pv_predel_image,                        
 *                          int      demuxMode,
 *                          int     rpu_field_processing
 *                        )
 *                                          
 * \brief 
 *    Muxing function to Create Base Layer and Enhancement Layer Views
 *
 * \param[out]  
 *       pv_view0_image - void pointer to View 0 Output
 *       pv_view1_image - void pointer to View 1 Output
 *\param[in] 
 *       pv_bl_image        - void pointer to Base Layer
 *       pv_el_image        - void pointer to Enhancement Layer
 *       pv_predel_image    - void pointer to Predicted Enhancement layer 
 *       pv_dec_layer_context  - void pointer to Decoder Layer Context
 *       demuxMode      - 0 - FC Reconstruction / 1- OM Reconstruction
 *      rpu_field_processing - Enable Field Processing (Only for MBAFF/PAFF , for field coding the incoming frame is already setup properly)
 *\return 
 *       0/1
 ***********************************************************************
 */
int   process3DDeMuxWrapper(void    *pv_dec_layer_context,
                        void    *pv_view0_image,
                        void    *pv_view1_image,
               const    void    *pv_bl_image,
               const    void    *pv_el_image,                        
               const    void    *pv_predel_image,                        
                        int      demuxMode,
                        int     rpu_field_processing
            
                        );

/*!
 ************************************************************************
 * int   free_mfc_decoder_wrapper(void  *pv_dec_layer_ctxt)
 * \brief 
 *    Free's up the memory for MFC Decoder Layer Context 
 *
 * \param[in]
 *       void pointer to the Decoder Layer Context
 ************************************************************************
 */
int free_mfc_decoder_wrapper(void  *pv_dec_layer_ctxt);    




#endif

