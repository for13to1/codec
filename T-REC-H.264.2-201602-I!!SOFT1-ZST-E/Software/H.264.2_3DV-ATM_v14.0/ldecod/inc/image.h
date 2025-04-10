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
 * \file image.h
 *
 * \brief
 *    prototypes for image.c
 *
 ************************************************************************
 */

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "mbuffer.h"

extern void calculate_frame_no(VideoParameters *p_Vid, StorablePicture *p);
extern void find_snr          (VideoParameters *p_Vid, StorablePicture *p, int *p_ref);
extern int  picture_order     ( Slice *pSlice );

extern void decode_one_slice  (Slice *currSlice);
extern int  read_new_slice    (Slice *currSlice);
extern void exit_picture      (VideoParameters *p_Vid, StorablePicture **dec_picture);
extern int  decode_one_frame  (DecoderParams *pDecoder);

extern int  is_new_picture(StorablePicture *dec_picture, Slice *currSlice, OldSliceParams *p_old_slice);
extern void init_old_slice(OldSliceParams *p_old_slice);
// For 4:4:4 independent mode
extern void copy_dec_picture_JV (VideoParameters *p_Vid, StorablePicture *dst, StorablePicture *src );

extern void frame_postprocessing(VideoParameters *p_Vid);
extern void field_postprocessing(VideoParameters *p_Vid);

#if MVC_EXTENSION_ENABLE||EXT3D
extern int GetVOIdx(VideoParameters *p_Vid, int iViewId);
extern int get_maxViewIdx(VideoParameters *p_Vid, int view_id, int anchor_pic_flag, int listidx);
#endif

extern void init_slice(VideoParameters *p_Vid, Slice *currSlice);
extern void decode_slice(Slice *currSlice, int current_header);

#if EXT3D
extern int get_viewsyn_picture(Slice* pSlice);
#endif

#endif
