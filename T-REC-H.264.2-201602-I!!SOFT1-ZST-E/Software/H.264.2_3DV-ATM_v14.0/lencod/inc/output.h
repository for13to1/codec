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
 *    output.h
 * \brief
 *    Picture writing routine headers
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Suehring        <suehring@hhi.de>
 ***************************************************************************************
 */

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

extern void flush_direct_output(VideoParameters *p_Vid, FrameFormat *output, int p_out);
extern void write_out_picture  ( StorablePicture *p, FrameFormat *output, int p_out);
extern void write_stored_frame (VideoParameters *p_Vid, FrameStore *fs, FrameFormat *output, int p_out);
extern void direct_output      (VideoParameters *p_Vid, StorablePicture *p, FrameFormat *output, int p_out);
extern void direct_output_paff (VideoParameters *p_Vid, StorablePicture *p, FrameFormat *output, int p_out);
extern void init_out_buffer    (VideoParameters *p_Vid);
extern void uninit_out_buffer  (VideoParameters *p_Vid);

#if EXT3D
extern void img2buf (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, 
                     int crop_left, int crop_right, int crop_top, int crop_bottom);

extern void img_upsampling(imgpel**imgX_low_res,imgpel**imgX_ori_res, int component, int method, ResizeParameters* upsampling_params,
                           int size_x, int size_y,int crop_left, int crop_right, int crop_top, int crop_bottom);

extern void post_dilation_filter(imgpel **high_imgY, imgpel *buffer_imgY, int width, int height);
#endif

#endif //_OUTPUT_H_
