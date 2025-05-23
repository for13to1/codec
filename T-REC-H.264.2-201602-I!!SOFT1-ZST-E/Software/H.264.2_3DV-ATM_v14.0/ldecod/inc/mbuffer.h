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
 
 *      - Jill Boyce               <jill.boyce@thomson.net>
 *      - Saurav K Bandyopadhyay   <saurav@ieee.org>
 *      - Zhenyu Wu                <Zhenyu.Wu@thomson.net
 *      - Purvin Pandit            <Purvin.Pandit@thomson.net>
 *
 ***********************************************************************
 */
#ifndef _MBUFFER_H_
#define _MBUFFER_H_

#include "global.h"

#define MAX_LIST_SIZE 33
//! definition of pic motion parameters
typedef struct pic_motion_params_old
{
  byte *      mb_field;      //!< field macroblock indicator

} PicMotionParamsOld;

//! definition of pic motion parameters
typedef struct pic_motion_params
{
  struct storable_picture *ref_pic[2];  //!< referrence picture pointer
  MotionVector             mv[2];       //!< motion vector  
  char                     ref_idx[2];  //!< reference picture   [list][subblock_y][subblock_x]
#if EXT3D
  int                      isVSPRef[2];
#endif
  //byte                   mb_field;    //!< field macroblock indicator
  //byte                   field_frame; //!< indicates if co_located is field or frame. Will be removed at some point
} PicMotionParams;

#if EXT3D
typedef struct slice_header_dual_params
{
  int                 no_output_of_prior_pics_flag;
  int                 long_term_reference_flag;
  int                 adaptive_ref_pic_buffering_flag;
  DecRefPicMarking_t *dec_ref_pic_marking_buffer;                   

  int                 start_mb_nr;
  char                colour_plane_id;
  unsigned int        frame_num;

  int                 depth_based_mvp_flag;

  unsigned int        field_pic_flag;
  byte                bottom_field_flag;
  PictureStructure    structure;
  Boolean             mb_aff_frame_flag;
  int                 idr_pic_id;
  unsigned int        pic_order_cnt_lsb;
  int                 delta_pic_order_cnt_bottom;
  int                 delta_pic_order_cnt[2];
  int                 redundant_pic_cnt;
  int                 redundant_slice_ref_idx;  
  char                direct_spatial_mv_pred_flag;
  char                num_ref_idx_active[2];

  int                *abs_diff_view_idx_minus1[2];
  int                 ref_pic_list_reordering_flag[2];
  int                *reordering_of_pic_nums_idc[2];
  int                *abs_diff_pic_num_minus1[2];
  int                *long_term_pic_idx[2];

  unsigned short      luma_log2_weight_denom;
  unsigned short      chroma_log2_weight_denom;
  int              ***wp_weight;  // weight in [list][index][component] order
  int              ***wp_offset;  // offset in [list][index][component] order
  int             ****wbp_weight; //weight in [list][fw_index][bw_index][component] order
  short               wp_round_luma;
  short               wp_round_chroma;

  int                 model_number;
  int                 sp_switch;
  int                 slice_qs_delta;
  int                 qs;
  char                DFDisableIdc;
  char                DFAlphaC0Offset;
  char                DFBetaOffset;
  int                 slice_group_change_cycle;
#if FIX_SLICE_HEAD_PRED
  int                 slice_type; //for conformance test
#endif
} SliceHeaderDualParams;
#endif

//! definition a picture (field or frame)
typedef struct storable_picture
{
  PictureStructure structure;

  int         poc;
  int         top_poc;
  int         bottom_poc;
  int         frame_poc;
  unsigned int  frame_num;
  unsigned int  recovery_frame;

  int         pic_num;
  int         long_term_pic_num;
  int         long_term_frame_idx;

  byte        is_long_term;
  int         used_for_reference;
  int         is_output;
  int         non_existing;
  int         separate_colour_plane_flag;

  short       max_slice_id;

  int         size_x, size_y, size_x_cr, size_y_cr;
  int         size_x_m1, size_y_m1, size_x_cr_m1, size_y_cr_m1;
  int         coded_frame;
  int         mb_aff_frame_flag;
  unsigned    PicWidthInMbs;
  unsigned    PicSizeInMbs;
  int         iLumaPadY, iLumaPadX;
  int         iChromaPadY, iChromaPadX;


  imgpel **     imgY;         //!< Y picture component
  imgpel ***    imgUV;        //!< U and V picture components
  imgpel ***    img_comp;     //!< Y,U, and V components

  struct pic_motion_params **mv_info;          //!< Motion info
  struct pic_motion_params **JVmv_info[MAX_PLANE];          //!< Motion info

  struct pic_motion_params_old  motion;              //!< Motion info  
  struct pic_motion_params_old  JVmotion[MAX_PLANE]; //!< Motion info for 4:4:4 independent mode decoding

  short **     slice_id;      //!< reference picture   [mb_x][mb_y]

  struct storable_picture *top_field;     // for mb aff, if frame for referencing the top field
  struct storable_picture *bottom_field;  // for mb aff, if frame for referencing the bottom field
  struct storable_picture *frame;         // for mb aff, if field for referencing the combined frame

  int         slice_type;
  int         idr_flag;
  int         no_output_of_prior_pics_flag;
  int         long_term_reference_flag;
  int         adaptive_ref_pic_buffering_flag;

  int         chroma_format_idc;
  int         frame_mbs_only_flag;
  int         frame_cropping_flag;
  int         frame_cropping_rect_left_offset;
  int         frame_cropping_rect_right_offset;
  int         frame_cropping_rect_top_offset;
  int         frame_cropping_rect_bottom_offset;
  int         qp;
  int         chroma_qp_offset[2];
  int         slice_qp_delta;
  DecRefPicMarking_t *dec_ref_pic_marking_buffer;                    //!< stores the memory management control operations

  // picture error concealment
  int         concealed_pic; //indicates if this is a concealed picture
  
  // variables for tone mapping
  int         seiHasTone_mapping;
  int         tone_mapping_model_id;
  int         tonemapped_bit_depth;  
  imgpel*     tone_mapping_lut;                //!< tone mapping look up table
#if EXT3D
  int*                      p_out;
  int                       view_id;
  int                       is_depth;
  int                       inter_view_flag;
  int                       anchor_pic_flag;
  double                    depth_near;
  double                    depth_far;
  seq_parameter_set_rbsp_t* sps;
#else
#if (MVC_EXTENSION_ENABLE)
  int         view_id;
  int         inter_view_flag;
  int         anchor_pic_flag;
#endif
#endif
  int         iLumaStride;
  int         iChromaStride;
  int         iLumaExpandedHeight;
  int         iChromaExpandedHeight;
  imgpel **cur_imgY; // for more efficient get_block_luma
  int no_ref;
  int iCodingType;

#if EXT3D
  int reduced_res;
  ResizeParameters* upsampling_params;
  int size_x_ori;
  int size_y_ori;
  int size_x_cr_ori;
  int size_y_cr_ori;

  // POZNAN_NONLINEAR_DEPTH
  char NonlinearDepthPoints[256];
  int  NonlinearDepthNum;

  // POZNAN_NTT_DEPTH_REPRESENTATION_SEI
  char SEI_NonlinearDepthPoints[256];
  int  SEI_NonlinearDepthNum;
#endif

  //
  char listXsize[2];
  struct storable_picture **listX[2];
} StorablePicture;

typedef StorablePicture *StorablePicturePtr;

//! definition a picture (field or frame)
typedef struct colocated_params
{
  int         mb_adaptive_frame_field_flag;
  int         size_x, size_y;
  byte        is_long_term;

} ColocatedParams;

//! Frame Stores for Decoded Picture Buffer
typedef struct frame_store
{
  int       is_used;                //!< 0=empty; 1=top; 2=bottom; 3=both fields (or frame)
  int       is_reference;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_long_term;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_orig_reference;      //!< original marking by nal_ref_idc: 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used

  int       is_non_existent;

  unsigned  frame_num;
  unsigned  recovery_frame;

  int       frame_num_wrap;
  int       long_term_frame_idx;
  int       is_output;
  int       poc;

  // picture error concealment
  int concealment_reference;

  StorablePicture *frame;
  StorablePicture *top_field;
  StorablePicture *bottom_field;
#if MVC_EXTENSION_ENABLE||EXT3D
  int       view_id;
  int       inter_view_flag[2];
  int       anchor_pic_flag[2];
#endif
#if EXT3D
  int*      p_out;
#endif
  
} FrameStore;


//! Decoded Picture Buffer
typedef struct decoded_picture_buffer
{
  VideoParameters *p_Vid;
  InputParameters *p_Inp;

  FrameStore  **fs;
  FrameStore  **fs_ref;
  FrameStore  **fs_ltref;
  unsigned      size;
  unsigned      used_size;
  unsigned      ref_frames_in_buffer;
  unsigned      ltref_frames_in_buffer;
  int           last_output_poc;
#if EXT3D
  int           last_output_view_id;
  int           max_long_term_pic_idx[MAX_CODEVIEW ];  
#else
#if (MVC_EXTENSION_ENABLE)
  int           last_output_view_id;
  int           max_long_term_pic_idx[MAX_VIEW_NUM];  
#else
  int           max_long_term_pic_idx;  
#endif
#endif

  int           init_done;
  int           num_ref_frames;

  FrameStore   *last_picture;
} DecodedPictureBuffer;

extern void              init_dpb(VideoParameters *p_Vid, DecodedPictureBuffer *p_Dpb);
extern void              re_init_dpb(VideoParameters *p_Vid, DecodedPictureBuffer *p_Dpb);
extern void              free_dpb(DecodedPictureBuffer *p_Dpb);


extern FrameStore*       alloc_frame_store(void);
extern void              free_frame_store (FrameStore* f);
extern StorablePicture*  alloc_storable_picture(VideoParameters *p_Vid, PictureStructure type, int size_x, int size_y, int size_x_cr, int size_y_cr);
extern void              free_storable_picture (StorablePicture* p);
extern void              store_picture_in_dpb(DecodedPictureBuffer *p_Dpb, StorablePicture* p);
#if EXT3D
extern StorablePicture*  get_short_term_pic (DecodedPictureBuffer *p_Dpb, int picNum, int view_id);
#else
extern StorablePicture*  get_short_term_pic (DecodedPictureBuffer *p_Dpb, int picNum);
#endif
extern StorablePicture*  get_long_term_pic  (DecodedPictureBuffer *p_Dpb, int LongtermPicNum);

#if EXT3D
extern void             flush_dpb(DecodedPictureBuffer *p_Dpb, int curr_view_id);
extern int              GetMaxDecFrameBuffering(VideoParameters *p_Vid);
extern void             append_interview_list(DecodedPictureBuffer *p_Dpb, 
                        PictureStructure currPicStructure, 
                        int list_idx, 
                        FrameStore **list,
                        int *listXsize, 
                        int currPOC, 
                        int curr_view_id, 
                        int anchor_pic_flag);
extern void             update_ref_list(DecodedPictureBuffer *p_Dpb,int curr_view_id);
extern void             update_ltref_list(DecodedPictureBuffer *p_Dpb, int curr_view_id);

extern void             get_camera_depth_range_info(VideoParameters* p_Vid);
extern void             get_disparity_table(VideoParameters* p_Vid);
#else
#if (MVC_EXTENSION_ENABLE)
extern void             flush_dpb(DecodedPictureBuffer *p_Dpb, int curr_view_id);
extern int              GetMaxDecFrameBuffering(VideoParameters *p_Vid);
extern void             append_interview_list(DecodedPictureBuffer *p_Dpb, 
                                              PictureStructure currPicStructure, 
                                              int list_idx, 
                                              FrameStore **list,
                                              int *listXsize, 
                                              int currPOC, 
                                              int curr_view_id, 
                                              int anchor_pic_flag);
extern void             update_ref_list(DecodedPictureBuffer *p_Dpb, int curr_view_id);
extern void             update_ltref_list(DecodedPictureBuffer *p_Dpb, int curr_view_id);
#else
extern void             flush_dpb(DecodedPictureBuffer *p_Dpb);
#endif
#endif
extern void             init_lists(Slice *currSlice);
extern void             init_lists_p_slice(Slice *currSlice);
extern void             init_lists_b_slice(Slice *currSlice);
extern void             init_lists_i_slice(Slice *currSlice);
extern void             update_pic_num    (Slice *currSlice);

extern void             dpb_split_field  (VideoParameters *p_Vid, FrameStore *fs);
extern void             dpb_combine_field(VideoParameters *p_Vid, FrameStore *fs);
extern void             dpb_combine_field_yuv(VideoParameters *p_Vid, FrameStore *fs);

extern void             reorder_ref_pic_list(Slice *currSlice, int cur_list);

extern void             init_mbaff_lists(VideoParameters *p_Vid, Slice *currSlice);
extern void             alloc_ref_pic_list_reordering_buffer(Slice *currSlice);
extern void             free_ref_pic_list_reordering_buffer(Slice *currSlice);

extern void             fill_frame_num_gap(VideoParameters *p_Vid, Slice *pSlice);

extern void compute_colocated (Slice *currSlice, StorablePicture **listX[6]);


extern void gen_pic_list_from_frame_list(PictureStructure currStructure, FrameStore **fs_list, int list_idx, StorablePicture **list, char *list_size, int long_term);


extern void pad_dec_picture(VideoParameters *p_Vid, StorablePicture *dec_picture);
extern void pad_buf(imgpel *pImgBuf, int iWidth, int iHeight, int iStride, int iPadX, int iPadY);
#endif

