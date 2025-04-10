/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001, International Telecommunications Union, Geneva
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
 ************************************************************************
 * \file vui_params.h
 *
 * \brief
 *    Input parameters related definitions
 *
 * \author
 *
 ************************************************************************
 */

#ifndef _VUI_PARAMS_H_
#define _VUI_PARAMS_H_

// VUI Parameters
typedef struct vui_parameters
{
  int aspect_ratio_info_present_flag;
  int aspect_ratio_idc;
  int sar_width;
  int sar_height;
  int overscan_info_present_flag;
  int overscan_appropriate_flag;
  int video_signal_type_present_flag;
  int video_format;
  int video_full_range_flag;
  int colour_description_present_flag;
  int colour_primaries;
  int transfer_characteristics; 
  int matrix_coefficients;
  int chroma_location_info_present_flag;
  int chroma_sample_loc_type_top_field;
  int chroma_sample_loc_type_bottom_field;
  int timing_info_present_flag;
  int num_units_in_tick;
  int time_scale;
  int fixed_frame_rate_flag;
  int nal_hrd_parameters_present_flag;
  int nal_cpb_cnt_minus1;
  int nal_bit_rate_scale;
  int nal_cpb_size_scale;
  int nal_bit_rate_value_minus1;
  int nal_cpb_size_value_minus1;
  int nal_vbr_cbr_flag;
  int nal_initial_cpb_removal_delay_length_minus1;
  int nal_cpb_removal_delay_length_minus1;
  int nal_dpb_output_delay_length_minus1;
  int nal_time_offset_length;
  int vcl_hrd_parameters_present_flag;
  int vcl_cpb_cnt_minus1;
  int vcl_bit_rate_scale;
  int vcl_cpb_size_scale;
  int vcl_bit_rate_value_minus1;
  int vcl_cpb_size_value_minus1;
  int vcl_vbr_cbr_flag;
  int vcl_initial_cpb_removal_delay_length_minus1;
  int vcl_cpb_removal_delay_length_minus1;
  int vcl_dpb_output_delay_length_minus1;
  int vcl_time_offset_length;
  int low_delay_hrd_flag;
  int pic_struct_present_flag;
  int bitstream_restriction_flag;
  int motion_vectors_over_pic_boundaries_flag;
  int max_bytes_per_pic_denom;
  int max_bits_per_mb_denom;
  int log2_max_mv_length_vertical;
  int log2_max_mv_length_horizontal;
  int num_reorder_frames;
  int max_dec_frame_buffering;
} VUIParameters;

#endif

