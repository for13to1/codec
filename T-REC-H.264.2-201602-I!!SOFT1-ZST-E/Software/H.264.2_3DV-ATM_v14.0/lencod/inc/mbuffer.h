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
 ***********************************************************************
 *  \file
 *      mbuffer.h
 *
 *  \brief
 *      Frame buffer functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Sühring          <suehring@hhi.de>
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 ***********************************************************************
 */
#ifndef _MBUFFER_H_
#define _MBUFFER_H_

#include "global.h"
#include "enc_statistics.h"

#define MAX_LIST_SIZE 33

typedef struct picture_stats
{
  double dsum[3];
  double dvar[3];
} PictureStats;

typedef struct distortion_estimation Dist_Estm;

//! definition of pic motion parameters
typedef struct pic_motion_params_old
{
  byte *      mb_field;      //!< field macroblock indicator
} PicMotionParamsOld;


//! definition of pic motion parameters
typedef struct pic_motion_params
{
  struct storable_picture *ref_pic[2];  //!< referrence picture pointer
  char                     ref_idx[2];  //!< reference picture   [list][subblock_y][subblock_x]
  MotionVector             mv[2];       //!< motion vector  
  byte                     field_frame; //!< indicates if co_located is field or frame. Will be removed at some point

#if EXT3D
  short                    isVSPRef[2];
#endif
} PicMotionParams;

//! definition a picture (field or frame)
typedef struct storable_picture
{
  PictureStructure structure;

  int         poc;
  int         top_poc;
  int         bottom_poc;
  int         frame_poc;
  int         order_num;
  unsigned    frame_num;
  int         pic_num;
  int         long_term_pic_num;
  int         long_term_frame_idx;
  int         temporal_layer;     

  byte        is_long_term;
  int         used_for_reference;
  int         is_output;
  int         non_existing;

  int         size_x, size_y, size_x_cr, size_y_cr;
  int         size_x_padded, size_y_padded;
  int         size_x_pad, size_y_pad;
  int         size_x_cr_pad, size_y_cr_pad;
  int         pad_size_uv_y, pad_size_uv_x; 
  int         chroma_vector_adjustment;
  int         coded_frame;
  int         mb_aff_frame_flag;

  imgpel **   imgY;          //!< Y picture component
  imgpel **** imgY_sub;      //!< Y picture component upsampled (Quarter pel)
  imgpel ***  imgUV;         //!< U and V picture components
  imgpel *****imgUV_sub;     //!< UV picture component upsampled (Quarter/One-Eighth pel)

  imgpel ***  p_dec_img[MAX_PLANE];      //!< pointer array for accessing decoded pictures in hypothetical decoders

  imgpel **   p_img[MAX_PLANE];          //!< pointer array for accessing imgY/imgUV[]
  imgpel **** p_img_sub[MAX_PLANE];      //!< pointer array for storing top address of imgY_sub/imgUV_sub[]
  imgpel **   p_curr_img;                //!< current int-pel ref. picture area to be used for motion estimation
  imgpel **** p_curr_img_sub;            //!< current sub-pel ref. picture area to be used for motion estimation
  //hme;
  imgpel ***  p_hme_int_img;     //!< [level][y][x];
  imgpel *****  p_hme_sub_img;   //!< [level][y_frac][x_frac][y][x];

  Dist_Estm * de_mem; 

  PicMotionParams **mv_info;                 //!< Motion info
  PicMotionParams **JVmv_info[MAX_PLANE];    //!< Motion info for 4:4:4 independent coding
  PicMotionParamsOld  motion;    //!< Motion info
  PicMotionParamsOld JVmotion[MAX_PLANE];    //!< Motion info for 4:4:4 independent coding

  int colour_plane_id;                     //!< colour_plane_id to be used for 4:4:4 independent mode encoding

  struct storable_picture *top_field;     // for mb aff, if frame for referencing the top field
  struct storable_picture *bottom_field;  // for mb aff, if frame for referencing the bottom field
  struct storable_picture *frame;         // for mb aff, if field for referencing the combined frame

  int         chroma_format_idc;

#if EXT3D
  int         force_yuv400;
#endif

  int         chroma_mask_mv_x;
  int         chroma_mask_mv_y;
  int         chroma_shift_y;
  int         chroma_shift_x;
  int         frame_mbs_only_flag;
  int         frame_cropping_flag;
  int         frame_cropping_rect_left_offset;
  int         frame_cropping_rect_right_offset;
  int         frame_cropping_rect_top_offset;
  int         frame_cropping_rect_bottom_offset;

  PictureStats p_stats;
  StatParameters stats;

  int         type;
#if EXT3D
  int size_x_ori;
  int size_y_ori;
  int size_x_cr_ori;
  int size_y_cr_ori;
  int reduced_res;
  ResizeParameters* upsampling_params;

  char        NonlinearDepthPoints[256];
  int         NonlinearDepthNum;

  int         PostDilation;

  int         is_depth;
  int         view_id;
  int         inter_view_flag[2];
  int         anchor_pic_flag[2];

  double      depth_near;
  double      depth_far;
#else
#if (MVC_EXTENSION_ENABLE)
  int         view_id;
  int         inter_view_flag[2];
  int         anchor_pic_flag[2];
#endif
#endif
  int  bInterpolated;
} StorablePicture;

typedef StorablePicture *StorablePicturePtr;

//! definition of motion parameters
typedef struct motion_params
{
  int64 ***   ref_pic_id;    //!< reference picture identifier [list][subblock_y][subblock_x]
  MotionVector ***mv;            //!< motion vector       [list][subblock_x][subblock_y][component]
  char  ***   ref_idx;       //!< reference picture   [list][subblock_y][subblock_x]
  byte **     moving_block;
} MotionParams;

//! definition a picture (field or frame)
typedef struct colocated_params
{
  int         mb_adaptive_frame_field_flag;
  int         size_x, size_y;
  byte        is_long_term;

  struct motion_params frame;
  struct motion_params top;
  struct motion_params bottom;
} ColocatedParams;

//! Frame Stores for Decoded Picture Buffer
struct frame_store
{
  int       is_used;                //!< 0=empty; 1=top; 2=bottom; 3=both fields (or frame)
  int       is_reference;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_long_term;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_orig_reference;      //!< original marking by nal_ref_idc: 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used

  int       is_non_existent;

  unsigned  frame_num;
  int       frame_num_wrap;
  int       long_term_frame_idx;
  int       is_output;
  int       poc;

  StorablePicture *frame;
  StorablePicture *top_field;
  StorablePicture *bottom_field;

#if MVC_EXTENSION_ENABLE||EXT3D
  int       view_id;
  int       inter_view_flag[2];
  int       anchor_pic_flag[2];
#endif

#if EXT3D
  StorablePicture *norm_res_frame;  //!> frame of the data at the normalized resolution (for mixed-resolution coding)
  int temporal_id;
#endif
};

typedef struct frame_store FrameStore;


//! Decoded Picture Buffer
struct decoded_picture_buffer
{
  VideoParameters *p_Vid;
  InputParameters *p_Inp;
  FrameStore  **fs;
  FrameStore  **fs_ref;
  FrameStore  **fs_ltref;
  int           num_ref_frames;
  unsigned      size;
  unsigned      used_size;
  unsigned      ref_frames_in_buffer;
  unsigned      ltref_frames_in_buffer;
  int           last_output_poc;
#if EXT3D
  int                   last_output_view_id;
  int                   listinterviewidx0;
  int                   listinterviewidx1;
  struct frame_store  **fs_listinterview0;
  struct frame_store  **fs_listinterview1;
#else
#if (MVC_EXTENSION_ENABLE)
  int           last_output_view_id;
#endif
#endif
  int           max_long_term_pic_idx;

  int           init_done;

  FrameStore   *last_picture;
};

typedef struct decoded_picture_buffer DecodedPictureBuffer;

extern void             init_dpb                  (VideoParameters *p_Vid, DecodedPictureBuffer *dpb);
extern void             free_dpb                  (DecodedPictureBuffer *p_Dpb);
extern FrameStore*      alloc_frame_store(void);
extern void             free_frame_store          (VideoParameters *p_Vid, FrameStore* f);
extern StorablePicture* alloc_storable_picture    (VideoParameters *p_Vid, PictureStructure type, int size_x, int size_y, int size_x_cr, int size_y_cr);
extern void             free_storable_picture     (VideoParameters *p_Vid, StorablePicture* p);
extern void             store_picture_in_dpb      (DecodedPictureBuffer *p_Dpb, StorablePicture* p, FrameFormat *output);
extern void             replace_top_pic_with_frame(DecodedPictureBuffer *p_Dpb, StorablePicture* p, FrameFormat *output);
extern void             flush_dpb                 (DecodedPictureBuffer *p_Dpb, FrameFormat *output);
extern void             dpb_split_field           (VideoParameters *p_Vid, FrameStore *fs);
extern void             dpb_combine_field         (VideoParameters *p_Vid, FrameStore *fs);
extern void             dpb_combine_field_yuv     (VideoParameters *p_Vid, FrameStore *fs);
extern void             init_lists_p_slice        (Slice *currSlice);
extern void             init_lists_b_slice        (Slice *currSlice);
extern void             init_lists_i_slice        (Slice *currSlice);
extern void             update_pic_num            (Slice *currSlice);
extern void             reorder_ref_pic_list      (Slice *currSlice, int cur_list);
extern void             init_mbaff_lists          (Slice *currSlice);
extern void             alloc_ref_pic_list_reordering_buffer (Slice *currSlice);
extern void             free_ref_pic_list_reordering_buffer  (Slice *currSlice);
extern void             fill_frame_num_gap        (VideoParameters *p_Vid, FrameFormat *output);
extern ColocatedParams* alloc_colocated           (int size_x, int size_y,int mb_adaptive_frame_field_flag);
extern void             free_colocated            (ColocatedParams* p);
extern void             compute_colocated         (Slice *currSlice, StorablePicture **listX[6]);

#if EXT3D
extern void init_interview_list(Slice *currSlice);
extern void alloc_interview_ref_pic_list_reordering_buffer(Slice*currSlice);
extern void free_interview_ref_pic_list_reordering_buffer(Slice*currSlice);
int  is_view_id_in_ref_view_list(int view_id, int *ref_view_id, int num_ref_views)   ;
void append_interview_list(DecodedPictureBuffer *p_Dpb, 
                           PictureStructure currPicStructure, //0: frame; 1:top field; 2: bottom field;
                           int list_idx, 
                           FrameStore **list,
                           int *listXsize, 
                           int currPOC, 
                           int curr_view_id, 
                           int anchor_pic_flag)  ;
void update_ref_list(DecodedPictureBuffer *p_Dpb);
void update_ltref_list(DecodedPictureBuffer *p_Dpb);
void check_num_ref(DecodedPictureBuffer *p_Dpb);


extern void check_neighbors(Macroblock* currMB,PixelPos* block,PicMotionParams** mv_info,int list,int ref);

#else
#if (MVC_EXTENSION_ENABLE)
void update_ref_list(DecodedPictureBuffer *p_Dpb);
void update_ltref_list(DecodedPictureBuffer *p_Dpb);
void check_num_ref(DecodedPictureBuffer *p_Dpb);
#endif
#endif

extern void ChangeLists(Slice *currSlice);
#endif

