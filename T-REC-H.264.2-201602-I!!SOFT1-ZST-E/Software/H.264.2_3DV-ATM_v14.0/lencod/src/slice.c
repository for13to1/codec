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
 *    slice.c
 * \brief
 *    generate the slice header, setup the bit buffer for slices,
 *    and generates the slice NALU(s)

 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Thomas Stockhammer            <stockhammer@ei.tum.de>
 *      - Detlev Marpe                  <marpe@hhi.de>
 *      - Stephan Wenger                <stewe@cs.tu-berlin.de>
 *      - Alexis Michael Tourapis       <alexismt@ieee.org>
 ***************************************************************************************
 */

#include "contributors.h"

#include <math.h>
#include <float.h>

#include "global.h"
#include "header.h"
#include "nal.h"
#include "rtp.h"
#include "fmo.h"
#include "vlc.h"
#include "image.h"
#include "cabac.h"
#include "biariencode.h"
#include "elements.h"
#include "macroblock.h"
#include "memalloc.h"
#include "mode_decision_p8x8.h"
#include "symbol.h"
#include "context_ini.h"
#include "enc_statistics.h"
#include "ratectl.h"
#include "me_epzs.h"
#include "me_epzs_int.h"
#include "wp.h"
#include "slice.h"
#include "rdoq.h"
#include "wp_mcprec.h"
#include "q_offsets.h"
#include "conformance.h"
#include "list_reorder.h"
#include "md_common.h"
#include "mmco.h"
#include "mv_search.h"
#include "quant4x4.h"
#include "quant8x8.h"
#include "quantChroma.h"
#include "rdopt.h"
#include "rdopt_coding_state.h"
#include "lambda.h"
#include "intra4x4.h"
#include "intra8x8.h"
#include "intra16x16.h"
#include "mc_prediction.h"
#include "rd_intra_jm.h"
#include "rd_intra_jm444.h"

#if EXT3D
#include "configfile.h"
#include "parset.h"
#include "me_fullsearch.h"
#include "alc.h"

void reset_params_after_alc( Macroblock* currMB);
#endif

// Local declarations
static Slice *malloc_slice(VideoParameters *p_Vid, InputParameters *p_Inp);
static Slice *malloc_slice_lite(VideoParameters *p_Vid, InputParameters *p_Inp);


//! convert from H.263 QP to H.264 quant given by: quant=pow(2,QP/6)

int allocate_block_mem(Slice *currSlice)
{
  int alloc_size = 0;
  alloc_size += get_mem2Dint(&currSlice->tblk4x4, BLOCK_SIZE, BLOCK_SIZE);
  alloc_size += get_mem2Dint(&currSlice->tblk16x16, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  alloc_size += get_mem4Dint(&currSlice->i16blk4x4, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);

  return (alloc_size);
}


void free_block_mem(Slice *currSlice)
{
  if(currSlice->i16blk4x4)
  free_mem4Dint(currSlice->i16blk4x4);
  if(currSlice->tblk16x16)
  free_mem2Dint(currSlice->tblk16x16);
  if(currSlice->tblk4x4)
  free_mem2Dint(currSlice->tblk4x4);
}

/*!
 ***********************************************************************
 * \brief
 *    Initializes the p_Vid->nz_coeff
 * \par Input:
 *    none
 * \par  Output:
 *    none
 * \ side effects
 *    sets omg->nz_coef[][][][] to -1
 ***********************************************************************
 */
static void CAVLC_init(Slice *currSlice)
{
  memset(&currSlice->p_Vid->nz_coeff[0][0][0], 0, currSlice->PicSizeInMbs * 4 * (4 + currSlice->num_blk8x8_uv)* sizeof(int));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate memory for mv
 * \par Input:
 *    Image Parameters VideoParameters *p_Vid                             \n
 *    int****** mv
 * \return memory size in bytes
 ************************************************************************
 */
static int get_mem_mv (Slice *currSlice, MotionVector ****** mv)
{
  // LIST, reference, block_type, block_y, block_x, component

#if EXT3D
    get_mem5Dmv (mv, 2, currSlice->max_num_references, 11, 4, 4); // (all_mv[list][ref][blocktype / mode][block_y][block_x]
    return 352 * currSlice->max_num_references * sizeof(MotionVector); // 2 * ref * 11 * 4 * 4
#else
  get_mem5Dmv (mv, 2, currSlice->max_num_references, 9, 4, 4);

  return 288 * currSlice->max_num_references * sizeof(MotionVector); // 2 * ref * 9 * 4 * 4
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Allocate memory for bipredictive mv 
 * \par Input:
 *    Image Parameters VideoParameters *p_Vid                             \n
 *    int****** mv
 * \return memory size in bytes
 ************************************************************************
 */
static int get_mem_bipred_mv (Slice *currSlice, MotionVector ******* bipred_mv) 
{
  get_mem6Dmv (bipred_mv, 2, 2, currSlice->max_num_references, 9, 4, 4);
  
  return 576 * currSlice->max_num_references * sizeof(MotionVector);
}

/*!
 ************************************************************************
 * \brief
 *    Free memory from mv
 * \par Input:
 *    int****** mv
 ************************************************************************
 */
static void free_mem_mv (MotionVector ***** mv)
{
  free_mem5Dmv(mv);
}


/*!
 ************************************************************************
 * \brief
 *    Free memory from mv
 * \par Input:
 *    int****** mv
 ************************************************************************
 */
static void free_mem_bipred_mv (MotionVector ****** bipred_mv) 
{
  free_mem6Dmv(bipred_mv);
}


static int alloc_rddata(Slice *currSlice, RD_DATA *rd_data)
{
  int alloc_size = 0;

  alloc_size += get_mem3Dpel(&(rd_data->rec_mb), 3, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

  alloc_size += get_mem_ACcoeff (currSlice->p_Vid, &(rd_data->cofAC));
  alloc_size += get_mem_DCcoeff (&(rd_data->cofDC));  

  if ((currSlice->slice_type != I_SLICE) && currSlice->slice_type != SI_SLICE)
  {          
    alloc_size += get_mem_mv (currSlice, &(rd_data->all_mv));
  }

  // Why is this stored as height_blk * width_blk?
  alloc_size += get_mem2D((byte***)&(rd_data->ipredmode), currSlice->height_blk, currSlice->width_blk);
  alloc_size += get_mem3D((byte****)&(rd_data->refar), 2, 4, 4);

  return alloc_size;
}


static void nullify_rddata(RD_DATA *rd_data)
{
  rd_data->rec_mb = NULL;

  rd_data->cofAC = NULL;
  rd_data->cofDC = NULL;  
  rd_data->all_mv = NULL;

  rd_data->ipredmode = NULL;
  rd_data->refar = NULL;
}

static void free_rddata(Slice *currSlice, RD_DATA *rd_data)
{
  if(rd_data->refar)
  free_mem3D((byte***) rd_data->refar);
  if(rd_data->ipredmode)
  free_mem2D((byte**)  rd_data->ipredmode);

  if ((currSlice->slice_type != I_SLICE) && currSlice->slice_type != SI_SLICE)
  {  
    if(rd_data->all_mv)
    free_mem_mv (rd_data->all_mv);
  }

  if(rd_data->cofDC)
  free_mem_DCcoeff (rd_data->cofDC);
  if(rd_data->cofAC)
  free_mem_ACcoeff (rd_data->cofAC);  

  if(rd_data->rec_mb)
  free_mem3Dpel(rd_data->rec_mb);
}


/*!
 ************************************************************************
 *  \brief
 *     This function generates the slice (and partition) header(s)
 *
 *  \return number of bits used for the slice (and partition) header(s)
 *
 *  \par Side effects:
 *      Adds slice/partition header symbols to the symbol buffer
 *      increments Picture->no_slices, allocates memory for the
 *      slice, sets p_Vid->currSlice
 ************************************************************************
*/
static int start_slice(Slice *currSlice, StatParameters *cur_stats)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  EncodingEnvironmentPtr eep;
  Bitstream *currStream;
  int header_len = 0;
  int i;
  int NumberOfPartitions = (currSlice->partition_mode == PAR_DP_1?1:3);

  //one  partition for an IDR image
  if(currSlice->idr_flag)
  {
    NumberOfPartitions = 1;
  }

  RTPUpdateTimestamp (p_Vid, currSlice->frame_no);   // this has no side effects, just leave it for all NALs

  for (i = 0; i < NumberOfPartitions; i++)
  {
    currStream = (currSlice->partArr[i]).bitstream;

    currStream->write_flag = 0;
    if (i==0)     // First partition
      header_len += SliceHeader (currSlice);
    else          // Second/Third partition
      header_len += Partition_BC_Header(currSlice, i);

    //! Initialize CABAC
    if (currSlice->symbol_mode == CABAC)
    {
      eep = &((currSlice->partArr[i]).ee_cabac);
      if (currStream->bits_to_go != 8)
        header_len += currStream->bits_to_go;
      writeVlcByteAlign(p_Vid, currStream, cur_stats);
      eep->p_Vid = p_Vid;
      arienco_start_encoding(eep, currStream->streamBuffer, &(currStream->byte_pos));

      arienco_reset_EC(eep);
    }
    else
    {
      // Initialize CA-VLC
      CAVLC_init(currSlice);
    }
  }

  if(currSlice->symbol_mode == CABAC)
  {
    init_contexts(currSlice);
  }

  return header_len;
}

/*!
************************************************************************
* \brief
*    This creates a NAL unit structures for all data partition of the slice
*
************************************************************************
*/
void create_slice_nalus(Slice *currSlice, int is_bottom)
{
  // KS: this is approx. max. allowed code picture size
  //const int buffer_size = 500 + p_Vid->FrameSizeInMbs * (128 + 256 * p_Vid->bitdepth_luma + 512 * p_Vid->bitdepth_chroma);
  int buffer_size = currSlice->partArr[0].bitstream->buffer_size;
#if MVC_EXTENSION_ENABLE||EXT3D
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
#endif
  
#if EXT3D
  int  voidx=GetVOIdx(p_Inp,p_Vid->view_id);
#endif

  int part;
  NALU_t *nalu;

  for (part=0; part< currSlice->max_part_nr; part++)
  {
    if (currSlice->partArr[part].bitstream->write_flag)
    {
      nalu = AllocNALU(buffer_size);
      currSlice->partArr[part].nal_unit = nalu;
      nalu->startcodeprefix_len = 1+ (currSlice->start_mb_nr == 0 && part == 0 ?ZEROBYTES_SHORTSTARTCODE+1:ZEROBYTES_SHORTSTARTCODE);
      nalu->forbidden_bit = 0;
#if EXT3D
      if(p_Inp->NumOfViews>1)
      {
        nalu->non_idr_flag        = p_Vid->non_idr_flag[is_bottom];
        nalu->priority_id         = p_Vid->priority_id;
        nalu->view_id             = p_Vid->view_id;
        nalu->temporal_id         = p_Vid->temporal_id;
        nalu->anchor_pic_flag     = p_Vid->anchor_pic_flag[is_bottom];
        nalu->inter_view_flag     = p_Vid->inter_view_flag[is_bottom];
        nalu->reserved_one_bit    = 1;

        nalu->svc_extension_flag  = p_Vid->is_depth? p_Vid->p_DualInp->CompatibilityCategory:p_Inp->CompatibilityCategory;
        nalu->depth_flag          = p_Vid->is_depth;
      }
#else
#if (MVC_EXTENSION_ENABLE)
      if(p_Inp->num_of_views==2)
      {
        nalu->non_idr_flag        = p_Vid->non_idr_flag[is_bottom];
        nalu->priority_id         = p_Vid->priority_id;
        nalu->view_id             = p_Vid->view_id;
        nalu->temporal_id         = p_Vid->temporal_id;
        nalu->anchor_pic_flag     = p_Vid->anchor_pic_flag[is_bottom];
        nalu->inter_view_flag     = p_Vid->inter_view_flag[is_bottom];
        nalu->reserved_one_bit    = 1;
      }
#endif
#endif

#if EXT3D
      if ((!p_Vid->is_depth)&&(currSlice->idr_flag)&&(voidx==0))
#else
      if (currSlice->idr_flag)
#endif
      {
        nalu->nal_unit_type = NALU_TYPE_IDR;
        nalu->nal_reference_idc = NALU_PRIORITY_HIGHEST;
      }
      else
      {
#if EXT3D
        nalu->non_idr_flag = p_Vid->non_idr_flag[is_bottom];
#else
#if (MVC_EXTENSION_ENABLE)
        nalu->non_idr_flag = 1;
#endif
#endif

        //different nal header for different partitions
        if(currSlice->partition_mode == 0)
        {
#if EXT3D
          if((p_Inp->NumOfViews>1)||((p_Inp->NumOfViews==1)&&(p_Vid->p_DualVid)))
          {
            if(p_Vid->is_depth)
              nalu->nal_unit_type=NALU_TYPE_3DV_EXT;
            else
              nalu->nal_unit_type = (voidx==0) ? NALU_TYPE_SLICE : 
              ((p_Inp->CompatibilityCategory==0)?NALU_TYPE_SLC_EXT:NALU_TYPE_3DV_EXT);
          }
          else
            nalu->nal_unit_type = NALU_TYPE_SLICE;
#else
#if (MVC_EXTENSION_ENABLE)
          if(p_Inp->num_of_views==2)
          {
              nalu->nal_unit_type = p_Vid->view_id==0 ? NALU_TYPE_SLICE : NALU_TYPE_SLC_EXT;
          }
          else
            nalu->nal_unit_type = NALU_TYPE_SLICE;
#else
          nalu->nal_unit_type = NALU_TYPE_SLICE;
#endif
#endif
        }
        else
        {
          nalu->nal_unit_type = (NaluType) (NALU_TYPE_DPA +  part);
        }

        if (currSlice->nal_reference_idc !=0)
        {
          nalu->nal_reference_idc = NALU_PRIORITY_HIGH;
        }
        else
        {
          nalu->nal_reference_idc = NALU_PRIORITY_DISPOSABLE;
        }
      }
    }
    else
    {
      currSlice->partArr[part].nal_unit = NULL;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    This function terminates a slice (but doesn't write it out),
 *    the old terminate_slice (0)
 * \return
 *    0 if OK,                                                         \n
 *    1 in case of error
 *
 ************************************************************************
 */
static int terminate_slice(Macroblock *currMB, int lastslice, StatParameters *cur_stats )
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  Bitstream *currStream;
  NALU_t    *currNalu;
  EncodingEnvironmentPtr eep;
  int part;
  int tmp_stuffingbits = currMB->bits.mb_stuffing;

  if (currSlice->symbol_mode == CABAC)
    write_terminating_bit (currSlice, 1);      // only once, not for all partitions

  create_slice_nalus(currSlice, p_Vid->structure == BOTTOM_FIELD ? 1 : 0);

  for (part = 0; part < currSlice->max_part_nr; part++)
  {
    currStream = (currSlice->partArr[part]).bitstream;
    currNalu   = (currSlice->partArr[part]).nal_unit;
    if (currStream->write_flag)
    {
      if (currSlice->symbol_mode == CAVLC)
      {
        SODBtoRBSP(currStream);
        currNalu->len = RBSPtoEBSP(currNalu->buf, currStream->streamBuffer, currStream->byte_pos);
      }
      else     // CABAC
      {
        eep = &((currSlice->partArr[part]).ee_cabac);
        // terminate the arithmetic code
        arienco_done_encoding(currMB, eep);
        set_pic_bin_count(p_Vid, eep);

        currStream->bits_to_go = eep->Ebits_to_go;
        currStream->byte_buf = 0;

        currNalu->len = RBSPtoEBSP(currNalu->buf, currStream->streamBuffer, currStream->byte_pos);

        // NumBytesInNALunit is: payload length + 1 byte header
        p_Vid->bytes_in_picture += currNalu->len + 1;

        if (lastslice && (part==(currSlice->max_part_nr - 1)))
        {
          addCabacZeroWords(p_Vid, currNalu, cur_stats);
        }
      }           // CABAC
    }
  }           // partition loop

  if( currSlice->symbol_mode == CABAC )
  {
    store_contexts(currSlice);
  }

  cur_stats->bit_use_stuffingBits[currSlice->slice_type] += currMB->bits.mb_stuffing - tmp_stuffingbits;
  return 0;
}

static void init_bipred_enabled(VideoParameters *p_Vid)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int mode;

  memset(p_Vid->bipred_enabled, 0, sizeof(int)*MAXMODE);
  if (p_Vid->currentSlice->slice_type != B_SLICE || !p_Inp->BiPredMotionEstimation)
    return;

  for(mode = 1; mode < 5; mode++)
    p_Vid->bipred_enabled[mode] = (p_Inp->BiPredSearch[mode - 1]) ? 1: 0;
}

/*!
************************************************************************
* \brief
*    Encodes one slice
* \par
*   returns the number of coded MBs in the SLice
************************************************************************
*/
int encode_one_slice (VideoParameters *p_Vid, int SliceGroupId, int TotalCodedMBs)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  Boolean end_of_slice = FALSE;

  int len;
  int NumberOfCodedMBs = 0;
  Macroblock* currMB   = NULL;
  int CurrentMbAddr;
  StatParameters *cur_stats = &p_Vid->enc_picture->stats;
  Slice *currSlice = NULL;  

  if( (p_Inp->separate_colour_plane_flag != 0) )
  {
    change_plane_JV( p_Vid, p_Vid->colour_plane_id );
  }

  p_Vid->cod_counter = 0;

  CurrentMbAddr = FmoGetFirstMacroblockInSlice (p_Vid, SliceGroupId);
  // printf ("\n\nEncode_one_slice: PictureID %d SliceGroupId %d  SliceID %d  FirstMB %d \n", p_Vid->frame_no, SliceGroupId, p_Vid->current_slice_nr, CurrentMbInScanOrder);

  p_Vid->enc_picture->temporal_layer = p_Vid->p_curr_frm_struct->temporal_layer; 
  init_slice (p_Vid, &currSlice, CurrentMbAddr);
  currSlice->rdoq_motion_copy = 0;
  init_bipred_enabled(p_Vid);

  // Initialize quantization functions based on rounding/quantization method
  // Done here since we may wish to disable adaptive rounding on occasional intervals (even at a frame or gop level).
  init_quant_4x4   (currSlice);
  init_quant_8x8   (currSlice);
  init_quant_Chroma(currSlice);

  currSlice->set_lagrangian_multipliers(currSlice);

  if (currSlice->symbol_mode == CABAC)
  {
    SetCtxModelNumber (currSlice);
  }

  p_Vid->checkref = (short) (p_Inp->rdopt && p_Inp->RestrictRef && (p_Vid->type==P_SLICE || p_Vid->type==SP_SLICE));

  len = start_slice (currSlice, cur_stats);

  // Rate control
  if (p_Inp->RCEnable)
    rc_store_slice_header_bits( p_Vid, p_Inp, len );

  // Update statistics
  p_Vid->p_Stats->bit_slice += len;
  cur_stats->bit_use_header[currSlice->slice_type] += len;

  if(currSlice->UseRDOQuant == 1 && currSlice->RDOQ_QP_Num > 1)
    get_dQP_table(currSlice);

#if EXT3D
  if ( p_Inp->VSD && p_Vid->is_depth && GetVOIdx(p_Vid->p_Inp,p_Vid->view_id) == 0 )
  {
    double dZnear, dZfar, dFL, dBaseLine;

    dZnear = p_Vid->ThreeDV_acquisition_info->depth_near_ae->original[0];
    dZfar  = p_Vid->ThreeDV_acquisition_info->depth_far_ae->original[0];
    dFL     = p_Vid->ThreeDV_acquisition_info->focal_length_x_ae->original[0];

    if( p_Inp->NumOfViews == 2 )
    {
      dBaseLine = p_Vid->ThreeDV_acquisition_info->translation_ae->original[1];
      p_Inp->dispCoeff = fabs( dFL * ( dBaseLine / 2.0 ) / 255.0 * ( 1.0/dZnear - 1.0/dZfar ) );
    }
    else if( p_Inp->NumOfViews == 3 )
    {
      dBaseLine = fabs( p_Vid->ThreeDV_acquisition_info->translation_ae->original[1] );
      dBaseLine += fabs( p_Vid->ThreeDV_acquisition_info->translation_ae->original[2] );
      p_Inp->dispCoeff = fabs( dFL * ( dBaseLine / 4.0 ) / 255.0 * ( 1.0/dZnear - 1.0/dZfar ) );
    }

    p_Inp->dispCoeffBase = p_Inp->dispCoeff;
  }

  if ( p_Inp->VSD && p_Vid->is_depth )
  {
    int voidx=GetVOIdx(p_Vid->p_Inp,p_Vid->view_id);
    p_Inp->dispCoeffSynDir = ( p_Inp->posCam[voidx] == 0 ) ? 1.0 : sqrt(0.5);

    if ( p_Vid->p_dual_picture )
      p_Inp->dispCoeff = p_Inp->dispCoeffBase * p_Inp->dispCoeffSynDir * p_Inp->dispCoeffSynPos;
    else
      p_Inp->dispCoeff = p_Inp->dispCoeffBase * p_Inp->dispCoeffSynDir * p_Inp->dispCoeffSynPos * p_Vid->p_DualInp->dRatio;

    //    printf("(%f --> %f)  %f  %f  %f \n", p_Inp->dispCoeffBase, p_Inp->dispCoeff, p_Inp->dispCoeffSynDir, p_Inp->dispCoeffSynPos, p_Vid->p_DualInp->dRatio );
  }
#endif

  while (end_of_slice == FALSE) // loop over macroblocks
  {
    Boolean recode_macroblock = FALSE;
    if (p_Vid->AdaptiveRounding && p_Inp->AdaptRndPeriod && (p_Vid->current_mb_nr % p_Inp->AdaptRndPeriod == 0))
    {
      CalculateOffset4x4Param(p_Vid);
      if(p_Inp->Transform8x8Mode)
        CalculateOffset8x8Param(p_Vid);
    }

    if(currSlice->UseRDOQuant) // This needs revisit
      currSlice->rddata = &currSlice->rddata_trellis_curr;
    else
      currSlice->rddata = &currSlice->rddata_top_frame_mb;   // store data in top frame MB

    start_macroblock (currSlice,  &currMB, CurrentMbAddr, FALSE);

    if(currSlice->UseRDOQuant)
    {
      trellis_coding(currMB);   
    }
    else
    {
      p_Vid->masterQP = p_Vid->qp;

#if EXT3D 
      encode_one_alc_macroblock(currMB);
#else
      currSlice->encode_one_macroblock (currMB);
#endif

      end_encode_one_macroblock(currMB);

      write_macroblock (currMB, 1);

#if EXT3D 
      // restores search options after alc cheking
      if (1 == currMB->is_alc_mode)
        reset_params_after_alc( currMB);
#endif
    }

    end_macroblock (currMB, &end_of_slice, &recode_macroblock);
    currMB->prev_recode_mb = recode_macroblock;
    //       printf ("encode_one_slice: mb %d,  slice %d,   bitbuf bytepos %d EOS %d\n",
    //       p_Vid->current_mb_nr, p_Vid->current_slice_nr,
    //       currSlice->partArr[0].bitstream->byte_pos, end_of_slice);

    if (recode_macroblock == FALSE)       // The final processing of the macroblock has been done
    {
      p_Vid->SumFrameQP += currMB->qp;
      CurrentMbAddr = FmoGetNextMBNr (p_Vid, CurrentMbAddr);
      if (CurrentMbAddr == -1)   // end of slice
      {
        // printf ("FMO End of Slice Group detected, current MBs %d, force end of slice\n", NumberOfCodedMBs+1);
        end_of_slice = TRUE;
#if EXT3D
    if(currSlice->series_7skip_len != 0)
    {
      const int*      partMap    = assignSE2partition[currSlice->partition_mode];

      if(currMB->mbAddrX < (int)p_Vid->PicSizeInMbs - 1 )
      {
        int i;
        // write unsuccess series flag
        biari_encode_symbol (&(currSlice->partArr[partMap[SE_MBTYPE]].ee_cabac), 0, &currSlice->mot_ctx->mb_type_contexts[0][7]);
        // puts skip and eos flags, which skipped in previous macroblocks
        for( i = SMRC_SKIP_SERIES_MAX_LEN; i > currSlice->series_7skip_len; i--)
        {
          biari_encode_symbol (&(currSlice->partArr[partMap[SE_MBTYPE]].ee_cabac), 1, &currSlice->mot_ctx->mb_type_contexts[2][7]);
          if( i != currSlice->series_7skip_len+1)
            biari_encode_symbol_final(&(currSlice->partArr[partMap[SE_MBTYPE]].ee_cabac), 0);
        }
      }
      else
      {
        int i;
        // write success flag for series (because just end of slice and end of picture both)
        biari_encode_symbol (&(currSlice->partArr[partMap[SE_MBTYPE]].ee_cabac), 1, &currSlice->mot_ctx->mb_type_contexts[0][7]);
        for( i = SMRC_SKIP_SERIES_MAX_LEN; i > currSlice->series_7skip_len; i--)
          if( i != currSlice->series_7skip_len+1)
            biari_encode_symbol_final(&(currSlice->partArr[partMap[SE_MBTYPE]].ee_cabac), 0);
      }

      // finishes series
      currSlice->series_7skip_len = 0;
    }
#endif
      }
      NumberOfCodedMBs++;       // only here we are sure that the coded MB is actually included in the slice
      next_macroblock (currMB);
    }
    else
    {
      //!Go back to the previous MB to recode it
      p_Vid->current_mb_nr = FmoGetPreviousMBNr(p_Vid, p_Vid->current_mb_nr);
      p_Vid->NumberofCodedMacroBlocks--;
      if(p_Vid->current_mb_nr == -1 )   // The first MB of the slice group  is too big,
        // which means it's impossible to encode picture using current slice bits restriction
      {
        snprintf (errortext, ET_SIZE, "Error encoding first MB with specified parameter, bits of current MB may be too big");
        error (errortext, 300);
      }
    }
  }


  if ((p_Inp->WPIterMC) && (p_Vid->frameOffsetAvail == 0) && p_Vid->nal_reference_idc)
  {
    compute_offset(currSlice);
  }
  p_Vid->num_ref_idx_l0_active = currSlice->num_ref_idx_active[LIST_0];
  p_Vid->num_ref_idx_l1_active = currSlice->num_ref_idx_active[LIST_1];

  terminate_slice (currMB, (NumberOfCodedMBs + TotalCodedMBs >= (int)p_Vid->PicSizeInMbs), cur_stats );
  return NumberOfCodedMBs;
}


/*!
************************************************************************
* \brief
*    Encodes one slice (MBAFF Frame)
* \par
*   returns the number of coded MBs in the SLice
************************************************************************
*/
int encode_one_slice_MBAFF (VideoParameters *p_Vid, int SliceGroupId, int TotalCodedMBs)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  Boolean end_of_slice = FALSE;
  Boolean recode_macroblock;
  int len;
  int NumberOfCodedMBs = 0;
  Macroblock* currMB      = NULL;
  int CurrentMbAddr;
  double FrameRDCost = DBL_MAX, FieldRDCost = DBL_MAX;
  StatParameters *cur_stats = &p_Vid->enc_picture->stats;
  Slice *currSlice = NULL;  

  if( (p_Inp->separate_colour_plane_flag != 0) )
  {
    change_plane_JV( p_Vid, p_Vid->colour_plane_id );
  }

  p_Vid->cod_counter = 0;

  CurrentMbAddr = FmoGetFirstMacroblockInSlice (p_Vid, SliceGroupId);
  // printf ("\n\nEncode_one_slice: PictureID %d SliceGroupId %d  SliceID %d  FirstMB %d \n", p_Vid->frame_no, SliceGroupId, p_Vid->current_slice_nr, CurrentMbInScanOrder);

  init_slice (p_Vid, &currSlice, CurrentMbAddr);
  currSlice->rdoq_motion_copy = 0;
  init_bipred_enabled(p_Vid);

  // Initialize quantization functions based on rounding/quantization method
  // Done here since we may wish to disable adaptive rounding on occasional intervals (even at a frame or gop level).
  init_quant_4x4   (currSlice);
  init_quant_8x8   (currSlice);
  init_quant_Chroma(currSlice);

  currSlice->set_lagrangian_multipliers(currSlice);

  if (currSlice->symbol_mode == CABAC)
  {
    SetCtxModelNumber (currSlice);
  }

  p_Vid->checkref = (short) (p_Inp->rdopt && p_Inp->RestrictRef && (currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE));

  len = start_slice (currSlice, cur_stats);

  // Rate control
  if (p_Inp->RCEnable)
    rc_store_slice_header_bits( p_Vid, p_Inp, len );

  // Update statistics
  p_Vid->p_Stats->bit_slice += len;
  cur_stats->bit_use_header[p_Vid->type] += len;

  while (end_of_slice == FALSE) // loop over macroblocks
  {

    if (p_Vid->AdaptiveRounding && p_Inp->AdaptRndPeriod && (p_Vid->current_mb_nr % p_Inp->AdaptRndPeriod == 0))
    {
      CalculateOffset4x4Param(p_Vid);
      if(p_Inp->Transform8x8Mode)
        CalculateOffset8x8Param(p_Vid);
    }


    //! This following ugly code breaks slices, at least for a slice mode that accumulates a certain
    //! number of bits into one slice.
    //! The suggested algorithm is as follows:
    //!
    //! SaveState (Bitstream, stats,  etc. etc.);
    //! BitsForThisMBPairInFrameMode = CodeMB (Upper, FRAME_MODE) + CodeMB (Lower, FRAME_MODE);
    //! DistortionForThisMBPairInFrameMode = CalculateDistortion(Upper) + CalculateDistortion (Lower);
    //! RestoreState();
    //! BitsForThisMBPairInFieldMode = CodeMB (Upper, FIELD_MODE) + CodeMB (Lower, FIELD_MODE);
    //! DistortionForThisMBPairInFrameMode = CalculateDistortion(Upper) + CalculateDistortion (Lower);
    //! FrameFieldMode = Decision (...)
    //! RestoreState()
    //! if (FrameFieldMode == FRAME) {
    //!   CodeMB (Upper, FRAME); CodeMB (Lower, FRAME);
    //! } else {
    //!   CodeMB (Upper FIELD); CodeMB (Lower, FIELD);
    //! }
    //!
    //! Open questions/issues:
    //!   1. CABAC/CA-VLC state:  It seems that the CABAC/CA_VLC states are changed during the
    //!      dummy encoding processes (for the R-D based selection), but that they are never
    //!      reset, once the selection is made.  I believe that this breaks the MB-adaptive
    //!      frame/field coding.  The necessary code for the state saves is readily available
    //!      in macroblock.c, start_macroblock() and end_macroblock() (this code needs
    //!      to be double checked that it works with CA-VLC as well
    //!   2. would it be an option to allocate Bitstreams with zero data in them (or copy the
    //!      already generated bitstream) for the "test coding"?

    p_Vid->write_macroblock = FALSE;
    if (p_Inp->MbInterlace == ADAPTIVE_CODING || p_Inp->MbInterlace == FRAME_MB_PAIR_CODING)
    {
      //================ code MB pair as frame MB ================
      //----------------------------------------------------------
      recode_macroblock = FALSE;
      //set mv limits to frame type
      update_mv_limits(p_Vid, FALSE);

      p_Vid->field_mode = FALSE;  // MB coded as frame
      p_Vid->top_field  = FALSE;   // Set top field to 0

      //Rate control
      p_Vid->write_macroblock = FALSE;
      p_Vid->bot_MB = FALSE;

      // save RC state only when it is going to change
      if ( p_Inp->RCEnable && p_Inp->RCUpdateMode <= MAX_RC_MODE )
      {
        if ( p_Inp->MbInterlace == ADAPTIVE_CODING
          && p_Vid->NumberofCodedMacroBlocks > 0 && (p_Vid->NumberofCodedMacroBlocks % p_Vid->BasicUnit) == 0 )
          rc_copy_quadratic( p_Vid, p_Inp, p_Vid->p_rc_quad_init, p_Vid->p_rc_quad ); // save initial RC status
        if ( p_Inp->MbInterlace == ADAPTIVE_CODING )
          rc_copy_generic( p_Vid, p_Vid->p_rc_gen_init, p_Vid->p_rc_gen ); // save initial RC status
      }

      start_macroblock (currSlice, &currMB, CurrentMbAddr, FALSE);

      currSlice->rddata = &currSlice->rddata_top_frame_mb; // store data in top frame MB
      p_Vid->masterQP = p_Vid->qp;
      currSlice->encode_one_macroblock (currMB);   // code the MB as frame
      end_encode_one_macroblock(currMB);


      FrameRDCost = currSlice->rddata->min_rdcost;
      //***   Top MB coded as frame MB ***//

      //Rate control
      p_Vid->bot_MB = TRUE; //for Rate control

      // go to the bottom MB in the MB pair
      p_Vid->field_mode = FALSE;  // MB coded as frame  //GB

      start_macroblock (currSlice, &currMB, CurrentMbAddr + 1, FALSE);
      currSlice->rddata = &currSlice->rddata_bot_frame_mb; // store data in top frame MB
      p_Vid->masterQP = p_Vid->qp;
      currSlice->encode_one_macroblock (currMB);         // code the MB as frame
      end_encode_one_macroblock(currMB);

      if ( p_Inp->RCEnable && p_Inp->RCUpdateMode <= MAX_RC_MODE )
      {
        if ( p_Inp->MbInterlace == ADAPTIVE_CODING
          && p_Vid->NumberofCodedMacroBlocks > 0 && (p_Vid->NumberofCodedMacroBlocks % p_Vid->BasicUnit) == 0 )
          rc_copy_quadratic( p_Vid, p_Inp, p_Vid->p_rc_quad_best, p_Vid->p_rc_quad ); // restore initial RC status

        if ( p_Inp->MbInterlace == ADAPTIVE_CODING )
          rc_copy_generic( p_Vid, p_Vid->p_rc_gen_best, p_Vid->p_rc_gen ); // save frame RC stats
      }

      FrameRDCost += currSlice->rddata->min_rdcost;
      //***   Bottom MB coded as frame MB ***//
    }

    if ((p_Inp->MbInterlace == ADAPTIVE_CODING) || (p_Inp->MbInterlace == FIELD_CODING))
    {
      //Rate control
      p_Vid->bot_MB = FALSE;
      //set mv limits to field type
      update_mv_limits(p_Vid, TRUE);

      //=========== start coding the MB pair as a field MB pair =============
      //---------------------------------------------------------------------
      p_Vid->field_mode = TRUE;  // MB coded as field
      p_Vid->top_field = TRUE;   // Set top field to 1
      p_Inp->num_ref_frames <<= 1;
      currSlice->num_ref_idx_active[LIST_0] <<= 1;
      currSlice->num_ref_idx_active[LIST_0] += 1;

      if ( p_Inp->RCEnable && p_Inp->RCUpdateMode <= MAX_RC_MODE )
      {
        if ( p_Inp->MbInterlace == ADAPTIVE_CODING
          && p_Vid->NumberofCodedMacroBlocks > 0 && (p_Vid->NumberofCodedMacroBlocks % p_Vid->BasicUnit) == 0 )
          rc_copy_quadratic( p_Vid, p_Inp, p_Vid->p_rc_quad, p_Vid->p_rc_quad_init ); // restore initial RC status

        if ( p_Inp->MbInterlace == ADAPTIVE_CODING )
          rc_copy_generic( p_Vid, p_Vid->p_rc_gen, p_Vid->p_rc_gen_init ); // reset RC stats
      }

      start_macroblock (currSlice, &currMB, CurrentMbAddr, TRUE);

      currSlice->rddata = &currSlice->rddata_top_field_mb; // store data in top frame MB
      //        TopFieldIsSkipped = 0;        // set the top field MB skipped flag to 0
      p_Vid->masterQP = p_Vid->qp;
      currSlice->encode_one_macroblock (currMB);         // code the MB as field
      end_encode_one_macroblock(currMB);

      FieldRDCost = currSlice->rddata->min_rdcost;
      //***   Top MB coded as field MB ***//
      //Rate control
      p_Vid->bot_MB = TRUE;//for Rate control

      p_Vid->top_field = FALSE;   // Set top field to 0
      start_macroblock (currSlice, &currMB, CurrentMbAddr+1, TRUE);
      currSlice->rddata = &currSlice->rddata_bot_field_mb; // store data in top frame MB
      p_Vid->masterQP = p_Vid->qp;
      currSlice->encode_one_macroblock (currMB);         // code the MB as field
      end_encode_one_macroblock(currMB);

      FieldRDCost += currSlice->rddata->min_rdcost;
      //***   Bottom MB coded as field MB ***//
    }

    //Rate control
    p_Vid->write_mbaff_frame = 0;  //Rate control

    //=========== decide between frame/field MB pair ============
    //-----------------------------------------------------------
    if ( ((p_Inp->MbInterlace == ADAPTIVE_CODING) && (FrameRDCost < FieldRDCost)) || p_Inp->MbInterlace == FRAME_MB_PAIR_CODING )
    {
      p_Vid->field_mode = FALSE;
      p_Vid->MBPairIsField = FALSE;
      if ( p_Inp->MbInterlace != FRAME_MB_PAIR_CODING )
      {
        p_Inp->num_ref_frames >>= 1;
        currSlice->num_ref_idx_active[LIST_0] -= 1;
        currSlice->num_ref_idx_active[LIST_0] >>= 1;
      }

      if ( p_Inp->RCEnable && p_Inp->RCUpdateMode <= MAX_RC_MODE )
      {
        if ( p_Inp->MbInterlace == ADAPTIVE_CODING
          && p_Vid->NumberofCodedMacroBlocks > 0 && (p_Vid->NumberofCodedMacroBlocks % p_Vid->BasicUnit) == 0 )
          rc_copy_quadratic( p_Vid, p_Inp, p_Vid->p_rc_quad, p_Vid->p_rc_quad_best ); // restore initial RC status

        if ( p_Inp->MbInterlace == ADAPTIVE_CODING )
          rc_copy_generic( p_Vid, p_Vid->p_rc_gen, p_Vid->p_rc_gen_best ); // restore frame RC stats
      }

      //Rate control
      p_Vid->write_mbaff_frame = 1;  //for Rate control
    }
    else
    {
      p_Vid->field_mode = TRUE;
      p_Vid->MBPairIsField = TRUE;
    }

    //Rate control
    p_Vid->write_macroblock = TRUE;//Rate control

    if (p_Vid->MBPairIsField)
      p_Vid->top_field = TRUE;
    else
      p_Vid->top_field = FALSE;

    //Rate control
    p_Vid->bot_MB = FALSE;// for Rate control

    // go back to the Top MB in the MB pair
    start_macroblock (currSlice, &currMB, CurrentMbAddr, p_Vid->field_mode);

    currSlice->rddata =  p_Vid->field_mode ? &currSlice->rddata_top_field_mb : &currSlice->rddata_top_frame_mb;
    copy_rdopt_data  (currMB);  // copy the MB data for Top MB from the temp buffers
    write_macroblock (currMB, 1);     // write the Top MB data to the bitstream
    end_macroblock   (currMB, &end_of_slice, &recode_macroblock);     // done coding the Top MB
    currMB->prev_recode_mb = recode_macroblock;

    if (recode_macroblock == FALSE)       // The final processing of the macroblock has been done
    {
      p_Vid->SumFrameQP += currMB->qp;

      CurrentMbAddr = FmoGetNextMBNr (p_Vid, CurrentMbAddr);
      if (CurrentMbAddr == -1)   // end of slice
      {
        end_of_slice = TRUE;
      }
      NumberOfCodedMBs++;       // only here we are sure that the coded MB is actually included in the slice
      next_macroblock (currMB);

      //Rate control
      p_Vid->bot_MB = TRUE;//for Rate control
      // go to the Bottom MB in the MB pair
      p_Vid->top_field = FALSE;
      start_macroblock (currSlice, &currMB, CurrentMbAddr, p_Vid->field_mode);

      currSlice->rddata = p_Vid->field_mode ? &currSlice->rddata_bot_field_mb : &currSlice->rddata_bot_frame_mb;
      copy_rdopt_data  (currMB);  // copy the MB data for Bottom MB from the temp buffers

      write_macroblock (currMB, 0);     // write the Bottom MB data to the bitstream
      end_macroblock   (currMB, &end_of_slice, &recode_macroblock);     // done coding the Top MB
      currMB->prev_recode_mb = recode_macroblock;
      if (recode_macroblock == FALSE)       // The final processing of the macroblock has been done
      {
        p_Vid->SumFrameQP += currMB->qp;

        CurrentMbAddr = FmoGetNextMBNr (p_Vid, CurrentMbAddr);
        if (CurrentMbAddr == -1)   // end of slice
        {
          end_of_slice = TRUE;
        }
        NumberOfCodedMBs++;       // only here we are sure that the coded MB is actually included in the slice
        next_macroblock (currMB);
      }
      else
      {
        //Go back to the beginning of the macroblock pair to recode it
        p_Vid->current_mb_nr = FmoGetPreviousMBNr(p_Vid, p_Vid->current_mb_nr);
        p_Vid->NumberofCodedMacroBlocks -= 2;
        if(p_Vid->current_mb_nr == -1 )   // The first MB of the slice group  is too big,
          // which means it's impossible to encode picture using current slice bits restriction
        {
          snprintf (errortext, ET_SIZE, "Error encoding first MB with specified parameter, bits of current MB may be too big");
          error (errortext, 300);
        }
      }
    }
    else
    {
      //!Go back to the previous MB to recode it
      p_Vid->current_mb_nr = FmoGetPreviousMBNr(p_Vid, p_Vid->current_mb_nr);
      p_Vid->NumberofCodedMacroBlocks--;
      if(p_Vid->current_mb_nr == -1 )   // The first MB of the slice group  is too big,
        // which means it's impossible to encode picture using current slice bits restriction
      {
        snprintf (errortext, ET_SIZE, "Error encoding first MB with specified parameter, bits of current MB may be too big");
        error (errortext, 300);
      }
    }

    if (p_Vid->MBPairIsField)    // if MB Pair was coded as field the buffer size variables back to frame mode
    {
      p_Inp->num_ref_frames >>= 1;
      currSlice->num_ref_idx_active[LIST_0] -= 1;
      currSlice->num_ref_idx_active[LIST_0] >>= 1;
    }

    p_Vid->field_mode = p_Vid->top_field = FALSE; // reset to frame mode

    if ( !end_of_slice )
    {
      assert( CurrentMbAddr < (int)p_Vid->PicSizeInMbs );
      assert( CurrentMbAddr >= 0 );
      if (CurrentMbAddr == FmoGetLastCodedMBOfSliceGroup (p_Vid, FmoMB2SliceGroup (p_Vid, CurrentMbAddr)))
        end_of_slice = TRUE;        // just in case it doesn't get set in end_macroblock
    }
  }

  p_Vid->num_ref_idx_l0_active = currSlice->num_ref_idx_active[LIST_0];
  p_Vid->num_ref_idx_l1_active = currSlice->num_ref_idx_active[LIST_1];

  terminate_slice (currMB, (NumberOfCodedMBs + TotalCodedMBs >= (int)p_Vid->PicSizeInMbs), cur_stats );
  return NumberOfCodedMBs;
}

static void setup_cabac(Slice *currSlice, char *listXsize)
{
  seq_parameter_set_rbsp_t *active_sps = currSlice->active_sps;
  int i; 

  if (currSlice->slice_type == I_SLICE)
  {
    currSlice->set_modes_and_reframe = set_modes_and_reframe;
    currSlice->store_8x8_motion_vectors = NULL;
    currSlice->writeMB_Skip         = NULL;
#if EXT3D
    currSlice->writeMB_VSSkip          = NULL;
    currSlice->writeMB_DirectModeType  = NULL;
#endif
    currSlice->writeMB_typeInfo     = writeMB_I_typeInfo_CABAC;
    currSlice->writeB8_typeInfo     = writeB8_typeInfo_CABAC;
    currSlice->writeMotionInfo2NAL  = NULL;
    currSlice->write_MB_layer       = writeMBLayerISlice;
    for (i=0; i<6; i++)
    {
      currSlice->writeRefFrame[i]   = NULL;
    }
  }
  else if (currSlice->slice_type == B_SLICE)
  {
    currSlice->set_modes_and_reframe    = set_modes_and_reframe_b_slice;
    currSlice->store_8x8_motion_vectors = store_8x8_motion_vectors_b_slice;
    currSlice->writeMB_Skip             = writeMB_Bskip_flagInfo_CABAC;
#if EXT3D
    currSlice->writeMB_VSSkip           =   writeMB_Bvsskip_flagInfo_CABAC;
    currSlice->writeMB_DirectModeType   = writeMB_Bdirect_typeInfo_CABAC;
#endif
#if EXT3D
    currSlice->writeMB_Ref_VSP_flag     = writeMB_Ref_BVSP_flagInfo_CABAC;
#endif
    currSlice->writeMB_typeInfo         = writeMB_B_typeInfo_CABAC;
    currSlice->writeB8_typeInfo         = writeB8_B_typeInfo_CABAC;
    currSlice->writeMotionInfo2NAL      = write_bslice_motion_info_to_NAL;
    currSlice->write_MB_layer           = writeMBLayerBSlice;

    for (i=0; i<6; i++)
    {
      switch (listXsize[i])
      {
      case 0:
        currSlice->writeRefFrame[i]   = NULL;
        break;
      case 1:
        currSlice->writeRefFrame[i]   = writeSE_Dummy;
        break;
      default:
        currSlice->writeRefFrame[i]   = writeRefPic_B_CABAC;
      }
    }
  }
  else
  {
    currSlice->set_modes_and_reframe = set_modes_and_reframe_p_slice;
    currSlice->store_8x8_motion_vectors = store_8x8_motion_vectors_p_slice;
    currSlice->writeMB_Skip         = writeMB_Pskip_flagInfo_CABAC;
#if EXT3D
    currSlice->writeMB_VSSkip          = writeMB_Pvsskip_flagInfo_CABAC;
    currSlice->writeMB_DirectModeType  = NULL;
#endif
#if EXT3D
    currSlice->writeMB_Ref_VSP_flag   = writeMB_Ref_PVSP_flagInfo_CABAC;
#endif
    currSlice->writeMB_typeInfo     = writeMB_P_typeInfo_CABAC;
    currSlice->writeB8_typeInfo     = writeB8_typeInfo_CABAC;
    currSlice->writeMotionInfo2NAL  = write_pslice_motion_info_to_NAL;
    currSlice->write_MB_layer       = writeMBLayerPSlice;


    for (i=0; i<6; i++)
    {
      switch (listXsize[i])
      {
      case 0:
        currSlice->writeRefFrame[i]   = NULL;
        break;
      case 1:
        currSlice->writeRefFrame[i]   = writeSE_Dummy;
        break;
      default:
        currSlice->writeRefFrame[i]   = writeRefPic_P_CABAC;
      }
    }
  }

  currSlice->writeIntraPredMode     = writeIntraPredMode_CABAC;  
  currSlice->writeCoeff16x16        = writeCoeff16x16_CABAC;
  currSlice->writeMVD               = writeMVD_CABAC;
  currSlice->writeCBP               = writeCBP_CABAC;
  currSlice->writeDquant            = writeDquant_CABAC;
  currSlice->writeCIPredMode        = writeCIPredMode_CABAC;
  currSlice->writeFieldModeInfo     = writeFieldModeInfo_CABAC;
  currSlice->writeMB_transform_size = writeMB_transform_size_CABAC;

  if (active_sps->chroma_format_idc == YUV444)
    currSlice->write_and_store_CBP_block_bit = write_and_store_CBP_block_bit_444;
  else
    currSlice->write_and_store_CBP_block_bit = write_and_store_CBP_block_bit;

  memset(currSlice->coeff, 0 , 64 * sizeof(int));
  currSlice->coeff_ctr = 0;
  currSlice->pos       = 0;
}

static void setup_cavlc(Slice *currSlice, char *listXsize)
{
  seq_parameter_set_rbsp_t *active_sps = currSlice->active_sps;

  int i;
  currSlice->writeMB_typeInfo   = writeUVLC_CAVLC;
  currSlice->writeIntraPredMode = writeIntraPredMode_CAVLC;
  currSlice->writeB8_typeInfo   = writeSE_UVLC;
  currSlice->writeCoeff16x16    = writeCoeff16x16_CAVLC;

  for (i=0; i<6; i++)
  {
    switch (listXsize[i])
    {
    case 0:
      currSlice->writeRefFrame[i]   = NULL;
      break;
    case 1:
      currSlice->writeRefFrame[i]   = writeSE_Dummy;
      break;
    case 2:
      currSlice->writeRefFrame[i]   = writeSE_invFlag;
      break;
    default:
      currSlice->writeRefFrame[i]   = writeSE_UVLC;
      break;
    }
  }

  if (currSlice->slice_type == I_SLICE || currSlice->slice_type == SI_SLICE)
  {
    currSlice->set_modes_and_reframe = set_modes_and_reframe;
    currSlice->store_8x8_motion_vectors = NULL;
    currSlice->writeMotionInfo2NAL  = NULL;
    currSlice->write_MB_layer       = writeMBLayerISlice;
  }
  else if (currSlice->slice_type == B_SLICE)
  {
    currSlice->set_modes_and_reframe = set_modes_and_reframe_b_slice;
    currSlice->store_8x8_motion_vectors = store_8x8_motion_vectors_b_slice;
    currSlice->writeMotionInfo2NAL  = write_bslice_motion_info_to_NAL;
    currSlice->write_MB_layer       = writeMBLayerBSlice;
  }
  else
  {
    currSlice->set_modes_and_reframe = set_modes_and_reframe_p_slice;
    currSlice->store_8x8_motion_vectors = store_8x8_motion_vectors_p_slice;
    currSlice->writeMotionInfo2NAL  = write_pslice_motion_info_to_NAL;
    currSlice->write_MB_layer       = writeMBLayerPSlice;
  }

  currSlice->writeMVD               = writeSVLC_CAVLC;
  currSlice->writeCBP               = writeCBP_VLC;
  currSlice->writeDquant            = writeSVLC_CAVLC;
  currSlice->writeCIPredMode        = writeUVLC_CAVLC;
  currSlice->writeFieldModeInfo     = writeFlag_CAVLC;
  currSlice->writeMB_transform_size = writeFlag_CAVLC;

  // We should move this to the sequence level and create a new function
  if (active_sps->chroma_format_idc == YUV444)
    currSlice->writeCoeff4x4_CAVLC = writeCoeff4x4_CAVLC_444;
  else
    currSlice->writeCoeff4x4_CAVLC = writeCoeff4x4_CAVLC_normal;
}


static void setup_slice(Slice *currSlice)
{  
#if EXT3D
  int voidx=GetVOIdx(currSlice->p_Inp,currSlice->view_id);
#endif
  if (currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE)
  {
    VideoParameters *p_Vid = currSlice->p_Vid;
    //InputParameters *p_Inp = currSlice->p_Inp;
    currSlice->weighted_prediction = p_Vid->active_pps->weighted_pred_flag;

    currSlice->set_modes_and_refs_for_blocks = set_modes_and_refs_for_blocks_p_slice;
    currSlice->set_coeff_and_recon_8x8       = set_coeff_and_recon_8x8_p_slice;
    currSlice->init_lists = init_lists_p_slice;
    currSlice->submacroblock_mode_decision = submacroblock_mode_decision_p_slice;
  }
  else if (currSlice->slice_type == B_SLICE)
  {
    VideoParameters *p_Vid = currSlice->p_Vid;
    //InputParameters *p_Inp = currSlice->p_Inp;
    currSlice->weighted_prediction = p_Vid->active_pps->weighted_bipred_idc;
    currSlice->set_modes_and_refs_for_blocks = set_modes_and_refs_for_blocks_b_slice;
    currSlice->set_coeff_and_recon_8x8 = set_coeff_and_recon_8x8_b_slice;
#if EXT3D
    if(voidx&&((currSlice->p_Vid->p_curr_frm_struct->simulcast_type==P_SLICE)||(currSlice->p_Vid->p_curr_frm_struct->simulcast_type==I_SLICE)))
      currSlice->init_lists=init_lists_p_slice;
    else
      currSlice->init_lists= init_lists_b_slice  ;
#else
    currSlice->init_lists = init_lists_b_slice;
#endif
    currSlice->submacroblock_mode_decision = submacroblock_mode_decision_b_slice;
  }
  else
  {
    currSlice->set_modes_and_refs_for_blocks = set_modes_and_refs_for_blocks_i_slice;
    currSlice->set_coeff_and_recon_8x8 = NULL;
    currSlice->init_lists = init_lists_i_slice;
    currSlice->submacroblock_mode_decision = NULL;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Initializes the parameters for a new slice and
 *     allocates the memory for the coded slice in the Picture structure
 *  \par Side effects:
 *      Adds slice/partition header symbols to the symbol buffer
 *      increments Picture->no_slices, allocates memory for the
 *      slice, sets p_Vid->currSlice
 ************************************************************************
 */
void init_slice (VideoParameters *p_Vid, Slice **currSlice, int start_mb_addr)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int i,j;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  Picture *currPic = p_Vid->currentPicture;
  DataPartition *dataPart;
  Bitstream *currStream;
  int active_ref_lists = (p_Vid->mb_aff_frame_flag) ? 6 : 2;
  DecodedPictureBuffer *p_Dpb = p_Vid->p_Dpb;
#if EXT3D //GRADUAL_VIEW_REFRESH
  int TL0_MMCO_flag;
#endif

# if EXT3D
  int voidx=GetVOIdx(p_Inp,p_Vid->view_id);
  if(p_Inp->NumOfViews>1)
  {
    if(p_Vid->structure!=BOTTOM_FIELD)
      p_Vid->anchor_pic_flag[0] = p_Vid->p_curr_frm_struct->anchor_pic_flag;
    else  // bottom will follow top for now
      p_Vid->anchor_pic_flag[1] = 0; //p_Vid->anchor_pic_flag[0];


    p_Vid->non_idr_flag[p_Vid->structure!=BOTTOM_FIELD ? 0:1] = ((p_Vid->currentPicture->idr_flag==1) ? 0:1);
    p_Vid->priority_id                                      = ((voidx==0) ? 0:1);
    // p_Vid->temporal_id                                      = 0;
    if(p_Inp->NumOfViews>1)
    {
      p_Vid->inter_view_flag[0]                 = p_Vid->p_curr_frm_struct->inter_view_flag  ;
      p_Vid->inter_view_flag[1]                = p_Vid->p_curr_frm_struct->inter_view_flag;
    }
    else
    {
      p_Vid->inter_view_flag[0]=p_Vid->inter_view_flag[1]=0;
    }
  }

#else
#if (MVC_EXTENSION_ENABLE)
  if(p_Inp->num_of_views == 2)
  {
    int enable_inter_view = (p_Vid->nal_reference_idc || p_Inp->enable_inter_view_flag) ? 1 : 0;
    p_Vid->non_idr_flag[p_Vid->structure!=BOTTOM_FIELD ? 0:1] = ((p_Vid->currentPicture->idr_flag==1) ? 0:1);
    p_Vid->priority_id                                      = ((p_Vid->view_id==0) ? 0:1);
    p_Vid->temporal_id                                      = 0;
    p_Vid->inter_view_flag[0]                               = ((p_Vid->view_id == 0) ? enable_inter_view : 0);
    p_Vid->inter_view_flag[1]                               = ((p_Vid->view_id == 0) ? enable_inter_view : 0);

    if(p_Vid->structure!=BOTTOM_FIELD)
      p_Vid->anchor_pic_flag[0] = (((p_Vid->currentPicture->idr_flag && p_Vid->nal_reference_idc!=0) || p_Vid->prev_view_is_anchor) ? 1:0);
    else  // bottom will follow top for now
      p_Vid->anchor_pic_flag[1] = 0; //p_Vid->anchor_pic_flag[0];
  }
#endif
#endif

  p_Vid->current_mb_nr = start_mb_addr;

  // Allocate new Slice in the current Picture, and set p_Vid->currentSlice
  assert (currPic != NULL);
  currPic->no_slices++;

  if (currPic->no_slices >= MAXSLICEPERPICTURE)
    error ("Too many slices per picture, increase MAXSLICEPERPICTURE in global.h.", -1);

  currPic->slices[currPic->no_slices - 1] = malloc_slice(p_Vid, p_Inp);
  *currSlice = currPic->slices[currPic->no_slices-1];

  p_Vid->currentSlice = *currSlice;
  // Using this trick we can basically reference back to the image and input parameter structures
  (*currSlice)->p_Vid             = p_Vid;
  (*currSlice)->p_Inp             = p_Inp;
  (*currSlice)->p_Dpb             = p_Dpb;
  (*currSlice)->active_sps        = active_sps;
  (*currSlice)->active_pps        = p_Vid->active_pps;
#if EXT3D
  (*currSlice)->active_dps        = p_Vid->active_dps;
#endif
  (*currSlice)->picture_id        = (p_Vid->frame_no & 0xFF); // % 255
  (*currSlice)->slice_nr          = p_Vid->current_slice_nr;
  (*currSlice)->idr_flag          = p_Vid->currentPicture->idr_flag;
  (*currSlice)->slice_type        = p_Vid->type;
  (*currSlice)->frame_no          = p_Vid->frame_no;
  (*currSlice)->frame_num         = p_Vid->frame_num;
  (*currSlice)->max_frame_num     = p_Vid->max_frame_num;
  (*currSlice)->framepoc          = p_Vid->framepoc;
  (*currSlice)->ThisPOC           = p_Vid->ThisPOC;
  (*currSlice)->qp                = p_Vid->p_curr_frm_struct->qp;
  (*currSlice)->start_mb_nr       = start_mb_addr;
  (*currSlice)->colour_plane_id   = p_Vid->colour_plane_id;

  (*currSlice)->si_frame_indicator =  p_Vid->type == SI_SLICE ? TRUE : FALSE;
  (*currSlice)->sp2_frame_indicator = p_Vid->sp2_frame_indicator;

  (*currSlice)->P444_joined       = p_Vid->P444_joined;
  (*currSlice)->disthres          = p_Inp->disthres;
  (*currSlice)->UseRDOQuant       = p_Inp->UseRDOQuant;
  (*currSlice)->RDOQ_QP_Num       = p_Inp->RDOQ_QP_Num;
  (*currSlice)->Transform8x8Mode  = p_Inp->Transform8x8Mode;

  (*currSlice)->slice_too_big     = dummy_slice_too_big;  
  (*currSlice)->width_blk         = p_Vid->width_blk;
  (*currSlice)->height_blk        = p_Vid->height_blk;
  (*currSlice)->partition_mode    = (short) p_Inp->partition_mode;
  (*currSlice)->PicSizeInMbs      = p_Vid->PicSizeInMbs;
  (*currSlice)->num_blk8x8_uv     = p_Vid->num_blk8x8_uv;
  (*currSlice)->nal_reference_idc = p_Vid->nal_reference_idc;
  (*currSlice)->bitdepth_luma     = p_Vid->bitdepth_luma;
  (*currSlice)->bitdepth_chroma   = p_Vid->bitdepth_chroma;  
  (*currSlice)->direct_spatial_mv_pred_flag  = p_Vid->direct_spatial_mv_pred_flag;
  (*currSlice)->mb_aff_frame_flag = p_Vid->mb_aff_frame_flag;
  (*currSlice)->structure      = p_Vid->structure;
  (*currSlice)->num_ref_idx_active[LIST_0] = p_Vid->active_pps->num_ref_idx_l0_active_minus1 + 1;
  (*currSlice)->num_ref_idx_active[LIST_1] = p_Vid->active_pps->num_ref_idx_l1_active_minus1 + 1;

  (*currSlice)->DFDisableIdc  = (p_Vid->TurnDBOff) ? 1 : p_Vid->DFDisableIdc;
  (*currSlice)->DFAlphaC0Offset = p_Vid->DFAlphaC0Offset;
  (*currSlice)->DFBetaOffset    = p_Vid->DFBetaOffset;
#if EXT3D
  (*currSlice)->view_id       = p_Vid->view_id;
  (*currSlice)->is_depth      = p_Vid->is_depth;
  (*currSlice)->bVspRefExist  = FALSE;
  (*currSlice)->depth_based_mvp_flag=p_Vid->DepthBasedMVP;
  (*currSlice)->depth_range_wp=(p_Vid->is_depth)&&(p_Inp->DepthRangeBasedWP);

  if(voidx==0 && (*currSlice)->is_depth == 0)
  {
    for(i=0; i<p_Inp->NumOfViews; i++)
    {
      p_Vid->dec_view_flag[i][0] = 0;
      p_Vid->dec_view_flag[i][1] = 0;
    }
  }
  
  (*currSlice)->slice_header_pred_flag = (voidx==0&& (*currSlice)->is_depth == 0)?0:p_Vid->SliceHeaderPred;
  if(p_Vid->SliceHeaderPred)
  {
    (*currSlice)->pre_slice_header_src         = p_Vid->PredSliceHeaderSrc;
    (*currSlice)->pre_ref_lists_src            = p_Vid->PredRefListsSrc;
    (*currSlice)->pre_pred_weight_table_src    = p_Vid->PredWeightTableSrc;
    (*currSlice)->pre_dec_ref_pic_marking_src  = p_Vid->PredDecRefPicMarkingSrc;

#if FIX_SLICE_HEAD_PRED
    if(p_Vid->PredSliceHeaderSrc)
    {
      if((*currSlice)->is_depth && !voidx)
      {
        (*currSlice)->pre_slice_header_src = 2; //base depth view
      }
      else
      {
        if(p_Vid->dec_view_flag[voidx][1-(*currSlice)->is_depth])
        {
          (*currSlice)->pre_slice_header_src = 2; 
        }
        else
        {
          (*currSlice)->pre_slice_header_src = 1; 
          (*currSlice)->pre_dec_ref_pic_marking_src     = 0;
        }
      }
    }
#endif
    if(voidx==0 && (*currSlice)->is_depth)
    {
      if((*currSlice)->pre_slice_header_src==1 || (*currSlice)->pre_slice_header_src==3)
        (*currSlice)->pre_slice_header_src  = 2;
      if((*currSlice)->pre_ref_lists_src==1    || (*currSlice)->pre_ref_lists_src==3)
        (*currSlice)->pre_ref_lists_src  = 2;
      if((*currSlice)->pre_pred_weight_table_src==1 || (*currSlice)->pre_pred_weight_table_src==3)
        (*currSlice)->pre_pred_weight_table_src  = 2;
      if((*currSlice)->pre_dec_ref_pic_marking_src==1    || (*currSlice)->pre_dec_ref_pic_marking_src==3)
        (*currSlice)->pre_dec_ref_pic_marking_src  = 2;
    }
#if FIX_SLICE_HEAD_PRED
    if(p_Vid->SliceHeaderPred&&((*currSlice)->pre_slice_header_src==2 || (*currSlice)->pre_ref_lists_src==2 || (*currSlice)->pre_dec_ref_pic_marking_src==2 || (*currSlice)->pre_pred_weight_table_src==2))
#else
    if(p_Vid->SliceHeaderPred&&(p_Vid->PredSliceHeaderSrc==2 || p_Vid->PredRefListsSrc==2 || p_Vid->PredDecRefPicMarkingSrc==2 || p_Vid->PredWeightTableSrc==2))
#endif
    {
      if(!p_Vid->dec_view_flag[voidx][1-(*currSlice)->is_depth])
      {
        snprintf(errortext,ET_SIZE,"prediction from T/D in current view is not allowed, reset to 0\n");
#if FIX_SLICE_HEAD_PRED
        (*currSlice)->pre_slice_header_src        = 0;
        (*currSlice)->pre_ref_lists_src           = 0;
        (*currSlice)->pre_dec_ref_pic_marking_src = 0;
        (*currSlice)->pre_pred_weight_table_src   = 0;
#else
        (*currSlice)->slice_header_pred_flag = 0;
#endif
      }else
      {
        if(p_Vid->active_pps->deblocking_filter_control_present_flag!= p_Vid->p_DualVid->active_pps->deblocking_filter_control_present_flag || p_Vid->direct_spatial_mv_pred_flag!= p_Vid->p_DualVid->direct_spatial_mv_pred_flag)
        {
          snprintf(errortext,ET_SIZE,"prediction from T/D in current view is not allowed, reset to 0\n");
#if FIX_SLICE_HEAD_PRED
        (*currSlice)->pre_slice_header_src        = 0;
        (*currSlice)->pre_ref_lists_src           = 0;
        (*currSlice)->pre_dec_ref_pic_marking_src = 0;
        (*currSlice)->pre_pred_weight_table_src   = 0;
#else
          (*currSlice)->slice_header_pred_flag = 0;
#endif
        }
        else if(p_Vid->active_pps->deblocking_filter_control_present_flag == 1 && p_Vid->p_DualVid->active_pps->deblocking_filter_control_present_flag==1)
        {
          if((p_Vid->DFDisableIdc!= p_Vid->p_DualVid->DFDisableIdc) ||(p_Vid->DFAlphaC0Offset!= p_Vid->p_DualVid->DFAlphaC0Offset) || (p_Vid->DFBetaOffset!= p_Vid->p_DualVid->DFBetaOffset))
          {
            snprintf(errortext,ET_SIZE,"prediction from T/D in current view is not allowed, reset to 0\n");
#if FIX_SLICE_HEAD_PRED
        (*currSlice)->pre_slice_header_src        = 0;
        (*currSlice)->pre_ref_lists_src           = 0;
        (*currSlice)->pre_dec_ref_pic_marking_src = 0;
        (*currSlice)->pre_pred_weight_table_src   = 0;
#else
            (*currSlice)->slice_header_pred_flag = 0;
#endif
          }
        }

      }
    }
    if(voidx==0 && (*currSlice)->is_depth && p_Inp->SliceHeaderPred)
    {
      if((p_Vid->PredSliceHeaderSrc==1 || p_Vid->PredRefListsSrc==1 || p_Vid->PredDecRefPicMarkingSrc==1 || p_Vid->PredWeightTableSrc==1)
        ||(p_Vid->PredSliceHeaderSrc==3 || p_Vid->PredRefListsSrc==3 || p_Vid->PredDecRefPicMarkingSrc==3 || p_Vid->PredWeightTableSrc==3))
      {
        snprintf(errortext,ET_SIZE,"prediction from D in current view is not allowed, reset to 0\n");
#if FIX_SLICE_HEAD_PRED
        (*currSlice)->pre_slice_header_src        = 0;
        (*currSlice)->pre_ref_lists_src           = 0;
        (*currSlice)->pre_dec_ref_pic_marking_src = 0;
        (*currSlice)->pre_pred_weight_table_src   = 0;
#else
        (*currSlice)->slice_header_pred_flag = 0;
#endif
      }
    }
#if FIX_SLICE_HEAD_PRED
    if((*currSlice)->slice_header_pred_flag)
    {
      short iCurrType = (*currSlice)->slice_type;
      short iPredType = 0;
      switch ((*currSlice)->pre_slice_header_src)
      {
      case 1:
        assert(voidx>0);
        iPredType = p_Vid->dec_view_slice_type[voidx-1][(*currSlice)->is_depth];
        break;
      case 2: 
        iPredType = p_Vid->dec_view_slice_type[voidx][1-(*currSlice)->is_depth];
        break;
      case 3:
        iPredType = p_Vid->dec_view_slice_type[0][(*currSlice)->is_depth];
        break;
      }
      if(iCurrType != iPredType && (iCurrType ==B_SLICE || iPredType == B_SLICE) )
      {
        //P/B slice predicted from B/P slice
        snprintf(errortext,ET_SIZE,"prediction from one slice with different inter type is not allowed, p_Inp->SliceHeaderPred shall be reset to 0\n");
        error (errortext, 300);
      }
    }
#endif
  }else
  {
    p_Enc->p_InpDepth->PredSliceHeaderSrc      = 0;
    p_Enc->p_InpDepth->PredRefListsSrc         = 0;
    p_Enc->p_InpDepth->PredWeightTableSrc      = 0;
    p_Enc->p_InpDepth->PredDecRefPicMarkingSrc = 0;
  }
#endif

  // primary and redundant slices: number of references overriding.
  if(p_Inp->redundant_pic_flag)
  {
    if(!p_Vid->redundant_coding)
    {
      (*currSlice)->num_ref_idx_active[LIST_0] = (char) imin(p_Vid->number,p_Inp->NumRefPrimary);
    }
    else
    {
      // 1 reference picture for redundant slices
      (*currSlice)->num_ref_idx_active[LIST_0] = 1;
    }
  }

  for (i = 0; i < (*currSlice)->max_part_nr; i++)
  {
    dataPart = &(*currSlice)->partArr[i];

    currStream = dataPart->bitstream;
    currStream->bits_to_go = 8;
    currStream->byte_pos = 0;
    currStream->byte_buf = 0;
  }

  // code now also considers fields. Issue whether we should account this within the appropriate input p_Inp directly
  if ((p_Vid->type == P_SLICE || p_Vid->type == SP_SLICE) && p_Inp->P_List0_refs)
  {
    (*currSlice)->num_ref_idx_active[LIST_0] = (char) imin((*currSlice)->num_ref_idx_active[LIST_0], p_Inp->P_List0_refs * ((p_Vid->structure !=0) + 1));
  }

  if (p_Vid->type == B_SLICE )
  {
    if (p_Inp->B_List0_refs)
    {
      (*currSlice)->num_ref_idx_active[LIST_0] = (char) imin((*currSlice)->num_ref_idx_active[LIST_0], p_Inp->B_List0_refs * ((p_Vid->structure !=0) + 1));
    }
    if (p_Inp->B_List1_refs)
    {
      (*currSlice)->num_ref_idx_active[LIST_1] = (char) imin((*currSlice)->num_ref_idx_active[LIST_1], p_Inp->B_List1_refs * ((p_Vid->structure !=0) + 1));
    }
    get_mem3D((byte ****)(void*)&(*currSlice)->direct_ref_idx, (*currSlice)->height_blk, (*currSlice)->width_blk, 2);
    get_mem2D((byte ***) (void*)&(*currSlice)->direct_pdir,    (*currSlice)->height_blk, (*currSlice)->width_blk);
  }

  if ((*currSlice)->P444_joined)
  {
    (*currSlice)->luma_residual_coding = luma_residual_coding_p444;
    (*currSlice)->luma_residual_coding_8x8 = luma_residual_coding_p444_8x8;
  }
  else
  {
    if ((*currSlice)->slice_type == SP_SLICE)
      (*currSlice)->luma_residual_coding = luma_residual_coding_sp;
    else
      (*currSlice)->luma_residual_coding = luma_residual_coding;
    (*currSlice)->luma_residual_coding_8x8 = luma_residual_coding_8x8;
  }
  if ((*currSlice)->slice_type == SP_SLICE)
    (*currSlice)->chroma_residual_coding = chroma_residual_coding_sp;
  else if ((*currSlice)->slice_type == SI_SLICE)
    (*currSlice)->chroma_residual_coding = chroma_residual_coding_si;
  else
    (*currSlice)->chroma_residual_coding = chroma_residual_coding;

  setup_slice(*currSlice);

  // generate reference picture lists
#if MVC_EXTENSION_ENABLE||EXT3D
  //if (p_Vid->p_Inp->num_of_views == 2)
  {
    update_ref_list(p_Dpb);
    update_ltref_list(p_Dpb);
    check_num_ref(p_Dpb);
  }
#endif
  update_pic_num(*currSlice);
  (*currSlice)->init_lists(*currSlice);
#if EXT3D
#if ITRI_INTERLACE
  if((p_Inp->NumOfViews>1)&&voidx&&(p_Vid->structure==FRAME))
#else
  if((p_Inp->NumOfViews>1)&&voidx)
#endif
    init_interview_list(*currSlice);
#endif

#if EXT3D 
  if (1 == p_Inp->AdaptiveLuminanceCompensation && 0 == p_Vid->is_depth && voidx > 0 && (P_SLICE == (*currSlice)->slice_type))
    (*currSlice)->alc_slice_flag = 1;
  else
    (*currSlice)->alc_slice_flag = 0;
#endif

  // assign list 0 size from list size
  (*currSlice)->num_ref_idx_active[LIST_0] = (*currSlice)->listXsize[LIST_0];
  (*currSlice)->num_ref_idx_active[LIST_1] = (*currSlice)->listXsize[LIST_1];

  //if ( p_Inp->WPMCPrecision && p_Inp->WPMCPrecFullRef )
    //wpxAdaptRefNum(*currSlice);

#if !EXT3D
  (*currSlice)->num_ref_idx_active[LIST_0] = (*currSlice)->listXsize[0];
  (*currSlice)->num_ref_idx_active[LIST_1] = (*currSlice)->listXsize[1];
#endif

  //Perform memory management based on poc distances  
#if EXT3D
  TL0_MMCO_flag= (p_Inp->PocMemoryManagement == 2 || p_Inp->GradualViewRefresh == 0 || 
    (p_Inp->GradualViewRefresh==1 && (voidx==0 || ((*currSlice)->framepoc % (4*p_Inp->intra_period) != 0))));

  if(((p_Vid->structure==FRAME)&&(p_Inp->NumOfViews>1)&&
    (p_Vid->anchor_pic_flag[0]||(p_Inp->HierarchicalCoding&&(p_Vid->temporal_id==0))))&&TL0_MMCO_flag)
  {
    //Wenyi >>
    //!<For multi-view coding, the MMCO is based on anchor and zero temporal pictures.
    //!<If hierarchical B is enabled, the MMCO is applied when zero temporal pictures appears
    //!<Otherwise, MMCO is based on anchor period(intra period) 
    mvc_based_ref_management_frame_pic(p_Dpb,*currSlice,p_Vid->frame_num);
  }
  else
#endif
  if (p_Inp->SetFirstAsLongTerm && p_Vid->number == 0)
  {
    mmco_long_term(p_Vid, p_Vid->number);
  }
  else if (p_Vid->nal_reference_idc && p_Inp->PocMemoryManagement == 1)
  {
    if (p_Vid->structure == FRAME && p_Dpb->ref_frames_in_buffer == active_sps->num_ref_frames)
      poc_based_ref_management_frame_pic(p_Dpb, p_Vid->frame_num);
    else if (p_Vid->structure == TOP_FIELD && p_Dpb->ref_frames_in_buffer== active_sps->num_ref_frames)
      poc_based_ref_management_field_pic(p_Dpb, (p_Vid->frame_num << 1) + 1);      
    else if (p_Vid->structure == BOTTOM_FIELD)
      poc_based_ref_management_field_pic(p_Dpb, (p_Vid->frame_num << 1) + 1);
  }
  else if (p_Vid->nal_reference_idc && p_Inp->PocMemoryManagement == 2) 
  {
    if (p_Vid->structure == FRAME)
      tlyr_based_ref_management_frame_pic(p_Vid, p_Vid->frame_num);
  }
#if EXT3D // ADAPTIVE_MMCO_REORDER
  else if (p_Vid->nal_reference_idc && p_Inp->PocMemoryManagement == 3 && !p_Vid->ShortGOP && p_Inp->HierarchicalCoding == 3) 
  {
    if (p_Vid->structure == FRAME)
      layered_sliding_ref_management_frame_pic(p_Dpb, p_Vid->frame_num);
  }
#endif

  if (p_Inp->EnableOpenGOP)
  {
    if ((*currSlice)->slice_type != I_SLICE && (*currSlice)->slice_type != SI_SLICE)
    {
      for (i = 0; i<(*currSlice)->listXsize[0]; i++)
      {
        if ((*currSlice)->listX[0][i]->poc < p_Vid->last_valid_reference && p_Vid->ThisPOC > p_Vid->last_valid_reference)
        {
          (*currSlice)->listXsize[0] = (*currSlice)->num_ref_idx_active[LIST_0] = (char) imax(1, i);
          break;
        }
      }

      for (i = 0; i<(*currSlice)->listXsize[1]; i++)
      {
        if ((*currSlice)->listX[1][i]->poc < p_Vid->last_valid_reference && p_Vid->ThisPOC > p_Vid->last_valid_reference)
        {
          (*currSlice)->listXsize[1] = (*currSlice)->num_ref_idx_active[LIST_1] = (char) imax(1,i);
          break;
        }
      }
    }
  }

#if EXT3D // GRADUAL_VIEW_REFRESH
    if (p_Inp->GradualViewRefresh && (p_Vid->anchor_pic_flag[0]||((p_Inp->HierarchicalCoding)&&(p_Vid->temporal_id==0))))
    {
      (*currSlice)->listXsize[0] = (*currSlice)->num_ref_idx_active[LIST_0] = (char) imin(1, (*currSlice)->listXsize[0]);
    }//gopopen
#endif

  init_ref_pic_list_reordering(*currSlice, p_Inp->ReferenceReorder);

#if EXT3D
    // reference list reordering 
    // RPLR for redundant pictures
    // !KS: that should actually be moved somewhere else
    if(p_Inp->NumOfViews==1)
    {
      if(p_Inp->redundant_pic_flag && p_Vid->redundant_coding)
      {
        alloc_ref_pic_list_reordering_buffer(*currSlice);
        (*currSlice)->ref_pic_list_reordering_flag[LIST_0] = 1;
        (*currSlice)->reordering_of_pic_nums_idc[LIST_0][0] = 0;
        (*currSlice)->reordering_of_pic_nums_idc[LIST_0][1] = 3;
        (*currSlice)->abs_diff_pic_num_minus1[LIST_0][0] = p_Vid->redundant_ref_idx - 1;
        (*currSlice)->long_term_pic_idx[LIST_0][0] = 0;

        reorder_ref_pic_list ( *currSlice, LIST_0);
      }
      else if ( (p_Vid->type == P_SLICE || p_Vid->type == B_SLICE) && p_Inp->WPMCPrecision && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR )
        wp_mcprec_reorder_lists( *currSlice );
      else
        reorder_lists( *currSlice );
    }
    else
    {
#if ITRI_INTERLACE
      if (p_Vid->structure == FRAME)
#endif
        get_mvc_lists(*currSlice);
#if ITRI_INTERLACE
      else
        (*currSlice)->num_ref_idx_active[LIST_0] = (*currSlice)->listXsize[0];
#endif
    }
#else
#if (MVC_EXTENSION_ENABLE)
  p_Vid->MVCInterViewReorder = 0;
  if(p_Inp->num_of_views==2 && p_Inp->MVCInterViewReorder == 1 && p_Vid->view_id == 1)
  {
    // enable MVC reordering
    p_Vid->MVCInterViewReorder = 1;

    alloc_ref_pic_list_reordering_buffer(*currSlice);
    if((*currSlice)->listXsize[0]!=0)
    {
      StorablePicture *Inter_ViewL0 = (*currSlice)->listX[0][(*currSlice)->listXsize[0]-1];
      for(i=(*currSlice)->listXsize[0] - 1; i>0; i--)
        (*currSlice)->listX[0][i] = (*currSlice)->listX[0][i - 1];
      (*currSlice)->listX[0][0] = Inter_ViewL0;
      (*currSlice)->listXsize[0] = (char) imin ((*currSlice)->listXsize[0], (*currSlice)->num_ref_idx_active[LIST_0]);
    }
    // perform reordering of interview pictures
  }
  else
#endif
    // reference list reordering 
    // RPLR for redundant pictures
    // !KS: that should actually be moved somewhere else
    if(p_Inp->redundant_pic_flag && p_Vid->redundant_coding)
    {
      alloc_ref_pic_list_reordering_buffer(*currSlice);
      (*currSlice)->ref_pic_list_reordering_flag[LIST_0] = 1;
      (*currSlice)->reordering_of_pic_nums_idc[LIST_0][0] = 0;
      (*currSlice)->reordering_of_pic_nums_idc[LIST_0][1] = 3;
      (*currSlice)->abs_diff_pic_num_minus1[LIST_0][0] = p_Vid->redundant_ref_idx - 1;
      (*currSlice)->long_term_pic_idx[LIST_0][0] = 0;

      reorder_ref_pic_list ( *currSlice, LIST_0);
    }
    else if ( (p_Vid->type == P_SLICE || p_Vid->type == B_SLICE) && p_Inp->WPMCPrecision && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR )
      wp_mcprec_reorder_lists( *currSlice );
    else
      reorder_lists( *currSlice );
#endif

#if 0
      if ((*currSlice)->framepoc == 1*2 && (*currSlice)->is_depth == 0)
      {
        FILE *fp = fopen("tmp_ref_enc.yuv", "ab");
        if (fp)
        {
          int listSize = (*currSlice)->listXsize[0]; 
          int iRefIdxTest;
          for(iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
          {
            StorablePicture* pElem = (*currSlice)->listX[0][iRefIdxTest];
            int x, y;
            //write_picture( pElem, p_Inp->output, fp);
            for (y=0; y<pElem->size_y; y++)
              for (x=0; x<pElem->size_x; x++)
                fwrite(&pElem->imgY[y][x], 1, 1, fp);
            for (y=0; y<pElem->size_y_cr; y++)
              for (x=0; x<pElem->size_x_cr; x++)
                fwrite(&pElem->imgUV[0][y][x], 1, 1, fp);
            for (y=0; y<pElem->size_y_cr; y++)
              for (x=0; x<pElem->size_x_cr; x++)
                fwrite(&pElem->imgUV[1][y][x], 1, 1, fp);
            printf("Print ref frame to tmp file %d.\n", iRefIdxTest);
          }
          fclose(fp);
        }
      }
#endif

#if EXT3D
    // update the idxInterviewRef array for alc
    if((*currSlice)->alc_slice_flag)
    {
      int currPoc    = (*currSlice)->framepoc;
      int currViewId = (*currSlice)->view_id;
      int currLayer  = (*currSlice)->is_depth;
      int listSize, iRefIdxTest;
      StorablePicture* pElem;
      (*currSlice)->idxInterviewRef[0] = (*currSlice)->idxInterviewRef[1] = -1;
      (*currSlice)->bIVRefExist = FALSE;
      if ((*currSlice)->slice_type == P_SLICE && !currLayer)
      {
        listSize = (*currSlice)->listXsize[0]; 
        for(iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
        {
          pElem = (*currSlice)->listX[0][iRefIdxTest];
          if (pElem->frame_poc == currPoc && !pElem->is_depth && pElem->view_id != currViewId)
          {
            (*currSlice)->bIVRefExist = TRUE;
            (*currSlice)->idxInterviewRef[0] = iRefIdxTest;
            break;
          }
        }
      }
    }

    if((*currSlice)->slice_type == B_SLICE && p_Inp->RLESkip && voidx>0 && !(*currSlice)->is_depth)
      (*currSlice)->RLCSkip_enabled = 1;
    else
      (*currSlice)->RLCSkip_enabled = 0;

    {
      int list, numlist=0;
      (*currSlice)->hasInterviewRef = 0;

      if( (*currSlice)->slice_type==P_SLICE || (*currSlice)->slice_type==SP_SLICE )
      {
        numlist = 1;
      }
      else if( (*currSlice)->slice_type==B_SLICE )
      {
        numlist = 2;
      }

      for( list = 0; list < numlist; list++)
      {
        for( i = 0; i < (*currSlice)->listXsize[list]; i++ )
        {
          if( ( (*currSlice)->framepoc == (*currSlice)->listX[list][i]->frame_poc ) && ( (*currSlice)->view_id != (*currSlice)->listX[list][i]->view_id ) )
          {
            (*currSlice)->hasInterviewRef=1;
            break;
          }
        }
      }
    }

      // The ref pic lists are prepared, update the VSP ref flag
    {
      int currPoc    = (*currSlice)->framepoc; // ThisPOC?
      int currViewId = (*currSlice)->view_id;
      int currLayer  = (*currSlice)->is_depth;
      int listSize;
      StorablePicture* pElem;
      int iRefIdxTest;

      (*currSlice)->Harmonize_VSP_IVP = 0;
      (*currSlice)->bVspRefExist = FALSE;
      (*currSlice)->idxVspRef[0] = (*currSlice)->idxVspRef[1] = -1;

      if ((*currSlice)->slice_type != I_SLICE && (*currSlice)->slice_type != SI_SLICE )
      {
        listSize = (*currSlice)->listXsize[0]; 
        for(iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
        {
          pElem = (*currSlice)->listX[0][iRefIdxTest];
          if ((p_Vid->p_Inp->CompatibilityCategory == 1)&&(pElem->frame_poc == currPoc && pElem->is_depth == currLayer && pElem->view_id != currViewId && currLayer==0))
          {
            (*currSlice)->bVspRefExist = TRUE;
            (*currSlice)->idxVspRef[0] = iRefIdxTest;
            break;
          }
        }
        if ((*currSlice)->slice_type == B_SLICE)
        {
          listSize = (*currSlice)->listXsize[1]; 
          for( iRefIdxTest = 0; iRefIdxTest < listSize; iRefIdxTest++ ) 
          {
            pElem = (*currSlice)->listX[1][iRefIdxTest];
            if ((p_Vid->p_Inp->CompatibilityCategory == 1)&&(pElem->frame_poc == currPoc && pElem->is_depth == currLayer && pElem->view_id != currViewId && currLayer==0)) // @DT: Public version change rev380
            {
              (*currSlice)->bVspRefExist = TRUE;
              (*currSlice)->idxVspRef[1] = iRefIdxTest;
              break;
            }
          }
        }
      }

      if ((*currSlice)->is_depth==0 )
      {
        if ( (*currSlice)->p_Vid->p_Inp->VSP_Enable==0 )
        {
          (*currSlice)->bVspRefExist = FALSE;
        }
      }

      (*currSlice)->Harmonize_VSP_IVP = (*currSlice)->bVspRefExist;
    }
#endif

    if ( p_Vid->type == P_SLICE )
    {
#if PRINTREFLIST
#if EXT3D
        // print out for debug purpose
        if(p_Inp->NumOfViews>=1)
        {
          if((*currSlice)->listXsize[0]>0)
          {
            printf("\n");
            printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
            for(i=0; i<(int)((*currSlice)->listXsize[0]); i++)  //ref list 0
            {
              printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d TId:%d\n", i, (*currSlice)->listX[0][i]->poc, (*currSlice)->listX[0][i]->pic_num, (*currSlice)->listX[0][i]->view_id,(*currSlice)->listX[0][i]->temporal_layer);
            }
          }
        }
#else
#if (MVC_EXTENSION_ENABLE)
      // print out for debug purpose
      if(p_Inp->num_of_views==2 && p_Vid->current_slice_nr==0)
      {
        if((*currSlice)->listXsize[0]>0)
        {
          printf("\n");
          printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
          for(i=0; i<(int)((*currSlice)->listXsize[0]); i++)  //ref list 0
          {
            printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, (*currSlice)->listX[0][i]->poc, (*currSlice)->listX[0][i]->pic_num, (*currSlice)->listX[0][i]->view_id);
          }
        }
      }
#endif
#endif
#endif
    }
    else if ( p_Vid->type == B_SLICE )
    {
#if PRINTREFLIST
#if EXT3D
        if(p_Inp->NumOfViews>=1 )
        {
          if(((*currSlice)->listXsize[0]>0) || ((*currSlice)->listXsize[1]>0))
            printf("\n");
          if((*currSlice)->listXsize[0]>0)
          {
            printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
            for(i=0; i<(int)((*currSlice)->listXsize[0]); i++)  //ref list 0
            {
              printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d  TId:%d\n", i, (*currSlice)->listX[0][i]->poc, (*currSlice)->listX[0][i]->pic_num, (*currSlice)->listX[0][i]->view_id,(*currSlice)->listX[0][i]->temporal_layer);
            }
          }
          if((*currSlice)->listXsize[1]>0)
          {
            printf(" ** (CurViewID:%d) %s Ref Pic List 1 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
            for(i=0; i<(int)((*currSlice)->listXsize[1]); i++)  //ref list 1
            {
              printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d  TId:%d\n", i, (*currSlice)->listX[1][i]->poc, (*currSlice)->listX[1][i]->pic_num, (*currSlice)->listX[1][i]->view_id,(*currSlice)->listX[1][i]->temporal_layer);
            }
          }
        }
#else
#if (MVC_EXTENSION_ENABLE)
      // print out for debug purpose
      if(p_Inp->num_of_views==2 && p_Vid->current_slice_nr==0)
      {
        if(((*currSlice)->listXsize[0]>0) || ((*currSlice)->listXsize[1]>0))
          printf("\n");
        if((*currSlice)->listXsize[0]>0)
        {
          printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
          for(i=0; i<(int)((*currSlice)->listXsize[0]); i++)  //ref list 0
          {
            printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, (*currSlice)->listX[0][i]->poc, (*currSlice)->listX[0][i]->pic_num, (*currSlice)->listX[0][i]->view_id);
          }
        }
        if((*currSlice)->listXsize[1]>0)
        {
          printf(" ** (CurViewID:%d) %s Ref Pic List 1 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
          for(i=0; i<(int)((*currSlice)->listXsize[1]); i++)  //ref list 1
          {
            printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, (*currSlice)->listX[1][i]->poc, (*currSlice)->listX[1][i]->pic_num, (*currSlice)->listX[1][i]->view_id);
          }
        }
      }
#endif
#endif
#endif
    }

  (*currSlice)->max_num_references = (short) p_Vid->max_num_references;


  if (((*currSlice)->slice_type != I_SLICE) && (*currSlice)->slice_type != SI_SLICE)
  {
    get_mem_mv(*currSlice, &(*currSlice)->all_mv);  
#if EXT3D
        (*currSlice)->mv_backup_size = get_mem_mv(*currSlice, &(*currSlice)->all_mv_backup); 
        get_mem5Dshort(&((*currSlice)->isVSPRef),2, (*currSlice)->max_num_references, 11, 4, 4);

        if (0 == p_Vid->is_depth && GetVOIdx(p_Inp, p_Vid->view_id) > 0)
        {
          (*currSlice)->all_mv_size = get_mem_mv(*currSlice, &(*currSlice)->all_std_mv);
          get_mem5Dshort(&((*currSlice)->all_std_VSP),2, (*currSlice)->max_num_references, 11, 4, 4);
        }
#endif

    if (p_Inp->BiPredMotionEstimation && ((*currSlice)->slice_type == B_SLICE))
    {
#if EXT3D
      (*currSlice)->bi_mv_backup_size = get_mem_bipred_mv(*currSlice, &(*currSlice)->bipred_mv);
      get_mem_bipred_mv(*currSlice, &(*currSlice)->bipred_mv_backup);
      get_mem_bipred_mv(*currSlice, &(*currSlice)->bipred_mv_implicit);	
#else
      get_mem_bipred_mv(*currSlice, &(*currSlice)->bipred_mv);
#endif
    }

    if (p_Inp->UseRDOQuant && p_Inp->RDOQ_QP_Num > 1)
    {
      if (p_Inp->Transform8x8Mode && p_Inp->RDOQ_CP_MV)
      {
        get_mem4Dmv (&(*currSlice)->tmp_mv8, 2, (*currSlice)->max_num_references, 4, 4);
        get_mem3Ddistblk(&(*currSlice)->motion_cost8, 2, (*currSlice)->max_num_references, 4);
        get_mem4Dmv (&(*currSlice)->tmp_mv4, 2, (*currSlice)->max_num_references, 4, 4);
        get_mem3Ddistblk(&(*currSlice)->motion_cost4, 2, (*currSlice)->max_num_references, 4);
      }
    }
  }

  if (p_Vid->mb_aff_frame_flag)
    init_mbaff_lists(*currSlice);

#if EXT3D
    if ((p_Vid->type != I_SLICE && p_Vid->type != SI_SLICE) && 
      (p_Vid->active_pps->weighted_pred_flag == 1 || 
      (p_Vid->active_pps->weighted_bipred_idc > 0 && (p_Vid->type == B_SLICE))||p_Vid->p_Inp->DepthRangeBasedWP))
#else
  if ((p_Vid->type != I_SLICE && p_Vid->type != SI_SLICE) && (p_Vid->active_pps->weighted_pred_flag == 1 || (p_Vid->active_pps->weighted_bipred_idc > 0 && (p_Vid->type == B_SLICE))))
#endif
  {
    if (p_Vid->type == P_SLICE || p_Vid->type == SP_SLICE)
    {
      int wp_type = (p_Inp->GenerateMultiplePPS && p_Inp->RDPictureDecision) && (p_Vid->enc_picture != p_Vid->enc_frame_picture[1]);
      p_Vid->EstimateWPPSlice (*currSlice, wp_type);
    }
    else
      p_Vid->EstimateWPBSlice (*currSlice);
  }

  if (p_Vid->type == B_SLICE)
  {
     // Allocation should be based on slice size, not image
      compute_colocated(*currSlice, (*currSlice)->listX);
  }

  if (p_Vid->type != I_SLICE && p_Vid->type != SI_SLICE)
  {
    if (p_Inp->SearchMode == EPZS)
    {
      if (((*currSlice)->p_EPZS =  (EPZSParameters*) calloc(1, sizeof(EPZSParameters)))==NULL) 
        no_mem_exit("init_slice: p_EPZS");
      EPZSStructInit (*currSlice);
      EPZSSliceInit  (*currSlice);

#if EXT3D 
        // init of EPZS search for alc
        if ((*currSlice)->alc_slice_flag)
        {
          struct epzs_params *p_EPZS_save = (*currSlice)->p_EPZS;

          if (((*currSlice)->p_alcEPZS =  (EPZSParameters*) calloc(1, sizeof(EPZSParameters)))==NULL) 
            no_mem_exit("init_slice: p_alcEPZS");

          (*currSlice)->p_EPZS = (*currSlice)->p_alcEPZS;

          EPZSStructInit (*currSlice);
          EPZSSliceInit  (*currSlice);

          (*currSlice)->p_EPZS = p_EPZS_save;
        }
#endif
    }
  }


  if ((*currSlice)->symbol_mode == CAVLC)
  {
    setup_cavlc(*currSlice, (*currSlice)->listXsize);
  }
  else
  {
    setup_cabac(*currSlice, (*currSlice)->listXsize);
  }

  // assign luma common reference picture pointers to be used for ME/sub-pel interpolation

  for(i = 0; i < active_ref_lists; i++)
  {
    for(j = 0; j < (*currSlice)->listXsize[i]; j++)
    {
      if( (*currSlice)->listX[i][j] )
      {
        (*currSlice)->listX[i][j]->p_curr_img     = (*currSlice)->listX[i][j]->p_img    [(short) p_Vid->colour_plane_id];
        (*currSlice)->listX[i][j]->p_curr_img_sub = (*currSlice)->listX[i][j]->p_img_sub[(short) p_Vid->colour_plane_id];
      }
    }
  }

  if (p_Inp->UseRDOQuant)
  {
    if (((*currSlice)->estBitsCabac = (estBitsCabacStruct*) calloc(NUM_BLOCK_TYPES, sizeof(estBitsCabacStruct)))==NULL) 
      no_mem_exit("init_slice: (*currSlice)->estBitsCabac"); 

    init_rdoq_slice(*currSlice);

    alloc_rddata(*currSlice, &(*currSlice)->rddata_trellis_curr);
    if (p_Inp->RDOQ_QP_Num > 1)
    {
      alloc_rddata(*currSlice, &(*currSlice)->rddata_trellis_best);
    }
  }

  if(p_Vid->mb_aff_frame_flag)
  {
    alloc_rddata(*currSlice, &(*currSlice)->rddata_top_frame_mb);
    alloc_rddata(*currSlice, &(*currSlice)->rddata_bot_frame_mb);
    if ( p_Inp->MbInterlace != FRAME_MB_PAIR_CODING )
    {
      alloc_rddata(*currSlice, &(*currSlice)->rddata_top_field_mb);
      alloc_rddata(*currSlice, &(*currSlice)->rddata_bot_field_mb);
    }
  }

  if ((*currSlice)->slice_type == B_SLICE)
  {
    (*currSlice)->set_motion_vectors_mb = SetMotionVectorsMBBSlice;
  }
  else if ((*currSlice)->slice_type == P_SLICE || (*currSlice)->slice_type == SP_SLICE)
  {
    (*currSlice)->set_motion_vectors_mb = SetMotionVectorsMBPSlice;
  }
  else
  {
    (*currSlice)->set_motion_vectors_mb = SetMotionVectorsMBISlice;
  }

  if (p_Vid->mb_aff_frame_flag)
  {
    (*currSlice)->set_intrapred_4x4 = set_intrapred_4x4_mbaff;
    (*currSlice)->set_intrapred_8x8 = set_intrapred_8x8_mbaff;
    (*currSlice)->set_intrapred_16x16 = set_intrapred_16x16_mbaff;
    (*currSlice)->rdo_low_intra_chroma_decision = rdo_low_intra_chroma_decision_mbaff;
    (*currSlice)->intra_chroma_prediction = intra_chroma_prediction_mbaff;
#if EXT3D
    if ((*currSlice)->direct_spatial_mv_pred_flag)  //spatial direct 
#else
    if (p_Vid->direct_spatial_mv_pred_flag)  //spatial direct 
#endif
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Spatial_MBAFF;
    else
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Temporal;
  }
  else
  {
    (*currSlice)->set_intrapred_4x4 = set_intrapred_4x4;
    (*currSlice)->set_intrapred_8x8 = set_intrapred_8x8;
    (*currSlice)->set_intrapred_16x16 = set_intrapred_16x16;
    (*currSlice)->rdo_low_intra_chroma_decision = rdo_low_intra_chroma_decision;
    (*currSlice)->intra_chroma_prediction = intra_chroma_prediction;
#if EXT3D
    if ((*currSlice)->direct_spatial_mv_pred_flag)  //spatial direct 
#else
    if (p_Vid->direct_spatial_mv_pred_flag)  //spatial direct 
#endif
    {
#if EXT3D          
      if((*currSlice)->depth_based_mvp_flag)
        (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Spatial_MVP;
      else
#endif
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Spatial_Normal;
    }
    else
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Temporal;
  }

  if (active_sps->chroma_format_idc == YUV444)
    (*currSlice)->mode_decision_for_I16x16_MB = mode_decision_for_I16x16_MB_444;
  else if(p_Inp->I16rdo)
    (*currSlice)->mode_decision_for_I16x16_MB = mode_decision_for_I16x16_MB_RDO;
  else
    (*currSlice)->mode_decision_for_I16x16_MB = mode_decision_for_I16x16_MB;

  get_mem3Dpel(&((*currSlice)->mb_pred),   MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  get_mem3Dint(&((*currSlice)->mb_rres),   MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  get_mem3Dint(&((*currSlice)->mb_ores),   MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  get_mem4Dpel(&((*currSlice)->mpr_4x4),   MAX_PLANE, 9, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  get_mem4Dpel(&((*currSlice)->mpr_8x8),   MAX_PLANE, 9, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  get_mem4Dpel(&((*currSlice)->mpr_16x16), MAX_PLANE, 5, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

  get_mem_ACcoeff (p_Vid, &((*currSlice)->cofAC));
  get_mem_DCcoeff (&((*currSlice)->cofDC));

  allocate_block_mem(*currSlice);
  init_coding_state_methods(*currSlice);
  init_rdopt(*currSlice);

#if EXT3D
      if((*currSlice)->depth_based_mvp_flag || (*currSlice)->Harmonize_VSP_IVP) 
      {   // camera array - [0]:left / [1]:center / [2]:right for 3 view config.
        (*currSlice)->disparity_coeff=iabs(p_Vid->p_DualVid->ThreeDV_acquisition_info->i_disparity_scale[voidx][0]);
      }

    (*currSlice)->series_7skip_len = 0;
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Lite-weight initialization of the parameters for a new slice and
 *     allocates the memory for the coded slice in the Picture structure
 *  \par Side effects:
 *      Adds slice/partition header symbols to the symbol buffer
 *      increments Picture->no_slices, allocates memory for the
 *      slice, sets p_Vid->currSlice
 ************************************************************************
 */

void init_slice_lite (VideoParameters *p_Vid, Slice **currSlice, int start_mb_addr)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int i,j;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  int active_ref_lists = (p_Vid->mb_aff_frame_flag) ? 6 : 2;
  DecodedPictureBuffer *p_Dpb = p_Vid->p_Dpb;

# if EXT3D
  int voidx=GetVOIdx(p_Inp,p_Vid->view_id);
  if(p_Inp->NumOfViews>1)
  {
    if(p_Vid->structure!=BOTTOM_FIELD)
      p_Vid->anchor_pic_flag[0] = p_Vid->p_curr_frm_struct->anchor_pic_flag;
    else  // bottom will follow top for now
      p_Vid->anchor_pic_flag[1] = 0; //p_Vid->anchor_pic_flag[0];

    p_Vid->non_idr_flag[p_Vid->structure!=BOTTOM_FIELD ? 0:1] = ((p_Vid->currentPicture->idr_flag==1) ? 0:1);
    p_Vid->priority_id                                      = ((voidx==0) ? 0:1);
    //  p_Vid->temporal_id                                      = 0;
    if(p_Inp->NumOfViews>1)
    {
      p_Vid->inter_view_flag[0]                 = p_Vid->p_curr_frm_struct->inter_view_flag  ;
      p_Vid->inter_view_flag[1]                = p_Vid->p_curr_frm_struct->inter_view_flag;
    }
    else
    {
      p_Vid->inter_view_flag[0]=p_Vid->inter_view_flag[1]=0;
    }
  }
#else
#if (MVC_EXTENSION_ENABLE)
  if(p_Inp->num_of_views == 2)
  {
    int enable_inter_view = (p_Vid->nal_reference_idc || p_Inp->enable_inter_view_flag) ? 1 : 0;
    p_Vid->non_idr_flag[p_Vid->structure!=BOTTOM_FIELD ? 0:1] = ((p_Vid->currentPicture->idr_flag==1) ? 0:1);
    p_Vid->priority_id                                      = ((p_Vid->view_id==0) ? 0:1);
    p_Vid->temporal_id                                      = 0;
    p_Vid->inter_view_flag[0]                               = ((p_Vid->view_id == 0) ? enable_inter_view : 0);
    p_Vid->inter_view_flag[1]                               = ((p_Vid->view_id == 0) ? enable_inter_view : 0);

    if(p_Vid->structure!=BOTTOM_FIELD)
      p_Vid->anchor_pic_flag[0] = (((p_Vid->currentPicture->idr_flag && p_Vid->nal_reference_idc!=0) || p_Vid->prev_view_is_anchor) ? 1:0);
    else  // bottom will follow top for now
      p_Vid->anchor_pic_flag[1] = 0; //p_Vid->anchor_pic_flag[0];
  }
#endif
#endif

  *currSlice = malloc_slice_lite(p_Vid, p_Inp);

  // Using this trick we can basically reference back to the image and input parameter structures
  (*currSlice)->p_Vid             = p_Vid;
  (*currSlice)->p_Inp             = p_Inp;
  (*currSlice)->p_Dpb             = p_Dpb;
  (*currSlice)->active_sps        = active_sps;
  (*currSlice)->active_pps        = p_Vid->active_pps;
  (*currSlice)->picture_id        = (p_Vid->frame_no & 0xFF); // % 255
  (*currSlice)->slice_nr          = p_Vid->current_slice_nr;
  (*currSlice)->idr_flag          = p_Vid->currentPicture->idr_flag;
  (*currSlice)->slice_type        = p_Vid->type;
  (*currSlice)->frame_no          = p_Vid->frame_no;
  (*currSlice)->frame_num         = p_Vid->frame_num;
  (*currSlice)->max_frame_num     = p_Vid->max_frame_num;
  (*currSlice)->framepoc          = p_Vid->framepoc;
  (*currSlice)->ThisPOC           = p_Vid->ThisPOC;
  (*currSlice)->qp                = p_Vid->p_curr_frm_struct->qp;
  (*currSlice)->start_mb_nr       = start_mb_addr;
  (*currSlice)->colour_plane_id   = p_Vid->colour_plane_id;

  (*currSlice)->si_frame_indicator =  p_Vid->type == SI_SLICE ? TRUE : FALSE;
  (*currSlice)->sp2_frame_indicator = p_Vid->sp2_frame_indicator;

  (*currSlice)->P444_joined       = p_Vid->P444_joined;
  (*currSlice)->disthres          = p_Inp->disthres;
  (*currSlice)->UseRDOQuant       = p_Inp->UseRDOQuant;
  (*currSlice)->RDOQ_QP_Num       = p_Inp->RDOQ_QP_Num;
  (*currSlice)->Transform8x8Mode  = p_Inp->Transform8x8Mode;

  (*currSlice)->slice_too_big     = dummy_slice_too_big;  
  (*currSlice)->width_blk         = p_Vid->width_blk;
  (*currSlice)->height_blk        = p_Vid->height_blk;
  (*currSlice)->partition_mode    = (short) p_Inp->partition_mode;
  (*currSlice)->PicSizeInMbs      = p_Vid->PicSizeInMbs;
  (*currSlice)->num_blk8x8_uv     = p_Vid->num_blk8x8_uv;
  (*currSlice)->nal_reference_idc = p_Vid->nal_reference_idc;
  (*currSlice)->bitdepth_luma     = p_Vid->bitdepth_luma;
  (*currSlice)->bitdepth_chroma   = p_Vid->bitdepth_chroma;  
  (*currSlice)->direct_spatial_mv_pred_flag  = p_Vid->direct_spatial_mv_pred_flag;
  (*currSlice)->mb_aff_frame_flag = p_Vid->mb_aff_frame_flag;
  (*currSlice)->structure      = p_Vid->structure;
  (*currSlice)->num_ref_idx_active[LIST_0] = p_Vid->active_pps->num_ref_idx_l0_active_minus1 + 1;
  (*currSlice)->num_ref_idx_active[LIST_1] = p_Vid->active_pps->num_ref_idx_l1_active_minus1 + 1;
  (*currSlice)->DFDisableIdc  = (p_Vid->TurnDBOff) ? 1 : p_Vid->DFDisableIdc;
  (*currSlice)->DFAlphaC0Offset = p_Vid->DFAlphaC0Offset;
  (*currSlice)->DFBetaOffset    = p_Vid->DFBetaOffset;
#if EXT3D
  (*currSlice)->view_id  = p_Vid->view_id;
  (*currSlice)->is_depth = p_Vid->is_depth;
#endif


  // code now also considers fields. Issue whether we should account this within the appropriate input p_Inp directly
  if ((p_Vid->type == P_SLICE || p_Vid->type == SP_SLICE) && p_Inp->P_List0_refs)
  {
    (*currSlice)->num_ref_idx_active[LIST_0] = (char) imin((*currSlice)->num_ref_idx_active[LIST_0], p_Inp->P_List0_refs * ((p_Vid->structure !=0) + 1));
  }

  if (p_Vid->type == B_SLICE )
  {
    if (p_Inp->B_List0_refs)
    {
      (*currSlice)->num_ref_idx_active[LIST_0] = (char) imin((*currSlice)->num_ref_idx_active[LIST_0], p_Inp->B_List0_refs * ((p_Vid->structure !=0) + 1));
    }
    if (p_Inp->B_List1_refs)
    {
      (*currSlice)->num_ref_idx_active[LIST_1] = (char) imin((*currSlice)->num_ref_idx_active[LIST_1], p_Inp->B_List1_refs * ((p_Vid->structure !=0) + 1));
    }
    (*currSlice)->direct_ref_idx = NULL;
    (*currSlice)->direct_pdir = NULL;
  }

  // not needed as coding will not occur
  if ((*currSlice)->P444_joined)
  {
    (*currSlice)->luma_residual_coding = luma_residual_coding_p444;
    (*currSlice)->luma_residual_coding_8x8 = luma_residual_coding_p444_8x8;
  }
  else
  {
    if ((*currSlice)->slice_type == SP_SLICE)
      (*currSlice)->luma_residual_coding = luma_residual_coding_sp;
    else
      (*currSlice)->luma_residual_coding = luma_residual_coding;
    (*currSlice)->luma_residual_coding_8x8 = luma_residual_coding_8x8;
  }
  if ((*currSlice)->slice_type == SP_SLICE)
    (*currSlice)->chroma_residual_coding = chroma_residual_coding_sp;
  else if ((*currSlice)->slice_type == SI_SLICE)
    (*currSlice)->chroma_residual_coding = chroma_residual_coding_si;
  else
    (*currSlice)->chroma_residual_coding = chroma_residual_coding;

  setup_slice(*currSlice);

  // generate reference picture lists
#if MVC_EXTENSION_ENABLE||EXT3D
  //if (p_Vid->p_Inp->num_of_views == 2)
  {
    update_ref_list(p_Dpb);
    update_ltref_list(p_Dpb);
    check_num_ref(p_Dpb);
  }
#endif
  update_pic_num(*currSlice);
  (*currSlice)->init_lists(*currSlice);

#if EXT3D
  if((p_Inp->NumOfViews>1)&&voidx)
    init_interview_list(*currSlice);
#endif

  // assign list 0 size from list size
  (*currSlice)->num_ref_idx_active[LIST_0] = (*currSlice)->listXsize[LIST_0];
  (*currSlice)->num_ref_idx_active[LIST_1] = (*currSlice)->listXsize[LIST_1];

#if EXT3D
  if(p_Inp->NumOfViews>1)
  {
    //!Wenyi>>
    //!<For multi-view coding, the MMCO is based on anchor and zero temporal pictures.
    //!<If hierarchical B Enable, the MMCO is enabled when zero temporal pictures appear
    //!<Otherwise, MMCO is based on anchor period(intra period) 
    assert(p_Vid->structure==FRAME) ;
    if((p_Vid->anchor_pic_flag[0])||((p_Inp->HierarchicalCoding)&&(p_Vid->temporal_id==0)))
      mvc_based_ref_management_frame_pic(p_Dpb,*currSlice,p_Vid->frame_num);
    else if((p_Vid->nal_reference_idc)&&p_Vid->structure == FRAME && p_Dpb->ref_frames_in_buffer == active_sps->num_ref_frames)
      poc_based_ref_management_frame_pic(p_Dpb, p_Vid->frame_num);
  }
  else
#endif
  //Perform memory management based on poc distances  
  if (p_Inp->SetFirstAsLongTerm && p_Vid->number == 0)
  {
    mmco_long_term(p_Vid, p_Vid->number);
  }
  else if (p_Vid->nal_reference_idc && p_Inp->PocMemoryManagement == 1)
  {
    if (p_Vid->structure == FRAME && p_Dpb->ref_frames_in_buffer == active_sps->num_ref_frames)
      poc_based_ref_management_frame_pic(p_Dpb, p_Vid->frame_num);
    else if (p_Vid->structure == TOP_FIELD && p_Dpb->ref_frames_in_buffer== active_sps->num_ref_frames)
      poc_based_ref_management_field_pic(p_Dpb, (p_Vid->frame_num << 1) + 1);      
    else if (p_Vid->structure == BOTTOM_FIELD)
      poc_based_ref_management_field_pic(p_Dpb, (p_Vid->frame_num << 1) + 1);
  }
  else if (p_Vid->nal_reference_idc && p_Inp->PocMemoryManagement == 2) 
  {
    if (p_Vid->structure == FRAME)
      tlyr_based_ref_management_frame_pic(p_Vid, p_Vid->frame_num);
  }

  if (p_Inp->EnableOpenGOP)
  {
    if ((*currSlice)->slice_type != I_SLICE && (*currSlice)->slice_type != SI_SLICE)
    {
      for (i = 0; i<(*currSlice)->listXsize[0]; i++)
      {
        if ((*currSlice)->listX[0][i]->poc < p_Vid->last_valid_reference && p_Vid->ThisPOC > p_Vid->last_valid_reference)
        {
          (*currSlice)->listXsize[0] = (*currSlice)->num_ref_idx_active[LIST_0] = (char) imax(1, i);
          break;
        }
      }

      for (i = 0; i<(*currSlice)->listXsize[1]; i++)
      {
        if ((*currSlice)->listX[1][i]->poc < p_Vid->last_valid_reference && p_Vid->ThisPOC > p_Vid->last_valid_reference)
        {
          (*currSlice)->listXsize[1] = (*currSlice)->num_ref_idx_active[LIST_1] = (char) imax(1,i);
          break;
        }
      }
    }
  }

  init_ref_pic_list_reordering(*currSlice, p_Inp->ReferenceReorder);

#if EXT3D
    // reference list reordering 
    // RPLR for redundant pictures
    // !KS: that should actually be moved somewhere else
    if(p_Inp->NumOfViews==1)
    {
      if(p_Inp->redundant_pic_flag && p_Vid->redundant_coding)
      {
        alloc_ref_pic_list_reordering_buffer(*currSlice);
        (*currSlice)->ref_pic_list_reordering_flag[LIST_0] = 1;
        (*currSlice)->reordering_of_pic_nums_idc[LIST_0][0] = 0;
        (*currSlice)->reordering_of_pic_nums_idc[LIST_0][1] = 3;
        (*currSlice)->abs_diff_pic_num_minus1[LIST_0][0] = p_Vid->redundant_ref_idx - 1;
        (*currSlice)->long_term_pic_idx[LIST_0][0] = 0;

        reorder_ref_pic_list ( *currSlice, LIST_0);
      }
      else if ( (p_Vid->type == P_SLICE || p_Vid->type == B_SLICE) && p_Inp->WPMCPrecision && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR )
        wp_mcprec_reorder_lists( *currSlice );
      else
        reorder_lists( *currSlice );
    }
    else
    {
      get_mvc_lists(*currSlice);
    }

#else
#if (MVC_EXTENSION_ENABLE)
  p_Vid->MVCInterViewReorder = 0;
  if(p_Inp->num_of_views == 2 && p_Inp->MVCInterViewReorder == 1 && p_Vid->view_id == 1)
  {
    // enable MVC reordering
    p_Vid->MVCInterViewReorder = 1;

    alloc_ref_pic_list_reordering_buffer(*currSlice);
    if((*currSlice)->listXsize[0]!=0)
    {
      StorablePicture *Inter_ViewL0 = (*currSlice)->listX[0][(*currSlice)->listXsize[0]-1];
      for(i=(*currSlice)->listXsize[0]-1; i>0; i--)
        (*currSlice)->listX[0][i] = (*currSlice)->listX[0][i-1];
      (*currSlice)->listX[0][0] = Inter_ViewL0;
      (*currSlice)->listXsize[0] = (char) imin ((*currSlice)->listXsize[0], (*currSlice)->num_ref_idx_active[LIST_0]);
    }
    // perform reordering of interview pictures
  }
  else
#endif
    // reference list reordering 
    // RPLR for redundant pictures
    // !KS: that should actually be moved somewhere else
    if(p_Inp->redundant_pic_flag && p_Vid->redundant_coding)
    {
#if EXT3D
      alloc_ref_pic_list_reordering_buffer(*currSlice);
#endif
      (*currSlice)->ref_pic_list_reordering_flag[LIST_0] = 1;
      (*currSlice)->reordering_of_pic_nums_idc[LIST_0][0] = 0;
      (*currSlice)->reordering_of_pic_nums_idc[LIST_0][1] = 3;
      (*currSlice)->abs_diff_pic_num_minus1[LIST_0][0] = p_Vid->redundant_ref_idx - 1;
      (*currSlice)->long_term_pic_idx[LIST_0][0] = 0;

      reorder_ref_pic_list ( *currSlice, LIST_0);
    }
    else if ( (p_Vid->type == P_SLICE || p_Vid->type == B_SLICE) && p_Inp->WPMCPrecision && p_Vid->pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR )
      wp_mcprec_reorder_lists( *currSlice );
    else
      reorder_lists( *currSlice );
#endif

#if EXT3D
      if ( p_Vid->type == P_SLICE )
      {
#if PRINTREFLIST
        // print out for debug purpose
        if(p_Inp->NumOfViews>1)
        {
          if((*currSlice)->listXsize[0]>0)
          {
            printf("\n");
            printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
            for(i=0; i<(int)((*currSlice)->listXsize[0]); i++)  //ref list 0
            {
              printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d TId:%d\n", i, (*currSlice)->listX[0][i]->poc, 
                (*currSlice)->listX[0][i]->pic_num, (*currSlice)->listX[0][i]->view_id,(*currSlice)->listX[0][i]->temporal_layer);
            }
          }
        }
#endif
      }
      else if ( p_Vid->type == B_SLICE )
      {
#if PRINTREFLIST
        if(p_Inp->NumOfViews>1 )
        {
          if(((*currSlice)->listXsize[0]>0) || ((*currSlice)->listXsize[1]>0))
            printf("\n");
          if((*currSlice)->listXsize[0]>0)
          {
            printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
            for(i=0; i<(int)((*currSlice)->listXsize[0]); i++)  //ref list 0
            {
              printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d  TId:%d\n", i, (*currSlice)->listX[0][i]->poc,
                (*currSlice)->listX[0][i]->pic_num, (*currSlice)->listX[0][i]->view_id,(*currSlice)->listX[0][i]->temporal_layer);
            }
          }
          if((*currSlice)->listXsize[1]>0)
          {
            printf(" ** (CurViewID:%d) %s Ref Pic List 1 ****\n", p_Vid->view_id, (*currSlice)->structure==FRAME ? "FRM":((*currSlice)->structure==TOP_FIELD ? "TOP":"BOT"));
            for(i=0; i<(int)((*currSlice)->listXsize[1]); i++)  //ref list 1
            {
              printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d  TId:%d\n", i, (*currSlice)->listX[1][i]->poc,
                (*currSlice)->listX[1][i]->pic_num, (*currSlice)->listX[1][i]->view_id,(*currSlice)->listX[1][i]->temporal_layer);
            }
          }
        }
#endif
      }
#endif

  (*currSlice)->max_num_references = (short) p_Vid->max_num_references;

  (*currSlice)->all_mv = NULL;  

#if EXT3D
    (*currSlice)->isVSPRef = NULL;

    (*currSlice)->all_std_mv  = NULL;
    (*currSlice)->all_mv_size = 0;
    (*currSlice)->all_std_VSP =NULL;
    
    (*currSlice)->bipred_mv_implicit = NULL;
    (*currSlice)->bipred_mv_backup   = NULL;
    (*currSlice)->bi_mv_backup_size  = 0;
#endif

  (*currSlice)->bipred_mv = NULL;

  (*currSlice)->tmp_mv8 = NULL;
  (*currSlice)->motion_cost8 = NULL;
  (*currSlice)->tmp_mv4 = NULL;
  (*currSlice)->motion_cost4 = NULL;

  if (p_Vid->mb_aff_frame_flag)
    init_mbaff_lists(*currSlice);

  (*currSlice)->p_EPZS = NULL;

  // assign luma common reference picture pointers to be used for ME/sub-pel interpolation

  for(i = 0; i < active_ref_lists; i++)
  {
    for(j = 0; j < (*currSlice)->listXsize[i]; j++)
    {
      if( (*currSlice)->listX[i][j] )
      {
        (*currSlice)->listX[i][j]->p_curr_img     = (*currSlice)->listX[i][j]->p_img    [(short) p_Vid->colour_plane_id];
        (*currSlice)->listX[i][j]->p_curr_img_sub = (*currSlice)->listX[i][j]->p_img_sub[(short) p_Vid->colour_plane_id];
      }
    }
  }

  (*currSlice)->estBitsCabac = NULL;
  nullify_rddata(&((*currSlice)->rddata_trellis_curr));
  nullify_rddata(&((*currSlice)->rddata_trellis_best));

  if(p_Vid->mb_aff_frame_flag)
  {
    alloc_rddata(*currSlice, &(*currSlice)->rddata_top_frame_mb);
    alloc_rddata(*currSlice, &(*currSlice)->rddata_bot_frame_mb);
    if ( p_Inp->MbInterlace != FRAME_MB_PAIR_CODING )
    {
      alloc_rddata(*currSlice, &(*currSlice)->rddata_top_field_mb);
      alloc_rddata(*currSlice, &(*currSlice)->rddata_bot_field_mb);
    }
  }

  if ((*currSlice)->slice_type == B_SLICE)
  {
    (*currSlice)->set_motion_vectors_mb = SetMotionVectorsMBBSlice;
  }
  else if ((*currSlice)->slice_type == P_SLICE || (*currSlice)->slice_type == SP_SLICE)
  {
    (*currSlice)->set_motion_vectors_mb = SetMotionVectorsMBPSlice;
  }
  else
  {
    (*currSlice)->set_motion_vectors_mb = SetMotionVectorsMBISlice;
  }

  if (p_Vid->mb_aff_frame_flag)
  {
    (*currSlice)->set_intrapred_4x4 = set_intrapred_4x4_mbaff;
    (*currSlice)->set_intrapred_8x8 = set_intrapred_8x8_mbaff;
    (*currSlice)->set_intrapred_16x16 = set_intrapred_16x16_mbaff;
    (*currSlice)->rdo_low_intra_chroma_decision = rdo_low_intra_chroma_decision_mbaff;
    (*currSlice)->intra_chroma_prediction = intra_chroma_prediction_mbaff;
#if EXT3D
     if((*currSlice)->direct_spatial_mv_pred_flag)
#else
    if (p_Vid->direct_spatial_mv_pred_flag)  //spatial direct 
#endif
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Spatial_MBAFF;
    else
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Temporal;
  }
  else
  {
    (*currSlice)->set_intrapred_4x4 = set_intrapred_4x4;
    (*currSlice)->set_intrapred_8x8 = set_intrapred_8x8;
    (*currSlice)->set_intrapred_16x16 = set_intrapred_16x16;
    (*currSlice)->rdo_low_intra_chroma_decision = rdo_low_intra_chroma_decision;
    (*currSlice)->intra_chroma_prediction = intra_chroma_prediction;
#if EXT3D
    if((*currSlice)->direct_spatial_mv_pred_flag)
#else
    if (p_Vid->direct_spatial_mv_pred_flag)  //spatial direct 
#endif
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Spatial_Normal;
    else
      (*currSlice)->Get_Direct_Motion_Vectors = Get_Direct_MV_Temporal;
  }

  if (active_sps->chroma_format_idc == YUV444)
    (*currSlice)->mode_decision_for_I16x16_MB = mode_decision_for_I16x16_MB_444;
  else if(p_Inp->I16rdo)
    (*currSlice)->mode_decision_for_I16x16_MB = mode_decision_for_I16x16_MB_RDO;
  else
    (*currSlice)->mode_decision_for_I16x16_MB = mode_decision_for_I16x16_MB;

  ((*currSlice)->mb_pred) = NULL;
  ((*currSlice)->mb_rres) = NULL;
  ((*currSlice)->mb_ores) = NULL;
  ((*currSlice)->mpr_4x4) = NULL;
  ((*currSlice)->mpr_8x8) = NULL;
  ((*currSlice)->mpr_16x16) = NULL;

  (*currSlice)->cofAC = NULL;
  (*currSlice)->cofDC = NULL;

  (*currSlice)->tblk4x4 = NULL;
  (*currSlice)->tblk16x16 = NULL;
  (*currSlice)->i16blk4x4 = NULL;

  init_coding_state_methods(*currSlice);
}

/*!
 ************************************************************************
 * \brief
 *    Allocates a slice structure along with its dependent data structures
 * \return
 *    Pointer to a Slice
 ************************************************************************
 */
static Slice *malloc_slice(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  int i;
  DataPartition *dataPart;
  Slice *currSlice;
  int cr_size = (p_Inp->separate_colour_plane_flag != 0) ? 0 : 512;

  int buffer_size;

  switch (p_Inp->slice_mode)
  {
  case 2:
    //buffer_size = imax(2 * p_Inp->slice_argument, 500 + (128 + 256 * p_Vid->bitdepth_luma + 512 * p_Vid->bitdepth_chroma));
    buffer_size = imax(2 * p_Inp->slice_argument, 764);
    break;
  case 1:
    buffer_size = 500 + p_Inp->slice_argument * ((128 + 256 * p_Vid->bitdepth_luma + cr_size * p_Vid->bitdepth_chroma) >> 3);
    break;
  default:
    buffer_size = 500 + p_Vid->FrameSizeInMbs * ((128 + 256 * p_Vid->bitdepth_luma + cr_size * p_Vid->bitdepth_chroma) >> 3);
    break;
  }

  // KS: this is approx. max. allowed code picture size
  if ((currSlice = (Slice *) calloc(1, sizeof(Slice))) == NULL) no_mem_exit ("malloc_slice: currSlice structure");

  currSlice->p_Vid             = p_Vid;
  currSlice->p_Inp             = p_Inp;

  if (((currSlice->p_RDO)  = (RDOPTStructure *) calloc(1, sizeof(RDOPTStructure)))==NULL) 
    no_mem_exit("malloc_slice: p_RDO");

  currSlice->symbol_mode  = (char) p_Inp->symbol_mode;

  if (currSlice->symbol_mode == CABAC)
  {
    // create all context models
    currSlice->mot_ctx = create_contexts_MotionInfo ();
    currSlice->tex_ctx = create_contexts_TextureInfo();
  }

  currSlice->max_part_nr = p_Inp->partition_mode==0?1:3;

  //for IDR p_Vid there should be only one partition
  if(p_Vid->currentPicture->idr_flag)
    currSlice->max_part_nr = 1;

  assignSE2partition[0] = assignSE2partition_NoDP;
  //ZL
  //for IDR p_Vid all the syntax element should be mapped to one partition
  if(!p_Vid->currentPicture->idr_flag && p_Inp->partition_mode == 1)
    assignSE2partition[1] =  assignSE2partition_DP;
  else
    assignSE2partition[1] =  assignSE2partition_NoDP;

  currSlice->num_mb = 0;          // no coded MBs so far

  if ((currSlice->partArr = (DataPartition *) calloc(currSlice->max_part_nr, sizeof(DataPartition))) == NULL) 
    no_mem_exit ("malloc_slice: partArr");
  for (i=0; i<currSlice->max_part_nr; i++) // loop over all data partitions
  {
    dataPart = &(currSlice->partArr[i]);
    if ((dataPart->bitstream = (Bitstream *) calloc(1, sizeof(Bitstream))) == NULL) 
      no_mem_exit ("malloc_slice: Bitstream");
    if ((dataPart->bitstream->streamBuffer = (byte *) calloc(buffer_size, sizeof(byte))) == NULL) 
      no_mem_exit ("malloc_slice: StreamBuffer");
    dataPart->bitstream->buffer_size = buffer_size;
    // Initialize storage of bitstream parameters
    // Set pointers
    dataPart->p_Slice = currSlice;
    dataPart->p_Vid   = p_Vid;
    dataPart->p_Inp   = p_Inp;
  }

#if EXT3D
  if (p_Inp->DepthRangeBasedWP||p_Inp->WeightedPrediction || p_Inp->WeightedBiprediction || p_Inp->GenerateMultiplePPS)
#else
  if (p_Inp->WeightedPrediction || p_Inp->WeightedBiprediction || p_Inp->GenerateMultiplePPS)
#endif
  {
    // Currently only use up to 32 references. Need to use different indicator such as maximum num of references in list
    get_mem3Dshort(&currSlice->wp_weight, 6, MAX_REFERENCE_PICTURES, 3);
    get_mem3Dshort(&currSlice->wp_offset, 6, MAX_REFERENCE_PICTURES, 3);
    get_mem4Dshort(&currSlice->wbp_weight, 6, MAX_REFERENCE_PICTURES, MAX_REFERENCE_PICTURES, 3);
  }

  return currSlice;
}




/*!
 ************************************************************************
 * \brief
 *    Allocates a light-weight slice structure along with its dependent data structures
 * \return
 *    Pointer to a Slice
 ************************************************************************
 */
static Slice *malloc_slice_lite(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  Slice *currSlice;
  //  int cr_size = (p_Inp->separate_colour_plane_flag != 0) ? 0 : 512;
  //
  //  int buffer_size;
  //
  //  switch (p_Inp->slice_mode)
  //  {
  //  case 2:
  //    //buffer_size = imax(2 * p_Inp->slice_argument, 500 + (128 + 256 * p_Vid->bitdepth_luma + 512 * p_Vid->bitdepth_chroma));
  //    buffer_size = imax(2 * p_Inp->slice_argument, 764);
  //    break;
  //  case 1:
  //    buffer_size = 500 + p_Inp->slice_argument * ((128 + 256 * p_Vid->bitdepth_luma + cr_size * p_Vid->bitdepth_chroma) >> 3);
  //    break;
  //  default:
  //    buffer_size = 500 + p_Vid->FrameSizeInMbs * ((128 + 256 * p_Vid->bitdepth_luma + cr_size * p_Vid->bitdepth_chroma) >> 3);
  //    break;
  //  }

  // KS: this is approx. max. allowed code picture size
  if ((currSlice = (Slice *) calloc(1, sizeof(Slice))) == NULL) no_mem_exit ("malloc_slice: currSlice structure");

  currSlice->p_Vid             = p_Vid;
  currSlice->p_Inp             = p_Inp;

  (currSlice->p_RDO)  = NULL;

  currSlice->symbol_mode  = (char) p_Inp->symbol_mode;
  {
    // create all context models
    currSlice->mot_ctx = NULL;
    currSlice->tex_ctx = NULL;
  }

  currSlice->max_part_nr = p_Inp->partition_mode==0?1:3;

  //for IDR p_Vid there should be only one partition
  if(p_Vid->currentPicture->idr_flag)
    currSlice->max_part_nr = 1;

  assignSE2partition[0] = assignSE2partition_NoDP;
  //ZL
  //for IDR p_Vid all the syntax element should be mapped to one partition
  if(!p_Vid->currentPicture->idr_flag && p_Inp->partition_mode == 1)
    assignSE2partition[1] =  assignSE2partition_DP;
  else
    assignSE2partition[1] =  assignSE2partition_NoDP;

  currSlice->num_mb = 0;          // no coded MBs so far

  currSlice->partArr = NULL;

#if EXT3D
  if (p_Inp->DepthRangeBasedWP||p_Inp->WeightedPrediction || p_Inp->WeightedBiprediction || p_Inp->GenerateMultiplePPS)
#else
  if (p_Inp->WeightedPrediction || p_Inp->WeightedBiprediction || p_Inp->GenerateMultiplePPS)
#endif
  {
    // Currently only use up to 32 references. Need to use different indicator such as maximum num of references in list
    get_mem3Dshort(&currSlice->wp_weight, 6, MAX_REFERENCE_PICTURES, 3);
    get_mem3Dshort(&currSlice->wp_offset, 6, MAX_REFERENCE_PICTURES, 3);
    get_mem4Dshort(&currSlice->wbp_weight, 6, MAX_REFERENCE_PICTURES, MAX_REFERENCE_PICTURES, 3);
  }

  return currSlice;
}



/*!
 ************************************************************************
 * \brief
 *    This function frees nal units
 *
 ************************************************************************
 */
static void free_nal_unit(Picture *pic)
{
  int partition, slice;
  Slice  *currSlice;

  // loop over all slices of the picture
  for (slice=0; slice < pic->no_slices; slice++)
  {
    currSlice = pic->slices[slice];

    // loop over the partitions
    if (currSlice != NULL)
    {
      for (partition=0; partition < currSlice->max_part_nr; partition++)
      {
        // free only if the partition has content
        if (currSlice->partArr[partition].bitstream->write_flag )
        {
          if (currSlice->partArr[partition].nal_unit != NULL)
          {
            FreeNALU(currSlice->partArr[partition].nal_unit);
            currSlice->partArr[partition].nal_unit = NULL;
          }
        }
      }
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Memory frees of the Slice structure and of its dependent
 *    data structures
 * \param currSlice
 *    Slice to be freed
 ************************************************************************
 */
void free_slice(Slice *currSlice)
{
  if (currSlice != NULL)
  {
    VideoParameters *p_Vid = currSlice->p_Vid;
    InputParameters *p_Inp = currSlice->p_Inp;

    int i;
    DataPartition *dataPart;

    for (i=0; i<6; i++)
    {
      if (currSlice->listX[i])
      {
        free (currSlice->listX[i]);
        currSlice->listX[i] = NULL;
      }
    }

    for (i=0; i<currSlice->max_part_nr; i++) // loop over all data partitions
    {
      dataPart = &(currSlice->partArr[i]);

      if (dataPart != NULL)
      {
        if (dataPart->bitstream != NULL)
        {
          if (dataPart->bitstream->streamBuffer != NULL)
          {
            free(dataPart->bitstream->streamBuffer);       
            dataPart->bitstream->streamBuffer = NULL;
          }
          free(dataPart->bitstream);
          dataPart->bitstream = NULL;
        }
      }
    }

    if (currSlice->slice_type != I_SLICE && currSlice->slice_type != SI_SLICE)
    {
      free_ref_pic_list_reordering_buffer (currSlice);
#if EXT3D
      free_interview_ref_pic_list_reordering_buffer(currSlice);
#endif
    }

    // free structure for rd-opt. mode decision
    if(currSlice->p_RDO)
    {
    clear_rdopt (currSlice);
    free (currSlice->p_RDO);
    }

    if(currSlice->cofAC)
    free_mem_ACcoeff (currSlice->cofAC);
    if(currSlice->cofDC)
    free_mem_DCcoeff (currSlice->cofDC);

    if(currSlice->mb_rres)
    free_mem3Dint(currSlice->mb_rres  );
    if(currSlice->mb_ores)
    free_mem3Dint(currSlice->mb_ores  );
    if(currSlice->mb_pred)
    free_mem3Dpel(currSlice->mb_pred  );
    if(currSlice->mpr_16x16)
    free_mem4Dpel(currSlice->mpr_16x16);
    if(currSlice->mpr_8x8)
    free_mem4Dpel(currSlice->mpr_8x8  );
    if(currSlice->mpr_4x4)
    free_mem4Dpel(currSlice->mpr_4x4  );


    if (currSlice->partArr != NULL)
    {
      free(currSlice->partArr);
      currSlice->partArr = NULL;
    }

    if (currSlice->symbol_mode == CABAC)
    {
      if(currSlice->mot_ctx)
      delete_contexts_MotionInfo(currSlice->mot_ctx);
      if(currSlice->tex_ctx)
      delete_contexts_TextureInfo(currSlice->tex_ctx);
    }

#if EXT3D
    if (p_Inp->DepthRangeBasedWP||p_Inp->WeightedPrediction || p_Inp->WeightedBiprediction || p_Inp->GenerateMultiplePPS)
#else
    if (p_Inp->WeightedPrediction || p_Inp->WeightedBiprediction || p_Inp->GenerateMultiplePPS)
#endif
    {
      free_mem3Dshort(currSlice->wp_weight );
      free_mem3Dshort(currSlice->wp_offset );
      free_mem4Dshort(currSlice->wbp_weight);
    }

    if (currSlice->UseRDOQuant)
    {
      free(currSlice->estBitsCabac);

      free_rddata(currSlice, &currSlice->rddata_trellis_curr);
      if(currSlice->RDOQ_QP_Num > 1)
      {
        free_rddata(currSlice, &currSlice->rddata_trellis_best);
      }
    }

    if(p_Vid->mb_aff_frame_flag)
    {
      free_rddata(currSlice, &currSlice->rddata_top_frame_mb);
      free_rddata(currSlice, &currSlice->rddata_bot_frame_mb);

      if ( p_Inp->MbInterlace != FRAME_MB_PAIR_CODING )
      {
        free_rddata(currSlice, &currSlice->rddata_top_field_mb);
        free_rddata(currSlice, &currSlice->rddata_bot_field_mb);
      }
    }

    if ((currSlice->slice_type == P_SLICE) || (currSlice->slice_type == SP_SLICE) || (currSlice->slice_type == B_SLICE))
    {
      if(currSlice->all_mv)
      free_mem_mv (currSlice->all_mv);

#if EXT3D
      if(currSlice->all_mv_backup)
        free_mem_mv(currSlice->all_mv_backup);
      if(currSlice->isVSPRef)
        free_mem5Dshort(currSlice->isVSPRef);

      if(currSlice->all_std_mv)
        free_mem_mv (currSlice->all_std_mv);
      currSlice->all_mv_size = 0;
      if(currSlice->all_std_VSP)
        free_mem5Dshort(currSlice->all_std_VSP);
#endif

#if EXT3D
      if (p_Inp->BiPredMotionEstimation && (currSlice->slice_type == B_SLICE) && currSlice->bipred_mv_backup)
      {
        free_mem_bipred_mv(currSlice->bipred_mv_backup);
      }
      if (p_Inp->BiPredMotionEstimation && (currSlice->slice_type == B_SLICE) && currSlice->bipred_mv_implicit)
      {
        free_mem_bipred_mv(currSlice->bipred_mv_implicit);
      }
#endif
      if (p_Inp->BiPredMotionEstimation && (currSlice->slice_type == B_SLICE) && currSlice->bipred_mv)
        free_mem_bipred_mv(currSlice->bipred_mv);

      if (currSlice->UseRDOQuant && currSlice->RDOQ_QP_Num > 1)
      {
        if (p_Inp->Transform8x8Mode && p_Inp->RDOQ_CP_MV)
        {
          if(currSlice->tmp_mv8)
          free_mem4Dmv (currSlice->tmp_mv8);
          if(currSlice->motion_cost8)
          free_mem3Ddistblk(currSlice->motion_cost8);
          if(currSlice->tmp_mv4)
          free_mem4Dmv (currSlice->tmp_mv4);
          if(currSlice->motion_cost4)
          free_mem3Ddistblk(currSlice->motion_cost4);
        }
      }
    }

    if (currSlice->slice_type == B_SLICE)
    {
      if(currSlice->direct_ref_idx)
      free_mem3D((byte ***)currSlice->direct_ref_idx);
      if(currSlice->direct_pdir)
      free_mem2D((byte **) currSlice->direct_pdir);
    }

    free_block_mem(currSlice);

    if (currSlice->slice_type != I_SLICE && currSlice->slice_type != SI_SLICE)
    {
      if (p_Inp->SearchMode == EPZS)
      {
        if(currSlice->p_EPZS)
        EPZSStructDelete (currSlice);    

#if EXT3D 
        if(currSlice->p_alcEPZS)
        {
          currSlice->p_EPZS = currSlice->p_alcEPZS;
          EPZSStructDelete (currSlice);
          currSlice->p_alcEPZS = NULL;
        }
#endif
      }
    }

    free(currSlice);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Memory frees of all Slice structures and of its dependent
 *    data structures
 * \par Input:
 *    Picture *currPic
 ************************************************************************
 */
void free_slice_list(Picture *currPic)
{
  int i;

  if (currPic !=  NULL)
  {
    free_nal_unit(currPic);

    for (i = 0; i < currPic->no_slices; i++)
    {
      free_slice (currPic->slices[i]);
      currPic->slices[i] = NULL;
    }
  }
}

void UpdateMELambda(Slice *currSlice)
{  
  InputParameters *p_Inp = currSlice->p_Inp;

  if (p_Inp->UpdateLambdaChromaME)
  {
    VideoParameters *p_Vid = currSlice->p_Vid;
    int j, k, qp;
    if (currSlice->slice_type == B_SLICE)
      j = currSlice->nal_reference_idc ? 5 : B_SLICE;
    else
      j = currSlice->slice_type;

    switch(p_Vid->yuv_format)
    {
    case YUV420:
      for (qp = -p_Vid->bitdepth_luma_qp_scale; qp < 52; qp++)
      { 
        for (k = 0; k < 3; k++)
        {
          if ((p_Inp->MEErrorMetric[k] == ERROR_SAD) && (p_Inp->ChromaMEEnable))
          {
            p_Vid->lambda_mf[j][qp][k] = (3 * p_Vid->lambda_mf[j][qp][k] + 1) >> 1;
            p_Vid->lambda_me[j][qp][k] *= 1.5;
          }
        }
      }
      break;
    case YUV422:
      for (qp = -p_Vid->bitdepth_luma_qp_scale; qp < 52; qp++)
      { 
        for (k = 0; k < 3; k++)
        {
          if ((p_Inp->MEErrorMetric[k] == ERROR_SAD) && (p_Inp->ChromaMEEnable))
          {
            p_Vid->lambda_mf[j][qp][k] *= 2;
            p_Vid->lambda_me[j][qp][k] *= 2.0;
          }
        }
      }
      break;
    case YUV444:
      for (qp = -p_Vid->bitdepth_luma_qp_scale; qp < 52; qp++)
      { 
        for (k = 0; k < 3; k++)
        {
          if ((p_Inp->MEErrorMetric[k] == ERROR_SAD) && (p_Inp->ChromaMEEnable))
          {
            p_Vid->lambda_mf[j][qp][k] *= 3;
            p_Vid->lambda_me[j][qp][k] *= 3.0;
          }
        }
      }
      break;
    default:
      break;
    }
  }
}

void SetLambda(VideoParameters *p_Vid, int j, int qp, double lambda_scale)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int k;
  p_Vid->lambda_md[j][qp] *= lambda_scale;

  for (k = F_PEL; k <= Q_PEL; k++)
  {
    p_Vid->lambda_me[j][qp][k] =  (p_Inp->MEErrorMetric[k] == ERROR_SSE) ? p_Vid->lambda_md[j][qp] : sqrt(p_Vid->lambda_md[j][qp]);
    p_Vid->lambda_mf[j][qp][k] = LAMBDA_FACTOR (p_Vid->lambda_me[j][qp][k]);
  }
}

void CalcMaxLamdaMD(VideoParameters *p_Vid, double *p_lambda_md)
{
  double max_lambda_md;
  int iBits;

  if(p_Vid->p_Inp->ProfileIDC >= FREXT_HP)
    iBits=128;  //Spec. Page 306
  else
    iBits=80;

  if(p_Vid->yuv_format == YUV420)
    iBits += 3072; //(256+128)*8;
  else if(p_Vid->yuv_format == YUV422)
    iBits += 4096; //(256+256)*8;
  else if(p_Vid->yuv_format == YUV444)
    iBits += 6144; //(256*3)*8;
  else if(p_Vid->yuv_format == YUV400)
    iBits += 2048; //256*8;

  max_lambda_md =  floor(((double)DISTBLK_MAX)/iBits/(1<<LAMBDA_ACCURACY_BITS));
#if JCOST_OVERFLOWCHECK
  {
    distblk cost = weight_cost(LAMBDA_FACTOR(max_lambda_md), iBits);
    assert(cost>=0 && cost<=DISTBLK_MAX);
  }
#endif
  *p_lambda_md = max_lambda_md;  
}

void ClipLambda(double *p_lambda_max, double *p_lambda)
{
  if(*p_lambda > *p_lambda_max)
  {
    //printf("Clip: %lf -> %lf\n", *p_lambda, *p_lambda_max);   
    *p_lambda = *p_lambda_max;
  }
}

void SetLagrangianMultipliersOn(Slice *currSlice)
{
  InputParameters *p_Inp = currSlice->p_Inp;

  if (p_Inp->UseExplicitLambdaParams == 1) // consideration of explicit lambda weights.
    get_explicit_lambda(currSlice);
  else if (p_Inp->UseExplicitLambdaParams == 2) // consideration of fixed lambda values.
    get_fixed_lambda(currSlice);
  else
  {    
    switch(currSlice->slice_type)
    {
    case I_SLICE:
      get_implicit_lambda_i_slice(currSlice);     
      break;
    default:
    case P_SLICE:      
      get_implicit_lambda_p_slice(currSlice);
      break;
    case B_SLICE:
      get_implicit_lambda_b_slice(currSlice);
      break;
    case SI_SLICE:
    case SP_SLICE:
      get_implicit_lambda_sp_slice(currSlice);
      break;
    }
  }

  UpdateMELambda(currSlice);
}


void SetLagrangianMultipliersOff(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;

  int qp, j, k;
  double qp_temp;

  for (j = 0; j < 6; j++)
  {
    for (qp = -p_Vid->bitdepth_luma_qp_scale; qp < 52; qp++)
    {
      qp_temp = (double)qp + p_Vid->bitdepth_luma_qp_scale - SHIFT_QP;

      switch (p_Inp->UseExplicitLambdaParams)
      {
      case 1:  // explicit lambda weights
        p_Vid->lambda_md[j][qp] = sqrt(p_Inp->LambdaWeight[j] * pow (2, qp_temp/3.0));
        break;
      case 2: // explicit lambda
        p_Vid->lambda_md[j][qp] = sqrt(p_Inp->FixedLambda[j]);
        break;
      default:
        p_Vid->lambda_md[j][qp] = QP2QUANT[imax(0,qp - SHIFT_QP)];
        break;
      }

      for (k = F_PEL; k <= Q_PEL; k++)
      {
        p_Vid->lambda_me[j][qp][k]  = (p_Inp->MEErrorMetric[k] == ERROR_SSE) ? (p_Vid->lambda_md[j][qp] * p_Vid->lambda_md[j][qp]) : p_Vid->lambda_md[j][qp];
        p_Vid->lambda_mf[j][qp][k]  = LAMBDA_FACTOR (p_Vid->lambda_me[j][qp][k]);
      }

      if (p_Inp->CtxAdptLagrangeMult == 1)
      {
        int lambda_qp = (qp >= 32 && !p_Inp->RCEnable) ? imax(0, qp-4) : imax(0, qp-6);
        p_Vid->lambda_mf_factor[j][qp] = log (p_Vid->lambda_me[j][lambda_qp][Q_PEL] + 1.0) / log (2.0);
      }
    }
  }
  UpdateMELambda(currSlice);
}


#if EXT3D 
/*!
************************************************************************
* \brief
*    Stores calculated macroblock parameters ( that isn't stored by set_stored_macroblock_parameters)
* \par Input:
*    Macroblock* currMB
************************************************************************
*/
void store_add_best_params( Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;
  RDOPTStructure  *p_RDO = currSlice->p_RDO;
  memcpy(&(currSlice->all_std_mv[0][0][0][0][0]), &(currSlice->all_mv[0][0][0][0][0]), currSlice->all_mv_size);
  memcpy(&(p_RDO->std_all_mv8x8[0][0][0][0]),     &(p_RDO->all_mv8x8[0][0][0][0]),     p_RDO->all_mv8x8_size);
  memcpy(&(currSlice->all_std_VSP[0][0][0][0][0]),&(currSlice->isVSPRef[0][0][0][0][0]),2*currSlice->max_num_references*11*4*4*sizeof(short));
  memcpy(&(p_RDO->std_isVSPRef8x8[0][0][0][0]),&(p_RDO->isVSPRef8x8[0][0][0][0]),p_RDO->std_isVSPRef8x8_size);
  memcpy(currSlice->std_mvd, currMB->mvd, BLOCK_CONTEXT * sizeof(short));
  currSlice->std_luma_transform_size_8x8_flag = currMB->luma_transform_size_8x8_flag;
  currSlice->std_temp_transform_size_8x8_flag = currMB->temp_transform_size_8x8_flag;
  currSlice->std_valid_8x8 = currMB->valid_8x8;
  currSlice->std_valid_4x4 = currMB->valid_4x4;
  memcpy(currSlice->std_part_4x4, currSlice->p_RDO->tr4x4->part, 4*sizeof(Info8x8));
  memcpy(currSlice->std_part_8x8, currSlice->p_RDO->tr8x8->part, 4*sizeof(Info8x8));
  memcpy(currSlice->std_best_ref, currMB->best_ref, 2*sizeof(int));
  memcpy(currSlice->std_best_mvp, currMB->best_mvp, 2*sizeof(MotionVector));
  currMB->best_mode_changed = 0;
}

/*!
************************************************************************
* \brief
*    Restores stored macroblock parameters ( that were stored store_add_best_params)
* \par Input:
*    Macroblock* currMB
************************************************************************
*/
void restore_add_best_params( Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;
  RDOPTStructure  *p_RDO = currSlice->p_RDO;
  memcpy(&(currSlice->all_mv[0][0][0][0][0]), &(currSlice->all_std_mv[0][0][0][0][0]), currSlice->all_mv_size);
  memcpy(&(p_RDO->all_mv8x8[0][0][0][0]),     &(p_RDO->std_all_mv8x8[0][0][0][0]),     p_RDO->all_mv8x8_size);
  memcpy(&(currSlice->isVSPRef[0][0][0][0][0]),&(currSlice->all_std_VSP[0][0][0][0][0]),2*currSlice->max_num_references*11*4*4*sizeof(short));
  memcpy(&(p_RDO->isVSPRef8x8[0][0][0][0]),&(p_RDO->std_isVSPRef8x8[0][0][0][0]),p_RDO->std_isVSPRef8x8_size);
  memcpy(currMB->mvd, currSlice->std_mvd, BLOCK_CONTEXT * sizeof(short));
  currMB->luma_transform_size_8x8_flag = currSlice->std_luma_transform_size_8x8_flag;
  currMB->temp_transform_size_8x8_flag = currSlice->std_temp_transform_size_8x8_flag;
  currMB->valid_8x8 = currSlice->std_valid_8x8;
  currMB->valid_4x4 = currSlice->std_valid_4x4;
  memcpy( currSlice->p_RDO->tr4x4->part, currSlice->std_part_4x4, 4*sizeof(Info8x8));
  memcpy( currSlice->p_RDO->tr8x8->part, currSlice->std_part_8x8, 4*sizeof(Info8x8));
  memcpy( currMB->best_ref, currSlice->std_best_ref, 2*sizeof(int));
  memcpy( currMB->best_mvp, currSlice->std_best_mvp, 2*sizeof(MotionVector));
  currMB->best_mode_changed = 0;
}

extern void update_refresh_map(Macroblock *currMB, int intra, int intra1);
/*!
************************************************************************
* \brief
*    Restores stored macroblock parameters ( that were stored store_add_best_params)
* \par Input:
*    Macroblock* currMB
************************************************************************
*/
void set_final_best_params( Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  RDOPTStructure  *p_RDO = currSlice->p_RDO;
  short       pslice      = (short) ((currSlice->slice_type == P_SLICE) || (currSlice->slice_type == SP_SLICE));
  short       intra       = (short) ((currSlice->slice_type == I_SLICE) || (currSlice->slice_type == SI_SLICE) || (pslice && currMB->mb_y == p_Vid->mb_y_upd && p_Vid->mb_y_upd != p_Vid->mb_y_intra));
  int intra1 = IS_INTRA(currMB);

  update_qp_cbp_tmp(currMB, p_RDO->cbp);
  currSlice->set_stored_mb_parameters (currMB);

  // Rate control
  if(p_Inp->RCEnable && p_Inp->RCUpdateMode <= MAX_RC_MODE)
    rc_store_mad(currMB);

  //===== Decide if this MB will restrict the reference frames =====
  if (p_Inp->RestrictRef)
    update_refresh_map(currMB, intra, intra1);
}

/*!
************************************************************************
* \brief
*    Sets parameters for applying ALC method
* \par Input:
*    Macroblock* currMB
************************************************************************
*/

void set_params_for_alc( Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  // prepare input data for ALC encoding
  // INTRA modes turned off
  currMB->DisableIntraInInter_std = p_Inp->DisableIntraInInter;
  p_Inp->DisableIntraInInter = 1;

  // change metric for motion search
  currMB->computeStdUniPred[F_PEL] = p_Vid->computeUniPred[F_PEL];
  currMB->computeStdUniPred[H_PEL] = p_Vid->computeUniPred[H_PEL];
  currMB->computeStdUniPred[Q_PEL] = p_Vid->computeUniPred[Q_PEL];

#if USE_MR_METRIC_FOR_ALC
  p_Vid->computeUniPred[F_PEL] = computeMRSAD4x4;
  p_Vid->computeUniPred[H_PEL] = computeMRSAD4x4;
  p_Vid->computeUniPred[Q_PEL] = computeMRSAD4x4;
#endif

  // set ALC search zone
  currMB->searchRange_std = p_Vid->searchRange;

  p_Vid->searchRange.min_x = -p_Inp->alc_search_range_x;
  p_Vid->searchRange.max_x =  p_Inp->alc_search_range_x;
  p_Vid->searchRange.min_y = -p_Inp->alc_search_range_y;
  p_Vid->searchRange.max_y =  p_Inp->alc_search_range_y;

#if FULL_SEARCH_FOR_ALC
  // turn on full search for ALC
  currMB->SearchMode_std = p_Inp->SearchMode;
  p_Inp->SearchMode = FULL_SEARCH;
  currMB->IntPelME_std = currMB->IntPelME;
  currMB->SubPelME_std = currMB->SubPelME;
  currMB->IntPelME = FullPelBlockMotionSearch;
  currMB->SubPelME = SubPelBlockMotionSearch;
#endif

  // change EPSZ parameters (if needed)
  if (p_Inp->SearchMode == EPZS)
  {
    currMB->p_EPZS_std = currSlice->p_EPZS;
    currSlice->p_EPZS = currSlice->p_alcEPZS;
  }
}

/*!
************************************************************************
* \brief
*    Resets parameters after applying ALC method
* \par Input:
*    Macroblock* currMB
************************************************************************
*/
void reset_params_after_alc( Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  // restore EPSZ parameters (if needed)
  if (p_Inp->SearchMode == EPZS)
    currSlice->p_EPZS = currMB->p_EPZS_std;

#if FULL_SEARCH_FOR_ALC
  // restore search method
  p_Inp->SearchMode = currMB->SearchMode_std;

  currMB->IntPelME = currMB->IntPelME_std;
  currMB->SubPelME = currMB->SubPelME_std;
#endif

  // restore search zone
  p_Vid->searchRange = currMB->searchRange_std;

  // restore metric for motion search
  p_Vid->computeUniPred[F_PEL] = currMB->computeStdUniPred[F_PEL];
  p_Vid->computeUniPred[H_PEL] = currMB->computeStdUniPred[H_PEL];
  p_Vid->computeUniPred[Q_PEL] = currMB->computeStdUniPred[Q_PEL];

  // restore INTRA modes flag
  p_Inp->DisableIntraInInter = currMB->DisableIntraInInter_std;
}

/*!
************************************************************************
* \brief
*    Mode Decision for a macroblock
************************************************************************
*/
void check_macroblock_modes(Macroblock* currMB)
{
  currMB->p_Slice->encode_one_macroblock (currMB);
  if (0 == currMB->best_mode_changed)
    restore_add_best_params( currMB);    // method is not the best, so restore previous state
  else
    store_add_best_params( currMB);    // method is best, so save recent changes
}

/*!
************************************************************************
* \brief
*    Enables ALC mode support in macroblock level
* \par Input:
*    Macroblock* currMB
************************************************************************
*/
void encode_one_alc_macroblock(Macroblock* currMB)
{
  Slice *currSlice = currMB->p_Slice;

  if (1 == currSlice->alc_slice_flag)
  {
    // check ALCskip mode
    currMB->is_alc_mode    = 1; // turn on ALC mode features (mode validation, MV prediction)
    currSlice->check_alc_skip = 1;
    check_macroblock_modes(currMB);
    currSlice->check_alc_skip = 0;
    currMB->mb_type = 0;

    // check all standard modes
    currMB->is_alc_mode    = 0;
    check_macroblock_modes(currMB);

    // check ALC inter modes
    currMB->is_alc_mode    = 1;
    set_params_for_alc( currMB);
    currMB->mb_type = 0;
    check_macroblock_modes(currMB);

    if (0 == currSlice->is_alc_mode)
      reset_params_after_alc( currMB);
  }
  else
  {
    // test standard modes only
    currMB->is_alc_mode    = 0;     // turn off ALC mode features (mode validation, MV prediction)
    currSlice->encode_one_macroblock(currMB);
  }

  set_final_best_params( currMB);
}
#endif

