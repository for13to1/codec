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
* \file  c3d_decoder_layer_pvt.c
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


/* User defined header files */
#include "c3d_decoder_layer_api.h"

int32_t rpu_header_info_conformance_test(
              RPUData *p_rpu_data
              )
{

    if((p_rpu_data->e_rpu_process_format != SBS) && 
       (p_rpu_data->e_rpu_process_format != OU))
    {
        return ERROR_UNKNOWN_RPU_PROCESS_FORMAT;
    } /* RPU process format */


    if(((p_rpu_data->u8_view0_grid_position_x != 0) && 
        (p_rpu_data->u8_view0_grid_position_x != 1)) ||
       ((p_rpu_data->u8_view0_grid_position_y != 0) && 
        (p_rpu_data->u8_view0_grid_position_y != 1)) ||
       ((p_rpu_data->u8_view1_grid_position_x != 0) && 
        (p_rpu_data->u8_view1_grid_position_x != 1)) ||
       ((p_rpu_data->u8_view1_grid_position_y != 0) && 
        (p_rpu_data->u8_view1_grid_position_y != 1)))
    {
        return ERROR_UNKNOWN_VIEW_GRID_POSITION;
    } /* View grid position */






    return SUCCESS;

} /* End of rpu_header_info_conformance_test() function */


int32_t image_data_conformance_test(
        const ImageData          *p_img,
              uint8_t            u8_packed_UV,
              RPU_PROCESS_FORMAT e_rpu_process_format
                                   )
{
    

    if(p_img->e_yuv_chroma_format != YUV_420)
    {
        return ERROR_UNKNOWN_CHROMA_FORMAT;
    } /* YUV chroma format */

    /* Y */
    if(!p_img->pa_buf[Y])
    {
        return ERROR_IMAGE_BUF_Y_PTR_NULL;
    } /* Buffer pointer Y */

    if(p_img->au16_frame_width[Y] > MAX_FRAME_WIDTH)
    {
        return ERROR_FRAME_WIDTH_Y_OUT_OF_RANGE;
    } /* Frame width Y range check */



    if((p_img->au16_frame_height[Y] > MAX_FRAME_HEIGHT))
    {
        return ERROR_FRAME_HEIGHT_Y_OUT_OF_RANGE;
    } /* Frame height Y range check */


    if(p_img->au16_buffer_stride[Y] > MAX_FRAME_STRIDE)
    {
        return ERROR_FRAME_STRIDE_Y_OUT_OF_RANGE;
    } /* Frame stride Y range check */

    if(p_img->au16_buffer_stride[Y] < p_img->au16_frame_width[Y])
    {
        return ERROR_FRAME_STRIDE_LESS_THAN_FRAME_WIDTH;
    } /* Frame stride < Frame width check */

    if(SBS == e_rpu_process_format)
    {
        if(p_img->au16_view_delimiter_sbs[Y] > MAX_FRAME_WIDTH)
        {
            return ERROR_ILLEGAL_VIEW_DELIMITER;
        } /* View delimiter */
    } /* SBS */
    else /* OU */
    {
        if(p_img->au16_view_delimiter_ou[Y] > MAX_FRAME_HEIGHT)
        {
            return ERROR_ILLEGAL_VIEW_DELIMITER;
        } /* View delimiter */
    } /* OU */

    /* U */
    if(!p_img->pa_buf[U])
    {
        return ERROR_IMAGE_BUF_U_PTR_NULL;
    } /* Buffer pointer U */

    /* V buffer comes into picture only if U and V are not packed */
    if(!u8_packed_UV)
    {
        if(!p_img->pa_buf[V])
        {
            return ERROR_IMAGE_BUF_V_PTR_NULL;
        } /* Buffer pointer V */
    } /* if(!u8_packed_UV) */



    return SUCCESS;

} /* End of image_data_conformance_test() function */


