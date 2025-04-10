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
* \file  c3d_encoder_layer_pvt.h
*
* \brief 
*        MFC SDK  Encoder layer internal functions 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_ENCODER_LAYER_PVT_H_
#define _C3D_ENCODER_LAYER_PVT_H_


/** STRUCTURE DEFINITIONS */



/**
 * Encoder Layer Context
 * This structure contains the high level information needed for RPU processing. 
 */
typedef struct encoder_layer_context
{

/* Pointer to RPU kernel handle */
    void    *pv_rpu_kernel;


/* Pointer to RPUData structure */
    RPUData        *p_rpu_data;

/* Pointer to DownSampling MuxFilter  */
    RPUFilter    a_muxing_down_filter[TOTAL_NUM_OF_DOWNSAMPLE_MUX_FILTER];

/* Pointer to Demuxing Upsampling filter used in EL Creation */
    RPUFilter    demuxing_up_filter;

/* Image buffer pointer that is used to store the intermediate Image in OM Muxing. */
    
    ImageData    *p_om_temp_img1;
    ImageData    *p_om_temp_img2;
    ImageData    *p_om_temp_img3;

} EncoderLayerContext;


/**
 * \fn int32_t init_muxing_filters(
 *                                EncoderLayerContext            *p_enc_layer_ctxt                         
 *                              )
 *
 * \brief Initialises the Muxing Filters
 *
 * \details 
 *
 * \param[out]    EncoderLayerContext    Pointer to the Encoder layer context.
 
 *
 * \return Success (0) or Failure (1)
 */
int32_t init_muxing_filters(
              EncoderLayerContext            *p_enc_layer_ctxt                        
              );

/**
 
 *    int32_t    mux_side_by_side(
 *                    ImageData               *p_muxed_image,
 *            const   ImageData               *p_view0_image,
 *            const   ImageData               *p_view1_image,
 *            const   RPUFilter               *p_mux_filter
 *                    uint8_t                  u8_view0_offset,
 *                    uint8_t                  u8_view1_offset,
 *                    uint8_t                  u8_packed_uv,
 
 *                    );
 *                         )
 *
 * \brief API function to generate muxed Side-by-Side layer from view0 and view1 ImageData.
 *
 * \details The mux_side_by_side filters the input view0 and view1 based on the filter specified by 'p_mux_filter'
 *            and create a side-by-side muxed layer

 * \param[out]    p_muxed_image     ImageData pointer to the Output side-by-side layer
 * \param[in]    p_view0_image     Pointer to View0 ImageData.
 * \param[in]    p_view1_image     Pointer to View1 ImageData.
 * \param[in]    u8_view0_offset     View 0 Offset.
 * \param[in]    u8_view1_offset     View 1 Offset.
 * \param[in]    u8_packed_uv     Packed or Unpacked UV.
 * \param[in]    p_mux_filter     Pointer to Muxing Filter Structure.
 *
 * \return Success (0) or Failure (Error code)
 */

int32_t mux_side_by_side(
              ImageData                *p_muxed_image,
    const     ImageData                *p_view0_image,
    const     ImageData                *p_view1_image,
    const     RPUFilter                *p_mux_filter,
              uint8_t                   u8_view0_offset,
              uint8_t                   u8_view1_offset,
              uint8_t                   u8_packed_uv            
            );




/**
 
 *    int32_t    mux_over_under(
 *                    ImageData                *p_muxed_image,
 *            const    ImageData                *p_view0_image,
 *            const    ImageData                *p_view1_image,
 *            const    RPUFilter                *p_mux_filter 
 *                    uint8_t                    u8_view0_offset,
 *                    uint8_t                    u8_view1_offset,
 *                    uint8_t                    u8_packed_uv,
 
 *                    );
 *                         )
 *
 * \brief API function to generate muxed Over&Under layer from view0 and view1 ImageData.
 *
 * \details The mux_over_under filters the input view0 and view1 based on the filter specified by 'p_mux_filter'
 *            and create a Over&Under muxed layer

 * \param[out]    p_muxed_image     ImageData pointer to the Output side-by-side layer
 * \param[in]    p_view0_image     Pointer to View0 ImageData.
 * \param[in]    p_view1_image     Pointer to View1 ImageData.
 * \param[in]    u8_view0_offset     View 0 Offset.
 * \param[in]    u8_view1_offset     View 1 Offset.
 * \param[in]    u8_packed_uv     Packed or Unpacked UV.
 * \param[in]    p_mux_filter     Pointer to Muxing Filter Structure.
 *
 * \return Success (0) or Failure (Error code)
 */

int32_t mux_over_under(
              ImageData                *p_muxed_image,
    const     ImageData                *p_view0_image,
    const     ImageData                *p_view1_image,
    const     RPUFilter                *p_mux_filter,            
              uint8_t                   u8_view0_offset,
              uint8_t                   u8_view1_offset,
              uint8_t                   u8_packed_uv            
              );


#endif /* _C3D_ENCODER_LAYER_PVT_H_ */
