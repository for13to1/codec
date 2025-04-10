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
* \file  c3d_encoder_layer_init.c
*
* \brief 
*        MFC SDK  Encoder layer initializing functions 
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
#include "c3d_mux_filters.h"



/**
 * \fn int32_t init_muxing_filters(
 *                                EncoderLayerContext            *p_enc_layer_ctxt                         
 *                              )
 *
 * \brief Initialises the Muxing Filters
 *
 * \details Transfers the Muxing Filters and Demuxing filter information to the Encoder Layer Context
 *
 * \param[out]    EncoderLayerContext    Pointer to the Encoder layer context.
 
 *
 * \return Success (0) or Failure (1)
 */
int32_t init_muxing_filters(
              EncoderLayerContext            *p_enc_layer_ctxt                        
              )
{
    int32_t i32_status = SUCCESS;

    RPUFilter *pa_muxing_filter = &p_enc_layer_ctxt->a_muxing_down_filter[0];

    int32_t i32_i, i32_j;    

    for(i32_i = 0; i32_i < TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER; i32_i++)
    {
        RPUFilter *p_flt = &pa_muxing_filter[i32_i];

                            
        /* Filter length */
        p_flt->u16_num_taps  = au16_1D_muxing_filter_length[i32_i];  
        p_flt->u16_taps_div2 = SHIFT_RIGHT(p_flt->u16_num_taps, 1);

        /* Filter indices */
        for (i32_j = 0; i32_j < p_flt->u16_num_taps; i32_j++)
        {
            p_flt->ai16_coef1[i32_j] = ai16_1D_muxing_coef[i32_i][i32_j];
        } /* for (i32_j = 0; i32_j < p_flt_1D->u16_num_taps; i32_j++) */
            
        /* Dynamic range of the filter */
        p_flt->u16_normal1 = au16_1D_muxing_filter_dynamic_range[i32_i];

        /* Filter offset */
        p_flt->u16_offset = au16_1D_muxing_filter_offset[i32_i];

        
    } /* for(i32_i = 0; i32_i < TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER; i32_i++) */

    
    


    return i32_status;

} /* End of init_muxing_filters() function */
