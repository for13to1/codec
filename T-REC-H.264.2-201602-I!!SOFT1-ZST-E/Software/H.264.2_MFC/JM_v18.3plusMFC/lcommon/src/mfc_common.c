/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2014, International Telecommunications Union, Geneva
*
* DISCLAIMER OF WARRANTY
*
* These software programs are available to the user without any
* license fee or royalty on an "as is" basis. The ITU disclaims
* any and all warranties, whether express, implied, or
* statutory, including any implied warranties of merchantability
* or of fitness for a particular purpose.  In no event shall the
* contributor or the ITU be liable for any incidental, punitive, or
* consequential damages of any kind whatsoever arising from the
* use of these programs.
*
* This disclaimer of warranty extends to the user of these programs
* and user's customers, employees, agents, transferees, successors,
* and assigns.
*
* The ITU does not represent or warrant that the programs furnished
* hereunder are free of infringement of any third-party patents.
* Commercial implementations of ITU-T Recommendations, including
* shareware, may be subject to royalty fees to patent holders.
* Information regarding the ITU-T patent policy is available from
* the ITU Web site at http://www.itu.int.
*
* THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
************************************************************************
*/


/*!
*************************************************************************************
* \file  mfc_common.c
*
* \brief 
*        MFC SDK  functions interface to JM 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#include "global.h"

#if (MFC_ENC_3D_FCFR || MFC_DEC_3D_FCFR)



void reset_mfc_image_format(
                            void    *inImgData,
                            int        picType
                            )
{
   MFC_IMG_DATA    mfc_ImageData;
   ImageData *imgData  = (ImageData *) inImgData;        

   int i_loop;
      
   if(1==picType)
   {  /* Set MFC Image corresponding to  TopField in JM Image Data */      
          for(i_loop=0;i_loop<3;i_loop++)
          {
            mfc_ImageData.p_buf[i_loop]               = imgData->top_data[i_loop][0];
            mfc_ImageData.au16_frame_width[i_loop]    = imgData->format.width[i_loop];
            mfc_ImageData.au16_frame_height[i_loop]   = imgData->format.height[i_loop]>>1;
            mfc_ImageData.au16_buffer_stride[i_loop]  = imgData->top_stride[i_loop];
          }  
          mfc_ImageData.au16_view_delimiter_sbs[0]    = ( imgData->format.width[0] - imgData->format.auto_crop_right)>>1;
          mfc_ImageData.au16_view_delimiter_sbs[1]    = ( imgData->format.width[1] - imgData->format.auto_crop_right_cr)>>1;
          mfc_ImageData.au16_view_delimiter_sbs[2]    = ( imgData->format.width[2] - imgData->format.auto_crop_right_cr)>>1;

          mfc_ImageData.au16_view_delimiter_ou[0]     = (imgData->format.height[0]  - imgData->format.auto_crop_bottom)>>2;
          mfc_ImageData.au16_view_delimiter_ou[1]     = (imgData->format.height[1] - imgData->format.auto_crop_bottom_cr)>>2;
          mfc_ImageData.au16_view_delimiter_ou[2]     = (imgData->format.height[2] - imgData->format.auto_crop_bottom_cr)>>2;
          mfc_ImageData.max_pel_value                 = imgData->format.max_value[0];
          mfc_ImageData.chroma_format                 = imgData->format.yuv_format;
          mfc_ImageData.picture_type                  = 1;
   }
   else if(2==picType)
   {      /* Set MFC Image corresponding to  BottomField in JM Image Data */      
          for(i_loop=0;i_loop<3;i_loop++)
          {
            mfc_ImageData.p_buf[i_loop]               = imgData->bot_data[i_loop][0];
            mfc_ImageData.au16_frame_width[i_loop]    = imgData->format.width[i_loop];
            mfc_ImageData.au16_frame_height[i_loop]   = imgData->format.height[i_loop]>>1;
            mfc_ImageData.au16_buffer_stride[i_loop]  = imgData->bot_stride[i_loop];
          }  
          mfc_ImageData.au16_view_delimiter_sbs[0]    = ( imgData->format.width[0] - imgData->format.auto_crop_right)>>1;
          mfc_ImageData.au16_view_delimiter_sbs[1]    = ( imgData->format.width[1] - imgData->format.auto_crop_right_cr)>>1;
          mfc_ImageData.au16_view_delimiter_sbs[2]    = ( imgData->format.width[2] - imgData->format.auto_crop_right_cr)>>1;

          mfc_ImageData.au16_view_delimiter_ou[0]     = (imgData->format.height[0]  - imgData->format.auto_crop_bottom)>>2;
          mfc_ImageData.au16_view_delimiter_ou[1]     = (imgData->format.height[1] - imgData->format.auto_crop_bottom_cr)>>2;
          mfc_ImageData.au16_view_delimiter_ou[2]     = (imgData->format.height[2] - imgData->format.auto_crop_bottom_cr)>>2;
          mfc_ImageData.max_pel_value                 = imgData->format.max_value[0];
          mfc_ImageData.chroma_format                 = imgData->format.yuv_format;
          mfc_ImageData.picture_type                  = 2;
   }
   else
   {    /* Set MFC Image corresponding to  Frame in JM Image Data */      
        for(i_loop=0;i_loop<3;i_loop++)
        {
          mfc_ImageData.p_buf[i_loop]               = imgData->frm_data[i_loop][0];
          mfc_ImageData.au16_frame_width[i_loop]    = imgData->format.width[i_loop];
          mfc_ImageData.au16_frame_height[i_loop]   = imgData->format.height[i_loop];
          mfc_ImageData.au16_buffer_stride[i_loop]  = imgData->frm_stride[i_loop];
        }  
        mfc_ImageData.au16_view_delimiter_sbs[0]    = ( imgData->format.width[0] - imgData->format.auto_crop_right)>>1;
        mfc_ImageData.au16_view_delimiter_sbs[1]    = ( imgData->format.width[1] - imgData->format.auto_crop_right_cr)>>1;
        mfc_ImageData.au16_view_delimiter_sbs[2]    = ( imgData->format.width[2] - imgData->format.auto_crop_right_cr)>>1;

        mfc_ImageData.au16_view_delimiter_ou[0]     = (imgData->format.height[0]  - imgData->format.auto_crop_bottom)>>1;
        mfc_ImageData.au16_view_delimiter_ou[1]     = (imgData->format.height[1] - imgData->format.auto_crop_bottom_cr)>>1;
        mfc_ImageData.au16_view_delimiter_ou[2]     = (imgData->format.height[2] - imgData->format.auto_crop_bottom_cr)>>1;
        mfc_ImageData.max_pel_value                 = imgData->format.max_value[0];
        mfc_ImageData.chroma_format                 = imgData->format.yuv_format;
        mfc_ImageData.picture_type                  = 0;
   }
   reset_mfc_imgData(&mfc_ImageData,imgData->pv_mfc_img_data);
     return;
}



#endif

int  map_spec_offset_to_mfc_format(int i_spec_offset)
{
    return (i_spec_offset==12 ? 1 : 0);

}