/*
 * Disclaimer of Warranty
 *
 * Copyright 2001-2015, International Telecommunication Union, Geneva
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis. The ITU disclaims
 * any and all warranties, whether express, implied, or statutory,
 * including any implied warranties of merchantability or of fitness
 * for a particular purpose.  In no event shall the ITU be liable for
 * any incidental, punitive, or consequential damages of any kind
 * whatsoever arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and the user's customers, employees, agents, transferees,
 * successors, and assigns.
 *
 * The ITU does not represent or warrant that the programs furnished
 * hereunder are free of infringement of any third-party patents.
 * Commercial implementations of ITU-T Recommendations, including
 * shareware, may be subject to royalty fees to patent holders.
 * Information regarding the ITU-T patent policy is available from the
 * ITU web site at http://www.itu.int.
 *
 * THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
 *
 */


#ifndef DB_TRANS_H
#define DB_TRANS_H

#if EXT3D

typedef struct 
{
  int32 sign;                 
  int32 exponent;
  int32 mantissa;
  int mantissa_bits;
} double_split;

typedef struct ThreeDVAcquisitionElement
{
  double_split element[MAX_CODEVIEW];
  int32 exponent_size;

  byte skip_flag[MAX_CODEVIEW];
  byte element_equal;
  byte exponent_skip_flag[MAX_CODEVIEW];

  int32 sign_diff[MAX_CODEVIEW];
  int32 exponent_diff[MAX_CODEVIEW];
  int32 mantissa_diff[MAX_CODEVIEW];

  byte delta_flag;
  byte element_flag;
  byte pred_mode;

  int32 precision;
  int32 mantissa_length;


  double original[MAX_CODEVIEW];
  double rec[MAX_CODEVIEW];
} ThreeDVAE;

typedef struct ThreeDVAcquisitionInfo
{
  ThreeDVAE *focal_length_x_ae;
  ThreeDVAE *focal_length_y_ae;
  ThreeDVAE *principal_point_x_ae;
  ThreeDVAE *principal_point_y_ae;
  ThreeDVAE *translation_ae;
  ThreeDVAE *depth_near_ae;
  ThreeDVAE *depth_far_ae;

#if EXT3D
  // DEPTH_PARAMETER_SET
  int num_views;
  int pred_direction;
  int pred_weight0;
  int ref_element_id0;
  int ref_element_id1;

  // NOKIA_DISP_CALC_B0150
  byte disp_param_flag;
  double d_disparity_scale[MAX_CODEVIEW][MAX_CODEVIEW];  //!< scale parameter for disparity [source_view_idx][target_view_idx]
  double d_disparity_offset[MAX_CODEVIEW][MAX_CODEVIEW];  //!< offset parameter for disparity [source_view_idx][target_view_idx]
  int i_disparity_scale[MAX_CODEVIEW][MAX_CODEVIEW];  //!< scale parameter for disparity [source_view_idx][target_view_idx]
  int i_disparity_offset[MAX_CODEVIEW][MAX_CODEVIEW];  //!< offset parameter for disparity [source_view_idx][target_view_idx]
  int i_disparity_scale_diff[MAX_CODEVIEW][MAX_CODEVIEW];  //!< scale parameter for disparity [source_view_idx][target_view_idx]
  int i_disparity_offset_diff[MAX_CODEVIEW][MAX_CODEVIEW];  //!< offset parameter for disparity [source_view_idx][target_view_idx]
#endif

} ThreeDVAcquisitionInfo;

typedef struct ThreeDVUpdateInfo 
{
  int pred_direction;
  int pred_weight0;
  int frame_num_diff0_minus1;
  int frame_num_diff1_minus1;
  int num_views_minus1;
  int camera_order[MAX_CODEVIEW];

  ThreeDVAcquisitionInfo* currr_3dv_acquisition_info;
  ThreeDVAcquisitionInfo* forward_3dv_acquisition_info;
  ThreeDVAcquisitionInfo* backward_3dv_acquisition_info;
  ThreeDVAcquisitionInfo* subsps_3dv_acquisition_info;
} ThreeDVUpdateInfo ;

//extern void GetExponentMantissa(ThreeDVAE* threeDV_ae);
extern void get_exponent_mantissa(ThreeDVAE* threeDV_ae,int voidx);
extern void get_rec_double_type(ThreeDVAE* threeDV, int voidx);
//extern int32 GetReconDoubleType(ThreeDVAE* threeDV_ae,int voidx,double* recon);
extern int  get_mem_acquisition_info(ThreeDVAcquisitionInfo** threeDV_acquisition_info);
extern void free_mem_acquisition_info(ThreeDVAcquisitionInfo* threeDV_acqusition_info);
extern void init_acquisition_info(ThreeDVAcquisitionInfo* ThreeDV_acquisition_info);
extern void copy_acquisition_info(ThreeDVAcquisitionInfo* dest_acquisition_infor, ThreeDVAcquisitionInfo* src_acquisition_info);

#if EXT3D
typedef struct ThreeDVAcquisitionElementSEI
{
  double_split element;
  int32 exponent_size;

  byte pred_mode;

  int32 precision;
  int32 mantissa_length;

  double original;
  double rec;
} ThreeDVAESEI;

typedef struct DepthAcquisitionInfoSEI
{
  ThreeDVAESEI depth_near_ae;
  ThreeDVAESEI depth_far_ae;
  ThreeDVAESEI d_min_ae;
  ThreeDVAESEI d_max_ae;
} DepthAcquisitionInfoSEI;

typedef struct MultiviewAcquisitionInfoSEI
{
  ThreeDVAESEI focal_length_x_ae;
  ThreeDVAESEI focal_length_y_ae;
  ThreeDVAESEI principal_point_x_ae;
  ThreeDVAESEI principal_point_y_ae;
  ThreeDVAESEI skew_factor_ae;
  ThreeDVAESEI rotation_ae[3][3];
  ThreeDVAESEI translation_ae[3];
} MultiviewAcquisitionInfoSEI;

extern void get_exponent_mantissa_SEI(ThreeDVAESEI* threeDV_ae);
extern void get_rec_double_type_SEI(ThreeDVAESEI* threeDV_ae);

extern void init_depth_acquisition_info_SEI(DepthAcquisitionInfoSEI* depth_acquisition_info);
extern void init_multiview_acquisition_info_SEI(MultiviewAcquisitionInfoSEI* multiview_acquisition_info);

#endif

#endif

#endif

