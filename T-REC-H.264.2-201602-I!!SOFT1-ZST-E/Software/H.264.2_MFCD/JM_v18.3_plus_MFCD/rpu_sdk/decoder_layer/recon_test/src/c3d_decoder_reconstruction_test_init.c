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


#include "c3d_decoder_reconstruction_test.h"




int32_t read_input_params_recon_test(
					int32_t             i32_num_arguments,
				    char                *pi8_arguments[],
					RPUData		        *p_init_params,					
					uint16_t			*pu16_isInterlaced,
					uint16_t			*pu16_deMuxMode,
					uint16_t			*pi16_num_iterations,
					uint16_t			*pi6_source_size_x,
					uint16_t			*pi6_source_size_y,
					char				**pi8_yuv_file_names					
					)
{
	int32_t i32_status = SUCCESS;
	int32_t i32_i = 1;
	uint32_t u32length;


	if(i32_num_arguments<2)
	{
		print_dec_layer_test_usage_info(pi8_arguments[0]);		
		exit(-1);
	}
	/* The RPU process format is a mandatory argument. Hence, the number of options is at least 5*/
	if((!strcmp(pi8_arguments[i32_i], HELP_SWITCH)) || (i32_num_arguments < 6))
	{
		print_dec_layer_test_usage_info(pi8_arguments[0]);		
		exit(-1);
	} /* HELP_SWITCH */

	/* 1st argument is BL_INFILE file name */
	u32length = (uint32_t) strlen(pi8_arguments[1]);
	*pi8_yuv_file_names = (char *) malloc(sizeof(char) * (u32length+1));
	strcpy(*pi8_yuv_file_names , pi8_arguments[1]);

	/* 2nd argument is EL_INFILE file name */ 			
	u32length = (uint32_t) strlen(pi8_arguments[2]);
	*(pi8_yuv_file_names+EL_INFILE) = (char *) malloc(sizeof(char) * (u32length+1));
	strcpy(*(pi8_yuv_file_names+EL_INFILE) , pi8_arguments[2]);

	/* 3rd argument is RPU_BL_INFILE file name */ 			
	u32length = (uint32_t) strlen(pi8_arguments[3]);
	*(pi8_yuv_file_names+RPU_BL_INFILE) = (char *) malloc(sizeof(char) * (u32length+1));
	strcpy(*(pi8_yuv_file_names+RPU_BL_INFILE) , pi8_arguments[3]);

	/* 4th argument is VIEW0_OUTFILE file name */ 			
	u32length = (uint32_t) strlen(pi8_arguments[4]);
	*(pi8_yuv_file_names+VIEW0_OUTFILE) = (char *) malloc(sizeof(char) * (u32length+1));
	strcpy(*(pi8_yuv_file_names+VIEW0_OUTFILE) , pi8_arguments[4]);

	/* 5th argument is VIEW1_OUTFILE file name */ 			
	u32length = (uint32_t) strlen(pi8_arguments[5]);
	*(pi8_yuv_file_names+VIEW1_OUTFILE) = (char *) malloc(sizeof(char) * (u32length+1));
	strcpy(*(pi8_yuv_file_names+VIEW1_OUTFILE) , pi8_arguments[5]);

	

	/* Serach For available switches */
	i32_i = 6;


	while(i32_i < i32_num_arguments)
	{
		/* Static Params */


		if(!strcmp(pi8_arguments[i32_i], RPU_PROCESS_FORMAT_SWITCH))
		{
			if(!strcmp(pi8_arguments[i32_i + 1], SBS_VAL))
			{
				p_init_params->e_rpu_process_format = SBS;				
			} /* SBS */
			else if(!strcmp(pi8_arguments[i32_i + 1], OU_VAL))
			{
				p_init_params->e_rpu_process_format = OU;
				
			} /* OU */
			else if(!strcmp(pi8_arguments[i32_i + 1], TAB_VAL))
			{
				p_init_params->e_rpu_process_format = OU;				
			} /* treat TAB as OU */
			else
			{
				printf("\nError: %s doesnt take the value %s", RPU_PROCESS_FORMAT_SWITCH, pi8_arguments[i32_i+1]);				
				exit(-1);
			} /* SBS/TAB */
			
			i32_i += 2;

		} /* RPU_PROCESS_FORMAT_SWITCH */

		else if(!strcmp(pi8_arguments[i32_i], RPU_FILTER_ENABLE_FLAG_SWITCH))
		{
			p_init_params->u8_rpu_filter_enabled_flag = atoi(pi8_arguments[i32_i + 1]);

			if((p_init_params->u8_rpu_filter_enabled_flag != 0) && (p_init_params->u8_rpu_filter_enabled_flag != 1))
			{
				printf("\nError: Unknown %s value %d", RPU_FILTER_ENABLE_FLAG_SWITCH, p_init_params->u8_rpu_filter_enabled_flag);
				exit(-1);

			} /* RPU_FILTER_ENABLE_FLAG_SWITCH */

			i32_i += 2;

		} /* RPU_FILTER_ENABLE_FLAG_SWITCH */

		else if(!strcmp(pi8_arguments[i32_i], DEFAULT_GRID_POS_SWITCH))
		{
			p_init_params->u8_default_grid_position_flag = atoi(pi8_arguments[i32_i + 1]);

			if((p_init_params->u8_default_grid_position_flag != 0) && (p_init_params->u8_default_grid_position_flag != 1))
			{
				printf("\nError: Unknown %s value %d", DEFAULT_GRID_POS_SWITCH, p_init_params->u8_default_grid_position_flag);
				exit(-1);

			} /* DEFAULT_GRID_POS_SWITCH */

			i32_i += 2;

		} /* DEFAULT_GRID_POS_SWITCH */

		else if(!strcmp(pi8_arguments[i32_i], GRID_POS_X_VIEW0_SWITCH))
		{
			p_init_params->u8_view0_grid_position_x = atoi(pi8_arguments[i32_i + 1]);
			if((p_init_params->u8_view0_grid_position_x  != 0) && (p_init_params->u8_view0_grid_position_x  != 1))
			{
				printf("\nError: Unknown %s value %d", GRID_POS_X_VIEW0_SWITCH, p_init_params->u8_view0_grid_position_x);
				exit(-1);

			} /* GRID_POS_X_VIEW0_SWITCH */
			
			i32_i += 2;

		} /* GRID_POS_X_VIEW0_SWITCH */


		else if(!strcmp(pi8_arguments[i32_i], GRID_POS_X_VIEW1_SWITCH))
		{
			p_init_params->u8_view1_grid_position_x = atoi(pi8_arguments[i32_i + 1]);
			if((p_init_params->u8_view1_grid_position_x  != 0) && (p_init_params->u8_view1_grid_position_x  != 1))
			{
				printf("\nError: Unknown %s value %d", GRID_POS_X_VIEW1_SWITCH, p_init_params->u8_view1_grid_position_x);
				exit(-1);

			} 
			
			i32_i += 2;

		} /* GRID_POS_X_VIEW1_SWITCH */

		else if(!strcmp(pi8_arguments[i32_i], GRID_POS_Y_VIEW0_SWITCH))
		{
			p_init_params->u8_view0_grid_position_y = atoi(pi8_arguments[i32_i + 1]);
			if((p_init_params->u8_view0_grid_position_y  != 0) && (p_init_params->u8_view0_grid_position_y  != 1))
			{
				printf("\nError: Unknown %s value %d", GRID_POS_Y_VIEW0_SWITCH, p_init_params->u8_view0_grid_position_y);
				exit(-1);

			} 
			
			i32_i += 2;

		} /* GRID_POS_Y_VIEW0_SWITCH */


		else if(!strcmp(pi8_arguments[i32_i], GRID_POS_Y_VIEW1_SWITCH))
		{
			p_init_params->u8_view1_grid_position_y = atoi(pi8_arguments[i32_i + 1]);
			if((p_init_params->u8_view1_grid_position_y  != 0) && (p_init_params->u8_view1_grid_position_y  != 1))
			{
				printf("\nError: Unknown %s value %d", GRID_POS_Y_VIEW1_SWITCH, p_init_params->u8_view1_grid_position_y);
				exit(-1);

			} 
			
			i32_i += 2;
		} /* GRID_POS_Y_VIEW1_SWITCH */


		else if(!strcmp(pi8_arguments[i32_i], PACKED_UV_SWITCH))
		{
			p_init_params->u8_packed_UV = atoi(pi8_arguments[i32_i + 1]);

			if((p_init_params->u8_packed_UV != 0) && (p_init_params->u8_packed_UV  != 1))
			{
				printf("\nError: Unknown %s value %d", PACKED_UV_SWITCH, p_init_params->u8_packed_UV);
				exit(-1);
			} 
			i32_i += 2;
		} /* PACKED_UV_SWITCH */
		


	    else if(!strcmp(pi8_arguments[i32_i], DEMUX_MODE_SWITCH))
		{
			if(!strcmp(pi8_arguments[i32_i+1], "OM"))
			{
				*pu16_deMuxMode = 1;
			}
			else if(!strcmp(pi8_arguments[i32_i+1], "FC"))
			{
				*pu16_deMuxMode = 0;
			}
			else
			{
				printf("\nError: Unknown %s value %d", DEMUX_MODE_SWITCH,pi8_arguments[i32_i+1]);
				exit(-1);
			} 
			i32_i += 2;
		} /* DEMUX_MODE_SWITCH */








		else if(!strcmp(pi8_arguments[i32_i], HELP_SWITCH)) 
		{
		print_dec_layer_test_usage_info(pi8_arguments[0]);		
		exit(-1);
		} /* HELP_SWITCH */

		else if(!strcmp(pi8_arguments[i32_i], SOURCE_SIZE_X_SWITCH))
		{
			*pi6_source_size_x =  atoi(pi8_arguments[i32_i + 1]);
			i32_i += 2;
		}/* SOURCE_SIZE_X_SWITCH */

		else if(!strcmp(pi8_arguments[i32_i], SOURCE_SIZE_Y_SWITCH))
		{
			*pi6_source_size_y =  atoi(pi8_arguments[i32_i + 1]);
			i32_i += 2;
		}/* SOURCE_SIZE_Y_SWITCH */



		else if(!strcmp(pi8_arguments[i32_i], NUM_ITERATIONS_SWITCH))
		{
			*pi16_num_iterations =  atoi(pi8_arguments[i32_i + 1]);
			i32_i += 2;
		}/* NUM_ITERATIONS_SWITCH */



		
		else if(!strcmp(pi8_arguments[i32_i], INTERLACED_SWICTH))
		{
			*pu16_isInterlaced = atoi(pi8_arguments[i32_i + 1]);

			if((*pu16_isInterlaced  != 0) && (*pu16_isInterlaced   != 1) )
			{
				printf("\nError: Unknown %s value %d ", INTERLACED_SWICTH, *pu16_isInterlaced );
				exit(-1);
			} 
			i32_i += 2;
		} /* INTERLACED_SWICTH */

		else
		{
		printf("\nError: Unknown Switch %s .\n\n ", pi8_arguments[i32_i]);
		print_dec_layer_test_usage_info(pi8_arguments[0]);		
		exit(-1);
		} /* HELP_SWITCH */

	}





	/* For Default Grid Position , set default values */
	if(1 == p_init_params->u8_default_grid_position_flag)
	{
		p_init_params->u8_view0_grid_position_x = DEFAULT_VIEW0_GRID_POS_X;
		p_init_params->u8_view0_grid_position_y = DEFAULT_VIEW0_GRID_POS_Y;
		p_init_params->u8_view1_grid_position_x = DEFAULT_VIEW1_GRID_POS_X;
		p_init_params->u8_view1_grid_position_y = DEFAULT_VIEW1_GRID_POS_Y;
	}

	return i32_status;
}


int32_t open_yuv_files(
					FILE				**pf_yuv_files,
				    char				**pi8_yuv_file_names					
					)
{
	int32_t i32_status = SUCCESS;
	int32_t i_loop;
	
	for(i_loop=0; i_loop<NUM_YUV_READ_FILES;i_loop++)
	{

		if(pi8_yuv_file_names[i_loop])
		{
			pf_yuv_files[i_loop] = fopen( pi8_yuv_file_names[i_loop], "rb" );
		
			if( !pf_yuv_files[i_loop] )
			{
				printf( "Can't Open Source File For Reading %s\n", pi8_yuv_file_names[i_loop] );
				exit(-1);
			}
		}
	}

	for(i_loop=NUM_YUV_READ_FILES; i_loop<NUM_YUV_WRITE_FILES + NUM_YUV_READ_FILES  ;i_loop++)
	{

		if(pi8_yuv_file_names[i_loop])
		{
			pf_yuv_files[i_loop] = fopen( pi8_yuv_file_names[i_loop], "wb" );
		
			if( !pf_yuv_files[i_loop] )
			{
				printf( "Can't Open Destination File For Writing%s\n", pi8_yuv_file_names[i_loop] );
				exit(-1);
			}
		}
	}

	

	return i32_status;
}



void close_yuv_files(
						FILE				**pf_yuv_files
						)
{
	int32_t i_loop;
	
	for(i_loop=0; i_loop<NUM_YUV_WRITE_FILES + NUM_YUV_READ_FILES  ;i_loop++)
	{
		if(NULL != pf_yuv_files[i_loop])
		{
		fclose(pf_yuv_files[i_loop]);
		pf_yuv_files[i_loop] = NULL;
		}
	}
}

/**
 * \fn void print_dec_layer_test_usage_info(const char *pi8_app_name)
 *
 * \brief Prints the usage info for running the c3d_decoder_layer_test application.
 *
 * \param[in]	pi8_app_name		Name of test app
 *
 * \return None
 */
void print_dec_layer_test_usage_info(const char *pi8_app_name)
{

	printf("Dolby MFC RPU - Reconstruction Test\n");
	printf("Copyright (c) 2001-2012 Dolby Laboratories, Inc.\n");
	printf("http://www.dolby.com\n");
	printf("Version:%s\n",VERSION_NUMBER);
	printf("\nUsage: %s %s", pi8_app_name, HELP_SWITCH);
	printf("\nUsage: %s <%s %s %s %s %s> [OPTIONS]", pi8_app_name, "bl_in.yuv","el_in.yuv","rpu_bl_in.yuv" ,"view0_out.yuv","view1_out.yuv");
	printf("\n\nAvailable switches -				# List:[items]  (DefaultValue)");	
	printf("\n\n%s: Help # Prints the usage info", HELP_SWITCH);
	printf("\n\n%s: RPU process format		# List:[%s, %s]		(%s)", RPU_PROCESS_FORMAT_SWITCH, SBS_VAL,  OU_VAL,SBS_VAL);
    printf("\n\n%s: RPU Filter flag	        # List:[0,1]		(%d)", RPU_FILTER_ENABLE_FLAG_SWITCH ,DEFAULT_RPU_FILTER_VALUE);
	printf("\n\n%s: Default Grid Position	# List:[0,1]		(%d)", DEFAULT_GRID_POS_SWITCH ,DEFAULT_GRID_POSITION_VALUE);
	printf("\n\n%s: View0_Grid_pos_X		# List:[0,1]		(%d)", GRID_POS_X_VIEW0_SWITCH ,DEFAULT_VIEW0_GRID_POS_X);
	printf("\n\n%s: View0_Grid_pos_Y		# List:[0,1]		(%d)", GRID_POS_Y_VIEW0_SWITCH ,DEFAULT_VIEW0_GRID_POS_Y);
	printf("\n\n%s: View1_Grid_pos_X		# List:[0,1]		(%d)", GRID_POS_X_VIEW0_SWITCH ,DEFAULT_VIEW1_GRID_POS_X);
	printf("\n\n%s: View1_Grid_pos_Y		# List:[0,1]		(%d)", GRID_POS_Y_VIEW0_SWITCH ,DEFAULT_VIEW1_GRID_POS_Y);
	printf("\n\n%s: Packed_UV				# List:[0,1]		(%d) (0-Planar,1-non-planar)", PACKED_UV_SWITCH ,DEFAULT_PACKED_UV);
	printf("\n\n%s: Interlaced Processing	# List:[0,1]		(%d)", INTERLACED_SWICTH ,0);
    printf("\n\n%s: DemuxMode           	# List:[OM,FC]		(%s)", DEMUX_MODE_SWITCH ,DEFAULT_DEMUX_MODE==1?"OM":"FC");

	printf("\n\n%s: YUV Source File Width			(%d)", SOURCE_SIZE_X_SWITCH ,DEFAULT_SOURCE_WIDTH);
	printf("\n\n%s: YUV Source File Height			(%d)", SOURCE_SIZE_Y_SWITCH ,DEFAULT_SOURCE_HEIGHT);



	printf("\n\n");
} /* End of print_usage_info() function */