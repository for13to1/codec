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



/*!
 **************************************************************************************
 * \file
 *    parset.h
 * \brief
 *    Picture and Sequence Parameter Sets, decoder operations
 * 
 * \date 25 November 2002
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger        <stewe@cs.tu-berlin.de>
 ***************************************************************************************
 */


#ifndef _PARSET_H_
#define _PARSET_H_

#include "parsetcommon.h"
#include "nalucommon.h"

static const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

extern void Scaling_List(int *scalingList, int sizeOfScalingList, Boolean *UseDefaultScalingMatrix, Bitstream *s);

extern void InitVUI(seq_parameter_set_rbsp_t *sps);
extern int  ReadVUI(DataPartition *p, seq_parameter_set_rbsp_t *sps);
extern int  ReadHRDParameters(DataPartition *p, hrd_parameters_t *hrd);

extern void PPSConsistencyCheck (pic_parameter_set_rbsp_t *pps);
extern void SPSConsistencyCheck (seq_parameter_set_rbsp_t *sps);
#if EXT3D
extern void DPSConsistencyCheck (depth_parameter_set_rbsp_t *dps);
#endif

extern void MakePPSavailable (VideoParameters *p_Vid, int id, pic_parameter_set_rbsp_t *pps);
extern void MakeSPSavailable (VideoParameters *p_Vid, int id, seq_parameter_set_rbsp_t *sps);
#if EXT3D
extern void MakeDPSavailable (VideoParameters *p_Vid, int id, depth_parameter_set_rbsp_t *dps);
#endif

extern void ProcessSPS (VideoParameters *p_Vid, NALU_t *nalu);
extern void ProcessPPS (VideoParameters *p_Vid, NALU_t *nalu);
#if EXT3D
extern void ProcessDPS (VideoParameters *p_Vid, NALU_t *nalu);
#endif

extern void CleanUpPPS(VideoParameters *p_Vid);
#if EXT3D
extern void activate_sps (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps,int is_depth);
#else
extern void activate_sps (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps);
#endif
extern void activate_pps (VideoParameters *p_Vid, pic_parameter_set_rbsp_t *pps);
#if EXT3D
extern void activate_dps (VideoParameters *p_Vid, depth_parameter_set_rbsp_t *dps);
extern void UseDepParameterSet (Slice *currSlice);
#endif

extern void UseParameterSet (Slice *currSlice);

#if MVC_EXTENSION_ENABLE||EXT3D
extern void SubsetSPSConsistencyCheck (subset_seq_parameter_set_rbsp_t *subset_sps);
extern void ProcessSubsetSPS (VideoParameters *p_Vid, NALU_t *nalu);

extern void mvc_vui_parameters_extension(MVCVUI_t *pMVCVUI, Bitstream *s);

#if EXT3D
extern void mvcd_vui_parameters_extension(MVCVUI_t *pMVCVUI, Bitstream *s);
extern void copy_acquisition_element(ThreeDVAE* dst_ae, ThreeDVAE* src_ae);
extern void copy_3dv_acquisition_info(ThreeDVAcquisitionInfo* dst_3dv_acquisition,ThreeDVAcquisitionInfo* src_3dv_acquisition);
extern void decode_acquisition_info(ThreeDVAcquisitionInfo* threeDV_acquisition_info, ThreeDVAcquisitionInfo* subsps_3dv_acquisition_info, ThreeDVAcquisitionInfo* forward_3dv_acquisition_info, ThreeDVAcquisitionInfo* backward_3dv_acquisition_info, Bitstream* bitstream, int is_vsp_param_flag);
extern void decode_acquisition_element(ThreeDVAcquisitionInfo* threeDV_acquisition_info, ThreeDVAE* curr_3dv_ae, ThreeDVAE* forward_3dv_ae, ThreeDVAE* backward_3dv_ae, Bitstream* bitstream);
extern void init_sps_3dv_extension_list(seq_parameter_set_3dv_extension *sps_3dv_extension_list, int iSize);
extern void interpret_sps_3dv_extension(seq_parameter_set_3dv_extension *sps_3dv_extension, int num_of_views,Bitstream *s);
extern void reset_sps_3dv_extension(seq_parameter_set_3dv_extension *sps_3dv_extension);
#endif

extern void seq_parameter_set_mvc_extension(subset_seq_parameter_set_rbsp_t *subset_sps, Bitstream *s);

extern void init_subset_sps_list(subset_seq_parameter_set_rbsp_t *subset_sps_list, int iSize);
extern void reset_subset_sps(subset_seq_parameter_set_rbsp_t *subset_sps);
extern int  GetBaseViewId(VideoParameters *p_Vid, subset_seq_parameter_set_rbsp_t **subset_sps);
extern void get_max_dec_frame_buf_size(seq_parameter_set_rbsp_t *sps);
#endif

#endif
