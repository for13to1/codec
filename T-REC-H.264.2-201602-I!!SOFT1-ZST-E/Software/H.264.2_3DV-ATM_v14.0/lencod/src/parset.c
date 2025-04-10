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
 *    parset.c
 * \brief
 *    Picture and Sequence Parameter set generation and handling
 *  \date 25 November 2002
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger        <stewe@cs.tu-berlin.de>
 *
 **************************************************************************************
 */

#include <time.h>
#include <limits.h>

#include "global.h"

#include "contributors.h"
#include "nal.h"
#include "mbuffer.h"
#include "parset.h"
#include "vlc.h"
#include "q_matrix.h"
#if EXT3D
#include "nonlinear_depth.h"
#include "configfile.h"
#include "3dv_acquisition.h"
#endif

// Local helpers
static int IdentifyProfile(InputParameters *p_Inp);
static int IdentifyLevel(InputParameters *p_Inp);
static int GenerateVUI_parameters_rbsp(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream);

static const byte ZZ_SCAN[16]  =
{  
  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};


/*!
 *************************************************************************************
 * \brief
 *    generates a sequence and picture parameter set and stores these in global
 *    active_sps and active_pps
 *
 * \return
 *    A NALU containing the Sequence ParameterSet
 *
 *************************************************************************************
*/
void GenerateParameterSets (VideoParameters *p_Vid)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int i;
  seq_parameter_set_rbsp_t *sps = NULL;

  sps = AllocSPS();

  for (i=0; i<MAXPPS; i++)
  {
    p_Vid->PicParSet[i] = NULL;
  }

#if EXT3D
  for (i=0; i<MAXDPS; i++)
  {
    p_Vid->DepParSet[i] = NULL;
  }

  GenerateSequenceParameterSet(sps, p_Vid, p_Enc->num_of_sps++);
#else
  GenerateSequenceParameterSet(sps, p_Vid, 0);
#endif

  if (p_Inp->GenerateMultiplePPS)
  {
    p_Vid->PicParSet[0] = AllocPPS();
    p_Vid->PicParSet[1] = AllocPPS();
    p_Vid->PicParSet[2] = AllocPPS();

    if (IS_FREXT_PROFILE(sps->profile_idc))
    {
#if EXT3D
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, 0, 0, p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
      GeneratePictureParameterSet( p_Vid->PicParSet[1], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, 1, 1, p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
      GeneratePictureParameterSet( p_Vid->PicParSet[2], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, 1, 2, p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
#else
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, 0, 0, 0, p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
      GeneratePictureParameterSet( p_Vid->PicParSet[1], sps, p_Vid, p_Inp, 1, 1, 1, p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
      GeneratePictureParameterSet( p_Vid->PicParSet[2], sps, p_Vid, p_Inp, 2, 1, 2, p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
#endif
    }
    else
    {
#if EXT3D
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, 0, 0, p_Inp->chroma_qp_index_offset, 0);
      GeneratePictureParameterSet( p_Vid->PicParSet[1], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, 1, 1, p_Inp->chroma_qp_index_offset, 0);
      GeneratePictureParameterSet( p_Vid->PicParSet[2], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, 1, 2, p_Inp->chroma_qp_index_offset, 0);
#else
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, 0, 0, 0, p_Inp->chroma_qp_index_offset, 0);
      GeneratePictureParameterSet( p_Vid->PicParSet[1], sps, p_Vid, p_Inp, 1, 1, 1, p_Inp->chroma_qp_index_offset, 0);
      GeneratePictureParameterSet( p_Vid->PicParSet[2], sps, p_Vid, p_Inp, 2, 1, 2, p_Inp->chroma_qp_index_offset, 0);
#endif
    }
  }
  else
  {
    p_Vid->PicParSet[0] = AllocPPS();
    if (IS_FREXT_PROFILE(sps->profile_idc))
#if EXT3D
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, p_Inp->WeightedPrediction, p_Inp->WeightedBiprediction,
      p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
#else
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, 0, p_Inp->WeightedPrediction, p_Inp->WeightedBiprediction,
                                   p_Inp->cb_qp_index_offset, p_Inp->cr_qp_index_offset);
#endif
    else
#if EXT3D
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, p_Enc->num_of_pps++, p_Inp->WeightedPrediction, p_Inp->WeightedBiprediction,
      p_Inp->chroma_qp_index_offset, 0);
#else
      GeneratePictureParameterSet( p_Vid->PicParSet[0], sps, p_Vid, p_Inp, 0, p_Inp->WeightedPrediction, p_Inp->WeightedBiprediction,
                                   p_Inp->chroma_qp_index_offset, 0);
#endif
  }

  p_Vid->active_sps = sps;
  p_Vid->active_pps = p_Vid->PicParSet[0];

#if EXT3D

  if(p_Vid->is_depth && p_Vid->p_Inp->AcquisitionIdx==0)
  {
    if(p_Enc->num_of_dps==0)
      p_Enc->num_of_dps=1;
    if ( NULL != p_Vid->DepParSet[p_Enc->num_of_dps])
    {
      FreeDPS(p_Vid->DepParSet[p_Enc->num_of_dps]);
      p_Vid->DepParSet[p_Enc->num_of_dps] = NULL;
    }
    p_Vid->DepParSet[p_Enc->num_of_dps] = AllocDPS();
    GenerateDepthParameterSet(p_Vid->DepParSet[p_Enc->num_of_dps], p_Vid, p_Enc->num_of_dps);
    p_Vid->active_dps = p_Vid->DepParSet[p_Enc->num_of_dps];
    p_Enc->num_of_dps++;
    if(p_Enc->num_of_dps == MAXDPS)
      p_Enc->num_of_dps = 1;
    copy_3dv_acquisition_info(p_Vid->active_dps->acquisition_info,p_Vid->ThreeDV_acquisition_info);
  }
#endif
}

/*!
*************************************************************************************
* \brief
*    frees global parameter sets active_sps and active_pps
*
* \return
*    A NALU containing the Sequence ParameterSet
*
*************************************************************************************
*/
void FreeParameterSets (VideoParameters *p_Vid)
{
  int i;
  for (i=0; i<MAXPPS; i++)
  {
    if ( NULL != p_Vid->PicParSet[i])
    {
      FreePPS(p_Vid->PicParSet[i]);
      p_Vid->PicParSet[i] = NULL;
    }
  }
  FreeSPS (p_Vid->active_sps);
#if EXT3D
  for (i=0; i<MAXDPS; i++)
  {
    if ( NULL != p_Vid->DepParSet[i])
    {
      FreeDPS(p_Vid->DepParSet[i]);
      p_Vid->DepParSet[i] = NULL;
    }
  }
#endif
}

/*!
*************************************************************************************
* \brief
*    int GenerateSeq_parameter_set_NALU (VideoParameters *p_Vid);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
* \param p_Vid
* VideoParameters structure
* \return
*    A NALU containing the Sequence ParameterSet
*
*************************************************************************************
*/

NALU_t *GenerateSeq_parameter_set_NALU (VideoParameters *p_Vid)
{
  NALU_t *n = AllocNALU(MAXNALUSIZE);
  int RBSPlen = 0;
#if !EXT3D
  int NALUlen;
#endif
  byte rbsp[MAXRBSPSIZE];

#if EXT3D
  RBSPlen = GenerateSeq_parameter_set_rbsp (p_Vid, p_Vid->active_sps, rbsp,0);
  //NALUlen = 
        RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SPS, NALU_PRIORITY_HIGHEST, 1);
#else
#if (MVC_EXTENSION_ENABLE)
  RBSPlen = GenerateSeq_parameter_set_rbsp (p_Vid, p_Vid->active_sps, rbsp, 0);
#else
  RBSPlen = GenerateSeq_parameter_set_rbsp (p_Vid, p_Vid->active_sps, rbsp);
#endif
  NALUlen = RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SPS, NALU_PRIORITY_HIGHEST, 1);
#endif
  n->startcodeprefix_len = 4;

  return n;
}

#if EXT3D
int ViewCompOrder(VideoParameters *p_Vid, int depthFlag, int VOIdx)
{
  int k;

  for(k=0;k<p_Vid->p_Inp->NumOfCodingView+p_Vid->p_DualInp->NumOfCodingView;++k)
  {
    if(p_Vid->ViewCompOrderDepthFlag[k]==depthFlag && p_Vid->ViewCompOrderVOIdx[k]==VOIdx)
      return k;
  }
  return -1;
}
#endif

#if MVC_EXTENSION_ENABLE||EXT3D

/*!
*************************************************************************************
* \brief
*    int GenerateSubsetSeq_parameter_set_NALU (VideoParameters *p_Vid);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
* \param p_Vid
* VideoParameters structure
* \return
*    A NALU containing the Sequence ParameterSet
*
*************************************************************************************
*/

NALU_t *GenerateSubsetSeq_parameter_set_NALU (VideoParameters *p_Vid)
{
  NALU_t *n = AllocNALU(MAXNALUSIZE);
  int RBSPlen = 0;
  //int NALUlen;
  byte rbsp[MAXRBSPSIZE];

#if EXT3D
  VideoParameters* p_DepthVid=p_Vid->is_depth? p_Vid :p_Vid->p_DualVid;
  VideoParameters* p_TextVid=p_DepthVid->p_DualVid;
  if(p_DepthVid->p_Inp->AcquisitionIdx)
  {
    if ( NULL != p_DepthVid->DepParSet[0])
    {
      FreeDPS(p_DepthVid->DepParSet[0]);
      p_DepthVid->DepParSet[0] = NULL;
    }
    p_DepthVid->DepParSet[0] = AllocDPS();
    GenerateDepthParameterSet(p_DepthVid->DepParSet[0], p_Vid, 0);
    prepare_3dv_acquisition_element_info(p_TextVid, p_DepthVid);
    cal_3dv_acquisition_element_info(p_TextVid, p_DepthVid, 1);
    copy_3dv_acquisition_info(p_DepthVid->DepParSet[0]->acquisition_info,p_DepthVid->ThreeDV_acquisition_info);
  }

  if(p_Vid->is_depth)
  {
    int k;
    int depth_order1=0,depth_order2=0,depth_idx=0;
    int textureVOIdx=0,depthVOIdx=0,idx,idx2;
    for(k=0;k<p_Vid->p_Inp->NumOfCodingView+p_Vid->p_DualInp->NumOfCodingView && depth_idx<p_Vid->p_Inp->NumOfCodingView;++k)
    {
      if(p_Vid->ThreeDVCodingOrder[k]>=0) depth_order2++;
      else
      {
        p_Vid->TexVoidxDelta[depth_idx++]=depth_order2-depth_order1;
        depth_order1=depth_order2=0;
      }
    }

    for(idx=0;idx<p_Vid->p_Inp->NumOfCodingView+p_Vid->p_DualInp->NumOfCodingView;)
      {
      if (depthVOIdx<p_Vid->p_Inp->NumOfCodingView)
        {
          for (idx2=idx;idx2<idx+p_Vid->TexVoidxDelta[depthVOIdx];idx2++)
          {
            p_Vid->ViewCompOrderDepthFlag[idx2]=0;
            p_Vid->ViewCompOrderVOIdx[idx2]=textureVOIdx;
            textureVOIdx++;
          }
          idx+=p_Vid->TexVoidxDelta[depthVOIdx];
          p_Vid->ViewCompOrderDepthFlag[idx]=1;
          p_Vid->ViewCompOrderVOIdx[idx]=p_Vid->CodingDepthView[depthVOIdx];
          depthVOIdx++;
      }
      else
      {
        p_Vid->ViewCompOrderDepthFlag[idx]=0;
        p_Vid->ViewCompOrderVOIdx[idx]=textureVOIdx;
        textureVOIdx++;
      }
      idx++;      
    }
  }
#endif

  RBSPlen = GenerateSeq_parameter_set_rbsp (p_Vid, p_Vid->active_sps, rbsp, 1);

  //NALUlen = 
        RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SUB_SPS, NALU_PRIORITY_HIGHEST, 1);
  n->startcodeprefix_len = 4;

  return n;
}
#endif

/*!
*************************************************************************************
* \brief
*    NALU_t *GeneratePic_parameter_set_NALU (VideoParameters *p_Vid, int PPS_id);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
*
* \return
*    A NALU containing the Picture Parameter Set
*
*************************************************************************************
*/

NALU_t *GeneratePic_parameter_set_NALU(VideoParameters *p_Vid, int PPS_id)
{
  NALU_t *n = AllocNALU(MAXNALUSIZE);
  int RBSPlen = 0;
  //int NALUlen;
  byte rbsp[MAXRBSPSIZE];

  RBSPlen = GeneratePic_parameter_set_rbsp (p_Vid, p_Vid->PicParSet[PPS_id], rbsp);

  //NALUlen = 
        RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_PPS, NALU_PRIORITY_HIGHEST, 1);
  n->startcodeprefix_len = 4;

  return n;
}

#if EXT3D
NALU_t *GenerateDep_parameter_set_NALU(VideoParameters *p_Vid)
{
  NALU_t *n = AllocNALU(MAXNALUSIZE);
  int RBSPlen = 0;
  //int NALUlen;
  byte rbsp[MAXRBSPSIZE];

  if((p_Vid->is_depth)&&(p_Vid->p_Inp->AcquisitionIdx))
  {
    VideoParameters* p_DepthVid=p_Vid;
    VideoParameters* p_TextVid=p_Vid->p_DualVid;
    prepare_3dv_acquisition_element_info(p_TextVid, p_DepthVid);
    cal_3dv_acquisition_element_info(p_TextVid, p_DepthVid, 0);
    set_cam_info_for_current_AU(p_TextVid,p_DepthVid);
  }
  RBSPlen = GenerateDep_parameter_set_rbsp (p_Vid, p_Vid->active_dps, rbsp);
  //NALUlen = 
        RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_DPS, NALU_PRIORITY_HIGHEST, 1);
  n->startcodeprefix_len = 4;

  return n;
}
#endif

/*!
 ************************************************************************
 * \brief
 *    GenerateSequenceParameterSet: extracts info from global variables and
 *    generates sequence parameter set structure
 * \param sps
 *    Sequence Parameter Set to be filled
 * \param p_Vid
 *    VideoParameters for encoding
 * \param SPS_id
 *    SPS ID
 *
 * \par
 *    Function reads all kinds of values from several global variables,
 *    including p_Inp-> and image-> and fills in the sps.  Many
 *    values are current hard-coded to defaults.
 *
 ************************************************************************
 */

void GenerateSequenceParameterSet( seq_parameter_set_rbsp_t *sps,  //!< Sequence Parameter Set to be filled
                                   VideoParameters *p_Vid,         //!< VideoParameters for encoding
                                   int SPS_id                      //!< SPS ID
                                  )
{
  static const int SubWidthC  [4]= { 1, 2, 2, 1};
  static const int SubHeightC [4]= { 1, 2, 1, 1};
  unsigned i;
  unsigned n_ScalingList;
  InputParameters *p_Inp = p_Vid->p_Inp;

  int frext_profile = ((IdentifyProfile(p_Inp)==FREXT_HP) ||
                      (IdentifyProfile(p_Inp)==FREXT_Hi10P) ||
                      (IdentifyProfile(p_Inp)==FREXT_Hi422) ||
                      (IdentifyProfile(p_Inp)==FREXT_Hi444) ||
                      (IdentifyProfile(p_Inp)==FREXT_CAVLC444)
#if EXT3D
                      || (IdentifyProfile(p_Inp)==MULTIVIEW_HIGH)
                      || (IdentifyProfile(p_Inp)==STEREO_HIGH)
                      || (IdentifyProfile(p_Inp)==ThreeDV_HIGH)
                      || (IdentifyProfile(p_Inp)==ThreeDV_EXTEND_HIGH)
#else
#if (MVC_EXTENSION_ENABLE)
                      || (IdentifyProfile(p_Inp)==MULTIVIEW_HIGH)
                      || (IdentifyProfile(p_Inp)==STEREO_HIGH)
#endif
#endif
                      );
  // *************************************************************************
  // Sequence Parameter Set
  // *************************************************************************
  assert (sps != NULL);
  // Profile and Level should be calculated using the info from the config
  // file.  Calculation is hidden in IndetifyProfile() and IdentifyLevel()
  sps->profile_idc = IdentifyProfile(p_Inp);
  sps->level_idc = IdentifyLevel(p_Inp);

  // needs to be set according to profile
  sps->constrained_set0_flag = FALSE;
  sps->constrained_set1_flag = FALSE;
  sps->constrained_set2_flag = FALSE;

  if ( (sps->level_idc == 9) && !IS_FREXT_PROFILE(sps->profile_idc) ) // Level 1.b
  {
    sps->constrained_set3_flag = TRUE;
    sps->level_idc = 11;
  }
  else if (frext_profile && p_Inp->IntraProfile)
  {
    sps->constrained_set3_flag = TRUE;
  }
  else
  {
    sps->constrained_set3_flag = FALSE;
  }

  // Parameter Set ID hard coded to zero
  sps->seq_parameter_set_id = SPS_id;

  // Fidelity Range Extensions stuff
  sps->bit_depth_luma_minus8   = p_Inp->output.bit_depth[0] - 8;
  sps->bit_depth_chroma_minus8 = p_Inp->output.bit_depth[1] - 8;
  p_Vid->lossless_qpprime_flag = p_Inp->LosslessCoding & 
      (sps->profile_idc==FREXT_Hi444 || sps->profile_idc==FREXT_CAVLC444);

  //! POC stuff:
  //! The following values are hard-coded in init_poc().  Apparently,
  //! the poc implementation covers only a subset of the poc functionality.
  //! Here, the same subset is implemented.  Changes in the POC stuff have
  //! also to be reflected here
  sps->log2_max_frame_num_minus4 = p_Vid->log2_max_frame_num_minus4;
  sps->log2_max_pic_order_cnt_lsb_minus4 = p_Vid->log2_max_pic_order_cnt_lsb_minus4;

  sps->pic_order_cnt_type = p_Inp->pic_order_cnt_type;
  sps->num_ref_frames_in_pic_order_cnt_cycle = p_Vid->num_ref_frames_in_pic_order_cnt_cycle;
  sps->delta_pic_order_always_zero_flag = p_Vid->delta_pic_order_always_zero_flag;
  sps->offset_for_non_ref_pic = p_Vid->offset_for_non_ref_pic;
  sps->offset_for_top_to_bottom_field = p_Vid->offset_for_top_to_bottom_field;

  for (i=0; i<p_Vid->num_ref_frames_in_pic_order_cnt_cycle; i++)
  {
    sps->offset_for_ref_frame[i] = p_Vid->offset_for_ref_frame[i];
  }
  // End of POC stuff

  // Number of Reference Frames
  sps->num_ref_frames = (unsigned char) p_Inp->num_ref_frames;

  //required_frame_num_update_behaviour_flag hardcoded to zero
  sps->gaps_in_frame_num_value_allowed_flag = FALSE;    // double check

  sps->frame_mbs_only_flag = (Boolean) !(p_Inp->PicInterlace || p_Inp->MbInterlace);

  // Picture size, finally a simple one :-)
  sps->pic_width_in_mbs_minus1        = (( p_Inp->output.width[0]  + p_Vid->auto_crop_right) >> 4) -1;
  sps->pic_height_in_map_units_minus1 = (((p_Inp->output.height[0] + p_Vid->auto_crop_bottom) >> 4)/ (2 - sps->frame_mbs_only_flag)) - 1;

  // a couple of flags, simple
  sps->mb_adaptive_frame_field_flag = (Boolean) (FRAME_CODING != p_Inp->MbInterlace);
  sps->direct_8x8_inference_flag = (Boolean) p_Inp->directInferenceFlag;

  // Sequence VUI not implemented, signalled as not present
  sps->vui_parameters_present_flag = (Boolean) ((p_Inp->output.color_model != CM_YUV && p_Inp->output.yuv_format == YUV444) || p_Inp->EnableVUISupport);
  sps->chroma_format_idc = p_Inp->output.yuv_format;
#if EXT3D
  sps->force_yuv400     = p_Inp->force_yuv400;
#endif
  sps->separate_colour_plane_flag = ( sps->chroma_format_idc == YUV444 ) ? p_Inp->separate_colour_plane_flag : 0;

  if ( sps->vui_parameters_present_flag )
    GenerateVUIParameters(sps, p_Inp);

  // Fidelity Range Extensions stuff
  if(frext_profile)
  {
    sps->seq_scaling_matrix_present_flag = (Boolean) (p_Inp->ScalingMatrixPresentFlag&1);
    n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
    for(i=0; i<n_ScalingList; i++)
    {
      if(i<6)
        sps->seq_scaling_list_present_flag[i] = (p_Inp->ScalingListPresentFlag[i]&1);
      else
      {
        if(p_Inp->Transform8x8Mode)
          sps->seq_scaling_list_present_flag[i] = (p_Inp->ScalingListPresentFlag[i]&1);
        else
          sps->seq_scaling_list_present_flag[i] = 0;
      }
      if( sps->seq_scaling_matrix_present_flag == FALSE )
        sps->seq_scaling_list_present_flag[i] = 0;
    }
  }
  else
  {
    sps->seq_scaling_matrix_present_flag = FALSE;
    for(i=0; i<12; i++)
      sps->seq_scaling_list_present_flag[i] = 0;

  }


  if (p_Vid->auto_crop_right || p_Vid->auto_crop_bottom)
  {
    sps->frame_cropping_flag = TRUE;
    sps->frame_cropping_rect_left_offset=0;
    sps->frame_cropping_rect_top_offset=0;
    sps->frame_cropping_rect_right_offset=  (p_Vid->auto_crop_right / SubWidthC[sps->chroma_format_idc]);
    sps->frame_cropping_rect_bottom_offset= (p_Vid->auto_crop_bottom / (SubHeightC[sps->chroma_format_idc] * (2 - sps->frame_mbs_only_flag)));
    if (p_Vid->auto_crop_right % SubWidthC[sps->chroma_format_idc])
    {
      error("automatic frame cropping (width) not possible",500);
    }
    if (p_Vid->auto_crop_bottom % (SubHeightC[sps->chroma_format_idc] * (2 - sps->frame_mbs_only_flag)))
    {
      error("automatic frame cropping (height) not possible",500);
    }
  }
  else
  {
    sps->frame_cropping_flag = FALSE;
  }
#if EXT3D
  if (p_Vid->is_depth)
  {
    if (p_Inp->depth_frame_cropping_flag)
    {
      if (sps->frame_cropping_flag==0)
      {
        sps->frame_cropping_flag = 1;
        sps->frame_cropping_rect_left_offset=p_Inp->depth_frame_crop_left_offset;
        sps->frame_cropping_rect_top_offset=p_Inp->depth_frame_crop_top_offset;
        sps->frame_cropping_rect_right_offset= p_Inp->depth_frame_crop_right_offset;
        sps->frame_cropping_rect_bottom_offset= p_Inp->depth_frame_crop_bottom_offset;
      }else
      {
        sps->frame_cropping_rect_left_offset +=p_Inp->depth_frame_crop_left_offset;
        sps->frame_cropping_rect_top_offset  +=p_Inp->depth_frame_crop_top_offset;
        sps->frame_cropping_rect_right_offset += p_Inp->depth_frame_crop_right_offset;
        sps->frame_cropping_rect_bottom_offset += p_Inp->depth_frame_crop_bottom_offset;
      }
    }
  }
#endif

}

/*!
 ************************************************************************
 * \brief
 *    GeneratePictureParameterSet:
 *    Generates a Picture Parameter Set structure
 * \par
 *    Regarding the QP
 *    The previous software versions coded the absolute QP only in the
 *    slice header.  This is kept, and the offset in the PPS is coded
 *    even if we could save bits by intelligently using this field.
 *
 ************************************************************************
 */
void GeneratePictureParameterSet( pic_parameter_set_rbsp_t *pps, //!< Picture Parameter Set to be filled
                                  seq_parameter_set_rbsp_t *sps, //!< used Sequence Parameter Set
                                  VideoParameters *p_Vid,        //!< the image pointer
                                  InputParameters *p_Inp,        //!< Input configuration parameters 
                                  int PPS_id,                    //!< PPS ID
                                  int WeightedPrediction,        //!< value of weighted_pred_flag
                                  int WeightedBiprediction,      //!< value of weighted_bipred_idc
                                  int cb_qp_index_offset,        //!< value of cb_qp_index_offset
                                  int cr_qp_index_offset         //!< value of cr_qp_index_offset
                                  )
{
  unsigned i;
  unsigned n_ScalingList;
  unsigned FrameSizeInMapUnits = 0;

  int frext_profile = ((IdentifyProfile(p_Inp)==FREXT_HP) ||
                      (IdentifyProfile(p_Inp)==FREXT_Hi10P) ||
                      (IdentifyProfile(p_Inp)==FREXT_Hi422) ||
                      (IdentifyProfile(p_Inp)==FREXT_Hi444) ||
                      (IdentifyProfile(p_Inp)==FREXT_CAVLC444)
#if EXT3D
                      || (IdentifyProfile(p_Inp)==MULTIVIEW_HIGH)
                      || (IdentifyProfile(p_Inp)==STEREO_HIGH)
                      || (IdentifyProfile(p_Inp)==ThreeDV_HIGH)
                      || (IdentifyProfile(p_Inp)==ThreeDV_EXTEND_HIGH)
#else
#if (MVC_EXTENSION_ENABLE)
                      || (IdentifyProfile(p_Inp)==MULTIVIEW_HIGH)
                      || (IdentifyProfile(p_Inp)==STEREO_HIGH)
#endif
#endif
                      );

  // *************************************************************************
  // Picture Parameter Set
  // *************************************************************************

  pps->seq_parameter_set_id = sps->seq_parameter_set_id;
  pps->pic_parameter_set_id = PPS_id;
  pps->entropy_coding_mode_flag = (p_Inp->symbol_mode == CAVLC ? FALSE : TRUE);
  // Fidelity Range Extensions stuff
  if(frext_profile)
  {
    pps->transform_8x8_mode_flag = (p_Inp->Transform8x8Mode ? TRUE:FALSE);
    pps->pic_scaling_matrix_present_flag = (Boolean) ((p_Inp->ScalingMatrixPresentFlag&2)>>1);
    n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
    for(i=0; i<n_ScalingList; i++)
    {
      if(i<6)
        pps->pic_scaling_list_present_flag[i] = (p_Inp->ScalingListPresentFlag[i]&2)>>1;
      else
      {
        if(pps->transform_8x8_mode_flag)
          pps->pic_scaling_list_present_flag[i] = (p_Inp->ScalingListPresentFlag[i]&2)>>1;
        else
          pps->pic_scaling_list_present_flag[i] = 0;
      }
      if( pps->pic_scaling_matrix_present_flag == FALSE )
        pps->pic_scaling_list_present_flag[i] = 0;
    }
  }
  else
  {
    pps->pic_scaling_matrix_present_flag = FALSE;
    for(i=0; i<12; i++)
      pps->pic_scaling_list_present_flag[i] = 0;

    pps->transform_8x8_mode_flag = FALSE;
    p_Inp->Transform8x8Mode = 0;
  }

  // JVT-Fxxx (by Stephan Wenger, make this flag unconditional
  pps->bottom_field_pic_order_in_frame_present_flag = p_Vid->bottom_field_pic_order_in_frame_present_flag;

  // Begin FMO stuff
  pps->num_slice_groups_minus1 = p_Inp->num_slice_groups_minus1;

  //! Following set the parameter for different slice group types
  if (pps->num_slice_groups_minus1 > 0)
  {
     if ((pps->slice_group_id = calloc ((sps->pic_height_in_map_units_minus1+1)*(sps->pic_width_in_mbs_minus1+1), sizeof(byte))) == NULL)
       no_mem_exit ("GeneratePictureParameterSet: slice_group_id");

    switch (p_Inp->slice_group_map_type)
    {
    case 0:
      pps->slice_group_map_type = 0;
      for(i=0; i<=pps->num_slice_groups_minus1; i++)
      {
        pps->run_length_minus1[i]=p_Inp->run_length_minus1[i];
      }
      break;
    case 1:
      pps->slice_group_map_type = 1;
      break;
    case 2:
      // i loops from 0 to num_slice_groups_minus1-1, because no info for background needed
      pps->slice_group_map_type = 2;
      FrameSizeInMapUnits = (sps->pic_height_in_map_units_minus1) * (sps->pic_width_in_mbs_minus1 + 1);
      for(i=0; i<pps->num_slice_groups_minus1; i++)
      {
        // make sure the macroblock map does not lie outside the picture
        pps->top_left[i] = imin (p_Inp->top_left[i], FrameSizeInMapUnits);
        pps->bottom_right[i] = imin (p_Inp->bottom_right[i], FrameSizeInMapUnits);
      }
     break;
    case 3:
    case 4:
    case 5:
      pps->slice_group_map_type = p_Inp->slice_group_map_type;
      pps->slice_group_change_direction_flag = (Boolean) p_Inp->slice_group_change_direction_flag;
      pps->slice_group_change_rate_minus1 = p_Inp->slice_group_change_rate_minus1;
      break;
    case 6:
      pps->slice_group_map_type = 6;
      pps->pic_size_in_map_units_minus1 =
        (((p_Inp->output.height[0] + p_Vid->auto_crop_bottom)/MB_BLOCK_SIZE)/(2-sps->frame_mbs_only_flag))
        *((p_Inp->output.width[0]  + p_Vid->auto_crop_right)/MB_BLOCK_SIZE) -1;

      for (i=0;i<=pps->pic_size_in_map_units_minus1; i++)
        pps->slice_group_id[i] = p_Inp->slice_group_id[i];

      break;
    default:
      printf ("Parset.c: slice_group_map_type invalid, default\n");
      assert (0==1);
    }
  }
// End FMO stuff

  pps->num_ref_idx_l0_active_minus1 = sps->frame_mbs_only_flag ? (sps->num_ref_frames - 1) : (2 * sps->num_ref_frames - 1) ;   // set defaults
  pps->num_ref_idx_l1_active_minus1 = sps->frame_mbs_only_flag ? (sps->num_ref_frames - 1) : (2 * sps->num_ref_frames - 1) ;   // set defaults

  pps->weighted_pred_flag  = (byte) WeightedPrediction;
  pps->weighted_bipred_idc = (byte) WeightedBiprediction;

  pps->pic_init_qp_minus26 = 0;         // hard coded to zero, QP lives in the slice header
  pps->pic_init_qs_minus26 = 0;

  pps->chroma_qp_index_offset = cb_qp_index_offset;
  if (frext_profile)
  {
    pps->cb_qp_index_offset     = cb_qp_index_offset;
    pps->cr_qp_index_offset     = cr_qp_index_offset;
  }
  else
    pps->cb_qp_index_offset = pps->cr_qp_index_offset = pps->chroma_qp_index_offset;

  pps->deblocking_filter_control_present_flag = (Boolean) ((p_Inp->DFSendParameters != 0) || (p_Inp->RDPictureDeblocking != 0));

  pps->constrained_intra_pred_flag = (Boolean) p_Inp->UseConstrainedIntraPred;

  // if redundant slice is in use.
  pps->redundant_pic_cnt_present_flag = (Boolean) p_Inp->redundant_pic_flag;
}

#if EXT3D
void GenerateDepthParameterSet(depth_parameter_set_rbsp_t *dps, VideoParameters *p_Vid, int DPS_id)
{
  InputParameters* p_DepthInp=p_Vid->p_Inp;

  dps->dep_parameter_set_id = DPS_id;
  dps->num_views_minus1 = p_DepthInp->NumOfViews-1;
  dps->frm_no_in_file = p_Vid->p_DualVid->frm_no_in_file;
}

void copy_3dv_acquisition_info(ThreeDVAcquisitionInfo* dst_3dv_acquisition,ThreeDVAcquisitionInfo* src_3dv_acquisition)
{
  memcpy(dst_3dv_acquisition->focal_length_x_ae,src_3dv_acquisition->focal_length_x_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->focal_length_y_ae,src_3dv_acquisition->focal_length_y_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->principal_point_x_ae,src_3dv_acquisition->principal_point_x_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->principal_point_y_ae,src_3dv_acquisition->principal_point_y_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->translation_ae,src_3dv_acquisition->translation_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->depth_near_ae,src_3dv_acquisition->depth_near_ae,sizeof(ThreeDVAE));
  memcpy(dst_3dv_acquisition->depth_far_ae,src_3dv_acquisition->depth_far_ae,sizeof(ThreeDVAE));

  dst_3dv_acquisition->disp_param_flag=src_3dv_acquisition->disp_param_flag;
  memcpy(dst_3dv_acquisition->d_disparity_scale[0],src_3dv_acquisition->d_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memcpy(dst_3dv_acquisition->d_disparity_offset[0],src_3dv_acquisition->d_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(double));
  memcpy(dst_3dv_acquisition->i_disparity_scale[0],src_3dv_acquisition->i_disparity_scale[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dst_3dv_acquisition->i_disparity_offset[0],src_3dv_acquisition->i_disparity_offset[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dst_3dv_acquisition->i_disparity_scale_diff[0],src_3dv_acquisition->i_disparity_scale_diff[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
  memcpy(dst_3dv_acquisition->i_disparity_offset_diff[0],src_3dv_acquisition->i_disparity_offset_diff[0],MAX_CODEVIEW*MAX_CODEVIEW*sizeof(int));
}
#endif

/*!
 *************************************************************************************
 * \brief
 *    syntax for scaling list matrix values
 *
 * \param scalingListinput
 *    input scaling list
 * \param scalingList
 *    scaling list to be used
 * \param sizeOfScalingList
 *    size of the scaling list
 * \param UseDefaultScalingMatrix
 *    usage of default Scaling Matrix
 * \param bitstream
 *    target bitstream for writing syntax
 *
 * \return
 *    size of the RBSP in bytes
 *
 *************************************************************************************
 */
int Scaling_List(short *scalingListinput, short *scalingList, int sizeOfScalingList, short *UseDefaultScalingMatrix, Bitstream *bitstream)
{
  int j, scanj;
  int len=0;
  int delta_scale, lastScale, nextScale;

  lastScale = 8;
  nextScale = 8;

  for(j=0; j<sizeOfScalingList; j++)
  {
    scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

    if(nextScale!=0)
    {
      delta_scale = scalingListinput[scanj]-lastScale; // Calculate delta from the scalingList data from the input file
      if(delta_scale>127)
        delta_scale=delta_scale-256;
      else if(delta_scale<-128)
        delta_scale=delta_scale+256;

      len+=se_v ("   : delta_sl   ",                      delta_scale,                       bitstream);
      nextScale = scalingListinput[scanj];
      *UseDefaultScalingMatrix|=(scanj==0 && nextScale==0); // Check first matrix value for zero
    }

    scalingList[scanj] = (short) ((nextScale==0) ? lastScale:nextScale); // Update the actual scalingList matrix with the correct values
    lastScale = scalingList[scanj];
  }

  return len;
}


/*!
 *************************************************************************************
 * \brief
 *    int GenerateSeq_parameter_set_rbsp (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps, char *rbsp);
 *
 * \param p_Vid
 *    Image parameters structure
 * \param sps
 *    sequence parameter structure
 * \param rbsp
 *    buffer to be filled with the rbsp, size should be at least MAXIMUMPARSETRBSPSIZE
 * \return
 *    size of the RBSP in bytes
 *
 * \note
 *    Sequence Parameter VUI function is called, but the function implements
 *    an exit (-1)
 *************************************************************************************
 */
#if MVC_EXTENSION_ENABLE||EXT3D
int GenerateSeq_parameter_set_rbsp (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps, byte *rbsp, short Is_Subset)
#else
int GenerateSeq_parameter_set_rbsp (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps, byte *rbsp)
#endif
{
  Bitstream *bitstream;
  int len = 0, LenInBytes;
  unsigned i;
  unsigned n_ScalingList;
#if EXT3D
  InputParameters* p_Inp=p_Vid->p_Inp;
  int j=0;
#endif
  assert (rbsp != NULL);

#if EXT3D
  if ((bitstream=(Bitstream*)calloc(1, sizeof(Bitstream)))==NULL) 
    no_mem_exit("SeqParameterSet:bitstream");
#else
  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) no_mem_exit("SeqParameterSet:bitstream");
#endif
  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;
#if EXT3D
  if( Is_Subset)
  {
    assert(sps->profile_idc==MULTIVIEW_HIGH || sps->profile_idc==STEREO_HIGH
      ||sps->profile_idc==ThreeDV_HIGH||sps->profile_idc==ThreeDV_EXTEND_HIGH);
    len+=u_v  (8, "SPS: profile_idc",                             sps->profile_idc,                               bitstream);
  }
  else
  {
    len+=u_v  (8, "SPS: profile_idc",                             100,                               bitstream);
  }
#else
#if (MVC_EXTENSION_ENABLE)
  if(p_Vid->p_Inp->num_of_views==2)
  {
    if(Is_Subset==1)
    {
      assert(sps->profile_idc==MULTIVIEW_HIGH || sps->profile_idc==STEREO_HIGH);
      len+=u_v  (8, "SPS: profile_idc",                             sps->profile_idc,                               bitstream);
    }
    else
      len+=u_v  (8, "SPS: profile_idc",                             100,                               bitstream);
  }
  else
#endif
    len+=u_v  (8, "SPS: profile_idc",                             sps->profile_idc,                               bitstream);
#endif


  len+=u_1  ("SPS: constrained_set0_flag",                      sps->constrained_set0_flag,    bitstream);
  len+=u_1  ("SPS: constrained_set1_flag",                      sps->constrained_set1_flag,    bitstream);
  len+=u_1  ("SPS: constrained_set2_flag",                      sps->constrained_set2_flag,    bitstream);
  len+=u_1  ("SPS: constrained_set3_flag",                      sps->constrained_set3_flag,    bitstream);
  len+=u_v  (4, "SPS: reserved_zero_4bits",                     0,                             bitstream);

  len+=u_v  (8, "SPS: level_idc",                               sps->level_idc,                                 bitstream);


  len+=ue_v ("SPS: seq_parameter_set_id",                    sps->seq_parameter_set_id,                      bitstream);

  // Fidelity Range Extensions stuff
  if( IS_FREXT_PROFILE(sps->profile_idc) )
  {
    len+=ue_v ("SPS: chroma_format_idc",                        sps->chroma_format_idc,                          bitstream);    

    if(p_Vid->yuv_format == YUV444)
      len+=u_1  ("SPS: separate_colour_plane_flag",             sps->separate_colour_plane_flag,                 bitstream);
    len+=ue_v ("SPS: bit_depth_luma_minus8",                    sps->bit_depth_luma_minus8,                      bitstream);
    len+=ue_v ("SPS: bit_depth_chroma_minus8",                  sps->bit_depth_chroma_minus8,                    bitstream);
    len+=u_1  ("SPS: lossless_qpprime_y_zero_flag",             p_Vid->lossless_qpprime_flag,                      bitstream);
    //other chroma info to be added in the future

    len+=u_1 ("SPS: seq_scaling_matrix_present_flag",           sps->seq_scaling_matrix_present_flag,            bitstream);

    if(sps->seq_scaling_matrix_present_flag)
    {
      ScaleParameters *p_QScale = p_Vid->p_QScale;
      n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
      for(i=0; i<n_ScalingList; i++)
      {
        len+=u_1 ("SPS: seq_scaling_list_present_flag",         sps->seq_scaling_list_present_flag[i],           bitstream);
        if(sps->seq_scaling_list_present_flag[i])
        {
          if(i<6)
            len+=Scaling_List(p_QScale->ScalingList4x4input[i], p_QScale->ScalingList4x4[i], 16, &p_QScale->UseDefaultScalingMatrix4x4Flag[i], bitstream);
          else
            len+=Scaling_List(p_QScale->ScalingList8x8input[i-6], p_QScale->ScalingList8x8[i-6], 64, &p_QScale->UseDefaultScalingMatrix8x8Flag[i-6], bitstream);
        }
      }
    }
  }

  len+=ue_v ("SPS: log2_max_frame_num_minus4",               sps->log2_max_frame_num_minus4,                 bitstream);
  len+=ue_v ("SPS: pic_order_cnt_type",                      sps->pic_order_cnt_type,                        bitstream);

  if (sps->pic_order_cnt_type == 0)
    len+=ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4",     sps->log2_max_pic_order_cnt_lsb_minus4,         bitstream);
  else if (sps->pic_order_cnt_type == 1)
  {
    len+=u_1  ("SPS: delta_pic_order_always_zero_flag",        sps->delta_pic_order_always_zero_flag,          bitstream);
    len+=se_v ("SPS: offset_for_non_ref_pic",                  sps->offset_for_non_ref_pic,                    bitstream);
    len+=se_v ("SPS: offset_for_top_to_bottom_field",          sps->offset_for_top_to_bottom_field,            bitstream);
    len+=ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle",   sps->num_ref_frames_in_pic_order_cnt_cycle,     bitstream);
    for (i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
      len+=se_v ("SPS: offset_for_ref_frame",                  sps->offset_for_ref_frame[i],                      bitstream);
  }
  len+=ue_v ("SPS: num_ref_frames",                          sps->num_ref_frames,                            bitstream);
  len+=u_1  ("SPS: gaps_in_frame_num_value_allowed_flag",    sps->gaps_in_frame_num_value_allowed_flag,      bitstream);
  len+=ue_v ("SPS: pic_width_in_mbs_minus1",                 sps->pic_width_in_mbs_minus1,                   bitstream);
  len+=ue_v ("SPS: pic_height_in_map_units_minus1",          sps->pic_height_in_map_units_minus1,            bitstream);
  len+=u_1  ("SPS: frame_mbs_only_flag",                     sps->frame_mbs_only_flag,                       bitstream);
  if (!sps->frame_mbs_only_flag)
  {
    len+=u_1  ("SPS: mb_adaptive_frame_field_flag",            sps->mb_adaptive_frame_field_flag,              bitstream);
  }
  len+=u_1  ("SPS: direct_8x8_inference_flag",               sps->direct_8x8_inference_flag,                 bitstream);

  len+=u_1  ("SPS: frame_cropping_flag",                      sps->frame_cropping_flag,                       bitstream);
  if (sps->frame_cropping_flag)
  {
    len+=ue_v ("SPS: frame_cropping_rect_left_offset",          sps->frame_cropping_rect_left_offset,           bitstream);
    len+=ue_v ("SPS: frame_cropping_rect_right_offset",         sps->frame_cropping_rect_right_offset,          bitstream);
    len+=ue_v ("SPS: frame_cropping_rect_top_offset",           sps->frame_cropping_rect_top_offset,            bitstream);
    len+=ue_v ("SPS: frame_cropping_rect_bottom_offset",        sps->frame_cropping_rect_bottom_offset,         bitstream);
  }

  len+=u_1  ("SPS: vui_parameters_present_flag",             sps->vui_parameters_present_flag,               bitstream);

  if (sps->vui_parameters_present_flag)
    len+=GenerateVUI_parameters_rbsp(sps, bitstream);    // currently a dummy, asserting

#if EXT3D
  if( Is_Subset)
  {
    // profile_idc fixed to 118 only
    len+=u_1  ("SPS: bit_equal_to_one",                                   1,           bitstream);

    // for multi-view with view_id>0
    {
      len+=ue_v ("SPS: num_views_minus1",                                 p_Inp->NumOfViews-1,           bitstream);
      for(i=0;(int)i<p_Inp->NumOfViews;++i)
      {
        len+=ue_v("SPS: view_id",                                     p_Inp->ViewCodingOrder[i],           bitstream);

        if(sps->profile_idc==ThreeDV_HIGH )
        {
          if (p_Vid->is_depth==0)
          {
            len+=u_1 ("SPS: depth_view_present_flag",                     p_Vid->p_DualInp->ViewPresentFlag[i],  bitstream);
            len+=u_1 ("SPS: texture_view_present_flag",                   p_Inp->ViewPresentFlag[i],             bitstream);
          }
          else
          {
            len+=u_1 ("SPS: depth_view_present_flag",                     p_Inp->ViewPresentFlag[i],             bitstream);
            len+=u_1 ("SPS: texture_view_present_flag",                   p_Vid->p_DualInp->ViewPresentFlag[i],  bitstream);
          }
        }
      }

      for(i=1;(int)i<p_Inp->NumOfViews;++i)
      {
        if (sps->profile_idc==ThreeDV_HIGH && p_Inp->ViewPresentFlag[i]==0) continue;

        len+=ue_v ("SPS: num_anchor_refs_l0",                          p_Inp->NumOfFwdAnchorRefs[i],           bitstream);
        for(j=0;j<p_Inp->NumOfFwdAnchorRefs[i];++j)
          len+=ue_v ("SPS: anchor_ref_l0",                          p_Inp->FwdAnchorRefs[i][j],           bitstream);

        len+=ue_v("SPS: num_anchor_refs_l1",                          p_Inp->NumOfBwdAnchorRefs[i],           bitstream);
        for(j=0;j<p_Inp->NumOfBwdAnchorRefs[i];++j)
          len+=ue_v ("SPS: anchor_ref_l1",                          p_Inp->BwdAnchorRefs[i][j],           bitstream);
      }

      for(i=1;(int)i<p_Inp->NumOfViews;++i)
      {
        if (sps->profile_idc==ThreeDV_HIGH && p_Inp->ViewPresentFlag[i]==0) continue;

        len+=ue_v ("SPS: num_non_anchor_refs_l0",            p_Inp->NumOfFwdNoAnchorRefs[i],           bitstream);
        for(j=0;j<p_Inp->NumOfFwdNoAnchorRefs[i];++j)
          len+=ue_v("SPS: non_anchor_refs_l0",            p_Inp->FwdNoAnchorRefs[i][j],           bitstream);
        len+=ue_v ("SPS: num_non_anchor_refs_l1",            p_Inp->NumOfBwdNoAnchorRefs[i],           bitstream);
        for(j=0;j<p_Inp->NumOfBwdNoAnchorRefs[i];++j)
          len+=ue_v ("SPS: non_anchor_ref_l1",            p_Inp->BwdNoAnchorRefs[i][j],           bitstream);
      }     

      len+=ue_v ("SPS: num_level_values_signalled_minus1",                0,           bitstream);
      len+=u_v  (8, "SPS: level_idc[ 0 ]",                                sps->level_idc,           bitstream);
      len+=ue_v ("SPS: num_applicable_ops_minus1[ 0 ]",                   0,           bitstream);
      len+=u_v  (3, "SPS: applicable_op_temporal_id[ 0 ][ 0 ]",           0,           bitstream);
      len+=ue_v ("SPS: applicable_op_num_target_views_minus1[ 0 ][ 0 ]",  0,           bitstream);
      len+=ue_v ("SPS: applicable_op_target_view_id[ 0 ][ 0 ][ 0 ]",      0,           bitstream);
      if (sps->profile_idc==ThreeDV_HIGH || sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {
        len+=u_1 ("SPS: applicable_op_depth_flag[ 0 ][ 0 ][ 0 ]",      0,           bitstream);
        len+=u_1 ("SPS: applicable_op_texture_flag[ 0 ][ 0 ][ 0 ]",      0,           bitstream);
      }
      if (sps->profile_idc==ThreeDV_HIGH || sps->profile_idc==ThreeDV_EXTEND_HIGH)

      {
        len+=ue_v ("SPS: applicable_op_num_texture_views_minus1[ 0 ][ 0 ]",  0,           bitstream);
        len+=ue_v ("SPS: applicable_op_num_depth_views[ 0 ][ 0 ]",           0,           bitstream);
      }
      else
        len+=ue_v ("SPS: applicable_op_num_views_minus1[ 0 ][ 0 ]",         0,           bitstream);
    }

    if (sps->profile_idc==ThreeDV_HIGH||sps->profile_idc==ThreeDV_EXTEND_HIGH)
      len+=u_1  ("SPS: 3dvc_vui_parameters_present_flag",                    sps->vui_parameters_present_flag,           bitstream);
    else
      len+=u_1  ("SPS: mvc_vui_parameters_present_flag",                    sps->vui_parameters_present_flag,           bitstream);

    if (sps->vui_parameters_present_flag)
    {
      len+=ue_v ("SPS: vui_mvc_num_ops_minus1",                           0,           bitstream);
      len+=u_v  (3, "SPS: vui_mvc_temporal_id[ 0 ]",                      0,           bitstream);
      len+=ue_v ("SPS: vui_mvc_num_target_output_views_minus1[ 0 ]",      0,           bitstream);
      len+=ue_v ("SPS: vui_mvc_view_id[ 0 ][ 0 ]",                        0,           bitstream);

      if (sps->profile_idc==ThreeDV_HIGH || sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {
        len+=u_1  ("SPS: vui_mvcd_depth_flag[ 0 ][ 0 ]",                  0,           bitstream);
        len+=u_1  ("SPS: vui_mvcd_texture_flag[ 0 ][ 0 ]",                0,           bitstream);
      }

      len+=u_1  ("SPS: vui_mvc_timing_info_present_flag[ 0 ]",            sps->vui_parameters_present_flag,               bitstream);
      if(sps->vui_parameters_present_flag)
      {
        len+=u_v (32,"SPS: vui_mvc_num_units_in_tick[ 0 ]",               sps->vui_seq_parameters.num_units_in_tick,      bitstream);
        len+=u_v (32,"SPS: vui_mvc_time_scale[ 0 ]",                      sps->vui_seq_parameters.time_scale,             bitstream);
        len+=u_1 ("SPS: vui_mvc_fixed_frame_rate_flag[ 0 ]",              sps->vui_seq_parameters.fixed_frame_rate_flag,  bitstream);
      }
      len+=u_1  ("SPS: vui_mvc_nal_hrd_parameters_present_flag[ 0 ]",     sps->vui_seq_parameters.nal_hrd_parameters_present_flag,           bitstream);
      if(sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
      {
        len += WriteHRDParameters(sps, bitstream);
      }
      len+=u_1 ("SPS: vui_mvc_vcl_hrd_parameters_present_flag[ 0 ]",      sps->vui_seq_parameters.vcl_hrd_parameters_present_flag,           bitstream);
      if ( sps->vui_seq_parameters.vcl_hrd_parameters_present_flag )
      {
        len += WriteHRDParameters(sps, bitstream);
      }
      if ( sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag )
      {
        len+=u_1 ("SPS: vui_mvc_low_delay_hrd_flag[ 0 ]",                 sps->vui_seq_parameters.low_delay_hrd_flag,                        bitstream );
      }
      len+=u_1 ("SPS: vui_mvc_pic_struct_present_flag[ 0 ]",              sps->vui_seq_parameters.pic_struct_present_flag,                   bitstream);
    }

    if (sps->profile_idc==ThreeDV_HIGH || sps->profile_idc==ThreeDV_EXTEND_HIGH)
    {
      len+=u_1  ("SPS: texture_vui_parameters_present_flag",                    0,           bitstream);
    }

    if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
    {
      VideoParameters *p_DepthVid = p_Vid->is_depth ? p_Vid : p_Vid->p_DualVid;
      VideoParameters *p_TextVid  = p_DepthVid->p_DualVid;
      InputParameters *p_TextInp  = p_TextVid->p_Inp;
      InputParameters *p_DepthInp = p_DepthVid->p_Inp;

      int k=0;
      len+=u_v(2,"3dv_acquisition_info:3dv_acquisition_idx",p_DepthInp->AcquisitionIdx,bitstream);
      for(k=0;k<p_DepthInp->NumOfViews;++k)
        len+=ue_v("3dv_acquisition_info:camera_order",p_DepthInp->CamOrder[k],bitstream);
      if(p_DepthInp->AcquisitionIdx)
      {
        len+=ue_v("3dv_acquisition_info: disparity parameter precision",p_DepthInp->DisparityParamPrec,bitstream);
        len+=encode_3dv_acquisition_info(p_DepthVid->ThreeDV_acquisition_info,bitstream, 1, 0);
        len+=encode_3dv_acquisition_info(p_DepthVid->ThreeDV_acquisition_info,bitstream, 1, 1);
      }

      if(sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {

        int num_of_grid_pos_view;
        int i;

        if (p_DepthVid->depth_width_padded<p_TextVid->width || p_DepthVid->depth_height_padded<p_TextVid->height)
        {
          int mb_div=(p_DepthInp->PicInterlace || p_DepthInp->MbInterlace) ? 2: 1;
          len+=u_1("SPS: reduced_resolution",  1,   bitstream);

          len+=ue_v("SPS: depth_pic_width_in_mbs_minus1",   p_DepthVid->depth_width_padded/MB_BLOCK_SIZE -1,bitstream);
          len+=ue_v("SPS: depth_pic_height_in_map_units_minus1",     (p_DepthVid->depth_height_padded/mb_div)/MB_BLOCK_SIZE-1,  bitstream);
          len+=ue_v("SPS: depth_hor_mult_minus1",     p_DepthVid->depth_hor_mult-1,  bitstream);
          len+=ue_v("SPS: depth_ver_mult_minus1",     p_DepthVid->depth_ver_mult-1,  bitstream);
          len+=ue_v("SPS: depth_hor_rsh",     p_DepthVid->depth_hor_rsh,  bitstream);
          len+=ue_v("SPS: depth_ver_rsh",     p_DepthVid->depth_ver_rsh,  bitstream);

        }else
        {
          len+=u_1("SPS: reduced_resolution",  0,   bitstream);
        }

        len+=u_1("SPS: depth_frame_cropping_flag",  p_DepthInp->depth_frame_cropping_flag,   bitstream);
        if (p_DepthInp->depth_frame_cropping_flag)
        {
          len+=ue_v("SPS: depth_frame_crop_left_offset",     sps->frame_cropping_rect_left_offset,  bitstream);
          len+=ue_v("SPS: depth_frame_crop_right_offset",     sps->frame_cropping_rect_right_offset,  bitstream);
          len+=ue_v("SPS: depth_frame_crop_top_offset",     sps->frame_cropping_rect_top_offset,  bitstream);
          len+=ue_v("SPS: depth_frame_crop_bottom_offset",     sps->frame_cropping_rect_bottom_offset,  bitstream);
        }

        num_of_grid_pos_view=0;
        for(i=0;i<MAX_CODEVIEW+1;i++)
        {
          if ( p_DepthInp->grid_pos_x[i] || p_DepthInp->grid_pos_y[i] )
          {
            num_of_grid_pos_view++;
          }
        }

        len+=ue_v("SPS: grid_pos_num_views",     num_of_grid_pos_view ,  bitstream);

        for(i=0;i<MAX_CODEVIEW+1;i++)
        {
          if ( p_DepthInp->grid_pos_x[i] || p_DepthInp->grid_pos_y[i] )
          {
            len+=ue_v("SPS: grid_pos_view_id",     i ,  bitstream);
            len+=se_v ("SPS: grid_pos_x",   p_DepthInp->grid_pos_x[i],     bitstream);
            len+=se_v ("SPS: grid_pos_y",   p_DepthInp->grid_pos_y[i],     bitstream);
          }
        }

        len+=u_1("SPS: slice_header_prediction",                      p_DepthInp->SliceHeaderPred,                 bitstream);

        len+=u_1("SPS: seq_view_synthesis_flag ",                              p_TextInp->VSP_Enable,                 bitstream);
      }
      if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {
        len+=u_1("SPS: enable_alc_encoder_flag",                p_TextInp->AdaptiveLuminanceCompensation,   bitstream);
      }

      if (sps->profile_idc==ThreeDV_EXTEND_HIGH)
      {
        len+=u_1("SPS: enable RLESkip flag",                    p_TextInp->RLESkip,   bitstream);
      }
    }

    if((sps->profile_idc==ThreeDV_HIGH)||(sps->profile_idc==ThreeDV_EXTEND_HIGH))
    {
      len+=u_1 ("SPS: additional_extension3_flag",                          0,           bitstream);
    }
    else
      len+=u_1 ("SPS: additional_extension2_flag",                          0,           bitstream);
  }
#else
#if (MVC_EXTENSION_ENABLE)
  if(Is_Subset==1)
  {
    // profile_idc fixed to 118 only
    len+=u_1  ("SPS: bit_equal_to_one",                                   1,           bitstream);

    // fixed to 2 views only
    {
      len+=ue_v ("SPS: num_views_minus1",                                 1,           bitstream);
      if ( p_Vid->p_Inp->MVCFlipViews )
      {
        len+=ue_v ("SPS: view_id[ 0 ]",                                     1,           bitstream);
        len+=ue_v ("SPS: view_id[ 1 ]",                                     0,           bitstream);

        len+=ue_v ("SPS: num_anchor_refs_l0[ 1 ]",                          1,           bitstream);
        len+=ue_v ("SPS: anchor_ref_l0[ 1 ][ 0 ]",                          1,           bitstream);
        len+=ue_v ("SPS: num_anchor_refs_l1[ 1 ]",                          1,           bitstream);
        len+=ue_v ("SPS: anchor_ref_l1[ 1 ][ 0 ]",                          1,           bitstream);

        len+=ue_v ("SPS: num_non_anchor_refs_l0[ 1 ]",                      1,           bitstream);
        len+=ue_v ("SPS: non_anchor_ref_l0[ 1 ][ 0 ]",                      1,           bitstream);
        len+=ue_v ("SPS: num_non_anchor_refs_l1[ 1 ]",                      1,           bitstream);
        len+=ue_v ("SPS: non_anchor_ref_l1[ 1 ][ 0 ]",                      1,           bitstream);
      }
      else
      {
        len+=ue_v ("SPS: view_id[ 0 ]",                                     0,           bitstream);
        len+=ue_v ("SPS: view_id[ 1 ]",                                     1,           bitstream);

        len+=ue_v ("SPS: num_anchor_refs_l0[ 1 ]",                          1,           bitstream);
        len+=ue_v ("SPS: anchor_ref_l0[ 1 ][ 0 ]",                          0,           bitstream);
        len+=ue_v ("SPS: num_anchor_refs_l1[ 1 ]",                          1,           bitstream);
        len+=ue_v ("SPS: anchor_ref_l1[ 1 ][ 0 ]",                          0,           bitstream);

        len+=ue_v ("SPS: num_non_anchor_refs_l0[ 1 ]",                      1,           bitstream);
        len+=ue_v ("SPS: non_anchor_ref_l0[ 1 ][ 0 ]",                      0,           bitstream);
        len+=ue_v ("SPS: num_non_anchor_refs_l1[ 1 ]",                      1,           bitstream);
        len+=ue_v ("SPS: non_anchor_ref_l1[ 1 ][ 0 ]",                      0,           bitstream);
      }

      len+=ue_v ("SPS: num_level_values_signalled_minus1",                0,           bitstream);
      len+=u_v  (8, "SPS: level_idc[ 0 ]",                                sps->level_idc,           bitstream);
      len+=ue_v ("SPS: num_applicable_ops_minus1[ 0 ]",                   0,           bitstream);
      len+=u_v  (3, "SPS: applicable_op_temporal_id[ 0 ][ 0 ]",           0,           bitstream);
      len+=ue_v ("SPS: applicable_op_num_target_views_minus1[ 0 ][ 0 ]",  0,           bitstream);
      len+=ue_v ("SPS: applicable_op_target_view_id[ 0 ][ 0 ][ 0 ]",      0,           bitstream);
      len+=ue_v ("SPS: applicable_op_num_views_minus1[ 0 ][ 0 ]",         0,           bitstream);
    }

    len+=u_1  ("SPS: mvc_vui_parameters_present_flag",                    sps->vui_parameters_present_flag,           bitstream);
    {
      len+=ue_v ("SPS: vui_mvc_num_ops_minus1",                           0,           bitstream);
      len+=u_v  (3, "SPS: vui_mvc_temporal_id[ 0 ]",                      0,           bitstream);
      len+=ue_v ("SPS: vui_mvc_num_target_output_views_minus1[ 0 ]",      0,           bitstream);
      len+=ue_v ("SPS: vui_mvc_view_id[ 0 ][ 0 ]",                        0,           bitstream);
      len+=u_1  ("SPS: vui_mvc_timing_info_present_flag[ 0 ]",            sps->vui_parameters_present_flag,               bitstream);
      if(sps->vui_parameters_present_flag)
      {
        len+=u_v (32,"SPS: vui_mvc_num_units_in_tick[ 0 ]",               sps->vui_seq_parameters.num_units_in_tick,      bitstream);
        len+=u_v (32,"SPS: vui_mvc_time_scale[ 0 ]",                      sps->vui_seq_parameters.time_scale,             bitstream);
        len+=u_1 ("SPS: vui_mvc_fixed_frame_rate_flag[ 0 ]",              sps->vui_seq_parameters.fixed_frame_rate_flag,  bitstream);
      }
      len+=u_1  ("SPS: vui_mvc_nal_hrd_parameters_present_flag[ 0 ]",     sps->vui_seq_parameters.nal_hrd_parameters_present_flag,           bitstream);
      if(sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
      {
        len += WriteHRDParameters(sps, bitstream);
      }
      len+=u_1 ("SPS: vui_mvc_vcl_hrd_parameters_present_flag[ 0 ]",      sps->vui_seq_parameters.vcl_hrd_parameters_present_flag,           bitstream);
      if ( sps->vui_seq_parameters.vcl_hrd_parameters_present_flag )
      {
        len += WriteHRDParameters(sps, bitstream);
      }
      if ( sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag )
      {
        len+=u_1 ("SPS: vui_mvc_low_delay_hrd_flag[ 0 ]",                 sps->vui_seq_parameters.low_delay_hrd_flag,                        bitstream );
      }
      len+=u_1 ("SPS: vui_mvc_pic_struct_present_flag[ 0 ]",              sps->vui_seq_parameters.pic_struct_present_flag,                   bitstream);
    }

    len+=u_1 ("SPS: additional_extension2_flag",                          0,           bitstream);
  }
#endif
#endif

  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  free (bitstream);

  return LenInBytes;
}


/*!
 ***********************************************************************************************
 * \brief
 *    int GeneratePic_parameter_set_rbsp (VideoParameters *p_Vid, pic_parameter_set_rbsp_t *pps, char *rbsp);
 *
 * \param p_Vid
 *    picture parameter structure
 * \param pps
 *    picture parameter structure
 * \param rbsp
 *    buffer to be filled with the rbsp, size should be at least MAXIMUMPARSETRBSPSIZE
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *    Picture Parameter VUI function is called, but the function implements
 *    an exit (-1)
 ************************************************************************************************
 */

int GeneratePic_parameter_set_rbsp (VideoParameters *p_Vid, pic_parameter_set_rbsp_t *pps, byte *rbsp)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  Bitstream *bitstream;
  int len = 0, LenInBytes;
  unsigned i;
  unsigned n_ScalingList;
  unsigned NumberBitsPerSliceGroupId;
  int profile_idc;

  assert (rbsp != NULL);

  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) no_mem_exit("PicParameterSet:bitstream");

  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;

  pps->bottom_field_pic_order_in_frame_present_flag = p_Vid->bottom_field_pic_order_in_frame_present_flag;

  {
  len+=ue_v ("PPS: pic_parameter_set_id",                    pps->pic_parameter_set_id,                      bitstream);
  len+=ue_v ("PPS: seq_parameter_set_id",                    pps->seq_parameter_set_id,                      bitstream);
  }
  len+=u_1  ("PPS: entropy_coding_mode_flag",                pps->entropy_coding_mode_flag,                  bitstream);
  len+=u_1  ("PPS: bottom_field_pic_order_in_frame_present_flag", pps->bottom_field_pic_order_in_frame_present_flag,                    bitstream);
  len+=ue_v ("PPS: num_slice_groups_minus1",                 pps->num_slice_groups_minus1,                   bitstream);

  // FMO stuff
  if(pps->num_slice_groups_minus1 > 0 )
  {
    len+=ue_v ("PPS: slice_group_map_type",                 pps->slice_group_map_type,                   bitstream);
    if (pps->slice_group_map_type == 0)
      for (i=0; i<=pps->num_slice_groups_minus1; i++)
        len+=ue_v ("PPS: run_length_minus1[i]",                           pps->run_length_minus1[i],                             bitstream);
    else if (pps->slice_group_map_type==2)
      for (i=0; i<pps->num_slice_groups_minus1; i++)
      {

        len+=ue_v ("PPS: top_left[i]",                          pps->top_left[i],                           bitstream);
        len+=ue_v ("PPS: bottom_right[i]",                      pps->bottom_right[i],                       bitstream);
      }
    else if (pps->slice_group_map_type == 3 ||
             pps->slice_group_map_type == 4 ||
             pps->slice_group_map_type == 5)
    {
      len+=u_1  ("PPS: slice_group_change_direction_flag",         pps->slice_group_change_direction_flag,         bitstream);
      len+=ue_v ("PPS: slice_group_change_rate_minus1",            pps->slice_group_change_rate_minus1,            bitstream);
    }
    else if (pps->slice_group_map_type == 6)
    {
      if (pps->num_slice_groups_minus1>=4)
        NumberBitsPerSliceGroupId=3;
      else if (pps->num_slice_groups_minus1>=2)
        NumberBitsPerSliceGroupId=2;
      else if (pps->num_slice_groups_minus1>=1)
        NumberBitsPerSliceGroupId=1;
      else
        NumberBitsPerSliceGroupId=0;

      len+=ue_v ("PPS: pic_size_in_map_units_minus1",                       pps->pic_size_in_map_units_minus1,             bitstream);
      for(i=0; i<=pps->pic_size_in_map_units_minus1; i++)
        len+= u_v  (NumberBitsPerSliceGroupId, "PPS: >slice_group_id[i]",   pps->slice_group_id[i],                        bitstream);
    }
  }
  // End of FMO stuff

  len+=ue_v ("PPS: num_ref_idx_l0_active_minus1",             pps->num_ref_idx_l0_active_minus1,              bitstream);
  len+=ue_v ("PPS: num_ref_idx_l1_active_minus1",             pps->num_ref_idx_l1_active_minus1,              bitstream);
  len+=u_1  ("PPS: weighted_pred_flag",                       pps->weighted_pred_flag,                        bitstream);
  len+=u_v  (2, "PPS: weighted_bipred_idc",                   pps->weighted_bipred_idc,                       bitstream);
  len+=se_v ("PPS: pic_init_qp_minus26",                      pps->pic_init_qp_minus26,                       bitstream);
  len+=se_v ("PPS: pic_init_qs_minus26",                      pps->pic_init_qs_minus26,                       bitstream);

  profile_idc = IdentifyProfile(p_Inp);
  if( IS_FREXT_PROFILE(profile_idc) )
    len+=se_v ("PPS: chroma_qp_index_offset",                 pps->cb_qp_index_offset,                        bitstream);
  else
    len+=se_v ("PPS: chroma_qp_index_offset",                 pps->chroma_qp_index_offset,                    bitstream);

  len+=u_1  ("PPS: deblocking_filter_control_present_flag",   pps->deblocking_filter_control_present_flag,    bitstream);
  len+=u_1  ("PPS: constrained_intra_pred_flag",              pps->constrained_intra_pred_flag,               bitstream);
  len+=u_1  ("PPS: redundant_pic_cnt_present_flag",           pps->redundant_pic_cnt_present_flag,            bitstream);

  // Fidelity Range Extensions stuff
  if( IS_FREXT_PROFILE(profile_idc) )
  {
    len+=u_1  ("PPS: transform_8x8_mode_flag",                pps->transform_8x8_mode_flag,                   bitstream);

    len+=u_1  ("PPS: pic_scaling_matrix_present_flag",        pps->pic_scaling_matrix_present_flag,           bitstream);

    if(pps->pic_scaling_matrix_present_flag)
    {
      ScaleParameters *p_QScale = p_Vid->p_QScale;
      n_ScalingList = 6 + ((p_Vid->active_sps->chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag;
      for(i=0; i<n_ScalingList; i++)  // SS-70226
      {
        len+=u_1  ("PPS: pic_scaling_list_present_flag",      pps->pic_scaling_list_present_flag[i],          bitstream);

        if(pps->pic_scaling_list_present_flag[i])
        {
          if(i<6)
            len+=Scaling_List(p_QScale->ScalingList4x4input[i], p_QScale->ScalingList4x4[i], 16, &p_QScale->UseDefaultScalingMatrix4x4Flag[i], bitstream);
          else
            len+=Scaling_List(p_QScale->ScalingList8x8input[i-6], p_QScale->ScalingList8x8[i-6], 64, &p_QScale->UseDefaultScalingMatrix8x8Flag[i-6], bitstream);
        }
      }
    }
    len+=se_v ("PPS: second_chroma_qp_index_offset",          pps->cr_qp_index_offset,                        bitstream);
  }

  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  // Get rid of the helper structures
  free (bitstream);

  return LenInBytes;
}



#if EXT3D
int  GenerateDep_parameter_set_rbsp (VideoParameters *p_Vid, depth_parameter_set_rbsp_t *dps, byte *rbsp)
{
  Bitstream *bitstream;
  int len = 0, LenInBytes;
  ThreeDVAcquisitionInfo* ThreeDV_acquisition_info = p_Vid->is_depth==1 ? p_Vid->ThreeDV_acquisition_info : p_Vid->p_DualVid->ThreeDV_acquisition_info;

  int k;
  InputParameters *p_DepthInp=p_Vid->is_depth==1 ? p_Vid->p_Inp : p_Vid->p_DualInp;

  assert (rbsp != NULL);

  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) 
    no_mem_exit("CamParameterSet:bitstream");

  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;

  len+=ue_v ("DPS: dep_parameter_set_id",              dps->dep_parameter_set_id,                      bitstream);
  len+=u_v  (2,"3dv_acquisition_info:pre_direcion",ThreeDV_acquisition_info->pred_direction,bitstream);
  if(ThreeDV_acquisition_info->pred_direction==0 || ThreeDV_acquisition_info->pred_direction==1){
    len+=ue_v("3dv_acquisition_info:ref_element_id0",ThreeDV_acquisition_info->ref_element_id0,bitstream);
  }
  if(ThreeDV_acquisition_info->pred_direction==0){
    len+=ue_v("3dv_acquisition_info:ref_element_id1",ThreeDV_acquisition_info->ref_element_id1,bitstream);
    len+=u_v(6,"3dv_acquisition_info:pred_weight0",ThreeDV_acquisition_info->pred_weight0,bitstream);
  }

  len+=ue_v ("DPS: num_views_minus1",                     dps->num_views_minus1,                      bitstream);

  len+=encode_3dv_acquisition_info(ThreeDV_acquisition_info,bitstream, 0, 0);
  len+=u_1 ("DPS: vsp_param_flag",  1, bitstream);
  len+=encode_3dv_acquisition_info(ThreeDV_acquisition_info,bitstream, 0, 1);

  len+=u_1("SPS: depth_param_additional_extension_flag", 0,   bitstream);
  
  len+=ue_v("SPS: nonlinear_depth_num", p_DepthInp->NonlinearDepthNum,   bitstream);

  p_DepthInp->NonlinearDepthPoints[0]=0; // beginning
  for (k=1; k<=p_DepthInp->NonlinearDepthNum; ++k)
  {
    len+=ue_v("SPS: nonlinear_depth_point", p_DepthInp->NonlinearDepthPoints[k],                 bitstream);
  }
  p_DepthInp->NonlinearDepthPoints[p_DepthInp->NonlinearDepthNum+1]=0; // end

  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  // Get rid of the helper structures
  free (bitstream);

  return LenInBytes;
}
#endif

/*!
 *************************************************************************************
 * \brief
 *    Returns the Profile
 *
 * \return
 *    Profile according to Annex A
 *
 * \note
 *    Function is currently a dummy.  Should "calculate" the profile from those
 *    config file parameters.  E.g.
 *
 *    Profile = Baseline;
 *    if (CABAC Used || Interlace used) Profile=Main;
 *    if (!Cabac Used) && (Bframes | SPframes) Profile = Streaming;
 *
 *************************************************************************************
 */
int IdentifyProfile(InputParameters *p_Inp)
{
  return p_Inp->ProfileIDC;
}

/*!
 *************************************************************************************
 * \brief
 *    Returns the Level
 *
 * \return
 *    Level according to Annex A
 *
 * \note
 *    This function is currently a dummy, but should calculate the level out of
 *    the config file parameters (primarily the picture size)
 *************************************************************************************
 */
int IdentifyLevel(InputParameters *p_Inp)
{
  return p_Inp->LevelIDC;
}


/*!
 *************************************************************************************
 * \brief
 *    Function body for VUI Parameter generation (to be done)
 *
 * \return
 *    exits with error message
 *************************************************************************************
 */
static int GenerateVUI_parameters_rbsp(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream)
{
  int len=0;
  vui_seq_parameters_t *vui_seq_parameters = &(sps->vui_seq_parameters);

  len+=u_1 ("VUI: aspect_ratio_info_present_flag", vui_seq_parameters->aspect_ratio_info_present_flag, bitstream);
  if (vui_seq_parameters->aspect_ratio_info_present_flag)
  {        
    len+=u_v (8,"VUI: aspect_ratio_idc", vui_seq_parameters->aspect_ratio_idc, bitstream);
    if (vui_seq_parameters->aspect_ratio_idc == 255)
    {
      len+=u_v (16,"VUI: sar_width",  vui_seq_parameters->sar_width,  bitstream);
      len+=u_v (16,"VUI: sar_height", vui_seq_parameters->sar_height, bitstream);
    }
  }  

  len+=u_1 ("VUI: overscan_info_present_flag", vui_seq_parameters->overscan_info_present_flag, bitstream);
  if (vui_seq_parameters->overscan_info_present_flag)
  {
    len+=u_1 ("VUI: overscan_appropriate_flag", vui_seq_parameters->overscan_appropriate_flag, bitstream);
  } 

  len+=u_1 ("VUI: video_signal_type_present_flag", vui_seq_parameters->video_signal_type_present_flag, bitstream);
  if (vui_seq_parameters->video_signal_type_present_flag)
  {
    len+=u_v (3,"VUI: video_format", vui_seq_parameters->video_format, bitstream);
    len+=u_1 ("VUI: video_full_range_flag", vui_seq_parameters->video_full_range_flag, bitstream);
    len+=u_1 ("VUI: colour_description_present_flag", vui_seq_parameters->colour_description_present_flag, bitstream);
    if (vui_seq_parameters->colour_description_present_flag)
    {
      len+=u_v (8,"VUI: colour_primaries", vui_seq_parameters->colour_primaries, bitstream);
      len+=u_v (8,"VUI: transfer_characteristics", vui_seq_parameters->transfer_characteristics, bitstream);
      len+=u_v (8,"VUI: matrix_coefficients", vui_seq_parameters->matrix_coefficients, bitstream);
    }
  }

  len+=u_1 ("VUI: chroma_loc_info_present_flag", vui_seq_parameters->chroma_location_info_present_flag, bitstream);
  if (vui_seq_parameters->chroma_location_info_present_flag)
  {
    len+=ue_v ("VUI: chroma_sample_loc_type_top_field", vui_seq_parameters->chroma_sample_loc_type_top_field, bitstream);
    len+=ue_v ("VUI: chroma_sample_loc_type_bottom_field", vui_seq_parameters->chroma_sample_loc_type_bottom_field, bitstream);
  }

  len+=u_1 ("VUI: timing_info_present_flag", vui_seq_parameters->timing_info_present_flag, bitstream);
  // timing parameters
  if (vui_seq_parameters->timing_info_present_flag)
  {
    len+=u_v (32,"VUI: num_units_in_tick",  vui_seq_parameters->num_units_in_tick, bitstream);
    len+=u_v (32,"VUI: time_scale",         vui_seq_parameters->time_scale, bitstream);
    len+=u_1 ("VUI: fixed_frame_rate_flag", vui_seq_parameters->fixed_frame_rate_flag, bitstream);
  }
  // end of timing parameters
  // nal_hrd_parameters_present_flag
  len+=u_1 ("VUI: nal_hrd_parameters_present_flag", vui_seq_parameters->nal_hrd_parameters_present_flag, bitstream);
  if ( vui_seq_parameters->nal_hrd_parameters_present_flag )
  {
    len += WriteHRDParameters(sps, bitstream);
  }
  // vcl_hrd_parameters_present_flag
  len+=u_1 ("VUI: vcl_hrd_parameters_present_flag", vui_seq_parameters->vcl_hrd_parameters_present_flag, bitstream);
  if ( vui_seq_parameters->vcl_hrd_parameters_present_flag )
  {
    len += WriteHRDParameters(sps, bitstream);
  }
  if ( vui_seq_parameters->nal_hrd_parameters_present_flag || vui_seq_parameters->vcl_hrd_parameters_present_flag )
  {
    len+=u_1 ("VUI: low_delay_hrd_flag", vui_seq_parameters->low_delay_hrd_flag, bitstream );
  }
  len+=u_1 ("VUI: pic_struct_present_flag", vui_seq_parameters->pic_struct_present_flag, bitstream);

  len+=u_1 ("VUI: bitstream_restriction_flag", vui_seq_parameters->bitstream_restriction_flag, bitstream);
  if (vui_seq_parameters->bitstream_restriction_flag)
  {
    len+=u_1  ("VUI: motion_vectors_over_pic_boundaries_flag", vui_seq_parameters->motion_vectors_over_pic_boundaries_flag, bitstream);
    len+=ue_v ("VUI: max_bytes_per_pic_denom", vui_seq_parameters->max_bytes_per_pic_denom, bitstream);
    len+=ue_v ("VUI: max_bits_per_mb_denom", vui_seq_parameters->max_bits_per_mb_denom, bitstream);
    len+=ue_v ("VUI: log2_max_mv_length_horizontal", vui_seq_parameters->log2_max_mv_length_horizontal, bitstream);
    len+=ue_v ("VUI: log2_max_mv_length_vertical", vui_seq_parameters->log2_max_mv_length_vertical, bitstream);
    len+=ue_v ("VUI: num_reorder_frames", vui_seq_parameters->num_reorder_frames, bitstream);
    len+=ue_v ("VUI: max_dec_frame_buffering", vui_seq_parameters->max_dec_frame_buffering, bitstream);
  }

  return len;
}

/*!
 *************************************************************************************
 * \brief
 *    Function body for SEI message NALU generation
 *
 * \return
 *    A NALU containing the SEI messages
 *
 *************************************************************************************
 */
NALU_t *GenerateSEImessage_NALU(InputParameters *p_Inp)
{
  NALU_t *n = AllocNALU(MAXNALUSIZE);
  int RBSPlen = 0;
  //int NALUlen;
  byte rbsp[MAXRBSPSIZE];

  RBSPlen = GenerateSEImessage_rbsp (p_Inp, NORMAL_SEI, rbsp);

  //NALUlen = 
        RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SEI, NALU_PRIORITY_DISPOSABLE, 1);
  n->startcodeprefix_len = 4;

  return n;
}


/*!
 *************************************************************************************
 * \brief
 *    int GenerateSEImessage_rbsp (int, bufferingperiod_information_struct*, char*)
 *
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *************************************************************************************
 */
int GenerateSEImessage_rbsp (InputParameters *p_Inp, int id, byte *rbsp)
{
  Bitstream *bitstream;

  int len = 0, LenInBytes;
  assert (rbsp != NULL);

  UNREFERENCED_PARAMETER(id);

  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) 
    no_mem_exit("SeqParameterSet:bitstream");

  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;

  {
    char sei_message[500] = "";
    char uuid_message[9] = "Random"; // This is supposed to be Random
    unsigned int i, message_size = (unsigned int) strlen(p_Inp->SEIMessageText);
    TIME_T start_time;
    gettime(&start_time);    // start time

    if (message_size == 0)
    {
      message_size = 13;
      strncpy(sei_message,"Empty Message",message_size);
    }
    else
      strncpy(sei_message,p_Inp->SEIMessageText,message_size);

    len+=u_v (8,"SEI: last_payload_type_byte", 5, bitstream);
    message_size += 17;
    while (message_size > 254)
    {
      len+=u_v (8,"SEI: ff_byte",255, bitstream);
      message_size -= 255;
    }
    len+=u_v (8,"SEI: last_payload_size_byte",message_size, bitstream);

    // Lets randomize uuid based on time
#ifdef _WIN32
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) start_time.HighPart, bitstream);
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) start_time.QuadPart, bitstream);
#else
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) start_time.tv_sec, bitstream);
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) start_time.tv_usec, bitstream);
#endif
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) (uuid_message[0] << 24) + (uuid_message[1] << 16)  + (uuid_message[2] << 8) + (uuid_message[3] << 0), bitstream);
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) (uuid_message[4] << 24) + (uuid_message[5] << 16)  + (uuid_message[6] << 8) + (uuid_message[7] << 0), bitstream);
    for (i = 0; i < strlen(sei_message); i++)
      len+=u_v (8,"SEI: user_data_payload_byte",sei_message[i], bitstream);

    len+=u_v (8,"SEI: user_data_payload_byte",   0, bitstream);
  }
  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  free(bitstream);
  return LenInBytes;
}

#if EXT3D
static int check_disp_param_flag(ThreeDVAcquisitionInfo* curr_3dv_acquisition_info, ThreeDVAcquisitionInfo* sps_3dv_acquisition_info)
{
  int i,j;
  int num_of_views=curr_3dv_acquisition_info->num_views;

  for (i=0;i<num_of_views;i++)
  {
    for (j=0;j<num_of_views;j++)
    {
      if ((curr_3dv_acquisition_info->i_disparity_scale[i][j] != sps_3dv_acquisition_info->i_disparity_scale[i][j])
        || (curr_3dv_acquisition_info->i_disparity_offset[i][j] != sps_3dv_acquisition_info->i_disparity_offset[i][j]))
      {
        return 1;
      }
    }
  }
  return 0;
}

static void PrepareDisparityElement(ThreeDVAcquisitionInfo* curr_3dv_acquisition_info,ThreeDVAcquisitionInfo* forward_3dv_acquisition_info,ThreeDVAcquisitionInfo* backward_3dv_acquisition_info)
{
  int32 i,j;
  int32 num_of_views=curr_3dv_acquisition_info->num_views;
  int32 pred_direction=curr_3dv_acquisition_info->pred_direction;
  int32 forward_weight=curr_3dv_acquisition_info->pred_weight0;
  int32 backward_weight=64-forward_weight;
  int temp;

  for (i=0;i<num_of_views;i++)
  {
    for (j=0;j<i;j++)
    {
      if(pred_direction==2)
      {
        curr_3dv_acquisition_info->i_disparity_scale_diff[j][i]=curr_3dv_acquisition_info->i_disparity_scale[j][i];
        curr_3dv_acquisition_info->i_disparity_offset_diff[j][i]=curr_3dv_acquisition_info->i_disparity_offset[j][i];
        curr_3dv_acquisition_info->i_disparity_scale_diff[i][j]=curr_3dv_acquisition_info->i_disparity_scale[i][j]+curr_3dv_acquisition_info->i_disparity_scale[j][i];
        curr_3dv_acquisition_info->i_disparity_offset_diff[i][j]=curr_3dv_acquisition_info->i_disparity_offset[i][j]+curr_3dv_acquisition_info->i_disparity_offset[j][i];
      }
      else
      {
        temp=(forward_3dv_acquisition_info->i_disparity_scale[j][i]*forward_weight+
          backward_3dv_acquisition_info->i_disparity_scale[j][i]*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_scale_diff[j][i]=curr_3dv_acquisition_info->i_disparity_scale[j][i]-temp;

        temp=(forward_3dv_acquisition_info->i_disparity_offset[j][i]*forward_weight+
          backward_3dv_acquisition_info->i_disparity_offset[j][i]*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_offset_diff[j][i]=curr_3dv_acquisition_info->i_disparity_offset[j][i]-temp;

        temp=((forward_3dv_acquisition_info->i_disparity_scale[i][j]+forward_3dv_acquisition_info->i_disparity_scale[j][i])*forward_weight+
          (backward_3dv_acquisition_info->i_disparity_scale[i][j]+backward_3dv_acquisition_info->i_disparity_scale[j][i])*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_scale_diff[i][j]=curr_3dv_acquisition_info->i_disparity_scale[i][j]+curr_3dv_acquisition_info->i_disparity_scale[j][i]-temp;

        temp=((forward_3dv_acquisition_info->i_disparity_offset[i][j]+forward_3dv_acquisition_info->i_disparity_offset[j][i])*forward_weight+
          (backward_3dv_acquisition_info->i_disparity_offset[i][j]+backward_3dv_acquisition_info->i_disparity_offset[j][i])*backward_weight+32)>>6;
        curr_3dv_acquisition_info->i_disparity_offset_diff[i][j]=curr_3dv_acquisition_info->i_disparity_offset[i][j]+curr_3dv_acquisition_info->i_disparity_offset[j][i]-temp;
      }
    }
  }
}

void cal_3dv_acquisition_element_info(VideoParameters* p_TextVid,VideoParameters* p_DepthVid, int in_subsps)
{
  ThreeDVAcquisitionInfo* ThreeDV_acquisition_info=p_DepthVid->ThreeDV_acquisition_info;
  ThreeDVAcquisitionInfo* sps_acquisition_info=p_DepthVid->DepParSet[0]->acquisition_info;
  ThreeDVAcquisitionInfo* forward_ThreeDV_acquisition_info = ThreeDV_acquisition_info;
  ThreeDVAcquisitionInfo* backward_ThreeDV_acquisition_info = ThreeDV_acquisition_info;
  InputParameters* p_DepthInp=p_DepthVid->p_Inp;
  int i;
  int ref_DPS_id0 = 0;
  //int ref_DPS_id1 = 0;
  int frm_no_in_file_diff0 = p_TextVid->frm_no_in_file;
  //int frm_no_in_file_diff1 = p_TextVid->frm_no_in_file;

  ThreeDV_acquisition_info->num_views = p_DepthInp->NumOfViews;
  ThreeDV_acquisition_info->depth_near_ae->mantissa_length=p_DepthInp->MantissaLengthDepthRange;
  ThreeDV_acquisition_info->depth_far_ae->mantissa_length=p_DepthInp->MantissaLengthDepthRange;


  if(in_subsps){
    ThreeDV_acquisition_info->pred_direction=2;
  }

  else{
    for(i=1; i<MAXDPS; i++){
      if(p_DepthVid->DepParSet[i]!=NULL){
        if( abs(p_DepthVid->DepParSet[i]->frm_no_in_file-p_TextVid->frm_no_in_file) < frm_no_in_file_diff0 && abs(p_DepthVid->DepParSet[i]->frm_no_in_file-p_TextVid->frm_no_in_file)>0){
          frm_no_in_file_diff0 = abs(p_DepthVid->DepParSet[i]->frm_no_in_file-p_TextVid->frm_no_in_file);
          ref_DPS_id0 = i;
        }

      }
    }
    if(ref_DPS_id0==0 && p_DepthInp->AcquisitionIdx==0)
      ThreeDV_acquisition_info->pred_direction=2;
    else{
      ThreeDV_acquisition_info->pred_direction=1;
      ThreeDV_acquisition_info->pred_weight0=64;
      ThreeDV_acquisition_info->ref_element_id0 = ref_DPS_id0;
      ThreeDV_acquisition_info->ref_element_id1 = ref_DPS_id0;
      forward_ThreeDV_acquisition_info = p_DepthVid->DepParSet[ref_DPS_id0]->acquisition_info;
      backward_ThreeDV_acquisition_info = p_DepthVid->DepParSet[ref_DPS_id0]->acquisition_info;
    }
  }

  PrepareAcquisitionElement(p_TextVid, p_DepthVid,ThreeDV_acquisition_info->depth_near_ae, sps_acquisition_info->depth_near_ae,
    forward_ThreeDV_acquisition_info->depth_near_ae, backward_ThreeDV_acquisition_info->depth_near_ae, in_subsps);
  PrepareAcquisitionElement(p_TextVid, p_DepthVid,ThreeDV_acquisition_info->depth_far_ae, sps_acquisition_info->depth_far_ae,
    forward_ThreeDV_acquisition_info->depth_far_ae, backward_ThreeDV_acquisition_info->depth_far_ae, in_subsps);

  ThreeDV_acquisition_info->disp_param_flag=(byte)check_disp_param_flag(ThreeDV_acquisition_info,sps_acquisition_info);
  if (ThreeDV_acquisition_info->disp_param_flag)
  {
    PrepareDisparityElement(ThreeDV_acquisition_info,forward_ThreeDV_acquisition_info,backward_ThreeDV_acquisition_info);
  }
}

void PrepareAcquisitionElement(VideoParameters* p_TextVid,VideoParameters* p_DepthVid, ThreeDVAE* curr_3dv_ae, ThreeDVAE* sps_3dv_ae, ThreeDVAE* forward_3dv_ae, ThreeDVAE* backward_3dv_ae, int in_subsps){
  ThreeDVAcquisitionInfo* ThreeDV_acquisition_info=p_DepthVid->ThreeDV_acquisition_info;
  InputParameters* p_DepthInp=p_DepthVid->p_Inp;
  int32 num_of_views=p_DepthInp->NumOfViews;
  int32 forward_weight = ThreeDV_acquisition_info->pred_weight0;
  int32 backward_weight = 64-forward_weight;
  int32 voidx;

  int32 sign_prediction=0;
  int32 exponent_prediction=0;
  int32 mantissa_prediction=0;
  int32 num_of_sets=num_of_views-curr_3dv_ae->delta_flag;

  UNREFERENCED_PARAMETER(p_TextVid);

  if(p_DepthVid->p_Inp->AcquisitionIdx!=0)
    curr_3dv_ae->element_flag=0;
  else
    curr_3dv_ae->element_flag=1;
  curr_3dv_ae->element_equal=1;
  for(voidx=curr_3dv_ae->delta_flag;voidx<num_of_views;++voidx){
    if(curr_3dv_ae->original[voidx]>=0)
      curr_3dv_ae->element[voidx].sign=0;
    else
      curr_3dv_ae->element[voidx].sign=1;
    get_exponent_mantissa(curr_3dv_ae,voidx);
    get_rec_double_type(curr_3dv_ae,voidx);
    if(in_subsps){
      curr_3dv_ae->element_flag=1;
    }
    else{
      if(curr_3dv_ae->element_flag==0){
        if((curr_3dv_ae->element[voidx].sign!=sps_3dv_ae->element[voidx].sign)||
          (curr_3dv_ae->element[voidx].exponent!=sps_3dv_ae->element[voidx].exponent)||
          (curr_3dv_ae->element[voidx].mantissa!=sps_3dv_ae->element[voidx].mantissa))
          curr_3dv_ae->element_flag=1;
      }
    }
    if((voidx>curr_3dv_ae->delta_flag)&&(curr_3dv_ae->element_equal)&&((curr_3dv_ae->element[voidx].sign!=curr_3dv_ae->element[curr_3dv_ae->delta_flag].sign)||
      (curr_3dv_ae->element[voidx].exponent!=curr_3dv_ae->element[curr_3dv_ae->delta_flag].exponent)||
      (curr_3dv_ae->element[voidx].mantissa!=curr_3dv_ae->element[curr_3dv_ae->delta_flag].mantissa)))
      curr_3dv_ae->element_equal=0;
    if(curr_3dv_ae->element[voidx].mantissa_bits>curr_3dv_ae->mantissa_length)
      curr_3dv_ae->mantissa_length=curr_3dv_ae->element[voidx].mantissa_bits;
  }

  if(curr_3dv_ae->element_flag==0)
    return;


  num_of_sets=num_of_views-curr_3dv_ae->delta_flag;
  if(curr_3dv_ae->element_equal)
    num_of_sets=1;
  for(voidx=curr_3dv_ae->delta_flag;voidx<num_of_sets+curr_3dv_ae->delta_flag;++voidx){
    if(ThreeDV_acquisition_info->pred_direction==2)
    {
      curr_3dv_ae->mantissa_diff[voidx]=curr_3dv_ae->element[voidx].mantissa;
      curr_3dv_ae->exponent_diff[voidx]=curr_3dv_ae->element[voidx].exponent;
      curr_3dv_ae->sign_diff[voidx]=curr_3dv_ae->element[voidx].sign;
    }
    else
    {
      sign_prediction=curr_3dv_ae->element[voidx].sign-forward_3dv_ae->element[voidx].sign;
      exponent_prediction=forward_3dv_ae->element[voidx].exponent;
      mantissa_prediction=(forward_3dv_ae->element[voidx].mantissa*forward_weight+
        backward_3dv_ae->element[voidx].mantissa*backward_weight+32)>>6;
      curr_3dv_ae->mantissa_diff[voidx]=curr_3dv_ae->element[voidx].mantissa-mantissa_prediction;
      curr_3dv_ae->exponent_diff[voidx]=curr_3dv_ae->element[voidx].exponent-exponent_prediction;
      curr_3dv_ae->sign_diff[voidx]=curr_3dv_ae->element[voidx].sign-sign_prediction;
      if((curr_3dv_ae->mantissa_diff[voidx]==0)&&(curr_3dv_ae->exponent_diff[voidx]==0)&&(curr_3dv_ae->sign_diff[voidx]==0))
        curr_3dv_ae->skip_flag[voidx]=1;
      else
      {
        curr_3dv_ae->skip_flag[voidx]=0;
        if(curr_3dv_ae->exponent_diff[voidx]==0)
          curr_3dv_ae->exponent_skip_flag[voidx]=1;
        else
          curr_3dv_ae->exponent_skip_flag[voidx]=0;

      }
    }
  }

}

static int encode_acquisition_element(ThreeDVAE* curr_3dv_ae,int num_of_views,Bitstream* bitstream, int prediction)
{
  int number_of_sets=num_of_views-curr_3dv_ae->delta_flag;
  int k=0;
  int len=0;

  if(number_of_sets>1)
  {
    len+=u_1("3dv_acquisition_info:element_equal_flag",curr_3dv_ae->element_equal,bitstream);
    if(curr_3dv_ae->element_equal)
      number_of_sets=1;
  }

  for(k=curr_3dv_ae->delta_flag;k<number_of_sets+curr_3dv_ae->delta_flag;++k)
  {
    if(prediction==2)
    {
      if(k==curr_3dv_ae->delta_flag)
      {
        if(curr_3dv_ae->pred_mode==0)
        {
          len+=u_v(5,"3dv_acquisition_info:matissa_len_minus1",curr_3dv_ae->mantissa_length-1,bitstream);
        }
        else
        {
          len+=u_v(5,"3dv_acquisition_info:prec",curr_3dv_ae->precision,bitstream);
        }
      }
      len+=u_1("3dv_acquisition_info:sign",curr_3dv_ae->element[k].sign,bitstream);
      len+=u_v(curr_3dv_ae->exponent_size,"3dv_acquisition_info:exponent",curr_3dv_ae->element[k].exponent,bitstream);
      len+=u_v(curr_3dv_ae->element[k].mantissa_bits,"3dv_acquisition_info:mantissa",curr_3dv_ae->element[k].mantissa,bitstream);
    }
    else
    {
      len+=u_1("3dv_acquisition_info:element_skip_skip_flag",curr_3dv_ae->skip_flag[k],bitstream);
      if(!curr_3dv_ae->skip_flag[k])
      {
        len+=u_1("3dv_acquisition_info:sign",curr_3dv_ae->element[k].sign,bitstream);
        len+=u_1("3dv_acquisition_info:exponent_skip",curr_3dv_ae->exponent_skip_flag[k],bitstream);
        if(!curr_3dv_ae->exponent_skip_flag[k])
          len+=u_v(curr_3dv_ae->exponent_size,"3dv_acquisition_info:exponent",curr_3dv_ae->element[k].exponent,bitstream);
        len+=se_v("3dv_acquisition_info:mantissa_diff",curr_3dv_ae->mantissa_diff[k],bitstream);
      }
    }
  }
  return len;
}

int encode_3dv_acquisition_info(ThreeDVAcquisitionInfo* threeDV_acquisition_info, Bitstream* bitstream, int in_subsps, int is_vsp_param_flag)
{
  int num_of_param_sets=threeDV_acquisition_info->num_views;
  int len=0;

  if(!is_vsp_param_flag){
    if(!in_subsps)
    {
      len+=u_1("depth_near_flag",threeDV_acquisition_info->depth_near_ae->element_flag,bitstream);
      len+=u_1("depth_far_flag",threeDV_acquisition_info->depth_far_ae->element_flag,bitstream);
    }
    else
    {
      len+=u_1("depth_near_flag",1,bitstream);
      len+=u_1("depth_far_flag",1,bitstream);
    }
    //!<depth near
    if(in_subsps||(threeDV_acquisition_info->depth_near_ae->element_flag))
      len+=encode_acquisition_element(threeDV_acquisition_info->depth_near_ae,num_of_param_sets,bitstream,threeDV_acquisition_info->pred_direction);
    //!<depth far
    if(in_subsps||(threeDV_acquisition_info->depth_far_ae->element_flag))
      len+=encode_acquisition_element(threeDV_acquisition_info->depth_far_ae,num_of_param_sets,bitstream,threeDV_acquisition_info->pred_direction);
  }

  else {
    if(!in_subsps){
      len+=u_1("disp_param_flag",threeDV_acquisition_info->disp_param_flag,bitstream);
    }
    else{
      len+=u_1("disp_param_flag",1,bitstream);
    }
    //!<disparity_scale_offset
    if(in_subsps||(threeDV_acquisition_info->disp_param_flag))
    {
      int i,j;
      for (i=0;i<num_of_param_sets;i++)
      {
        for (j=0;j<i;j++)
        {
          len+=se_v("3dv_acquisition_info: scale parameter for disparity",threeDV_acquisition_info->i_disparity_scale_diff[j][i],bitstream);
          len+=se_v("3dv_acquisition_info: offset parameter for disparity",threeDV_acquisition_info->i_disparity_offset_diff[j][i],bitstream);
          len+=se_v("3dv_acquisition_info: scale parameter for disparity",threeDV_acquisition_info->i_disparity_scale_diff[i][j],bitstream);
          len+=se_v("3dv_acquisition_info: offset parameter for disparity",threeDV_acquisition_info->i_disparity_offset_diff[i][j],bitstream);
        }
      }
    }
  }

  return len;

}
#endif

/*!
 *************************************************************************************
 * \brief
 *    int WriteHRDParameters((seq_parameter_set_rbsp_t *sps, Bitstream *bitstream)
 *
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *************************************************************************************
 */

int WriteHRDParameters(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream)
{
  // hrd_parameters()
  int len = 0;
  unsigned int SchedSelIdx = 0;
  hrd_parameters_t *hrd = &(sps->vui_seq_parameters.nal_hrd_parameters);

  len+=ue_v ("VUI: cpb_cnt_minus1", hrd->cpb_cnt_minus1, bitstream);
  len+=u_v  (4, "VUI: bit_rate_scale", hrd->bit_rate_scale, bitstream);
  len+=u_v  (4, "VUI: cpb_size_scale", hrd->cpb_size_scale, bitstream);

  for( SchedSelIdx = 0; SchedSelIdx <= (hrd->cpb_cnt_minus1); SchedSelIdx++ )
  {
    len+=ue_v ("VUI: bit_rate_value_minus1", hrd->bit_rate_value_minus1[SchedSelIdx], bitstream);
    len+=ue_v ("VUI: cpb_size_value_minus1", hrd->cpb_size_value_minus1[SchedSelIdx], bitstream);
    len+=u_1  ("VUI: cbr_flag", hrd->cbr_flag[SchedSelIdx], bitstream);
  }

  len+=u_v  (5, "VUI: initial_cpb_removal_delay_length_minus1", hrd->initial_cpb_removal_delay_length_minus1, bitstream);
  len+=u_v  (5, "VUI: cpb_removal_delay_length_minus1", hrd->cpb_removal_delay_length_minus1, bitstream);
  len+=u_v  (5, "VUI: dpb_output_delay_length_minus1", hrd->dpb_output_delay_length_minus1, bitstream);
  len+=u_v  (5, "VUI: time_offset_length", hrd->time_offset_length, bitstream);

  return len;
}

/*!
 ************************************************************************
 * \brief
 *    Returns the size of the dpb depending on level and picture size
 *
 *
 ************************************************************************
 */
static int getMaxDpbSize(seq_parameter_set_rbsp_t *active_sps)
{
  int pic_size = (active_sps->pic_width_in_mbs_minus1 + 1) * (active_sps->pic_height_in_map_units_minus1 + 1) * (active_sps->frame_mbs_only_flag?1:2) * 384;

  int size = 0;

  switch (active_sps->level_idc)
  {
  case 9:
    size = 152064;
    break;
  case 10:
    size = 152064;
    break;
  case 11:
    if (!IS_FREXT_PROFILE(active_sps->profile_idc) && (active_sps->constrained_set3_flag == 1))
      size = 152064;
    else
      size = 345600;
    break;
  case 12:
    size = 912384;
    break;
  case 13:
    size = 912384;
    break;
  case 20:
    size = 912384;
    break;
  case 21:
    size = 1824768;
    break;
  case 22:
    size = 3110400;
    break;
  case 30:
    size = 3110400;
    break;
  case 31:
    size = 6912000;
    break;
  case 32:
    size = 7864320;
    break;
  case 40:
    size = 12582912;
    break;
  case 41:
    size = 12582912;
    break;
  case 42:
    size = 13369344;
    break;
  case 50:
    size = 42393600;
    break;
  case 51:
    size = 70778880;
    break;
  default:
    error ("undefined level", 500);
    break;
  }

  size /= pic_size;
  size = imin( size, 16);

  return size;
}

/*!
 *************************************************************************************
 * \brief
 *    void GenerateVUIParameters(seq_parameter_set_rbsp_t *sps, InputParameters *p_Inp)
 *
 *
 * \return
 *    none
 *
 * \note
 *************************************************************************************
 */

void GenerateVUIParameters(seq_parameter_set_rbsp_t *sps, InputParameters *p_Inp)
{
  unsigned int          SchedSelIdx;
  hrd_parameters_t     *nal_hrd = &(sps->vui_seq_parameters.nal_hrd_parameters);
  hrd_parameters_t     *vcl_hrd = &(sps->vui_seq_parameters.vcl_hrd_parameters);
  vui_seq_parameters_t *vui     = &(sps->vui_seq_parameters);
  VUIParameters *iVui = &p_Inp->VUI;

  vui->aspect_ratio_info_present_flag      = (Boolean) iVui->aspect_ratio_info_present_flag;
  vui->aspect_ratio_idc                    = (unsigned int) iVui->aspect_ratio_idc;
  vui->sar_width                           = (unsigned int) iVui->sar_width;
  vui->sar_height                          = (unsigned int) iVui->sar_height;
  vui->overscan_info_present_flag          = (Boolean) iVui->overscan_info_present_flag;
  vui->overscan_appropriate_flag           = (Boolean) iVui->overscan_appropriate_flag;
  vui->video_signal_type_present_flag      = (Boolean) iVui->video_signal_type_present_flag;
  vui->video_format                        = (unsigned int) iVui->video_format;
  vui->video_full_range_flag               = (Boolean) iVui->video_full_range_flag;
  vui->colour_description_present_flag     = (Boolean) iVui->colour_description_present_flag;
  vui->colour_primaries                    = (unsigned int) iVui->colour_primaries;
  vui->transfer_characteristics            = (unsigned int) iVui->transfer_characteristics;
  vui->matrix_coefficients                 = (unsigned int) iVui->matrix_coefficients;
  vui->chroma_location_info_present_flag   = (Boolean) iVui->chroma_location_info_present_flag;
  vui->chroma_sample_loc_type_top_field    = (unsigned int) iVui->chroma_sample_loc_type_top_field;
  vui->chroma_sample_loc_type_bottom_field = (unsigned int) iVui->chroma_sample_loc_type_bottom_field;
  vui->timing_info_present_flag            = (Boolean) iVui->timing_info_present_flag;
  vui->num_units_in_tick                   = (unsigned int) iVui->num_units_in_tick;
  vui->time_scale                          = (unsigned int) iVui->time_scale;
  vui->fixed_frame_rate_flag               = (Boolean) iVui->fixed_frame_rate_flag;  

  // NAL HRD parameters
  vui->nal_hrd_parameters_present_flag             = (Boolean) iVui->nal_hrd_parameters_present_flag;  
  nal_hrd->cpb_cnt_minus1                          = (unsigned int) iVui->nal_cpb_cnt_minus1;
  nal_hrd->bit_rate_scale                          = (unsigned int) iVui->nal_bit_rate_scale;
  nal_hrd->cpb_size_scale                          = (unsigned int) iVui->nal_cpb_size_scale;
  for ( SchedSelIdx = 0; SchedSelIdx <= nal_hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    nal_hrd->bit_rate_value_minus1[SchedSelIdx]    = (unsigned int) iVui->nal_bit_rate_value_minus1;
    nal_hrd->cpb_size_value_minus1[SchedSelIdx]    = (unsigned int) iVui->nal_cpb_size_value_minus1;
    nal_hrd->cbr_flag[SchedSelIdx]                 = (unsigned int) iVui->nal_vbr_cbr_flag;
  }
  nal_hrd->initial_cpb_removal_delay_length_minus1 = (unsigned int) iVui->nal_initial_cpb_removal_delay_length_minus1;
  nal_hrd->cpb_removal_delay_length_minus1         = (unsigned int) iVui->nal_cpb_removal_delay_length_minus1;
  nal_hrd->dpb_output_delay_length_minus1          = (unsigned int) iVui->nal_dpb_output_delay_length_minus1;
  nal_hrd->time_offset_length                      = (unsigned int) iVui->nal_time_offset_length;

  // VCL HRD parameters
  vui->vcl_hrd_parameters_present_flag             = (Boolean) iVui->vcl_hrd_parameters_present_flag;  
  vcl_hrd->cpb_cnt_minus1                          = (unsigned int) iVui->vcl_cpb_cnt_minus1;
  vcl_hrd->bit_rate_scale                          = (unsigned int) iVui->vcl_bit_rate_scale;
  vcl_hrd->cpb_size_scale                          = (unsigned int) iVui->vcl_cpb_size_scale;
  for ( SchedSelIdx = 0; SchedSelIdx <= vcl_hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    vcl_hrd->bit_rate_value_minus1[SchedSelIdx]    = (unsigned int) iVui->vcl_bit_rate_value_minus1;
    vcl_hrd->cpb_size_value_minus1[SchedSelIdx]    = (unsigned int) iVui->vcl_cpb_size_value_minus1;
    vcl_hrd->cbr_flag[SchedSelIdx]                 = (unsigned int) iVui->vcl_vbr_cbr_flag;
  }
  vcl_hrd->initial_cpb_removal_delay_length_minus1 = (unsigned int) iVui->vcl_initial_cpb_removal_delay_length_minus1;
  vcl_hrd->cpb_removal_delay_length_minus1         = (unsigned int) iVui->vcl_cpb_removal_delay_length_minus1;
  vcl_hrd->dpb_output_delay_length_minus1          = (unsigned int) iVui->vcl_dpb_output_delay_length_minus1;
  vcl_hrd->time_offset_length                      = (unsigned int) iVui->vcl_time_offset_length;

  vui->low_delay_hrd_flag                      = (Boolean) iVui->low_delay_hrd_flag;
  vui->pic_struct_present_flag                 = (Boolean) iVui->pic_struct_present_flag;
  vui->bitstream_restriction_flag              = (Boolean) iVui->bitstream_restriction_flag;
  vui->motion_vectors_over_pic_boundaries_flag = (Boolean) iVui->motion_vectors_over_pic_boundaries_flag;
  vui->max_bytes_per_pic_denom                 = (unsigned int) iVui->max_bytes_per_pic_denom;
  vui->max_bits_per_mb_denom                   = (unsigned int) iVui->max_bits_per_mb_denom;
  vui->log2_max_mv_length_horizontal           = (unsigned int) iVui->log2_max_mv_length_horizontal;
  vui->log2_max_mv_length_vertical             = (unsigned int) iVui->log2_max_mv_length_vertical;  
  vui->max_dec_frame_buffering                 = (unsigned int) imin( iVui->max_dec_frame_buffering, getMaxDpbSize(sps) );
  vui->num_reorder_frames                      = (unsigned int) imin( iVui->num_reorder_frames, (int)vui->max_dec_frame_buffering );

  // special case to signal the RGB format
  if (p_Inp->output.color_model != CM_YUV && p_Inp->output.yuv_format == YUV444)
  {
    printf   ("VUI: writing Sequence Parameter VUI to signal RGB format\n");

    vui->aspect_ratio_info_present_flag = FALSE;
    vui->overscan_info_present_flag = FALSE;
    vui->video_signal_type_present_flag = TRUE;
    vui->video_format = 2;
    vui->video_full_range_flag = TRUE;
    vui->colour_description_present_flag = TRUE;
    vui->colour_primaries = 2;
    vui->transfer_characteristics = 2;
    vui->matrix_coefficients = 0;
    vui->chroma_location_info_present_flag = FALSE;
    vui->timing_info_present_flag = FALSE;
    vui->nal_hrd_parameters_present_flag = FALSE;
    vui->vcl_hrd_parameters_present_flag = FALSE;
    vui->pic_struct_present_flag = FALSE;
    vui->bitstream_restriction_flag = FALSE;
  } 
}
