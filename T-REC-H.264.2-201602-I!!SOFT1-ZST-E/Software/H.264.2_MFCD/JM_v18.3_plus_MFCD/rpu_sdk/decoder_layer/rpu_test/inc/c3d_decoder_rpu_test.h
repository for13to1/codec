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


/**
 * \author Dolby Laboratories
 *
 * \file   
 *
 * \brief  
 *
 * \note   Confidential Information - Limited distribution to authorized persons
 *         only. This material is protected under international copyright laws
 *         as an unpublished work. Do not copy.
 *         Copyright (C) 2012 Dolby Laboratories Inc. All rights reserved.
 *
 */

#ifndef _DEC_RPU_TEST_H_
#define _DEC_RPU_TEST_H_

#include "c3d_decoder_layer_api.h"
#include "c3d_utilities_api.h"


/** CONSTANT MACROS */


#define DEFAULT_NUM_ITERATIONS	        0xFFFF
#define DEFAULT_SOURCE_WIDTH			1920
#define DEFAULT_SOURCE_HEIGHT			1080


#define VERSION_NUMBER			    "3.0.0"
/* Switch names */
#define HELP_SWITCH					"--help"
#define RPU_PROCESS_FORMAT_SWITCH	"--rpu_format"
#define VERIFY_SWITCH				"--verify"
#define NUM_ITERATIONS_SWITCH		"--num_itr"
#define PACKED_UV_SWITCH			"--packed_uv"
#define RPU_FILTER_ENABLE_FLAG_SWITCH      "--rpu_filter_enable_flag"
#define DEFAULT_GRID_POS_SWITCH		"--default_grid_position"
#define GRID_POS_X_VIEW0_SWITCH		"--grid_position_x_view0"
#define GRID_POS_X_VIEW1_SWITCH		"--grid_position_x_view1"
#define GRID_POS_Y_VIEW0_SWITCH		"--grid_position_y_view0"
#define GRID_POS_Y_VIEW1_SWITCH		"--grid_position_y_view1"
#define INTERLACED_SWICTH			"--interlaced"
#define SOURCE_SIZE_X_SWITCH		"--source_size_x"
#define SOURCE_SIZE_Y_SWITCH		"--source_size_y"



/* Switch values */
#define SBS_VAL						"SBS"
#define OU_VAL						"OU"
#define TAB_VAL						"TAB"
#define OM_RPU_TYPE					"OM"







typedef enum io_file_order
{
	BL_INFILE = 0,	
    RPU_BL_OUTFILE,
    TOTAL_NUM_OF_FILES
} IOFILE_ORDER;

#define NUM_YUV_READ_FILES 1
#define NUM_YUV_WRITE_FILES 1


/**
 * \fn int32_t read_input_params_recon_test(
 *					int32_t i32_num_arguments,
 *				    char    *pi8_arguments[],
 *					RPUData		*p_init_params,
 *					uint16_t            *isInterlaced, 
 *					uint16_t			*pi16_num_iterations,
 *					uint16_t			*pi6_source_size_x,
 *					  uint16_t			*pi6_source_size_y,
 *					char    **pi8_yuv_file_names					
 *					);
 *
 * \brief read command line arguments
 *
 * \param[in]   i32_num_arguments        argc.
 * \param[in]   pi8_arguments            *argv.
 * \param[out]  p_init_params           Pointer to strore init RPU parameters.
 * \param[out]  isInterlaced            Specifes Interlaced/Progressive Content. 
 * \param[out]  pi6_source_size_x       Width of Source YUV file.
 * \param[out]  pi6_source_size_y       Height of Source YUV files.
 * \param[out]  pi8_yuv_file_names      Pointer to array stroing all YUv file names.
 *
 * \return        0
 */
int32_t read_input_params_rpu_test(
					int32_t i32_num_arguments,
				    char    *pi8_arguments[],
					RPUData		*p_init_params,
					uint16_t            *isInterlaced,
					uint16_t			*pi16_num_iterations,
					uint16_t			*pi6_source_size_x,
					uint16_t			*pi6_source_size_y,
					char    **pi8_yuv_file_names					
					);


/**
 * \fn int32_t open_yuv_files(
 *					FILE				**pf_yuv_files,
 *				    char				**pi8_yuv_file_names					
 *					);
 *
 * \brief open all i/o files specified in the commane line
 *
 * \param[out]   pf_yuv_files            list of all opened yuv file pointers.
 * \param[in]    pi8_yuv_file_names       list of all i/o yuv file names.
 *
 * \return        0
 */
int32_t open_yuv_files(
					FILE				**pf_yuv_files,
				    char				**pi8_yuv_file_names					
					);

/**
 * \fn void close_yuv_files(
 *						FILE				**pf_yuv_files
 *						);
 *
 * \brief close all yuv files.
 *
 * \param[in]   pf_yuv_files            list of all opened yuv file pointers. 
 *
 * \return        None
 */


void close_yuv_files(
						FILE				**pf_yuv_files
						);


/**
 * \fn void print_dec_rpu_test_usage_info(const char *pi8_app_name);
 *
 * \brief Prints the usage of  rpu test application.
 *
 * \param[in]   pi8_app_name           Application Name. 
 *
 * \return        None
 */

void print_dec_rpu_test_usage_info(const char *pi8_app_name);

#endif /* _DEC_RPU_TEST_H_ */
