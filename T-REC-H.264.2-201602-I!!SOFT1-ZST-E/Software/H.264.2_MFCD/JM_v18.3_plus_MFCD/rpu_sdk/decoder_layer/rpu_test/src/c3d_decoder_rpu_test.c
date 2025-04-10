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
 * \details 
 *			
 *			
 *
 * \note   Confidential Information - Limited distribution to authorized persons
 *         only. This material is protected under international copyright laws
 *         as an unpublished work. Do not copy.
 *         Copyright (C) 2012 Dolby Laboratories Inc. All rights reserved.
 *
 */

/* Standard header files */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "c3d_decoder_rpu_test.h"


/**
 * \fn main()
 *
 * \brief This is the entry point to the filter test program.
 *
 * \details 
 *
 * \return -1 in case of error, 0 otherwise.
 */
int main(int argc, char *argv[])
{
	int32_t i32_status,i32_loop;
	uint32_t u32_k;

	RPUData		        *p_init_params = NULL;	
	void				*pv_dec_layer_ctxt;

	
	
	ImageData *p_actual_bl = NULL;
	ImageData *p_predenhlayer = NULL;
	
	
	ImageData *p_tempImage1 = NULL;
	ImageData  ref_image;

	ImageData p_baselayer_dup ;
	ImageData p_predenhlayer_dup ;

	

	
    uint16_t            isInterlaced                          = 0;    
	uint16_t			u16_actual_size_x					  = DEFAULT_SOURCE_WIDTH;
	uint16_t			u16_actual_size_y					  = DEFAULT_SOURCE_HEIGHT;
	CHROMA_FORMAT		e_chroma_format                       = YUV_420;
	uint16_t			u16_num_iterations                    = DEFAULT_NUM_ITERATIONS;	
	char				*yuv_file_names[NUM_YUV_READ_FILES + NUM_YUV_WRITE_FILES] = {NULL,NULL}; /*base layer, predicted enhancement layer "*/
	FILE				*pf_yuv_files[NUM_YUV_READ_FILES + NUM_YUV_WRITE_FILES] ={NULL,NULL} ;

	
	p_init_params = (RPUData *) allocate_memory(sizeof(RPUData));
	if(!p_init_params)
	{
		printf("\nError allocating memory for static init params structure");
		return NULL;
	} /* if(!p_init_params) */





	/* setting Default Values */
	p_init_params->e_rpu_process_format					= DEFAULT_RPU_PROCESS_FORMAT;
    p_init_params->u8_rpu_filter_enabled_flag           = DEFAULT_RPU_FILTER_VALUE;
	p_init_params->u8_default_grid_position_flag    	= DEFAULT_GRID_POSITION_VALUE;
	p_init_params->u8_view0_grid_position_x				= DEFAULT_VIEW0_GRID_POS_X;
	p_init_params->u8_view0_grid_position_y				= DEFAULT_VIEW0_GRID_POS_Y;
	p_init_params->u8_view1_grid_position_x				= DEFAULT_VIEW1_GRID_POS_X;	
	p_init_params->u8_view1_grid_position_y				= DEFAULT_VIEW1_GRID_POS_Y;
	p_init_params->u8_packed_UV							= DEFAULT_PACKED_UV;



	

	/* Read Input arguments from Command Line */
	i32_status = read_input_params_rpu_test(argc,
									argv,
									p_init_params,
									&isInterlaced,
									&u16_num_iterations,
									&u16_actual_size_x,
									&u16_actual_size_y,
									yuv_file_names);

	
	
	i32_status = open_yuv_files(pf_yuv_files,yuv_file_names);

 /* Finalize the maximum number of frames to process based on file size)*/
    u16_num_iterations = MIN_VAL(u16_num_iterations,(GetNumberYUVFrames(pf_yuv_files[BL_INFILE],u16_actual_size_x,u16_actual_size_y,0)));
    



	/* Setup Reference Image Size */	
	ref_image.au16_frame_width[Y]		= u16_actual_size_x;
	ref_image.au16_frame_height[Y]		= u16_actual_size_y;
	ref_image.au16_buffer_stride[Y]		= u16_actual_size_x;
	ref_image.au16_view_delimiter_sbs[Y]= u16_actual_size_x >> 1;
	ref_image.au16_view_delimiter_ou[Y] = u16_actual_size_y  >> 1;
	ref_image.e_yuv_chroma_format		= e_chroma_format;	
	

    /* Allocate memory for Input Images */
	i32_status = allocate_image(&p_actual_bl,
							&ref_image,			
							p_init_params->u8_packed_UV);


	/* Allocate memory for Output Images */
	i32_status = allocate_image(&p_predenhlayer,
							&ref_image,							
							p_init_params->u8_packed_UV);

	i32_status = allocate_image(&p_tempImage1,
							&ref_image,							
							p_init_params->u8_packed_UV);

	

	/* Initialize Decoder Layer */
	pv_dec_layer_ctxt		= allocate_mfc_decoder_layer();
    initialize_mfc_decoder_layer(pv_dec_layer_ctxt,p_init_params); 

	
	
    /* Apply RPU Process frame by frame */
	for(u32_k = 0; u32_k < u16_num_iterations; u32_k++)
	{
		printf("\nRunning MFC RPU Test App : %d out of %d\n", u32_k + 1, u16_num_iterations);

		/* Read a Frame of View 0 */
		i32_status =  read_yuv_image(p_actual_bl , p_tempImage1,pf_yuv_files[BL_INFILE],p_init_params->u8_packed_UV);

		if(i32_status != SUCCESS)
		{
			printf("\nEnd of YUV Data in View 0 File.");
			printf("\nTotal Frames Completed: %d\n\n", u32_k + 1);
			break;
		}

		

		for(i32_loop=isInterlaced;i32_loop<2*isInterlaced	 + 1;i32_loop++)
		{
			
			/* Setup Image Format for Input/Output Images */
			change_image_format(&p_baselayer_dup	,p_actual_bl	,(PICTURE_TYPE)i32_loop);
			change_image_format(&p_predenhlayer_dup	,p_predenhlayer	,(PICTURE_TYPE)i32_loop);
			

		
			/* Apply RPU to get PredEnh Layer */		
			i32_status = process_dec_rpu(pv_dec_layer_ctxt,
								&p_baselayer_dup,
								&p_predenhlayer_dup
								);

       

		}/* end of loop i32_loop */
	
			/* Write output Images*/			
			write_yuv_image(p_predenhlayer ,p_tempImage1, pf_yuv_files[RPU_BL_OUTFILE],p_init_params->u8_packed_UV);

	}/* end of loop u32_k*/
		

		
	/* Free and Close */

	free_mfc_decoder_layer(pv_dec_layer_ctxt);
	
    
	free_image_mem(p_actual_bl);
	free_image_mem(p_predenhlayer);	
	free_image_mem(p_tempImage1);

	free_memory(p_init_params);	
	close_yuv_files(pf_yuv_files);

	free(yuv_file_names[BL_INFILE]);
	free(yuv_file_names[RPU_BL_OUTFILE]);
	



} /* End of main() */
