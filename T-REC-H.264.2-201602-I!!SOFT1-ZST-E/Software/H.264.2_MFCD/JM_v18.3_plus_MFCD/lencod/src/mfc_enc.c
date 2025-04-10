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
* \file  mfc_enc.c
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


#include "c3d_encoder_layer_api.h"
#include "mfc_enc.h"




void* allocate_and_set_muxing_params(MFC_MUXING_PARAMS *mfc_muxing_params){

    MuxingParams *p_muxing_params = NULL;
    

    p_muxing_params = (MuxingParams *) allocate_memory(sizeof(MuxingParams));
    if(!p_muxing_params)
    {
        printf("\nError allocating memory for muxing params structure");
        return NULL;
    } /* if(!p_muxing_params) */

    if(mfc_muxing_params->Mux3DBaseFilter < TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER)
    {
        p_muxing_params->e_mux_baselayer_filter = (DOWNSAMPLE_MUX_FILTER) mfc_muxing_params->Mux3DBaseFilter;
    }
    else
    {
        printf("\nMFC HIGH PROFILE : Unsupported Value  for 'Mux3DBaseFilter' : %d\n",mfc_muxing_params->Mux3DBaseFilter);
        printf("\nMFC HIGH PROFILE : Supported   Values for 'Mux3DBaseFilter' : 0 - %d \n",TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER-1);
        return NULL;
    }
    
    if(mfc_muxing_params->Mux3DEnhFilter < TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER)
    {
        p_muxing_params->e_mux_enhlayer_filter = (DOWNSAMPLE_MUX_FILTER) mfc_muxing_params->Mux3DEnhFilter;
    }
    else
    {
        printf("\nMFC HIGH PROFILE : Unsupported Value  for 'Mux3DBaseFilter': %d\n",mfc_muxing_params->Mux3DEnhFilter);
        printf("\nMFC HIGH PROFILE : Supported   Values for 'Mux3DEnhFilter' : 0 - %d \n",TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER-1);
        return NULL;
    }
    

    return (void *) p_muxing_params;

}


void* allocate_mfc_encoder_wrapper(){            

    return(allocate_mfc_encoder_layer());

}

int initialize_mfc_encoder_wrapper(void *pv_enc_lyr_ctxt,void  *pv_rpu_data){            

    return(initialize_mfc_encoder_layer(pv_enc_lyr_ctxt,(RPUData *)pv_rpu_data));

}

int free_mfc_encoder_wrapper(void  *pv_enc_layer_ctxt){        
    
    free_mfc_encoder_layer(pv_enc_layer_ctxt);
  

    return 0;

}

int process3DMuxWrapper(
                        void    *pv_dst,
                const   void    *pv_src1,
                const   void    *pv_src2,
                const   void    *pv_recon_bl,
                const   void    *pv_muxingctxt,                
                const   void    *pv_enc_layer_context,
                        int     i_layerId,
                        int     i_use_recon_bl_for_residue,
                        int     i_use_recon_bl_for_carrier)
{
    int status = 0;

    status = process3DMux((ImageData *)pv_dst,
            (ImageData *)pv_src1,
            (ImageData *)pv_src2,
            (ImageData *)pv_recon_bl,
            (MuxingParams *)pv_muxingctxt,                        
            pv_enc_layer_context,
            i_layerId,
            i_use_recon_bl_for_residue,
            i_use_recon_bl_for_carrier);

         
    return status;

}



int processEncRpuWrapper(
                        void    *pv_enc_layer_context,
                const   void    *pv_recon_bl,                
                        void    *pv_pred_el                        
                )
{
    int status = 0;


    status = process_enc_rpu(pv_enc_layer_context,
                        (ImageData *)pv_recon_bl,
                        (ImageData *)pv_pred_el
                        );
                        
     
    return status;

}









