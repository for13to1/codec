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
* \file  c3d_rpu_kernel.h
*
* \brief 
*        MFC SDK  RPU Kernel structures 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*         - Santosh Chilkunda     (santosh.chilkunda@dolby.com)
*
*************************************************************************************
*/


#ifndef _C3D_RPU_KERNEL_H_
#define _C3D_RPU_KERNEL_H_

/* Standard header files */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "c3d_rpu_kernel_utils.h"
/** CONSTANT MACROS */

/** Status */
#define SUCCESS    0
#define FAILURE    1

/** Maximum image dimensions */
#define MAX_FRAME_WIDTH     1920
#define MAX_FRAME_HEIGHT    1088
#define MAX_FRAME_STRIDE    4096

/** Maximum number of filter taps */
#define MAX_NUM_FILTER_TAPS    26

/* Default Values for RPU Header */
#define DEFAULT_RPU_PROCESS_FORMAT              SBS

#define DEFAULT_RPU_FILTER_VALUE                 1   
#define DEFAULT_GRID_POSITION_VALUE              1
#define DEFAULT_VIEW0_GRID_POS_X                 0
#define DEFAULT_VIEW0_GRID_POS_Y                 0
#define DEFAULT_VIEW1_GRID_POS_X                 1
#define DEFAULT_VIEW1_GRID_POS_Y                 1
#define DEFAULT_PACKED_UV                        0


/* Error codes */


#define ERROR_UNKNOWN_RPU_PROCESS_FORMAT              (FAILURE + 1)
#define ERROR_UNKNOWN_RPU_PROCESS_LEVEL               (ERROR_UNKNOWN_RPU_PROCESS_FORMAT + 1)
#define ERROR_UNKNOWN_VIEW_GRID_POSITION              (ERROR_UNKNOWN_RPU_PROCESS_LEVEL + 1)
#define ERROR_UNKNOWN_INTERLACE_PROCESSING_FLAG       (ERROR_UNKNOWN_VIEW_GRID_POSITION + 1)
#define ERROR_UNKNOWN_PICTURE_TYPE                    (ERROR_UNKNOWN_INTERLACE_PROCESSING_FLAG + 1)
#define ERROR_IMAGE_BUF_Y_PTR_NULL                    (ERROR_UNKNOWN_PICTURE_TYPE + 1)
#define ERROR_FRAME_WIDTH_Y_OUT_OF_RANGE              (ERROR_IMAGE_BUF_Y_PTR_NULL + 1)
#define ERROR_FRAME_HEIGHT_Y_OUT_OF_RANGE             (ERROR_FRAME_WIDTH_Y_OUT_OF_RANGE + 1)
#define ERROR_FRAME_STRIDE_Y_OUT_OF_RANGE             (ERROR_FRAME_HEIGHT_Y_OUT_OF_RANGE + 1)
#define ERROR_FRAME_STRIDE_LESS_THAN_FRAME_WIDTH      (ERROR_FRAME_STRIDE_Y_OUT_OF_RANGE + 1)
#define ERROR_ILLEGAL_VIEW_DELIMITER                  (ERROR_FRAME_STRIDE_LESS_THAN_FRAME_WIDTH + 1)
#define ERROR_IMAGE_BUF_U_PTR_NULL                    (ERROR_ILLEGAL_VIEW_DELIMITER + 1)
#define ERROR_IMAGE_BUF_V_PTR_NULL                    (ERROR_IMAGE_BUF_U_PTR_NULL + 1)
#define ERROR_UNKNOWN_CHROMA_FORMAT                   (ERROR_IMAGE_BUF_V_PTR_NULL + 1)
#define ERROR_IN_RPU_KERNEL                           (ERROR_UNKNOWN_CHROMA_FORMAT + 1)
#define ERROR_IN_DECODER_LAYER_INIT                   (ERROR_IN_RPU_KERNEL + 1)
#define ERROR_READING_FROM_INPUT_FILE                 (ERROR_IN_DECODER_LAYER_INIT + 1)
#define ERROR_WRITING_TO_OUTPUT_FILE                  (ERROR_READING_FROM_INPUT_FILE + 1)


#define UNREFERENCED_PARAMETER(PARAM)    (PARAM)

#define MAX_PIXEL_VALUE            255


#define RPU_OFFSET_VALUE    128

/** ENUMERATIONS */

/** This enumeration lists the supported RPU formats */
typedef enum rpu_process_format
{
    SBS = 0,
    OU  = 1

} RPU_PROCESS_FORMAT;






/** This enumeration lists the supported YUV chroma formats */
typedef enum chroma_format
{
    YUV_400 = 0,
    YUV_420,
    YUV_422,
    YUV_444

} CHROMA_FORMAT;


/** This enumeration lists the different picture types */
typedef enum picture_type
{
    FRAME = 0,
    TOP_FIELD,
    BOTTOM_FIELD
    
} PICTURE_TYPE;

/** Y, U, V components */
typedef enum chroma_components
{
    Y = 0,
    U = 1,
    V = 2,
    MAX_NUM_COMPONENTS = 3
    
} CHROMA_COMPONENTS;



/** TYPEDEFS */

/** Image buffer datatype */
typedef    uint8_t    img_pel_t;

/** STRUCTURES */

/**
 * Image data structure
 * This structure contains the pointers to Y, U and V buffers, the buffer strides and additional info about the frame.
 */
typedef struct mfc_image_data
{
    /* Picture type */
    PICTURE_TYPE    e_picture_type;

    /* YUV chroma format */
    CHROMA_FORMAT    e_yuv_chroma_format;

    /* Pointers to the Y, U, V buffers */
    img_pel_t        *pa_buf[MAX_NUM_COMPONENTS];
    
    /* Y, U, V buffer strides */
    uint16_t        au16_buffer_stride[MAX_NUM_COMPONENTS];
    
    /*
    * Frame width and height of each color component
    * These are multiples are macro-block width and height, respectively.
    */
    uint16_t        au16_frame_width[MAX_NUM_COMPONENTS];
    uint16_t        au16_frame_height[MAX_NUM_COMPONENTS];

    /* View delimiter of SBS and OU of each component */
    uint16_t        au16_view_delimiter_sbs[MAX_NUM_COMPONENTS];
    uint16_t        au16_view_delimiter_ou[MAX_NUM_COMPONENTS];



} ImageData;



#endif /* _C3D_RPU_KERNEL_H_ */
