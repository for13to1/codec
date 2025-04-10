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
* \file  c3d_decoder_layer_pvt.h
*
* \brief 
*        MFC SDK  Decoder layer internal functions 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_DECODER_LAYER_PVT_H_
#define _C3D_DECODER_LAYER_PVT_H_


/** STRUCTURE DEFINITIONS */

/**
 * Decoder Layer Context
 * This structure contains the high level information needed for RPU processing. 
 */
typedef struct decoder_layer_context
{
    /* Pointer to RPU kernel handle */
    void    *pv_rpu_kernel;

    /* RPUData structure */
    RPUData    *p_rpu_data;

    /* Filter definitions for BL & EL Demxuing */
    RPUFilter  demuxing_up_filter;

    /* Temporary Image allocations used in various internal stages */
    ImageData    *p_om_dec_temp_img1;
    ImageData    *p_om_dec_temp_img2;
    ImageData    *p_om_dec_temp_img3;

} DecoderLayerContext;

/** FUNCTION DECLARATIONS */



/**
 * \fn int32_t rpu_header_info_conformance_test(
 *                                                RPUData *p_rpu_data
 *                                               )
 *
 * \brief Checks if RPU header params conform to specifications.
 *
 * \details Each param in the RPU header info structure is checked to see if it as per the specifications. If not, the
 * corresponding error code is returned.
 * 
 * \param[in]    p_rpu_data    Pointer to RPU header info.
 *
 * \return Success (0) or Error code in case of failure.
 */
int32_t rpu_header_info_conformance_test(
                                         RPUData *p_rpu_data
                                        );

/**
 * \fn int32_t image_data_conformance_test(
 *                                           cosnt ImageData          *p_img,
 *                                                 uint8_t            u8_packed_UV,
 *                                                 RPU_PROCESS_FORMAT e_rpu_process_format
 *                                          )
 *
 * \brief Checks if image data params conform to specifications.
 *
 * \details Each param in the ImageData structure is checked to see if it as per the specifications. If not, the
 * corresponding error code is returned.
 * 
 * \param[in]    p_img                    Pointer to image data.
 * \param[in]    u8_packed_UV            Flag to indicate if chroma is packed into a single buffer or not.
 * \param[in]    e_rpu_process_format    SBS or OU
 *
 * \return Success (0) or Error code in case of failure.
 */
int32_t image_data_conformance_test(
                                    const ImageData          *p_img,
                                          uint8_t            u8_packed_UV,
                                          RPU_PROCESS_FORMAT e_rpu_process_format
                                   );







#endif /* _C3D_DECODER_LAYER_PVT_H_ */
