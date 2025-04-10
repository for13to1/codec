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
 *    Picture and Sequence Parameter Sets, encoder operations
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
#include "nalu.h"
#include "sei.h"

extern void GenerateParameterSets (VideoParameters *p_Vid);
extern void FreeParameterSets     (VideoParameters *p_Vid);

extern NALU_t *GenerateSeq_parameter_set_NALU (VideoParameters *p_Vid);
extern NALU_t *GeneratePic_parameter_set_NALU (VideoParameters *p_Vid, int);
extern NALU_t *GenerateSEImessage_NALU(InputParameters *p_Inp);
#if MVC_EXTENSION_ENABLE||EXT3D
extern NALU_t *GenerateSubsetSeq_parameter_set_NALU (VideoParameters *p_Vid);
#endif
#if EXT3D
extern int     ViewCompOrder(VideoParameters *p_Vid, int depthFlag, int VOIdx);
extern NALU_t *GenerateDep_parameter_set_NALU (VideoParameters *p_Vid);
#endif

// The following are local helpers, but may come handy in the future, hence public
extern void GenerateSequenceParameterSet(seq_parameter_set_rbsp_t *sps, VideoParameters *p_Vid, int SPS_id);
extern void GeneratePictureParameterSet( pic_parameter_set_rbsp_t *pps, seq_parameter_set_rbsp_t *sps, 
                                 VideoParameters *p_Vid,
                                 InputParameters *p_Inp, int PPS_id,
                                 int WeightedPrediction, int WeightedBiprediction,
                                 int cb_qp_index_offset, int cr_qp_index_offset);
#if EXT3D
extern void GenerateDepthParameterSet(depth_parameter_set_rbsp_t *dps, VideoParameters *p_Vid, int DPS_id);
extern void copy_3dv_acquisition_info(ThreeDVAcquisitionInfo* dst_3dv_acquisition,ThreeDVAcquisitionInfo* src_3dv_acquisition);
#endif

extern int  Scaling_List(short *scalingListinput, short *scalingList, int sizeOfScalingList, short *UseDefaultScalingMatrix, Bitstream *bitstream);

#if MVC_EXTENSION_ENABLE||EXT3D
extern int  GenerateSeq_parameter_set_rbsp (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps, byte *buf, short Is_Subset);
#else
extern int  GenerateSeq_parameter_set_rbsp (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps, byte *buf);
#endif

#if EXT3D
extern int  GenerateDep_parameter_set_rbsp (VideoParameters *p_Vid, depth_parameter_set_rbsp_t *dps, byte *buf);

extern void prepare_3dv_acquisition_element_info(VideoParameters* p_TextVid,VideoParameters* p_DepthVid);
extern void cal_3dv_acquisition_element_info(VideoParameters* p_TextVid,VideoParameters* p_DepthVid, int in_subsps);
extern void PrepareAcquisitionElement(VideoParameters* p_TextVid,VideoParameters* p_DepthVid, ThreeDVAE* curr_3dv_ae, ThreeDVAE* sps_3dv_ae, ThreeDVAE* forward_3dv_ae, ThreeDVAE* backward_3dv_ae, int in_subsps);
extern void set_cam_info_for_current_AU(VideoParameters*p_TextVid,VideoParameters* p_DepthVid);
extern int encode_3dv_acquisition_info(ThreeDVAcquisitionInfo* threeDV_acquisition_info, Bitstream* bitstream, int in_subsps, int is_vsp_param_flag);
extern void prepare_3dv_update_info(VideoParameters* p_TextVid,VideoParameters* p_DepthVid, int in_subsps);
#endif

extern int  GeneratePic_parameter_set_rbsp (VideoParameters *p_Vid, pic_parameter_set_rbsp_t *pps, byte *buf);
extern int  GenerateSEImessage_rbsp (InputParameters *p_Inp, int id, byte *buf);
extern void FreeSPS (seq_parameter_set_rbsp_t *sps);
extern void FreePPS (pic_parameter_set_rbsp_t *pps);
#if EXT3D
extern void FreeDPS (depth_parameter_set_rbsp_t *dps);
#endif

extern int  WriteHRDParameters(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream);
extern void GenerateVUIParameters(seq_parameter_set_rbsp_t *sps, InputParameters *p_Inp);

extern pic_parameter_set_rbsp_t *AllocPPS (void);
extern seq_parameter_set_rbsp_t *AllocSPS (void);
#if EXT3D
extern depth_parameter_set_rbsp_t *AllocDPS (void);
#endif

#endif
