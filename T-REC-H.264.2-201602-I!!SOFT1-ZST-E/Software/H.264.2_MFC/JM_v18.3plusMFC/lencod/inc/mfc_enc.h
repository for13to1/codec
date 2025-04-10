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
* \file  mfc_enc.h
*
* \brief 
*        MFC SDK  encoder layer interface to JM encoder
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/



#ifndef _MFC_ENC_INC_
#define _MFC_ENC_INC_


#include <stdio.h>
#include "mfc_common.h"



typedef struct MFC_MUXING_PARAMS
{                                       //!< MUX PARAMS
  int Mux3DBaseFilter;                  //!< 3D Mux filter for Base Layer
  int Mux3DEnhFilter;                   //!< 3D Mux filter for Enhancement Layer  
 } MFC_MUXING_PARAMS;



/*!
 ************************************************************************
 * void* allocate_and_set_muxing_params(MFC_MUXING_PARAMS *MFC_MUXING_PARAMS)
 * \brief 
 *    Alloctae meory for MFC SDK Muxing paramters and sets them based on JM Input
 * \param[in]  
 *       mfc_muxing_params - JM Inputs correspoind to Muxing
 * \return 
 *     void pointer to the allocated muxing parameters
 ************************************************************************
 */
void* allocate_and_set_muxing_params(MFC_MUXING_PARAMS *mfc_muxing_params);

/*!
 ************************************************************************
 * void*   allocate_mfc_encoder_wrapper()
 * \brief 
 *    Alloctaes memory for MFC Encoder Layer Context 
 *
 * \param[out]
 *       void pointer to the allocated structure
 ************************************************************************
 */
void* allocate_mfc_encoder_wrapper();

/*!
 ************************************************************************
 * int initialize_mfc_encoder_wrapper(void *pv_enc_lyr_ctxt,void  *init_params)
                                          
 * \brief 
 *    Initialize the MFC Encoder Layer Context , i.e setups RPU filters, muxing/demuxing filters
 *
 * \param[in]  
 *       pv_rpu_data - void pointer to Initialized RPU Data
 *\param[out] 
 *       pv_enc_lyr_ctxt - void pointer to Encoder Layer Context
 *\return 
 *       0/1
 ***********************************************************************
 */
int initialize_mfc_encoder_wrapper(void *pv_enc_lyr_ctxt,void  *pv_rpu_data);

/*!
 ************************************************************************
 * int   free_mfc_encoder_wrapper(void  *pv_enc_layer_ctxt)
 * \brief 
 *    Free's up the memory for MFC Encoder Layer Context 
 *
 * \param[in]
 *       void pointer to the Encoder Layer Context
 ************************************************************************
 */
int   free_mfc_encoder_wrapper(void  *pv_enc_layer_ctxt);

/*!
 ************************************************************************
 * int process3DMuxWrapper(
 *                        void   *pv_dst,
 *                const   void   *pv_src1,
 *                const   void   *pv_src2,
 *                const   void   *pv_recon_bl,
 *                const   void   *pv_muxingctxt,                
 *                const   void   *pv_enc_layer_context,
 *                        int     i_layerId,
 *                        int     i_use_recon_bl_for_residue,
 *                        int     i_use_recon_bl_for_carrier)
 *                                          
 * \brief 
 *    Muxing function to Create Base Layer and Enhancement Layer Views
 *
 * \param[out]  
 *       pv_dst - void pointer to muxed output image
 *\param[in] 
 *       pv_src1        - void pointer to input view 0
 *       pv_src1        - void pointer to input view 1
 *       pv_recon_bl    - void pointer to input recon base layer( Used in Enh Layer Creation if needed)
 *       pv_muxingctxt  - void pointer to muxing parameter structure
 *       pv_enc_layer_context  - void pointer to Encoder Layer Context
 *       i_layerId      - 0 - Baselayer / 1- Enhancement Layer
 *       i_use_recon_bl_for_residue   - Use Recon BL for EL Residue.(For MFC always set to 0)
 *       i_use_recon_bl_for_carrier   - Use Recon BL for EL Carrier.(For MFC always set to 0)
 *\return 
 *       0/1
 ***********************************************************************
 */
int process3DMuxWrapper(
                        void    *pv_dst,
                const    void   *pv_src1,
                const    void   *pv_src2,
                const    void   *pv_recon_bl,
                const    void   *pv_muxingctxt,                
                const    void   *pv_enc_layer_context,
                        int     i_layerId,
                        int     i_use_recon_bl_for_residue,
                        int     i_use_recon_bl_for_carrier);



/*!
 ************************************************************************
 *  int processEncRpuWrapper(
 *                        void   *pv_enc_layer_context,
 *                const   void   *pv_recon_bl,                
 *                        void   *pv_pred_el                        
 *                )
 *                                          
 * \brief 
 *    Process RPU Wrapper Fnction
 *
 * \param[out]  
 *       pv_pred_el - void pointer to RPU'ed Image
 *\param[in] 
 *       pv_recon_bl    - void pointer to input reconstruced Base Layer Image
 *       pv_enc_layer_context  - void pointer to Encoder Layer Context
 *\return 
 *       0/1
 ***********************************************************************
 */
int processEncRpuWrapper(
                        void    *pv_enc_layer_context,
                const    void   *pv_recon_bl,                
                        void    *pv_pred_el                        
                );








#endif

