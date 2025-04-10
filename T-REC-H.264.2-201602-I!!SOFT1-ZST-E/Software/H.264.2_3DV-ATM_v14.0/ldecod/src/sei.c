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
 ************************************************************************
 * \file  sei.c
 *
 * \brief
 *    Functions to implement SEI messages
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Dong Tian        <tian@cs.tut.fi>
 *    - Karsten Suehring <suehring@hhi.de>
 ************************************************************************
 */

#include "contributors.h"

#include <math.h>
#include "global.h"
#include "memalloc.h"
#include "sei.h"
#include "vlc.h"
#include "header.h"
#include "mbuffer.h"
#include "parset.h"


// #define PRINT_BUFFERING_PERIOD_INFO    // uncomment to print buffering period SEI info
// #define PRINT_PCITURE_TIMING_INFO      // uncomment to print picture timing SEI info
// #define WRITE_MAP_IMAGE                // uncomment to write spare picture map
// #define PRINT_SUBSEQUENCE_INFO         // uncomment to print sub-sequence SEI info
// #define PRINT_SUBSEQUENCE_LAYER_CHAR   // uncomment to print sub-sequence layer characteristics SEI info
// #define PRINT_SUBSEQUENCE_CHAR         // uncomment to print sub-sequence characteristics SEI info
// #define PRINT_SCENE_INFORMATION        // uncomment to print scene information SEI info
// #define PRINT_PAN_SCAN_RECT            // uncomment to print pan-scan rectangle SEI info
// #define PRINT_RECOVERY_POINT           // uncomment to print random access point SEI info
// #define PRINT_FILLER_PAYLOAD_INFO      // uncomment to print filler payload SEI info
// #define PRINT_DEC_REF_PIC_MARKING      // uncomment to print decoded picture buffer management repetition SEI info
// #define PRINT_RESERVED_INFO            // uncomment to print reserved SEI info
// #define PRINT_USER_DATA_UNREGISTERED_INFO          // uncomment to print unregistered user data SEI info
// #define PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO  // uncomment to print ITU-T T.35 user data SEI info
// #define PRINT_FULL_FRAME_FREEZE_INFO               // uncomment to print full-frame freeze SEI info
// #define PRINT_FULL_FRAME_FREEZE_RELEASE_INFO       // uncomment to print full-frame freeze release SEI info
// #define PRINT_FULL_FRAME_SNAPSHOT_INFO             // uncomment to print full-frame snapshot SEI info
// #define PRINT_PROGRESSIVE_REFINEMENT_END_INFO      // uncomment to print Progressive refinement segment start SEI info
// #define PRINT_PROGRESSIVE_REFINEMENT_END_INFO      // uncomment to print Progressive refinement segment end SEI info
// #define PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO    // uncomment to print Motion-constrained slice group set SEI info
// #define PRINT_FILM_GRAIN_CHARACTERISTICS_INFO      // uncomment to print Film grain characteristics SEI info
// #define PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO // uncomment to print deblocking filter display preference SEI info
// #define PRINT_STEREO_VIDEO_INFO_INFO               // uncomment to print stereo video SEI info
// #define PRINT_TONE_MAPPING                         // uncomment to print tone-mapping SEI info
// #define PRINT_POST_FILTER_HINT_INFO                // uncomment to print post-filter hint SEI info
// #define PRINT_FRAME_PACKING_ARRANGEMENT_INFO       // uncomment to print frame packing arrangement SEI info
#if EXT3D // NOKIA_DEPTH_SEI_C0162
//#define PRINT_DEPTH_REPRESENTATION_SEI              // uncomment to print depth representation SEI info
//#define PRINT_MULTIVIEW_ACQUISITION_INFO            // uncomment to print multi-view acquisition SEI info
//#define PRINT_REFERENCE_DISPLAY_INFO_SEI              // uncomment to print reference display info sei on command line
#endif
/*!
 ************************************************************************
 *  \brief
 *     Interpret the SEI rbsp
 *  \param msg
 *     a pointer that point to the sei message.
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void InterpretSEIMessage(byte* msg, int size, VideoParameters *p_Vid, Slice *pSlice)
{
  int payload_type = 0;
  int payload_size = 0;
  int offset = 1;
  byte tmp_byte;
  
  UNREFERENCED_PARAMETER(size);

  do
  {
    // sei_message();
    payload_type = 0;
    tmp_byte = msg[offset++];
    while (tmp_byte == 0xFF)
    {
      payload_type += 255;
      tmp_byte = msg[offset++];
    }
    payload_type += tmp_byte;   // this is the last byte

    payload_size = 0;
    tmp_byte = msg[offset++];
    while (tmp_byte == 0xFF)
    {
      payload_size += 255;
      tmp_byte = msg[offset++];
    }
    payload_size += tmp_byte;   // this is the last byte

    switch ( payload_type )     // sei_payload( type, size );
    {
    case  SEI_BUFFERING_PERIOD:
#if EXT3D
      interpret_buffering_period_info( msg+offset, payload_size, p_Vid ,pSlice->is_depth);
#else
      interpret_buffering_period_info( msg+offset, payload_size, p_Vid );
#endif
      break;
    case  SEI_PIC_TIMING:
      interpret_picture_timing_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_PAN_SCAN_RECT:
      interpret_pan_scan_rect_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_FILLER_PAYLOAD:
      interpret_filler_payload_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_USER_DATA_REGISTERED_ITU_T_T35:
      interpret_user_data_registered_itu_t_t35_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_USER_DATA_UNREGISTERED:
      interpret_user_data_unregistered_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_RECOVERY_POINT:
      interpret_recovery_point_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_DEC_REF_PIC_MARKING_REPETITION:
      interpret_dec_ref_pic_marking_repetition_info( msg+offset, payload_size, p_Vid, pSlice );
      break;
    case  SEI_SPARE_PIC:
      interpret_spare_pic( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_SCENE_INFO:
      interpret_scene_information( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_SUB_SEQ_INFO:
      interpret_subsequence_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_SUB_SEQ_LAYER_CHARACTERISTICS:
      interpret_subsequence_layer_characteristics_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_SUB_SEQ_CHARACTERISTICS:
      interpret_subsequence_characteristics_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_FULL_FRAME_FREEZE:
      interpret_full_frame_freeze_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_FULL_FRAME_FREEZE_RELEASE:
      interpret_full_frame_freeze_release_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_FULL_FRAME_SNAPSHOT:
      interpret_full_frame_snapshot_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START:
      interpret_progressive_refinement_start_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END:
      interpret_progressive_refinement_end_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET:
      interpret_motion_constrained_slice_group_set_info( msg+offset, payload_size, p_Vid );
    case  SEI_FILM_GRAIN_CHARACTERISTICS:
      interpret_film_grain_characteristics_info ( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE:
      interpret_deblocking_filter_display_preference_info ( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_STEREO_VIDEO_INFO:
      interpret_stereo_video_info_info ( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_TONE_MAPPING:
      interpret_tone_mapping( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_POST_FILTER_HINTS:
      interpret_post_filter_hints_info ( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_FRAME_PACKING_ARRANGEMENT:
      interpret_frame_packing_arrangement_info( msg+offset, payload_size, p_Vid );
      break;
#if EXT3D // NOKIA_DEPTH_SEI_C0162
    case SEI_3DVC_SCALABLE_NESTING:
      interpret_3dvc_scalable_nesting( msg+offset, payload_size, p_Vid );
      break;
    case SEI_MULTIVIEW_ACQUISITION_INFO:
      interpret_multiview_acquisition_info( msg+offset, payload_size, p_Vid );
      break;
    case SEI_3DV_DEPTH_REPRESENTATION_INFO:
      interpret_depth_representation_info( msg+offset, payload_size, p_Vid );
      break;
    case  SEI_3DV_REFERENCE_DISPLAY_INFO:
      interpret_reference_display_info( msg+offset, payload_size, p_Vid );
      break;
#endif
    default:
      interpret_reserved_info( msg+offset, payload_size, p_Vid );
      break;    
    }
    offset += payload_size;

  } while( msg[offset] != 0x80 );    // more_rbsp_data()  msg[offset] != 0x80
  // ignore the trailing bits rbsp_trailing_bits();
  assert(msg[offset] == 0x80);      // this is the trailing bits
  assert( offset+1 == size );
}


/*!
************************************************************************
*  \brief
*     Interpret the spare picture SEI message
*  \param payload
*     a pointer that point to the sei payload
*  \param size
*     the size of the sei message
*  \param p_Vid
*     the image pointer
*
************************************************************************
*/
void interpret_spare_pic( byte* payload, int size, VideoParameters *p_Vid )
{
  int i,x,y;
  Bitstream* buf;
  int bit0, bit1, no_bit0; // , bitc
  int target_frame_num = 0;
  int num_spare_pics;
  int delta_spare_frame_num, CandidateSpareFrameNum, SpareFrameNum = 0;
  int ref_area_indicator;

  int m, n, left, right, top, bottom,directx, directy;
  byte ***map;

#ifdef WRITE_MAP_IMAGE
  int symbol_size_in_bytes = p_Vid->pic_unit_bitsize_on_disk/8;
  int  j, k, i0, j0, tmp, kk;
  char filename[20] = "map_dec.yuv";
  FILE *fp;
  imgpel** Y;
  static int old_pn=-1;
  static int first = 1;

  printf("Spare picture SEI message\n");
#endif

  p_Dec->UsedBits = 0;

  assert( payload!=NULL);
  assert( p_Vid!=NULL);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  target_frame_num = ue_v("SEI: target_frame_num", buf);

#ifdef WRITE_MAP_IMAGE
  printf( "target_frame_num is %d\n", target_frame_num );
#endif

  num_spare_pics = 1 + ue_v("SEI: num_spare_pics_minus1", buf);

#ifdef WRITE_MAP_IMAGE
  printf( "num_spare_pics is %d\n", num_spare_pics );
#endif

  get_mem3D(&map, num_spare_pics, p_Vid->height >> 4, p_Vid->width >> 4);

  for (i=0; i<num_spare_pics; i++)
  {
    if (i==0)
    {
      CandidateSpareFrameNum = target_frame_num - 1;
      if ( CandidateSpareFrameNum < 0 ) CandidateSpareFrameNum = MAX_FN - 1;
    }
    else
      CandidateSpareFrameNum = SpareFrameNum;

    delta_spare_frame_num = ue_v("SEI: delta_spare_frame_num", buf);

    SpareFrameNum = CandidateSpareFrameNum - delta_spare_frame_num;
    if( SpareFrameNum < 0 )
      SpareFrameNum = MAX_FN + SpareFrameNum;

    ref_area_indicator = ue_v("SEI: ref_area_indicator", buf);

    switch ( ref_area_indicator )
    {
    case 0:   // The whole frame can serve as spare picture
      for (y=0; y<p_Vid->height >> 4; y++)
        for (x=0; x<p_Vid->width >> 4; x++)
          map[i][y][x] = 0;
      break;
    case 1:   // The map is not compressed
      for (y=0; y<p_Vid->height >> 4; y++)
        for (x=0; x<p_Vid->width >> 4; x++)
        {
          map[i][y][x] = (byte) u_1("SEI: ref_mb_indicator", buf);
        }
      break;
    case 2:   // The map is compressed
              //!KS: could not check this function, description is unclear (as stated in Ed. Note)
      bit0 = 0;
      bit1 = 1;
      //bitc = bit0;
      no_bit0 = -1;

      x = ( (p_Vid->width >> 4) - 1 ) / 2;
      y = ( (p_Vid->height >> 4) - 1 ) / 2;
      left = right = x;
      top = bottom = y;
      directx = 0;
      directy = 1;

      for (m=0; m<p_Vid->height >> 4; m++)
        for (n=0; n<p_Vid->width >> 4; n++)
        {

          if (no_bit0<0)
          {
            no_bit0 = ue_v("SEI: zero_run_length", buf);
          }
          if (no_bit0>0) 
            map[i][y][x] = (byte) bit0;
          else 
            map[i][y][x] = (byte) bit1;
          no_bit0--;

          // go to the next mb:
          if ( directx == -1 && directy == 0 )
          {
            if (x > left) x--;
            else if (x == 0)
            {
              y = bottom + 1;
              bottom++;
              directx = 1;
              directy = 0;
            }
            else if (x == left)
            {
              x--;
              left--;
              directx = 0;
              directy = 1;
            }
          }
          else if ( directx == 1 && directy == 0 )
          {
            if (x < right) x++;
            else if (x == (p_Vid->width >> 4) - 1)
            {
              y = top - 1;
              top--;
              directx = -1;
              directy = 0;
            }
            else if (x == right)
            {
              x++;
              right++;
              directx = 0;
              directy = -1;
            }
          }
          else if ( directx == 0 && directy == -1 )
          {
            if ( y > top) y--;
            else if (y == 0)
            {
              x = left - 1;
              left--;
              directx = 0;
              directy = 1;
            }
            else if (y == top)
            {
              y--;
              top--;
              directx = -1;
              directy = 0;
            }
          }
          else if ( directx == 0 && directy == 1 )
          {
            if (y < bottom) y++;
            else if (y == (p_Vid->height >> 4) - 1)
            {
              x = right+1;
              right++;
              directx = 0;
              directy = -1;
            }
            else if (y == bottom)
            {
              y++;
              bottom++;
              directx = 1;
              directy = 0;
            }
          }


        }
      break;
    default:
      printf( "Wrong ref_area_indicator %d!\n", ref_area_indicator );
      exit(0);
      break;
    }

  } // end of num_spare_pics

#ifdef WRITE_MAP_IMAGE
  // begin to write map seq
  if ( old_pn != p_Vid->number )
  {
    old_pn = p_Vid->number;
    get_mem2Dpel(&Y, p_Vid->height, p_Vid->width);
    if (first)
    {
      fp = fopen( filename, "wb" );
      first = 0;
    }
    else
      fp = fopen( filename, "ab" );
    assert( fp != NULL );
    for (kk=0; kk<num_spare_pics; kk++)
    {
      for (i=0; i < p_Vid->height >> 4; i++)
        for (j=0; j < p_Vid->width >> 4; j++)
        {
          tmp=map[kk][i][j]==0? p_Vid->max_pel_value_comp[0] : 0;
          for (i0=0; i0<16; i0++)
            for (j0=0; j0<16; j0++)
              Y[i*16+i0][j*16+j0]=tmp;
        }

      // write the map image
      for (i=0; i < p_Vid->height; i++)
        for (j=0; j < p_Vid->width; j++)
          fwrite(&(Y[i][j]), symbol_size_in_bytes, 1, p_out);

      for (k=0; k < 2; k++)
        for (i=0; i < p_Vid->height>>1; i++)
          for (j=0; j < p_Vid->width>>1; j++)
            fwrite(&(p_Vid->dc_pred_value_comp[1]), symbol_size_in_bytes, 1, p_out);
    }
    fclose( fp );
    free_mem2Dpel( Y );
  }
  // end of writing map image
#undef WRITE_MAP_IMAGE
#endif

  free_mem3D( map );

  free(buf);
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Sub-sequence information SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_subsequence_info( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  int sub_seq_frame_num_flag;
#ifdef PRINT_SUBSEQUENCE_INFO
  int sub_seq_layer_num, sub_seq_frame_num, last_pic_flag, sub_seq_id, first_ref_pic_flag, leading_non_ref_pic_flag;
#endif
  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_SUBSEQUENCE_INFO
  sub_seq_layer_num        =
#endif
          ue_v("SEI: sub_seq_layer_num"       , buf);
#ifdef PRINT_SUBSEQUENCE_INFO
  sub_seq_id               = 
#endif
          ue_v("SEI: sub_seq_id"              , buf);
#ifdef PRINT_SUBSEQUENCE_INFO
  first_ref_pic_flag       =
#endif
          u_1 ("SEI: first_ref_pic_flag"      , buf);
#ifdef PRINT_SUBSEQUENCE_INFO
  leading_non_ref_pic_flag = 
#endif
          u_1 ("SEI: leading_non_ref_pic_flag", buf);
#ifdef PRINT_SUBSEQUENCE_INFO
  last_pic_flag            =
#endif
          u_1 ("SEI: last_pic_flag"           , buf);
  sub_seq_frame_num_flag   = u_1 ("SEI: sub_seq_frame_num_flag"  , buf);
  if (sub_seq_frame_num_flag)
  {
#ifdef PRINT_SUBSEQUENCE_INFO
    sub_seq_frame_num        = 
#endif
            ue_v("SEI: sub_seq_frame_num"       , buf);
  }

#ifdef PRINT_SUBSEQUENCE_INFO
  printf("Sub-sequence information SEI message\n");
  printf("sub_seq_layer_num        = %d\n", sub_seq_layer_num );
  printf("sub_seq_id               = %d\n", sub_seq_id);
  printf("first_ref_pic_flag       = %d\n", first_ref_pic_flag);
  printf("leading_non_ref_pic_flag = %d\n", leading_non_ref_pic_flag);
  printf("last_pic_flag            = %d\n", last_pic_flag);
  printf("sub_seq_frame_num_flag   = %d\n", sub_seq_frame_num_flag);
  if (sub_seq_frame_num_flag)
  {
    printf("sub_seq_frame_num        = %d\n", sub_seq_frame_num);
  }
#endif

  free(buf);
#ifdef PRINT_SUBSEQUENCE_INFO
#undef PRINT_SUBSEQUENCE_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the Sub-sequence layer characteristics SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_subsequence_layer_characteristics_info( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  long num_sub_layers;
#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
  long accurate_statistics_flag, average_bit_rate, average_frame_rate;
#endif
  int i;
  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  num_sub_layers = 1 + ue_v("SEI: num_sub_layers_minus1", buf);

#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
  printf("Sub-sequence layer characteristics SEI message\n");
  printf("num_sub_layers_minus1 = %d\n", num_sub_layers - 1);
#endif

  for (i=0; i<num_sub_layers; i++)
  {
#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
    accurate_statistics_flag = 
#endif
            u_1(   "SEI: accurate_statistics_flag", buf);
#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
    average_bit_rate         = 
#endif
            u_v(16,"SEI: average_bit_rate"        , buf);
#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
    average_frame_rate       =
#endif
            u_v(16,"SEI: average_frame_rate"      , buf);

#ifdef PRINT_SUBSEQUENCE_LAYER_CHAR
    printf("layer %d: accurate_statistics_flag = %ld \n", i, accurate_statistics_flag);
    printf("layer %d: average_bit_rate         = %ld \n", i, average_bit_rate);
    printf("layer %d: average_frame_rate       = %ld \n", i, average_frame_rate);
#endif
  }
  free (buf);
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Sub-sequence characteristics SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_subsequence_characteristics_info( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  int i;
  int duration_flag, average_rate_flag;
  int num_referenced_subseqs;
#ifdef PRINT_SUBSEQUENCE_CHAR
  int sub_seq_layer_num, sub_seq_id, accurate_statistics_flag;
  unsigned long sub_seq_duration, average_bit_rate, average_frame_rate;
  int ref_sub_seq_layer_num, ref_sub_seq_id, ref_sub_seq_direction;
#endif
  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_SUBSEQUENCE_CHAR
  sub_seq_layer_num =
#endif
          ue_v("SEI: sub_seq_layer_num", buf);
#ifdef PRINT_SUBSEQUENCE_CHAR
  sub_seq_id        =
#endif
          ue_v("SEI: sub_seq_id", buf);
  duration_flag     = u_1 ("SEI: duration_flag", buf);

#ifdef PRINT_SUBSEQUENCE_CHAR
  printf("Sub-sequence characteristics SEI message\n");
  printf("sub_seq_layer_num = %d\n", sub_seq_layer_num );
  printf("sub_seq_id        = %d\n", sub_seq_id);
  printf("duration_flag     = %d\n", duration_flag);
#endif

  if ( duration_flag )
  {
#ifdef PRINT_SUBSEQUENCE_CHAR
    sub_seq_duration =
#endif
            u_v (32, "SEI: duration_flag", buf);
#ifdef PRINT_SUBSEQUENCE_CHAR
    printf("sub_seq_duration = %ld\n", sub_seq_duration);
#endif
  }

  average_rate_flag = u_1 ("SEI: average_rate_flag", buf);

#ifdef PRINT_SUBSEQUENCE_CHAR
  printf("average_rate_flag = %d\n", average_rate_flag);
#endif

  if ( average_rate_flag )
  {
#ifdef PRINT_SUBSEQUENCE_CHAR
    accurate_statistics_flag =
#endif
            u_1 (    "SEI: accurate_statistics_flag", buf);
#ifdef PRINT_SUBSEQUENCE_CHAR
    average_bit_rate         =
#endif
            u_v (16, "SEI: average_bit_rate", buf);
#ifdef PRINT_SUBSEQUENCE_CHAR
    average_frame_rate       =
#endif
            u_v (16, "SEI: average_frame_rate", buf);

#ifdef PRINT_SUBSEQUENCE_CHAR
    printf("accurate_statistics_flag = %d\n", accurate_statistics_flag);
    printf("average_bit_rate         = %ld\n", average_bit_rate);
    printf("average_frame_rate       = %ld\n", average_frame_rate);
#endif
  }

  num_referenced_subseqs  = ue_v("SEI: num_referenced_subseqs", buf);

#ifdef PRINT_SUBSEQUENCE_CHAR
  printf("num_referenced_subseqs = %d\n", num_referenced_subseqs);
#endif

  for (i=0; i<num_referenced_subseqs; i++)
  {
#ifdef PRINT_SUBSEQUENCE_CHAR
    ref_sub_seq_layer_num  =
#endif
            ue_v("SEI: ref_sub_seq_layer_num", buf);
#ifdef PRINT_SUBSEQUENCE_CHAR
    ref_sub_seq_id         = 
#endif
            ue_v("SEI: ref_sub_seq_id", buf);
#ifdef PRINT_SUBSEQUENCE_CHAR
    ref_sub_seq_direction  =
#endif
            u_1 ("SEI: ref_sub_seq_direction", buf);

#ifdef PRINT_SUBSEQUENCE_CHAR
    printf("ref_sub_seq_layer_num = %d\n", ref_sub_seq_layer_num);
    printf("ref_sub_seq_id        = %d\n", ref_sub_seq_id);
    printf("ref_sub_seq_direction = %d\n", ref_sub_seq_direction);
#endif
  }

  free( buf );
#ifdef PRINT_SUBSEQUENCE_CHAR
#undef PRINT_SUBSEQUENCE_CHAR
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Scene information SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_scene_information( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  int scene_transition_type;
#ifdef PRINT_SCENE_INFORMATION
  int scene_id, second_scene_id;
#endif
  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_SCENE_INFORMATION
  scene_id              = 
#endif
          ue_v("SEI: scene_id"             , buf);
  scene_transition_type = ue_v("SEI: scene_transition_type", buf);
  if ( scene_transition_type > 3 )
  {
#ifdef PRINT_SCENE_INFORMATION
    second_scene_id     = 
#endif
            ue_v("SEI: scene_transition_type", buf);;
  }

#ifdef PRINT_SCENE_INFORMATION
  printf("Scene information SEI message\n");
  printf("scene_transition_type = %d\n", scene_transition_type);
  printf("scene_id              = %d\n", scene_id);
  if ( scene_transition_type > 3 )
  {
    printf("second_scene_id       = %d\n", second_scene_id);
  }
#endif
  free( buf );
#ifdef PRINT_SCENE_INFORMATION
#undef PRINT_SCENE_INFORMATION
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Filler payload SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_filler_payload_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int payload_cnt = 0;
  UNREFERENCED_PARAMETER(p_Vid);

  while (payload_cnt<size)
  {
    if (payload[payload_cnt] == 0xFF)
    {
       payload_cnt++;
    }
  }


#ifdef PRINT_FILLER_PAYLOAD_INFO
  printf("Filler payload SEI message\n");
  if (payload_cnt==size)
  {
    printf("read %d bytes of filler payload\n", payload_cnt);
  }
  else
  {
    printf("error reading filler payload: not all bytes are 0xFF (%d of %d)\n", payload_cnt, size);
  }
#endif

#ifdef PRINT_FILLER_PAYLOAD_INFO
#undef PRINT_FILLER_PAYLOAD_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the User data unregistered SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_user_data_unregistered_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int offset = 0;
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
  byte payload_byte;
#endif

#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
  printf("User data unregistered SEI message\n");
  printf("uuid_iso_11578 = 0x");
#endif
  UNREFERENCED_PARAMETER(payload);
  UNREFERENCED_PARAMETER(p_Vid);

  assert (size>=16);

  for (offset = 0; offset < 16; offset++)
  {
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
    printf("%02x",payload[offset]);
#endif
  }

#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
    printf("\n");
#endif

  while (offset < size)
  {
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
    payload_byte = payload[offset];
#endif
    offset ++;
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
    printf("Unreg data payload_byte = %d\n", payload_byte);
#endif
  }
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
#undef PRINT_USER_DATA_UNREGISTERED_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the User data registered by ITU-T T.35 SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_user_data_registered_itu_t_t35_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int offset = 0;
  byte itu_t_t35_country_code;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
  byte itu_t_t35_country_code_extension_byte, payload_byte;
#endif
  UNREFERENCED_PARAMETER(p_Vid);


  itu_t_t35_country_code = payload[offset];
  offset++;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
  printf("User data registered by ITU-T T.35 SEI message\n");
  printf(" itu_t_t35_country_code = %d \n", itu_t_t35_country_code);
#endif
  if(itu_t_t35_country_code == 0xFF)
  {
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
    itu_t_t35_country_code_extension_byte = payload[offset];
#endif
    offset++;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
    printf(" ITU_T_T35_COUNTRY_CODE_EXTENSION_BYTE %d \n", itu_t_t35_country_code_extension_byte);
#endif
  }
  while (offset < size)
  {
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
    payload_byte = payload[offset];
#endif
    offset ++;
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
    printf("itu_t_t35 payload_byte = %d\n", payload_byte);
#endif
  }
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#undef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Pan scan rectangle SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_pan_scan_rect_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int pan_scan_rect_cancel_flag;
  int pan_scan_cnt_minus1, i;
#ifdef PRINT_PAN_SCAN_RECT
  int pan_scan_rect_repetition_period;
  int pan_scan_rect_id, pan_scan_rect_left_offset, pan_scan_rect_right_offset;
  int pan_scan_rect_top_offset, pan_scan_rect_bottom_offset;
#endif

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_PAN_SCAN_RECT
  pan_scan_rect_id = 
#endif
          ue_v("SEI: pan_scan_rect_id", buf);

  pan_scan_rect_cancel_flag = u_1("SEI: pan_scan_rect_cancel_flag", buf);
  if (!pan_scan_rect_cancel_flag) 
  {
    pan_scan_cnt_minus1 = ue_v("SEI: pan_scan_cnt_minus1", buf);
    for (i = 0; i <= pan_scan_cnt_minus1; i++) 
    {
#ifdef PRINT_PAN_SCAN_RECT
      pan_scan_rect_left_offset   = 
#endif
              se_v("SEI: pan_scan_rect_left_offset"  , buf);
#ifdef PRINT_PAN_SCAN_RECT
      pan_scan_rect_right_offset  =
#endif
              se_v("SEI: pan_scan_rect_right_offset" , buf);
#ifdef PRINT_PAN_SCAN_RECT
      pan_scan_rect_top_offset    =
#endif
              se_v("SEI: pan_scan_rect_top_offset"   , buf);
#ifdef PRINT_PAN_SCAN_RECT
      pan_scan_rect_bottom_offset =
#endif
              se_v("SEI: pan_scan_rect_bottom_offset", buf);
#ifdef PRINT_PAN_SCAN_RECT
      printf("Pan scan rectangle SEI message %d/%d\n", i, pan_scan_cnt_minus1);
      printf("pan_scan_rect_id            = %d\n", pan_scan_rect_id);
      printf("pan_scan_rect_left_offset   = %d\n", pan_scan_rect_left_offset);
      printf("pan_scan_rect_right_offset  = %d\n", pan_scan_rect_right_offset);
      printf("pan_scan_rect_top_offset    = %d\n", pan_scan_rect_top_offset);
      printf("pan_scan_rect_bottom_offset = %d\n", pan_scan_rect_bottom_offset);
#endif
    }
#ifdef PRINT_PAN_SCAN_RECT
    pan_scan_rect_repetition_period =
#endif
            ue_v("SEI: pan_scan_rect_repetition_period", buf);
  }

  free (buf);
#ifdef PRINT_PAN_SCAN_RECT
#undef PRINT_PAN_SCAN_RECT
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Random access point SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_recovery_point_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int recovery_frame_cnt;
#ifdef PRINT_RECOVERY_POINT
  int exact_match_flag, broken_link_flag, changing_slice_group_idc;
#endif

  Bitstream* buf;


  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  recovery_frame_cnt       = ue_v(    "SEI: recovery_frame_cnt"      , buf);
#ifdef PRINT_RECOVERY_POINT
  exact_match_flag         = 
#endif
          u_1 (    "SEI: exact_match_flag"        , buf);
#ifdef PRINT_RECOVERY_POINT
  broken_link_flag         =
#endif
          u_1 (    "SEI: broken_link_flag"        , buf);
#ifdef PRINT_RECOVERY_POINT
  changing_slice_group_idc =
#endif
          u_v ( 2, "SEI: changing_slice_group_idc", buf);

  p_Vid->recovery_point = 1;
  p_Vid->recovery_frame_cnt = recovery_frame_cnt;

#ifdef PRINT_RECOVERY_POINT
  printf("Recovery point SEI message\n");
  printf("recovery_frame_cnt       = %d\n", recovery_frame_cnt);
  printf("exact_match_flag         = %d\n", exact_match_flag);
  printf("broken_link_flag         = %d\n", broken_link_flag);
  printf("changing_slice_group_idc = %d\n", changing_slice_group_idc);
#endif
  free (buf);
#ifdef PRINT_RECOVERY_POINT
#undef PRINT_RECOVERY_POINT
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Decoded Picture Buffer Management Repetition SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_dec_ref_pic_marking_repetition_info( byte* payload, int size, VideoParameters *p_Vid, Slice *pSlice )
{
  int original_idr_flag, original_field_pic_flag;
#ifdef PRINT_DEC_REF_PIC_MARKING
  int original_frame_num, original_bottom_field_flag;
#endif

  DecRefPicMarking_t *tmp_drpm;

  DecRefPicMarking_t *old_drpm;
  int old_idr_flag , old_no_output_of_prior_pics_flag, old_long_term_reference_flag , old_adaptive_ref_pic_buffering_flag;


  Bitstream* buf;

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  original_idr_flag     = u_1 (    "SEI: original_idr_flag"    , buf);
#ifdef PRINT_DEC_REF_PIC_MARKING
  original_frame_num    = 
#endif
          ue_v(    "SEI: original_frame_num"   , buf);

  if ( !p_Vid->active_sps->frame_mbs_only_flag )
  {
    original_field_pic_flag = u_1 ( "SEI: original_field_pic_flag", buf);
    if ( original_field_pic_flag )
    {
#ifdef PRINT_DEC_REF_PIC_MARKING
      original_bottom_field_flag = 
#endif
              u_1 ( "SEI: original_bottom_field_flag", buf);
    }
  }

#ifdef PRINT_DEC_REF_PIC_MARKING
  printf("Decoded Picture Buffer Management Repetition SEI message\n");
  printf("original_idr_flag       = %d\n", original_idr_flag);
  printf("original_frame_num      = %d\n", original_frame_num);
  if ( active_sps->frame_mbs_only_flag )
  {
    printf("original_field_pic_flag = %d\n", original_field_pic_flag);
    if ( original_field_pic_flag )
    {
      printf("original_bottom_field_flag = %d\n", original_bottom_field_flag);
    }
  }
#endif

  // we need to save everything that is probably overwritten in dec_ref_pic_marking()
  old_drpm = pSlice->dec_ref_pic_marking_buffer;
  old_idr_flag = pSlice->idr_flag; //p_Vid->idr_flag;

  old_no_output_of_prior_pics_flag = pSlice->no_output_of_prior_pics_flag; //p_Vid->no_output_of_prior_pics_flag;
  old_long_term_reference_flag = pSlice->long_term_reference_flag;
  old_adaptive_ref_pic_buffering_flag = pSlice->adaptive_ref_pic_buffering_flag;

  // set new initial values
  //p_Vid->idr_flag = original_idr_flag;
  pSlice->idr_flag = original_idr_flag;
  pSlice->dec_ref_pic_marking_buffer = NULL;

  dec_ref_pic_marking(p_Vid, buf, pSlice);

  // print out decoded values
#ifdef PRINT_DEC_REF_PIC_MARKING
  if (p_Vid->idr_flag)
  {
    printf("no_output_of_prior_pics_flag = %d\n", p_Vid->no_output_of_prior_pics_flag);
    printf("long_term_reference_flag     = %d\n", p_Vid->long_term_reference_flag);
  }
  else
  {
    printf("adaptive_ref_pic_buffering_flag  = %d\n", p_Vid->adaptive_ref_pic_buffering_flag);
    if (p_Vid->adaptive_ref_pic_buffering_flag)
    {
      tmp_drpm=p_Vid->dec_ref_pic_marking_buffer;
      while (tmp_drpm != NULL)
      {
        printf("memory_management_control_operation  = %d\n", tmp_drpm->memory_management_control_operation);

        if ((tmp_drpm->memory_management_control_operation==1)||(tmp_drpm->memory_management_control_operation==3))
        {
          printf("difference_of_pic_nums_minus1        = %d\n", tmp_drpm->difference_of_pic_nums_minus1);
        }
        if (tmp_drpm->memory_management_control_operation==2)
        {
          printf("long_term_pic_num                    = %d\n", tmp_drpm->long_term_pic_num);
        }
        if ((tmp_drpm->memory_management_control_operation==3)||(tmp_drpm->memory_management_control_operation==6))
        {
          printf("long_term_frame_idx                  = %d\n", tmp_drpm->long_term_frame_idx);
        }
        if (tmp_drpm->memory_management_control_operation==4)
        {
          printf("max_long_term_pic_idx_plus1          = %d\n", tmp_drpm->max_long_term_frame_idx_plus1);
        }
        tmp_drpm = tmp_drpm->Next;
      }
    }
  }
#endif

  while (pSlice->dec_ref_pic_marking_buffer)
  {
    tmp_drpm=pSlice->dec_ref_pic_marking_buffer;

    pSlice->dec_ref_pic_marking_buffer=tmp_drpm->Next;
    free (tmp_drpm);
  }

  // restore old values in p_Vid
  pSlice->dec_ref_pic_marking_buffer = old_drpm;
  pSlice->idr_flag = old_idr_flag;
  pSlice->no_output_of_prior_pics_flag = old_no_output_of_prior_pics_flag;
  p_Vid->no_output_of_prior_pics_flag = pSlice->no_output_of_prior_pics_flag;
  pSlice->long_term_reference_flag = old_long_term_reference_flag;
  pSlice->adaptive_ref_pic_buffering_flag = old_adaptive_ref_pic_buffering_flag;

  free (buf);
#ifdef PRINT_DEC_REF_PIC_MARKING
#undef PRINT_DEC_REF_PIC_MARKING
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the Full-frame freeze SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_full_frame_freeze_info( byte* payload, int size, VideoParameters *p_Vid )
{
#ifdef PRINT_FULL_FRAME_FREEZE_INFO
  int full_frame_freeze_repetition_period;
#endif
  Bitstream* buf;
  UNREFERENCED_PARAMETER(p_Vid);

  buf = (Bitstream*)malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

#ifdef PRINT_FULL_FRAME_FREEZE_INFO
  full_frame_freeze_repetition_period  = 
#endif
          ue_v(    "SEI: full_frame_freeze_repetition_period"   , buf);

#ifdef PRINT_FULL_FRAME_FREEZE_INFO
  printf("full_frame_freeze_repetition_period = %d\n", full_frame_freeze_repetition_period);
#endif

  free (buf);
#ifdef PRINT_FULL_FRAME_FREEZE_INFO
#undef PRINT_FULL_FRAME_FREEZE_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Full-frame freeze release SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_full_frame_freeze_release_info( byte* payload, int size, VideoParameters *p_Vid )
{
  UNREFERENCED_PARAMETER(payload);
  UNREFERENCED_PARAMETER(size);
  UNREFERENCED_PARAMETER(p_Vid);

#ifdef PRINT_FULL_FRAME_FREEZE_RELEASE_INFO
  printf("Full-frame freeze release SEI message\n");
  if (size)
  {
    printf("payload size of this message should be zero, but is %d bytes.\n", size);
  }
#endif

#ifdef PRINT_FULL_FRAME_FREEZE_RELEASE_INFO
#undef PRINT_FULL_FRAME_FREEZE_RELEASE_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the Full-frame snapshot SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_full_frame_snapshot_info( byte* payload, int size, VideoParameters *p_Vid )
{
#ifdef PRINT_FULL_FRAME_SNAPSHOT_INFO
  int snapshot_id;
#endif

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_FULL_FRAME_SNAPSHOT_INFO
  snapshot_id = 
#endif
          ue_v("SEI: snapshot_id", buf);

#ifdef PRINT_FULL_FRAME_SNAPSHOT_INFO
  printf("Full-frame snapshot SEI message\n");
  printf("snapshot_id = %d\n", snapshot_id);
#endif
  free (buf);
#ifdef PRINT_FULL_FRAME_SNAPSHOT_INFO
#undef PRINT_FULL_FRAME_SNAPSHOT_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the Progressive refinement segment start SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_progressive_refinement_start_info( byte* payload, int size, VideoParameters *p_Vid )
{
#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
  int progressive_refinement_id, num_refinement_steps_minus1;
#endif

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
  progressive_refinement_id   = 
#endif
          ue_v("SEI: progressive_refinement_id"  , buf);
#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
  num_refinement_steps_minus1 = 
#endif
          ue_v("SEI: num_refinement_steps_minus1", buf);

#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
  printf("Progressive refinement segment start SEI message\n");
  printf("progressive_refinement_id   = %d\n", progressive_refinement_id);
  printf("num_refinement_steps_minus1 = %d\n", num_refinement_steps_minus1);
#endif
  free (buf);
#ifdef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
#undef PRINT_PROGRESSIVE_REFINEMENT_START_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Progressive refinement segment end SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_progressive_refinement_end_info( byte* payload, int size, VideoParameters *p_Vid )
{
#ifdef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
  int progressive_refinement_id;
#endif

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
  progressive_refinement_id   =
#endif
          ue_v("SEI: progressive_refinement_id"  , buf);

#ifdef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
  printf("Progressive refinement segment end SEI message\n");
  printf("progressive_refinement_id   = %d\n", progressive_refinement_id);
#endif
  free (buf);
#ifdef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
#undef PRINT_PROGRESSIVE_REFINEMENT_END_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Motion-constrained slice group set SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_motion_constrained_slice_group_set_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int num_slice_groups_minus1, pan_scan_rect_flag;
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
  int slice_group_id, exact_match_flag, pan_scan_rect_id;
#endif
  int i;
  int sliceGroupSize;

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  num_slice_groups_minus1   = ue_v("SEI: num_slice_groups_minus1"  , buf);
  sliceGroupSize = CeilLog2( num_slice_groups_minus1 + 1 );
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
  printf("Motion-constrained slice group set SEI message\n");
  printf("num_slice_groups_minus1   = %d\n", num_slice_groups_minus1);
#endif

  for (i=0; i<=num_slice_groups_minus1;i++)
  {

#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
    slice_group_id   = 
#endif
            u_v (sliceGroupSize, "SEI: slice_group_id" , buf)    ;
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
    printf("slice_group_id            = %d\n", slice_group_id);
#endif
  }

#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
  exact_match_flag   = 
#endif
          u_1("SEI: exact_match_flag"  , buf);
  pan_scan_rect_flag = u_1("SEI: pan_scan_rect_flag"  , buf);

#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
  printf("exact_match_flag         = %d\n", exact_match_flag);
  printf("pan_scan_rect_flag       = %d\n", pan_scan_rect_flag);
#endif

  if (pan_scan_rect_flag)
  {
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
    pan_scan_rect_id = 
#endif
            ue_v("SEI: pan_scan_rect_id"  , buf);
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
    printf("pan_scan_rect_id         = %d\n", pan_scan_rect_id);
#endif
  }

  free (buf);
#ifdef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
#undef PRINT_MOTION_CONST_SLICE_GROUP_SET_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the film grain characteristics SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_film_grain_characteristics_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int film_grain_characteristics_cancel_flag;
  int separate_colour_description_present_flag;
  int comp_model_present_flag[3];
  int num_intensity_intervals_minus1, num_model_values_minus1;
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
  int model_id;
  int film_grain_bit_depth_luma_minus8, film_grain_bit_depth_chroma_minus8, film_grain_full_range_flag, film_grain_colour_primaries, film_grain_transfer_characteristics, film_grain_matrix_coefficients;
  int blending_mode_id, log2_scale_factor;
  int intensity_interval_lower_bound, intensity_interval_upper_bound;
  int comp_model_value;
  int film_grain_characteristics_repetition_period;
#endif

  int c, i, j;

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  film_grain_characteristics_cancel_flag = u_1("SEI: film_grain_characteristics_cancel_flag", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
  printf("film_grain_characteristics_cancel_flag = %d\n", film_grain_characteristics_cancel_flag);
#endif
  if(!film_grain_characteristics_cancel_flag)
  {

#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    model_id                                    = 
#endif
            u_v(2, "SEI: model_id", buf);
    separate_colour_description_present_flag    = u_1("SEI: separate_colour_description_present_flag", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    printf("model_id = %d\n", model_id);
    printf("separate_colour_description_present_flag = %d\n", separate_colour_description_present_flag);
#endif
    if (separate_colour_description_present_flag)
    {
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      film_grain_bit_depth_luma_minus8          = 
#endif
              u_v(3, "SEI: film_grain_bit_depth_luma_minus8", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      film_grain_bit_depth_chroma_minus8        = 
#endif
              u_v(3, "SEI: film_grain_bit_depth_chroma_minus8", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      film_grain_full_range_flag                = 
#endif
              u_v(1, "SEI: film_grain_full_range_flag", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      film_grain_colour_primaries               = 
#endif
              u_v(8, "SEI: film_grain_colour_primaries", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      film_grain_transfer_characteristics       = 
#endif
              u_v(8, "SEI: film_grain_transfer_characteristics", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      film_grain_matrix_coefficients            = 
#endif
              u_v(8, "SEI: film_grain_matrix_coefficients", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      printf("film_grain_bit_depth_luma_minus8 = %d\n", film_grain_bit_depth_luma_minus8);
      printf("film_grain_bit_depth_chroma_minus8 = %d\n", film_grain_bit_depth_chroma_minus8);
      printf("film_grain_full_range_flag = %d\n", film_grain_full_range_flag);
      printf("film_grain_colour_primaries = %d\n", film_grain_colour_primaries);
      printf("film_grain_transfer_characteristics = %d\n", film_grain_transfer_characteristics);
      printf("film_grain_matrix_coefficients = %d\n", film_grain_matrix_coefficients);
#endif
    }
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    blending_mode_id                            = 
#endif
            u_v(2, "SEI: blending_mode_id", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    log2_scale_factor                           = 
#endif
            u_v(4, "SEI: log2_scale_factor", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    printf("blending_mode_id = %d\n", blending_mode_id);
    printf("log2_scale_factor = %d\n", log2_scale_factor);
#endif
    for (c = 0; c < 3; c ++)
    {
      comp_model_present_flag[c]                = u_1("SEI: comp_model_present_flag", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
      printf("comp_model_present_flag = %d\n", comp_model_present_flag[c]);
#endif
    }
    for (c = 0; c < 3; c ++)
      if (comp_model_present_flag[c])
      {
        num_intensity_intervals_minus1          = u_v(8, "SEI: num_intensity_intervals_minus1", buf);
        num_model_values_minus1                 = u_v(3, "SEI: num_model_values_minus1", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
        printf("num_intensity_intervals_minus1 = %d\n", num_intensity_intervals_minus1);
        printf("num_model_values_minus1 = %d\n", num_model_values_minus1);
#endif
        for (i = 0; i <= num_intensity_intervals_minus1; i ++)
        {
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
          intensity_interval_lower_bound        = 
#endif
                  u_v(8, "SEI: intensity_interval_lower_bound", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
          intensity_interval_upper_bound        = 
#endif
                  u_v(8, "SEI: intensity_interval_upper_bound", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
          printf("intensity_interval_lower_bound = %d\n", intensity_interval_lower_bound);
          printf("intensity_interval_upper_bound = %d\n", intensity_interval_upper_bound);
#endif
          for (j = 0; j <= num_model_values_minus1; j++)
          {
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
            comp_model_value                    = 
#endif
                    se_v("SEI: comp_model_value", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
            printf("comp_model_value = %d\n", comp_model_value);
#endif
          }
        }
      }
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    film_grain_characteristics_repetition_period = 
#endif
            ue_v("SEI: film_grain_characteristics_repetition_period", buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
    printf("film_grain_characteristics_repetition_period = %d\n", film_grain_characteristics_repetition_period);
#endif
  }

  free (buf);
#ifdef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
#undef PRINT_FILM_GRAIN_CHARACTERISTICS_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the deblocking filter display preference SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_deblocking_filter_display_preference_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int deblocking_display_preference_cancel_flag;
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
  int display_prior_to_deblocking_preferred_flag, dec_frame_buffering_constraint_flag, deblocking_display_preference_repetition_period;
#endif

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  deblocking_display_preference_cancel_flag             = u_1("SEI: deblocking_display_preference_cancel_flag", buf);
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
  printf("deblocking_display_preference_cancel_flag = %d\n", deblocking_display_preference_cancel_flag);
#endif
  if(!deblocking_display_preference_cancel_flag)
  {
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
    display_prior_to_deblocking_preferred_flag            = 
#endif
            u_1("SEI: display_prior_to_deblocking_preferred_flag", buf);
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
    dec_frame_buffering_constraint_flag                   = 
#endif
            u_1("SEI: dec_frame_buffering_constraint_flag", buf);
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
    deblocking_display_preference_repetition_period       = 
#endif
            ue_v("SEI: deblocking_display_preference_repetition_period", buf);
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
    printf("display_prior_to_deblocking_preferred_flag = %d\n", display_prior_to_deblocking_preferred_flag);
    printf("dec_frame_buffering_constraint_flag = %d\n", dec_frame_buffering_constraint_flag);
    printf("deblocking_display_preference_repetition_period = %d\n", deblocking_display_preference_repetition_period);
#endif
  }

  free (buf);
#ifdef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
#undef PRINT_DEBLOCKING_FILTER_DISPLAY_PREFERENCE_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the stereo video info SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_stereo_video_info_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int field_views_flags;
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
  int top_field_is_left_view_flag, current_frame_is_left_view_flag, next_frame_is_second_view_flag;
  int left_view_self_contained_flag;
  int right_view_self_contained_flag;
#endif

  Bitstream* buf;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = (Bitstream*)malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  field_views_flags = u_1("SEI: field_views_flags", buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
  printf("field_views_flags = %d\n", field_views_flags);
#endif
  if (field_views_flags)
  {
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
    top_field_is_left_view_flag         = 
#endif
            u_1("SEI: top_field_is_left_view_flag", buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
    printf("top_field_is_left_view_flag = %d\n", top_field_is_left_view_flag);
#endif
  }
  else
  {
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
    current_frame_is_left_view_flag     = 
#endif
            u_1("SEI: current_frame_is_left_view_flag", buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
    next_frame_is_second_view_flag      = 
#endif
            u_1("SEI: next_frame_is_second_view_flag", buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
    printf("current_frame_is_left_view_flag = %d\n", current_frame_is_left_view_flag);
    printf("next_frame_is_second_view_flag = %d\n", next_frame_is_second_view_flag);
#endif
  }

#ifdef PRINT_STEREO_VIDEO_INFO_INFO
  left_view_self_contained_flag         = 
#endif
          u_1("SEI: left_view_self_contained_flag", buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
  right_view_self_contained_flag        = 
#endif
          u_1("SEI: right_view_self_contained_flag", buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
  printf("left_view_self_contained_flag = %d\n", left_view_self_contained_flag);
  printf("right_view_self_contained_flag = %d\n", right_view_self_contained_flag);
#endif

  free (buf);
#ifdef PRINT_STEREO_VIDEO_INFO_INFO
#undef PRINT_STEREO_VIDEO_INFO_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the Reserved SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_reserved_info( byte* payload, int size, VideoParameters *p_Vid )
{
  int offset = 0;
#ifdef PRINT_RESERVED_INFO
  byte payload_byte;
#endif
  UNREFERENCED_PARAMETER(payload);
  UNREFERENCED_PARAMETER(p_Vid);

#ifdef PRINT_RESERVED_INFO
  printf("Reserved SEI message\n");
#endif

  while (offset < size)
  {
#ifdef PRINT_RESERVED_INFO
    payload_byte = payload[offset];
#endif
    
    offset ++;
#ifdef PRINT_RESERVED_INFO
    printf("reserved_sei_message_payload_byte = %d\n", payload_byte);
#endif
  }
#ifdef PRINT_RESERVED_INFO
#undef PRINT_RESERVED_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Buffering period SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
#if EXT3D
void interpret_buffering_period_info( byte* payload, int size, VideoParameters *p_Vid ,int is_depth)
#else
void interpret_buffering_period_info( byte* payload, int size, VideoParameters *p_Vid )
#endif
{
  int seq_parameter_set_id;
#ifdef PRINT_BUFFERING_PERIOD_INFO
  int initial_cpb_removal_delay, initial_cpb_removal_delay_offset;
#endif
  unsigned int k;

  Bitstream* buf;
  seq_parameter_set_rbsp_t *sps;

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  seq_parameter_set_id   = ue_v("SEI: seq_parameter_set_id"  , buf);

  sps = &p_Vid->SeqParSet[seq_parameter_set_id];
#if EXT3D
  activate_sps(p_Vid, sps,is_depth);
#else
  activate_sps(p_Vid, sps);
#endif
#ifdef PRINT_BUFFERING_PERIOD_INFO
  printf("Buffering period SEI message\n");
  printf("seq_parameter_set_id   = %d\n", seq_parameter_set_id);
#endif

  // Note: NalHrdBpPresentFlag and CpbDpbDelaysPresentFlag can also be set "by some means not specified in this Recommendation | International Standard"
  if (sps->vui_parameters_present_flag)
  {

    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
    {
      for (k=0; k<sps->vui_seq_parameters.nal_hrd_parameters.cpb_cnt_minus1+1; k++)
      {
#ifdef PRINT_BUFFERING_PERIOD_INFO
        initial_cpb_removal_delay        = 
#endif
                u_v(sps->vui_seq_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay"        , buf);
#ifdef PRINT_BUFFERING_PERIOD_INFO
        initial_cpb_removal_delay_offset = 
#endif
                u_v(sps->vui_seq_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay_offset" , buf);

#ifdef PRINT_BUFFERING_PERIOD_INFO
        printf("nal initial_cpb_removal_delay[%d]        = %d\n", k, initial_cpb_removal_delay);
        printf("nal initial_cpb_removal_delay_offset[%d] = %d\n", k, initial_cpb_removal_delay_offset);
#endif
      }
    }

    if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      for (k=0; k<sps->vui_seq_parameters.vcl_hrd_parameters.cpb_cnt_minus1+1; k++)
      {
#ifdef PRINT_BUFFERING_PERIOD_INFO
        initial_cpb_removal_delay        = 
#endif
                u_v(sps->vui_seq_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay"        , buf);
#ifdef PRINT_BUFFERING_PERIOD_INFO
        initial_cpb_removal_delay_offset = 
#endif
                u_v(sps->vui_seq_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length_minus1+1, "SEI: initial_cpb_removal_delay_offset" , buf);

#ifdef PRINT_BUFFERING_PERIOD_INFO
        printf("vcl initial_cpb_removal_delay[%d]        = %d\n", k, initial_cpb_removal_delay);
        printf("vcl initial_cpb_removal_delay_offset[%d] = %d\n", k, initial_cpb_removal_delay_offset);
#endif
      }
    }
  }

  free (buf);
#ifdef PRINT_BUFFERING_PERIOD_INFO
#undef PRINT_BUFFERING_PERIOD_INFO
#endif
}


/*!
 ************************************************************************
 *  \brief
 *     Interpret the Picture timing SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
void interpret_picture_timing_info( byte* payload, int size, VideoParameters *p_Vid )
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  int picture_structure_present_flag, picture_structure;
  int clock_time_stamp_flag;
  int full_timestamp_flag = 0;
  int seconds_flag, minutes_flag, hours_flag;
#ifdef PRINT_PCITURE_TIMING_INFO
  int cpb_removal_delay, dpb_output_delay;
  int ct_type, nuit_field_based_flag, counting_type, discontinuity_flag, cnt_dropped_flag, nframes;
  int seconds_value, minutes_value, hours_value, time_offset;
#endif
  int NumClockTs = 0;
  int i;

  int cpb_removal_len = 24;
  int dpb_output_len  = 24;

  Boolean CpbDpbDelaysPresentFlag;

  Bitstream* buf;

  if (NULL==active_sps)
  {
    fprintf (stderr, "Warning: no active SPS, timing SEI cannot be parsed\n");
    return;
  }

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;


#ifdef PRINT_PCITURE_TIMING_INFO
  printf("Picture timing SEI message\n");
#endif

  // CpbDpbDelaysPresentFlag can also be set "by some means not specified in this Recommendation | International Standard"
  CpbDpbDelaysPresentFlag =  (Boolean) (active_sps->vui_parameters_present_flag
                              && (   (active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag != 0)
                                   ||(active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag != 0)));

  if (CpbDpbDelaysPresentFlag )
  {
    if (active_sps->vui_parameters_present_flag)
    {
      if (active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
      {
        cpb_removal_len = active_sps->vui_seq_parameters.nal_hrd_parameters.cpb_removal_delay_length_minus1 + 1;
        dpb_output_len  = active_sps->vui_seq_parameters.nal_hrd_parameters.dpb_output_delay_length_minus1  + 1;
      }
      else if (active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
      {
        cpb_removal_len = active_sps->vui_seq_parameters.vcl_hrd_parameters.cpb_removal_delay_length_minus1 + 1;
        dpb_output_len  = active_sps->vui_seq_parameters.vcl_hrd_parameters.dpb_output_delay_length_minus1  + 1;
      }
    }

    if ((active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag)||
      (active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag))
    {
#ifdef PRINT_PCITURE_TIMING_INFO
      cpb_removal_delay = 
#endif
              u_v(cpb_removal_len, "SEI: cpb_removal_delay" , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
      dpb_output_delay  = 
#endif
              u_v(dpb_output_len,  "SEI: dpb_output_delay"  , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
      printf("cpb_removal_delay = %d\n",cpb_removal_delay);
      printf("dpb_output_delay  = %d\n",dpb_output_delay);
#endif
    }
  }

  if (!active_sps->vui_parameters_present_flag)
  {
    picture_structure_present_flag = 0;
  }
  else
  {
    picture_structure_present_flag  =  active_sps->vui_seq_parameters.pic_struct_present_flag;
  }

  if (picture_structure_present_flag)
  {
    picture_structure = u_v(4, "SEI: pic_struct" , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
    printf("picture_structure = %d\n",picture_structure);
#endif
    switch (picture_structure)
    {
    case 0:
    case 1:
    case 2:
      NumClockTs = 1;
      break;
    case 3:
    case 4:
    case 7:
      NumClockTs = 2;
      break;
    case 5:
    case 6:
    case 8:
      NumClockTs = 3;
      break;
    default:
      error("reserved picture_structure used (can't determine NumClockTs)", 500);
    }
    for (i=0; i<NumClockTs; i++)
    {
      clock_time_stamp_flag = u_1("SEI: clock_time_stamp_flag"  , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
      printf("clock_time_stamp_flag = %d\n",clock_time_stamp_flag);
#endif
      if (clock_time_stamp_flag)
      {
#ifdef PRINT_PCITURE_TIMING_INFO
        ct_type               = 
#endif
                u_v(2, "SEI: ct_type"               , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
        nuit_field_based_flag = 
#endif
                u_1(   "SEI: nuit_field_based_flag" , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
        counting_type         = 
#endif
                u_v(5, "SEI: counting_type"         , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
        full_timestamp_flag   = 
#endif
                u_1(   "SEI: full_timestamp_flag"   , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
        discontinuity_flag    = 
#endif
                u_1(   "SEI: discontinuity_flag"    , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
        cnt_dropped_flag      = 
#endif
                u_1(   "SEI: cnt_dropped_flag"      , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
        nframes               = 
#endif
                u_v(8, "SEI: nframes"               , buf);

#ifdef PRINT_PCITURE_TIMING_INFO
        printf("ct_type               = %d\n",ct_type);
        printf("nuit_field_based_flag = %d\n",nuit_field_based_flag);
        printf("full_timestamp_flag   = %d\n",full_timestamp_flag);
        printf("discontinuity_flag    = %d\n",discontinuity_flag);
        printf("cnt_dropped_flag      = %d\n",cnt_dropped_flag);
        printf("nframes               = %d\n",nframes);
#endif
        if (full_timestamp_flag)
        {
#ifdef PRINT_PCITURE_TIMING_INFO
          seconds_value         = 
#endif
                  u_v(6, "SEI: seconds_value"   , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
          minutes_value         = 
#endif
                  u_v(6, "SEI: minutes_value"   , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
          hours_value           = 
#endif
                  u_v(5, "SEI: hours_value"     , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
          printf("seconds_value = %d\n",seconds_value);
          printf("minutes_value = %d\n",minutes_value);
          printf("hours_value   = %d\n",hours_value);
#endif
        }
        else
        {
          seconds_flag          = u_1(   "SEI: seconds_flag" , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
          printf("seconds_flag = %d\n",seconds_flag);
#endif
          if (seconds_flag)
          {
#ifdef PRINT_PCITURE_TIMING_INFO
            seconds_value         = 
#endif
                    u_v(6, "SEI: seconds_value"   , buf);
            minutes_flag          = u_1(   "SEI: minutes_flag" , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
            printf("seconds_value = %d\n",seconds_value);
            printf("minutes_flag  = %d\n",minutes_flag);
#endif
            if(minutes_flag)
            {
#ifdef PRINT_PCITURE_TIMING_INFO
              minutes_value         = 
#endif
                      u_v(6, "SEI: minutes_value"   , buf);
              hours_flag            = u_1(   "SEI: hours_flag" , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
              printf("minutes_value = %d\n",minutes_value);
              printf("hours_flag    = %d\n",hours_flag);
#endif
              if(hours_flag)
              {
#ifdef PRINT_PCITURE_TIMING_INFO
                hours_value           = 
#endif
                        u_v(5, "SEI: hours_value"     , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
                printf("hours_value   = %d\n",hours_value);
#endif
              }
            }
          }
        }
        {
          int time_offset_length;
          if (active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
            time_offset_length = active_sps->vui_seq_parameters.vcl_hrd_parameters.time_offset_length;
          else if (active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
            time_offset_length = active_sps->vui_seq_parameters.nal_hrd_parameters.time_offset_length;
          else
            time_offset_length = 24;
          if (time_offset_length)
#ifdef PRINT_PCITURE_TIMING_INFO
            time_offset = 
#endif
                    i_v(time_offset_length, "SEI: time_offset"   , buf);
#ifdef PRINT_PCITURE_TIMING_INFO
          else
            time_offset = 0;
#endif
#ifdef PRINT_PCITURE_TIMING_INFO
          printf("time_offset   = %d\n",time_offset);
#endif
        }
      }
    }
  }

  free (buf);
#ifdef PRINT_PCITURE_TIMING_INFO
#undef PRINT_PCITURE_TIMING_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the Frame Packing Arrangement SEI message
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 ************************************************************************
 */
void interpret_frame_packing_arrangement_info( byte* payload, int size, VideoParameters *p_Vid )
{
  frame_packing_arrangement_information_struct seiFramePackingArrangement;
  Bitstream* buf;
  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
  printf("Frame packing arrangement SEI message\n");
#endif

  seiFramePackingArrangement.frame_packing_arrangement_id = (unsigned int)ue_v( "SEI: frame_packing_arrangement_id", buf );
  seiFramePackingArrangement.frame_packing_arrangement_cancel_flag = u_1( "SEI: frame_packing_arrangement_cancel_flag", buf );
#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
  printf("frame_packing_arrangement_id                 = %d\n", seiFramePackingArrangement.frame_packing_arrangement_id);
  printf("frame_packing_arrangement_cancel_flag        = %d\n", seiFramePackingArrangement.frame_packing_arrangement_cancel_flag);
#endif
  if ( seiFramePackingArrangement.frame_packing_arrangement_cancel_flag == FALSE )
  {
    seiFramePackingArrangement.frame_packing_arrangement_type = (unsigned char)u_v( 7, "SEI: frame_packing_arrangement_type", buf );
    seiFramePackingArrangement.quincunx_sampling_flag         = u_1( "SEI: quincunx_sampling_flag", buf );
    seiFramePackingArrangement.content_interpretation_type    = (unsigned char)u_v( 6, "SEI: content_interpretation_type", buf );
    seiFramePackingArrangement.spatial_flipping_flag          = u_1( "SEI: spatial_flipping_flag", buf );
    seiFramePackingArrangement.frame0_flipped_flag            = u_1( "SEI: frame0_flipped_flag", buf );
    seiFramePackingArrangement.field_views_flag               = u_1( "SEI: field_views_flag", buf );
    seiFramePackingArrangement.current_frame_is_frame0_flag   = u_1( "SEI: current_frame_is_frame0_flag", buf );
    seiFramePackingArrangement.frame0_self_contained_flag     = u_1( "SEI: frame0_self_contained_flag", buf );
    seiFramePackingArrangement.frame1_self_contained_flag     = u_1( "SEI: frame1_self_contained_flag", buf );
#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
    printf("frame_packing_arrangement_type    = %d\n", seiFramePackingArrangement.frame_packing_arrangement_type);
    printf("quincunx_sampling_flag            = %d\n", seiFramePackingArrangement.quincunx_sampling_flag);
    printf("content_interpretation_type       = %d\n", seiFramePackingArrangement.content_interpretation_type);
    printf("spatial_flipping_flag             = %d\n", seiFramePackingArrangement.spatial_flipping_flag);
    printf("frame0_flipped_flag               = %d\n", seiFramePackingArrangement.frame0_flipped_flag);
    printf("field_views_flag                  = %d\n", seiFramePackingArrangement.field_views_flag);
    printf("current_frame_is_frame0_flag      = %d\n", seiFramePackingArrangement.current_frame_is_frame0_flag);
    printf("frame0_self_contained_flag        = %d\n", seiFramePackingArrangement.frame0_self_contained_flag);
    printf("frame1_self_contained_flag        = %d\n", seiFramePackingArrangement.frame1_self_contained_flag);
#endif
    if ( seiFramePackingArrangement.quincunx_sampling_flag == FALSE && seiFramePackingArrangement.frame_packing_arrangement_type != 5 )
    {
      seiFramePackingArrangement.frame0_grid_position_x = (unsigned char)u_v( 4, "SEI: frame0_grid_position_x", buf );
      seiFramePackingArrangement.frame0_grid_position_y = (unsigned char)u_v( 4, "SEI: frame0_grid_position_y", buf );
      seiFramePackingArrangement.frame1_grid_position_x = (unsigned char)u_v( 4, "SEI: frame1_grid_position_x", buf );
      seiFramePackingArrangement.frame1_grid_position_y = (unsigned char)u_v( 4, "SEI: frame1_grid_position_y", buf );
#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
      printf("frame0_grid_position_x      = %d\n", seiFramePackingArrangement.frame0_grid_position_x);
      printf("frame0_grid_position_y      = %d\n", seiFramePackingArrangement.frame0_grid_position_y);
      printf("frame1_grid_position_x      = %d\n", seiFramePackingArrangement.frame1_grid_position_x);
      printf("frame1_grid_position_y      = %d\n", seiFramePackingArrangement.frame1_grid_position_y);
#endif
    }
    seiFramePackingArrangement.frame_packing_arrangement_reserved_byte = (unsigned char)u_v( 8, "SEI: frame_packing_arrangement_reserved_byte", buf );
    seiFramePackingArrangement.frame_packing_arrangement_repetition_period = (unsigned int)ue_v( "SEI: frame_packing_arrangement_repetition_period", buf );
#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
    printf("frame_packing_arrangement_reserved_byte          = %d\n", seiFramePackingArrangement.frame_packing_arrangement_reserved_byte);
    printf("frame_packing_arrangement_repetition_period      = %d\n", seiFramePackingArrangement.frame_packing_arrangement_repetition_period);
#endif
  }
  seiFramePackingArrangement.frame_packing_arrangement_extension_flag = u_1( "SEI: frame_packing_arrangement_extension_flag", buf );
#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
  printf("frame_packing_arrangement_extension_flag          = %d\n", seiFramePackingArrangement.frame_packing_arrangement_extension_flag);
#endif

  free (buf);
#ifdef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
#undef PRINT_FRAME_PACKING_ARRANGEMENT_INFO
#endif
}

/*!
 ************************************************************************
 *  \brief
 *     Interpret the HDR tone-mapping SEI message (JVT-T060)
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *
 ************************************************************************
 */
typedef struct
{
  unsigned int  tone_map_id;
  unsigned char tone_map_cancel_flag;
  unsigned int  tone_map_repetition_period;
  unsigned char coded_data_bit_depth;
  unsigned char sei_bit_depth;
  unsigned int  model_id;
  // variables for model 0
  int  min_value;
  int  max_value;
  // variables for model 1
  int  sigmoid_midpoint;
  int  sigmoid_width;
  // variables for model 2
  int start_of_coded_interval[1<<MAX_SEI_BIT_DEPTH];
  // variables for model 3
  int num_pivots;
  int coded_pivot_value[MAX_NUM_PIVOTS];
  int sei_pivot_value[MAX_NUM_PIVOTS];
} tone_mapping_struct_tmp;

void interpret_tone_mapping( byte* payload, int size, VideoParameters *p_Vid )
{
  tone_mapping_struct_tmp seiToneMappingTmp;
  Bitstream* buf;
  int i = 0, max_coded_num, max_output_num;

  memset (&seiToneMappingTmp, 0, sizeof (tone_mapping_struct_tmp));

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  seiToneMappingTmp.tone_map_id = ue_v("SEI: tone_map_id", buf);
  seiToneMappingTmp.tone_map_cancel_flag = (unsigned char) u_1("SEI: tone_map_cancel_flag", buf);

#ifdef PRINT_TONE_MAPPING
  printf("Tone-mapping SEI message\n");
  printf("tone_map_id = %d\n", seiToneMappingTmp.tone_map_id);

  if (seiToneMappingTmp.tone_map_id != 0)
    printf("WARNING! Tone_map_id != 0, print the SEI message info only. The tone mapping is actually applied only when Tone_map_id==0\n\n");
  printf("tone_map_cancel_flag = %d\n", seiToneMappingTmp.tone_map_cancel_flag);
#endif

  if (!seiToneMappingTmp.tone_map_cancel_flag) 
  {
    seiToneMappingTmp.tone_map_repetition_period  = ue_v(  "SEI: tone_map_repetition_period", buf);
    seiToneMappingTmp.coded_data_bit_depth        = (unsigned char)u_v (8,"SEI: coded_data_bit_depth"      , buf);
    seiToneMappingTmp.sei_bit_depth               = (unsigned char)u_v (8,"SEI: sei_bit_depth"             , buf);
    seiToneMappingTmp.model_id                    = ue_v(  "SEI: model_id"                  , buf);

#ifdef PRINT_TONE_MAPPING
    printf("tone_map_repetition_period = %d\n", seiToneMappingTmp.tone_map_repetition_period);
    printf("coded_data_bit_depth = %d\n", seiToneMappingTmp.coded_data_bit_depth);
    printf("sei_bit_depth = %d\n", seiToneMappingTmp.sei_bit_depth);
    printf("model_id = %d\n", seiToneMappingTmp.model_id);
#endif

    max_coded_num  = 1<<seiToneMappingTmp.coded_data_bit_depth;
    max_output_num = 1<<seiToneMappingTmp.sei_bit_depth;

    if (seiToneMappingTmp.model_id == 0) 
    { // linear mapping with clipping
      seiToneMappingTmp.min_value   = u_v (32,"SEI: min_value", buf);
      seiToneMappingTmp.max_value   = u_v (32,"SEI: min_value", buf);
#ifdef PRINT_TONE_MAPPING
      printf("min_value = %d, max_value = %d\n", seiToneMappingTmp.min_value, seiToneMappingTmp.max_value);
#endif
    }
    else if (seiToneMappingTmp.model_id == 1) 
    { // sigmoidal mapping
      seiToneMappingTmp.sigmoid_midpoint = u_v (32,"SEI: sigmoid_midpoint", buf);
      seiToneMappingTmp.sigmoid_width    = u_v (32,"SEI: sigmoid_width", buf);
#ifdef PRINT_TONE_MAPPING
      printf("sigmoid_midpoint = %d, sigmoid_width = %d\n", seiToneMappingTmp.sigmoid_midpoint, seiToneMappingTmp.sigmoid_width);
#endif
    }
    else if (seiToneMappingTmp.model_id == 2) 
    { // user defined table mapping
      for (i=0; i<max_output_num; i++) 
      {
        seiToneMappingTmp.start_of_coded_interval[i] = u_v((((seiToneMappingTmp.coded_data_bit_depth+7)>>3)<<3), "SEI: start_of_coded_interval"  , buf);
#ifdef PRINT_TONE_MAPPING // too long to print
        //printf("start_of_coded_interval[%d] = %d\n", i, seiToneMappingTmp.start_of_coded_interval[i]);
#endif
      }
    }
    else if (seiToneMappingTmp.model_id == 3) 
    {  // piece-wise linear mapping
      seiToneMappingTmp.num_pivots = u_v (16,"SEI: num_pivots", buf);
#ifdef PRINT_TONE_MAPPING
      printf("num_pivots = %d\n", seiToneMappingTmp.num_pivots);
#endif
      seiToneMappingTmp.coded_pivot_value[0] = 0;
      seiToneMappingTmp.sei_pivot_value[0] = 0;
      seiToneMappingTmp.coded_pivot_value[seiToneMappingTmp.num_pivots+1] = max_coded_num-1;
      seiToneMappingTmp.sei_pivot_value[seiToneMappingTmp.num_pivots+1] = max_output_num-1;

      for (i=1; i < seiToneMappingTmp.num_pivots+1; i++) 
      {
        seiToneMappingTmp.coded_pivot_value[i] = u_v( (((seiToneMappingTmp.coded_data_bit_depth+7)>>3)<<3), "SEI: coded_pivot_value", buf);
        seiToneMappingTmp.sei_pivot_value[i] = u_v( (((seiToneMappingTmp.sei_bit_depth+7)>>3)<<3), "SEI: sei_pivot_value", buf);
#ifdef PRINT_TONE_MAPPING
        printf("coded_pivot_value[%d] = %d, sei_pivot_value[%d] = %d\n", i, seiToneMappingTmp.coded_pivot_value[i], i, seiToneMappingTmp.sei_pivot_value[i]);
#endif
      }
    }

#if (ENABLE_OUTPUT_TONEMAPPING)
    // Currently, only when the map_id == 0, the tone-mapping is actually applied.
    if (seiToneMappingTmp.tone_map_id== 0) 
    {
      int j;
      p_Vid->seiToneMapping->seiHasTone_mapping = TRUE;
      p_Vid->seiToneMapping->tone_map_repetition_period = seiToneMappingTmp.tone_map_repetition_period;
      p_Vid->seiToneMapping->coded_data_bit_depth = seiToneMappingTmp.coded_data_bit_depth;
      p_Vid->seiToneMapping->sei_bit_depth = seiToneMappingTmp.sei_bit_depth;
      p_Vid->seiToneMapping->model_id = seiToneMappingTmp.model_id;
      p_Vid->seiToneMapping->count = 0;

      // generate the look up table of tone mapping
      switch(seiToneMappingTmp.model_id)
      {
      case 0:            // linear mapping with clipping
        for (i=0; i<=seiToneMappingTmp.min_value; i++)
          p_Vid->seiToneMapping->lut[i] = 0;

        for (i=seiToneMappingTmp.min_value+1; i < seiToneMappingTmp.max_value; i++)
          p_Vid->seiToneMapping->lut[i] = (imgpel) ((i-seiToneMappingTmp.min_value) * (max_output_num-1)/(seiToneMappingTmp.max_value- seiToneMappingTmp.min_value));

        for (i=seiToneMappingTmp.max_value; i<max_coded_num; i++)
          p_Vid->seiToneMapping->lut[i] = (imgpel) (max_output_num - 1);
        break;
      case 1: // sigmoid mapping

        for (i=0; i < max_coded_num; i++) 
        {
          double tmp = 1.0 + exp( -6*(double)(i-seiToneMappingTmp.sigmoid_midpoint)/seiToneMappingTmp.sigmoid_width);
          p_Vid->seiToneMapping->lut[i] = (imgpel)( (double)(max_output_num-1)/ tmp + 0.5);
        }
        break;
      case 2: // user defined table
        if (0 < max_output_num-1)
        {
          for (j=0; j<max_output_num-1; j++) 
          {
            for (i=seiToneMappingTmp.start_of_coded_interval[j]; i<seiToneMappingTmp.start_of_coded_interval[j+1]; i++) 
            {
              p_Vid->seiToneMapping->lut[i] = (imgpel) j;
            }
          }
          p_Vid->seiToneMapping->lut[i] = (imgpel) (max_output_num - 1);
        }
        break;
      case 3: // piecewise linear mapping
        for (j=0; j<seiToneMappingTmp.num_pivots+1; j++) 
        {
          double slope = (double)(seiToneMappingTmp.sei_pivot_value[j+1] - seiToneMappingTmp.sei_pivot_value[j])/(seiToneMappingTmp.coded_pivot_value[j+1]-seiToneMappingTmp.coded_pivot_value[j]);
          for (i=seiToneMappingTmp.coded_pivot_value[j]; i <= seiToneMappingTmp.coded_pivot_value[j+1]; i++) 
          {
            p_Vid->seiToneMapping->lut[i] = (imgpel) (seiToneMappingTmp.sei_pivot_value[j] + (int)(( (i - seiToneMappingTmp.coded_pivot_value[j]) * slope)));
          }
        }
        break;

      default:
        break;
      } // end switch
    }
#endif
  } // end !tone_map_cancel_flag
  free (buf);
}

#if (ENABLE_OUTPUT_TONEMAPPING)
// tone map using the look-up-table generated according to SEI tone mapping message
void tone_map (imgpel** imgX, imgpel* lut, int size_x, int size_y)
{
  int i, j;

  for(i=0;i<size_y;i++)
  {
    for(j=0;j<size_x;j++)
    {
      imgX[i][j] = (imgpel)lut[imgX[i][j]];
    }
  }
}

void init_tone_mapping_sei(ToneMappingSEI *seiToneMapping) 
{
  seiToneMapping->seiHasTone_mapping = FALSE;
  seiToneMapping->count = 0;
}

void update_tone_mapping_sei(ToneMappingSEI *seiToneMapping) 
{

  if(seiToneMapping->tone_map_repetition_period == 0)
  {
    seiToneMapping->seiHasTone_mapping = FALSE;
    seiToneMapping->count = 0;
  }
  else if (seiToneMapping->tone_map_repetition_period>1)
  {
    seiToneMapping->count++;
    if (seiToneMapping->count>=seiToneMapping->tone_map_repetition_period) 
    {
      seiToneMapping->seiHasTone_mapping = FALSE;
      seiToneMapping->count = 0;
    }
  }
}
#endif

/*!
 ************************************************************************
 *  \brief
 *     Interpret the post filter hints SEI message (JVT-U035)
 *  \param payload
 *     a pointer that point to the sei payload
 *  \param size
 *     the size of the sei message
 *  \param p_Vid
 *     the image pointer
 *    
 ************************************************************************
 */
void interpret_post_filter_hints_info( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  unsigned int filter_hint_size_y, filter_hint_size_x, color_component, cx, cy;
#ifdef PRINT_POST_FILTER_HINT_INFO
  unsigned int filter_hint_type, additional_extension_flag;
#endif
  int ***filter_hint;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  filter_hint_size_y = ue_v("SEI: filter_hint_size_y", buf); // interpret post-filter hint SEI here
  filter_hint_size_x = ue_v("SEI: filter_hint_size_x", buf); // interpret post-filter hint SEI here
#ifdef PRINT_POST_FILTER_HINT_INFO
  filter_hint_type   = 
#endif
          u_v(2, "SEI: filter_hint_type", buf); // interpret post-filter hint SEI here

  get_mem3Dint (&filter_hint, 3, filter_hint_size_y, filter_hint_size_x);

  for (color_component = 0; color_component < 3; color_component ++)
    for (cy = 0; cy < filter_hint_size_y; cy ++)
      for (cx = 0; cx < filter_hint_size_x; cx ++)
        filter_hint[color_component][cy][cx] = se_v("SEI: filter_hint", buf); // interpret post-filter hint SEI here

#ifdef PRINT_POST_FILTER_HINT_INFO
  additional_extension_flag = 
#endif
          u_1("SEI: additional_extension_flag", buf); // interpret post-filter hint SEI here

#ifdef PRINT_POST_FILTER_HINT_INFO
  printf(" Post-filter hint SEI message\n");
  printf(" post_filter_hint_size_y %d \n", filter_hint_size_y);
  printf(" post_filter_hint_size_x %d \n", filter_hint_size_x);
  printf(" post_filter_hint_type %d \n",   filter_hint_type);
  for (color_component = 0; color_component < 3; color_component ++)
    for (cy = 0; cy < filter_hint_size_y; cy ++)
      for (cx = 0; cx < filter_hint_size_x; cx ++)
        printf(" post_filter_hint[%d][%d][%d] %d \n", color_component, cy, cx, filter_hint[color_component][cy][cx]);

  printf(" additional_extension_flag %d \n", additional_extension_flag);

#undef PRINT_POST_FILTER_HINT_INFO
#endif

  free_mem3Dint (filter_hint);
  free( buf );
}

#if EXT3D // NOKIA_DEPTH_SEI_C0162

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

static void decode_depth_acquisition_sei_element( int voidx, ThreeDVAESEI* curr_3dv_ae, Bitstream* buf){
  //int i;
  int prec=0;
  int manLen=0;
  int expLen = curr_3dv_ae->exponent_size;
  int precMode = curr_3dv_ae->pred_mode;

  curr_3dv_ae->element.sign     = u_1( "SEI: sing0", buf);
  curr_3dv_ae->element.exponent = u_v( expLen, "SEI: exponent0", buf);
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
  printf("expLen %d\n",expLen);
  printf("sign0                    = %d\n",curr_3dv_ae->element.sign);
  printf("exponent0                = %d\n",curr_3dv_ae->element.exponent);
#endif
  if(precMode==0){
    manLen = u_v(5, "SEI: mantissa_len_minus1", buf);
    manLen += 1;
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
    printf("mantissa_len_minus1      = %d\n",manLen-1);
#endif
  }else{
    prec = curr_3dv_ae->precision;
    if(curr_3dv_ae->element.exponent==0){
      manLen = max(0, prec-30);
    }else{
      manLen = max(0, curr_3dv_ae->element.exponent+prec-31);
    }
  }
  curr_3dv_ae->element.mantissa = u_v(manLen, "SEI: mantissa0", buf);
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
  printf("mantissa0                = %d\n",curr_3dv_ae->element.mantissa);
#endif
  curr_3dv_ae->mantissa_length = manLen;

  get_rec_double_type_SEI(curr_3dv_ae);
  printf("view%d %lf\n",voidx,curr_3dv_ae->rec);
}

void interpret_depth_representation_info                    ( byte* payload, int size, VideoParameters *p_Vid )
{

  Bitstream* buf;
  int i, k;
  depth_representation_info seiDepthRepresentationInfo;

  seiDepthRepresentationInfo.depth_acquisition_update_info_sei=(DepthAcquisitionInfoSEI*)calloc(MAX_CODEVIEW,sizeof(DepthAcquisitionInfoSEI));
  if(seiDepthRepresentationInfo.depth_acquisition_update_info_sei==NULL)
    no_mem_exit("seiDepthRepresentationInfo.depth_acquisition_update_info_sei");

  for(i=0;i<MAX_CODEVIEW;i++)
  {
    init_depth_acquisition_info_SEI(&seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i]);
  }

  buf = (Bitstream*)malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  seiDepthRepresentationInfo.all_views_equal_flag          = u_1( "SEI: all_views_equal_flag", buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
  printf("all_views_equal_flag     = %d\n",seiDepthRepresentationInfo.all_views_equal_flag );
#endif
  if(seiDepthRepresentationInfo.all_views_equal_flag==0){
    seiDepthRepresentationInfo.num_views_minus1            = ue_v( "SEI: num_views_minus1", buf);
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
    printf("num_views_minus1         = %d\n",seiDepthRepresentationInfo.num_views_minus1     );
#endif
  }else{
    seiDepthRepresentationInfo.num_views_minus1            = 0;
  }

  seiDepthRepresentationInfo.z_near_flag              = u_1( "SEI: z_near_flag"            , buf );
  seiDepthRepresentationInfo.z_far_flag               = u_1( "SEI: z_far_flag"             , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
  printf("z_near_flag              = %d\n",seiDepthRepresentationInfo.z_near_flag           );
  printf("z_far_flag               = %d\n",seiDepthRepresentationInfo.z_far_flag            );
#endif

  seiDepthRepresentationInfo.z_axis_equal_flag = 1;
  if (seiDepthRepresentationInfo.z_near_flag || seiDepthRepresentationInfo.z_far_flag)
  {
    seiDepthRepresentationInfo.z_axis_equal_flag      = u_1 ( "SEI: z_axis_equal_flag"     , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
    printf("z_axis_equal_flag      = %d\n",seiDepthRepresentationInfo.z_axis_equal_flag            );
#endif
    if (seiDepthRepresentationInfo.z_axis_equal_flag)
    {
      seiDepthRepresentationInfo.common_z_axis_reference_view = ue_v( "SEI: common_z_axis_reference_view" , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
      printf("common_z_axis_reference_view      = %d\n",seiDepthRepresentationInfo.common_z_axis_reference_view            );
#endif
    }
  }

  seiDepthRepresentationInfo.d_min_flag               = u_1( "SEI: d_min_flag"             , buf );
  seiDepthRepresentationInfo.d_max_flag               = u_1( "SEI: d_max_flag"             , buf );
  seiDepthRepresentationInfo.depth_representation_type = ue_v( "SEI: depth_representation_type" , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
  printf("d_min_flag               = %d\n",seiDepthRepresentationInfo.d_min_flag            );
  printf("d_max_flag               = %d\n",seiDepthRepresentationInfo.d_max_flag            );
  printf("depth_representation_type      = %d\n",seiDepthRepresentationInfo.depth_representation_type  );
#endif

  for(i=0;i<seiDepthRepresentationInfo.num_views_minus1+1;i++){
    seiDepthRepresentationInfo.depth_info_view_id[i]=ue_v( "SEI: depth_info_view_id"    , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
    printf("depth_info_view_id      = %d\n",seiDepthRepresentationInfo.depth_info_view_id[i]  );
#endif
    if ((seiDepthRepresentationInfo.z_near_flag || seiDepthRepresentationInfo.z_far_flag) && seiDepthRepresentationInfo.z_axis_equal_flag == 0)
    {
      seiDepthRepresentationInfo.z_axis_reference_view[i] = ue_v( "SEI: z_axis_reference_view" , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
      printf("z_axis_reference_view      = %d\n",seiDepthRepresentationInfo.z_axis_reference_view[i]  );
#endif
    }
    if (seiDepthRepresentationInfo.d_min_flag || seiDepthRepresentationInfo.d_max_flag)
    {
      seiDepthRepresentationInfo.disparity_reference_view[i] = ue_v( "SEI: disparity_reference_view" , buf );
#ifdef PRINT_DEPTH_REPRESENTATION_SEI
      printf("disparity_reference_view      = %d\n",seiDepthRepresentationInfo.disparity_reference_view[i]  );
#endif
    }

    if(seiDepthRepresentationInfo.z_near_flag){
      printf("z_near\n");
      decode_depth_acquisition_sei_element( i, &seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].depth_near_ae, buf);
    }
    if(seiDepthRepresentationInfo.z_far_flag){
      printf("z_far\n");
      decode_depth_acquisition_sei_element( i, &seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].depth_far_ae, buf);
    }
    if(seiDepthRepresentationInfo.d_min_flag){
      printf("d_min\n");
      decode_depth_acquisition_sei_element( i, &seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].d_min_ae, buf);
    }
    if(seiDepthRepresentationInfo.d_max_flag){
      printf("d_max\n");
      decode_depth_acquisition_sei_element( i, &seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].d_max_ae, buf);
    }
  }

  if ( seiDepthRepresentationInfo.depth_representation_type == 3 ) 
  {  // actual NDR SEI coding:
    p_Vid->SEI_NonlinearDepthNum[0]=ue_v("SEI: depth_nonlinear_representation_num_minus1", buf);
    p_Vid->SEI_NonlinearDepthNum[0] ++;
    p_Vid->SEI_NonlinearDepthPoints[0][0] = 0;
    p_Vid->SEI_NonlinearDepthPoints[0][p_Vid->SEI_NonlinearDepthNum[0]+1] = 0;

    for (k=1; k<=p_Vid->SEI_NonlinearDepthNum[0]; ++k)
    {
      p_Vid->SEI_NonlinearDepthPoints[0][k] = (char)ue_v("SEI: depth_nonlinear_representation_model", buf);
    }
  }
  for(i=1;i<MAX_CODEVIEW;i++)
  {
    p_Vid->SEI_NonlinearDepthNum[i] = p_Vid->SEI_NonlinearDepthNum[0];
    memcpy(p_Vid->SEI_NonlinearDepthPoints[i], p_Vid->SEI_NonlinearDepthPoints[0], 256*sizeof(char));
  };

  if(seiDepthRepresentationInfo.depth_acquisition_update_info_sei)
  {
    free(seiDepthRepresentationInfo.depth_acquisition_update_info_sei);
  }

  free (buf);
}

void interpret_3dvc_scalable_nesting                    ( byte* payload, int size, VideoParameters *p_Vid )
{

  Bitstream* buf;
  int i;
  threedvc_scalable_nesting sei3DVCScalableNesting;

  UNREFERENCED_PARAMETER(p_Vid);

  buf = (Bitstream*)malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  sei3DVCScalableNesting.operation_point_flag = u_1 (   "SEI: operation_point_flag"         , buf   );

  if ( !sei3DVCScalableNesting.operation_point_flag )
  {
    sei3DVCScalableNesting.all_view_components_in_au_flag = u_1 (   "SEI: all_view_components_in_au_flag"         , buf   );
    if( !sei3DVCScalableNesting.all_view_components_in_au_flag )
    {
      sei3DVCScalableNesting.num_view_components_minus1 = ue_v (   "SEI: num_view_components_minus1"              , buf   );
      for( i = 0; i <= sei3DVCScalableNesting.num_view_components_minus1; i++ )
      {
        sei3DVCScalableNesting.sei_view_id[i] = ue_v (   "SEI: sei_view_id"         , buf   );
        sei3DVCScalableNesting.sei_view_applicability_flag[i] = u_1 (   "SEI: sei_view_applicability_flag"        , buf   );
      }
    }
  }
  else
  {
    sei3DVCScalableNesting.sei_op_texture_only_flag = u_1 (   "SEI: sei_op_texture_only_flag"                    , buf   );
    sei3DVCScalableNesting.num_view_components_op_minus1 = ue_v (   "SEI: num_view_components_op_minus1"         , buf   );
    for( i = 0; i <= sei3DVCScalableNesting.num_view_components_op_minus1; i++ )
    {
      sei3DVCScalableNesting.sei_op_view_id[i] = ue_v (   "SEI: sei_op_view_id"            , buf   );
    }
    sei3DVCScalableNesting.sei_op_temporal_id = ue_v (   "SEI: sei_op_temporal_id"         , buf   );
  }
  
  free (buf);
}

void interpret_multiview_acquisition_info                    ( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  int i, j, k, PrecFocalLength, PrecPrincipalPoint, PrecSkewFactor, PrecRotation, PrecTranslation, num_of_param_sets;
  multiview_acquisition_info seiMultiviewAcquisitionInfo;
  
  UNREFERENCED_PARAMETER(p_Vid);
  
  seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei=(MultiviewAcquisitionInfoSEI*)calloc(MAX_CODEVIEW,sizeof(MultiviewAcquisitionInfoSEI));
  if(seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei==NULL)
    no_mem_exit("seiDepthRepresentationInfo.depth_acquisition_update_info_sei");

  for(i=0;i<MAX_CODEVIEW;i++)
  {
    init_multiview_acquisition_info_SEI(&seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i]);
  }

  buf = (Bitstream*)malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;

  seiMultiviewAcquisitionInfo.num_views_minus1 = ue_v ( "SEI: num_views_minus1", buf );
  seiMultiviewAcquisitionInfo.intrinsic_param_flag = u_1 ( "SEI: intrinsic_param_flag", buf );
  seiMultiviewAcquisitionInfo.extrinsic_param_flag = u_1 ( "SEI: extrinsic_param_flag", buf );
#ifdef PRINT_MULTIVIEW_ACQUISITION_INFO
  printf("num_views_minus1     = %d\n", seiMultiviewAcquisitionInfo.num_views_minus1 );
  printf("intrinsic_param_flag     = %d\n", seiMultiviewAcquisitionInfo.intrinsic_param_flag );
  printf("extrinsic_param_flag     = %d\n", seiMultiviewAcquisitionInfo.extrinsic_param_flag );
#endif

  if (seiMultiviewAcquisitionInfo.intrinsic_param_flag)
  {
    seiMultiviewAcquisitionInfo.intrinsic_params_equal = u_1  ( "SEI: intrinsic_params_equal", buf );
    PrecFocalLength = ue_v ( "SEI: prec_focal_length", buf );
    PrecPrincipalPoint = ue_v ( "SEI: prec_principal_point", buf );
    PrecSkewFactor = ue_v ( "SEI: prec_skew_factor", buf );
#ifdef PRINT_MULTIVIEW_ACQUISITION_INFO
    printf("intrinsic_params_equal   = %d\n", seiMultiviewAcquisitionInfo.intrinsic_params_equal );
    printf("prec_focal_length        = %d\n", PrecFocalLength );
    printf("prec_principal_point     = %d\n", PrecPrincipalPoint );
    printf("prec_skew_factor         = %d\n", PrecSkewFactor );
#endif

    if (seiMultiviewAcquisitionInfo.intrinsic_params_equal)
      num_of_param_sets = 1;
    else
      num_of_param_sets = seiMultiviewAcquisitionInfo.num_views_minus1 + 1;

    for( i = 0; i < num_of_param_sets; i++ ) 
    {
      seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].focal_length_x_ae.precision = PrecFocalLength;
      seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].focal_length_y_ae.precision = PrecFocalLength;
      seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].principal_point_x_ae.precision = PrecPrincipalPoint;
      seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].principal_point_y_ae.precision = PrecPrincipalPoint;
      seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].skew_factor_ae.precision = PrecSkewFactor;

      printf("focal_length_x\n");
      decode_depth_acquisition_sei_element( i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].focal_length_x_ae, buf);
      printf("focal_length_y_ae\n");
      decode_depth_acquisition_sei_element( i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].focal_length_y_ae, buf);
      printf("principal_point_x_ae\n");
      decode_depth_acquisition_sei_element( i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].principal_point_x_ae, buf);
      printf("principal_point_y_ae\n");
      decode_depth_acquisition_sei_element( i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].principal_point_y_ae, buf);
      printf("skew_factor_ae\n");
      decode_depth_acquisition_sei_element( i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].skew_factor_ae, buf);
    }
  }

  if (seiMultiviewAcquisitionInfo.extrinsic_param_flag)
  {
    PrecRotation = ue_v ( "SEI: prec_rotation_param", buf);
    PrecTranslation = ue_v ( "SEI: prec_translation_param", buf);
#ifdef PRINT_MULTIVIEW_ACQUISITION_INFO
    printf("prec_rotation_param            = %d\n", PrecRotation );
    printf("prec_translation_param         = %d\n", PrecTranslation );
#endif
    for( i = 0; i <= seiMultiviewAcquisitionInfo.num_views_minus1; i++ ) 
    {
      for ( j = 0; j <= 2; j++) { /* row */
        for ( k = 0; k <= 2; k++) { /* column */
          seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].rotation_ae[j][k].precision = PrecRotation;
          decode_depth_acquisition_sei_element(i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].rotation_ae[j][k], buf);
        }
        seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].translation_ae[j].precision = PrecTranslation;
        decode_depth_acquisition_sei_element(i, &seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].translation_ae[j], buf);
      }
    }
  }

  if(!p_Vid->RefDispSEI)
    update_reference_display_info(p_Vid, &seiMultiviewAcquisitionInfo);
  p_Vid->RefDispSEI  = 0;
  for( i = 0; i <= seiMultiviewAcquisitionInfo.num_views_minus1; i++ ) 
  {
    for ( j = 0; j <= 2; j++) 
    {
      (*p_Vid).seiMultiviewAcquisition[i].translation_ae[j] = seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].translation_ae[j];
    }
  }

  if(seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei)
  {
    free(seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei);
  }
  free (buf);
}

static void decode_reference_display_sei_element(VideoParameters *p_Vid, ThreeDVAESEI* curr_3dv_ae, Bitstream* buf, FILE* RefDispFile){
  int prec=0;
  int manLen=0;
  int expLen = curr_3dv_ae->exponent_size;
  int precMode = curr_3dv_ae->pred_mode;

  curr_3dv_ae->element.sign     = 0;
  curr_3dv_ae->element.exponent = u_v( expLen, "SEI: exponent0", buf);

  if(precMode==0){
    manLen = u_v(5, "SEI: mantissa_len_minus1", buf);
    manLen += 1;
  }else{
    prec = curr_3dv_ae->precision;
    if(curr_3dv_ae->element.exponent==0){
      manLen = max(0, prec-30);
    }else{
      manLen = max(0, curr_3dv_ae->element.exponent+prec-31);
    }
  }
  curr_3dv_ae->element.mantissa = u_v(manLen, "SEI: mantissa0", buf);
  curr_3dv_ae->mantissa_length = manLen;

  get_rec_double_type_SEI(curr_3dv_ae);
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
  printf(" %.20g\n",curr_3dv_ae->rec);
#endif
  if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    fprintf(RefDispFile," %.20g\n",curr_3dv_ae->rec);
}

void interpret_reference_display_info( byte* payload, int size, VideoParameters *p_Vid )
{
  Bitstream* buf;
  int i, PrecRefBaseline, PrecRefWidth, PrecRefViewDist=0, NumDisplaysMinus1;
  reference_display_info seiReferenceDisplayInfo;

  UNREFERENCED_PARAMETER(p_Vid);
  if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) && p_Vid->FileOpen)
  {
    p_Vid->RefDispFile = fopen( p_Vid->p_Inp->RefDispOutputFile, "w+" );
    p_Vid->FileOpen = 0;
  }

  seiReferenceDisplayInfo.reference_display_update_info_sei=(*p_Vid).seiReferenceDisplay;
  (*p_Vid).seiReferenceDisplay = seiReferenceDisplayInfo.reference_display_update_info_sei;
  if(seiReferenceDisplayInfo.reference_display_update_info_sei==NULL)
    no_mem_exit("seiRefereneDisplayInfo.reference_display_update_info_sei");

  for(i=0;i<MAX_DISPLAYS;i++)
  {
    init_ref_display_info_SEI(&seiReferenceDisplayInfo.reference_display_update_info_sei[i]);
  }

  buf = (Bitstream*)malloc(sizeof(Bitstream));
  buf->bitstream_length = size;
  buf->streamBuffer = payload;
  buf->frame_bitoffset = 0;

  p_Dec->UsedBits = 0;
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
  printf("\n####Received reference display info SEI content####\n");
#endif
  if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    fprintf(p_Vid->RefDispFile, "\n####Received reference display info SEI content####\n");

  PrecRefBaseline = ue_v ( "SEI: prec_ref_baseline", buf );
  PrecRefWidth = ue_v    ( "SEI: prec_ref_display_width", buf );
  seiReferenceDisplayInfo.ref_viewing_distance_flag = u_1 ( "SEI: ref_viewing_distance_flag", buf );
  if (seiReferenceDisplayInfo.ref_viewing_distance_flag)
    PrecRefViewDist = ue_v ( "SEI: prec_ref_viewing_dist", buf );

  NumDisplaysMinus1 = ue_v    ( "SEI: num_ref_displays_minus1", buf );
  seiReferenceDisplayInfo.num_ref_displays_minus1 = NumDisplaysMinus1;

  for( i = 0; i < NumDisplaysMinus1 + 1; i++ ) 
  {
    seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_baseline.precision = PrecRefBaseline;
    seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_display_width.precision = PrecRefWidth;
    if (seiReferenceDisplayInfo.ref_viewing_distance_flag)
      seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_viewing_distance.precision = PrecRefViewDist;
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
    printf("\nReference display %d\n\n",i + 1);
    printf("Reference baseline           : ");
#endif
    if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    {
      fprintf(p_Vid->RefDispFile,"\nReference display %d\n\n",i + 1);
      fprintf(p_Vid->RefDispFile,"Reference baseline           : ");
    }
    decode_reference_display_sei_element(p_Vid, &seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_baseline, buf, p_Vid->RefDispFile);
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
    printf("Reference width              : ");
#endif
    if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
      fprintf(p_Vid->RefDispFile,"Reference width              : ");
    decode_reference_display_sei_element(p_Vid, &seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_display_width, buf, p_Vid->RefDispFile);

    if (seiReferenceDisplayInfo.ref_viewing_distance_flag)
    {
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
      printf("Reference viewing distance   : ");
#endif
      if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
        fprintf(p_Vid->RefDispFile,"Reference viewing distance   : ");
      decode_reference_display_sei_element(p_Vid, &seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_viewing_distance, buf, p_Vid->RefDispFile);
    }

    seiReferenceDisplayInfo.additional_shift_present_flag[i] = u_1 ( "SEI: additional_shift_present_flag[ i ]", buf );

    if(seiReferenceDisplayInfo.additional_shift_present_flag[i])
    {
      seiReferenceDisplayInfo.num_sample_shift_plus512[i] = u_v ( 10, "SEI: num_sample_shift_plus512[ i ]", buf );
      (*p_Vid).NumSampleShiftPlus512[i] = seiReferenceDisplayInfo.num_sample_shift_plus512[i];
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
      printf("Additional shift             :  %d\n",seiReferenceDisplayInfo.num_sample_shift_plus512[i]);
#endif
      if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
        fprintf(p_Vid->RefDispFile,"Additional shift             :  %d\n",seiReferenceDisplayInfo.num_sample_shift_plus512[i]);
    }
  }

  seiReferenceDisplayInfo.three_dimensional_extension_flag = u_1 ( "SEI:   three_dimensional_reference_displays_extension_flag", buf );     
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
  printf("\n\n####Reference display SEI finished####\n\n");
#endif
  if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) && p_Vid->frame_no != 0)
  {
    fprintf(p_Vid->RefDispFile,"\n####Reference display SEI finished ");
    p_Vid->RefDispSEIUpdated = 1;
  }
  else if( strlen( p_Vid->p_Inp->RefDispOutputFile ))
    fprintf(p_Vid->RefDispFile,"\n####Reference display SEI finished for POC %d ####\n\n",p_Vid->frame_no);

  p_Vid->NumDisplays = NumDisplaysMinus1 + 1;
  p_Vid->RefDispSEI  = 1;
  p_Vid->RefDispSEIPOC = p_Vid->frame_no;
  free (buf);
}
void update_reference_display_info(VideoParameters *p_Vid, multiview_acquisition_info* seiMultiviewAcquisitionInfo)
{
  double *TranslationFrame0 =  malloc((seiMultiviewAcquisitionInfo->num_views_minus1 + 1)*sizeof(double));
  double *TranslationFrameN =  malloc((seiMultiviewAcquisitionInfo->num_views_minus1 + 1)*sizeof(double));
  double baselineRef, scaleFactor, baselineScaled;
  int i, shiftScaled;

  for (i = 0; i < seiMultiviewAcquisitionInfo->num_views_minus1 + 1; i++)
  {
    TranslationFrame0[i] = (*p_Vid).seiMultiviewAcquisition[i].translation_ae[0].rec;    //Translation params of all views of previous Multiviewacquisiton sei
    TranslationFrameN[i] = seiMultiviewAcquisitionInfo->multiview_acquisition_update_info_sei[i].translation_ae[0].rec;   //Translation params of all views of current Multiviewacquisiton sei
  }
  if (TranslationFrame0[0] != TranslationFrame0[1] && TranslationFrameN[0] != TranslationFrameN[1] )
    scaleFactor = fabs(TranslationFrameN[0]-TranslationFrameN[1])/fabs(TranslationFrame0[0]-TranslationFrame0[1]);
  else
    scaleFactor = 1;
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
  printf("\n####Decoder derived reference display info SEI content####\n");
#endif
  if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    fprintf(p_Vid->RefDispFile,"\n####Derived reference display information (decoder)####\n");
  for (i = 0; i < (*p_Vid).NumDisplays; i++)
  {
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
    printf("\nReference display %d\n\n",i + 1);
    printf("Reference baseline           : ");
#endif
    if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    {
      fprintf(p_Vid->RefDispFile,"\nReference display %d\n\n",i + 1);
      fprintf(p_Vid->RefDispFile,"Reference baseline           : ");
    }
    baselineRef = (*p_Vid).seiReferenceDisplay[i].ref_baseline.rec;
    baselineScaled = baselineRef * scaleFactor;
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
    printf(" %.20g\n",baselineScaled);
    printf("Reference width              : ");
    printf(" %.20g\n",(*p_Vid).seiReferenceDisplay[i].ref_display_width.rec);
    printf("Reference viewing distance   : ");
    printf(" %.20g\n",(*p_Vid).seiReferenceDisplay[i].ref_viewing_distance.rec);
#endif
    if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    {
      fprintf(p_Vid->RefDispFile," %.20g\n",baselineScaled);
      fprintf(p_Vid->RefDispFile,"Reference width              : ");
      fprintf(p_Vid->RefDispFile," %.20g\n",(*p_Vid).seiReferenceDisplay[i].ref_display_width.rec);
      fprintf(p_Vid->RefDispFile,"Reference viewing distance   : ");
      fprintf(p_Vid->RefDispFile," %.20g\n",(*p_Vid).seiReferenceDisplay[i].ref_viewing_distance.rec);
    }
    shiftScaled = (int)(*p_Vid).NumSampleShiftPlus512[i] * scaleFactor;
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
    printf("Additional shift             :  %d\n", shiftScaled);
#endif
    if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
      fprintf(p_Vid->RefDispFile,"Additional shift             :  %d\n", shiftScaled);
  }
#if PRINT_REFERENCE_DISPLAY_INFO_SEI
  printf("\n\n####Reference display SEI finished####\n\n");
#endif
  if ( strlen( p_Vid->p_Inp->RefDispOutputFile ) )
    fprintf(p_Vid->RefDispFile,"\n####Reference display SEI finished ");

  p_Vid->RefDispSEIUpdated = 1;
  p_Vid->RefDispSEIPOC = p_Vid->frame_no;
}
#endif
