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
* \file  mfc_common.h
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



#ifndef _MFC_COMMON_INC_
#define _MFC_COMMON_INC_



/* Structures */

typedef struct mfc_ImageData
{
    void            *p_buf[3];
    unsigned int    au16_buffer_stride[3];
    unsigned int    au16_frame_width[3];
    unsigned int    au16_frame_height[3];
    unsigned int    au16_view_delimiter_sbs[3];
    unsigned int    au16_view_delimiter_ou[3];
    unsigned int    max_pel_value;
    unsigned int    chroma_format;
    unsigned int    picture_type;
}MFC_IMG_DATA;

typedef struct mfc_static_params
{  
  int mfc_format_idc;                       //!< SBS/TAB  
  int default_grid_position_flag;       
  int view0_grid_position_x;            //!< Specifies non-default grid positions for view offsets
  int view0_grid_position_y;            //!< 
  int view1_grid_position_x;            //!< 
  int view1_grid_position_y;            //!<   
  int rpu_filter_enabled_flag;          //!< DC(0)/F0 (1) 
  int rpu_field_processing_flag;
} MFC_RPU_DATA;

/* Function Declarations */

/*!
 ************************************************************************
 * void reset_mfc_image_format(void *inImgData,int picType)
 * \brief 
 *    ImageData structure in JM has a void pointer (pv_mfc_img_data) to mfc_sdk  Image Data Structure.
 *    Each JM imagedata can be accessed as Frame/TF/BF by mfc_sdk , by modifying imageData structure used by mfc_sdk.
 *
 * \param *inImgData        
 *    Pointer to JM imageData Structure
 * \param picType
 *    Type of the destination mfc_sdk ImageData
 *            0- Frame (frm_data pointers , ,height and frm_stride is set mfc_sdk ImageData as it is)
 *            1-Top Field (top_data pointers, top_stride , width is set  as it is while height is halved in the mfc_sdk ImageData)
 *            2-Bottom Field(bot_data pointers, bot_stride , width is set  as it is while height is halved in the mfc_sdk ImageData)
 ************************************************************************
 */
void reset_mfc_image_format(
                            void    *inImgData,
                            int        picType
                            );

/*!
 ************************************************************************
 * int  map_spec_offset_to_mfc_format(int i_spec_offset)
 * \brief 
 *    Map the view grid offset values from AVC specification format to MFC format.
 *
 * \param[in] i_spec_offset
 *    i_spec_offset     - Offset as per AVC Specification
 * \return 
 *                      - Offset as per MFC specifiation
 ************************************************************************
 */
int  map_spec_offset_to_mfc_format(int i_spec_offset);


/*!
 ************************************************************************
 * void*   alloc_mfc_rpu_data()
 * \brief 
 *    Alloctaes memory for RPUData which hold the information used by MFC RPU 
 *
 * \param[out]
 *       void Pointer to the allocated structure
 ************************************************************************
 */
void*   alloc_mfc_rpu_data();


/*!
 ************************************************************************
 * int     set_mfc_rpu_data(MFC_RPU_DATA *mfc_static_params, 
                                          void *pv_rpu_init_params)
 * \brief 
 *    Checks and copies the JM input paramters  RPUData Structure.
 *
 * \param[in]  
 *       mfc_static_params - JM Input Parameters
 *\param[out] 
 *       pv_rpu_init_params - Void Pointer to the RPUData Structure   
 *\return 
 *       0/1
 ************************************************************************
 */
int     set_mfc_rpu_data(MFC_RPU_DATA *mfc_static_params, 
                                          void *pv_rpu_init_params);


/*!
 ************************************************************************
 * void allocate_mfc_image_format()
 * \brief 
 *    Allocate memory for mfc sdk Image Data 
 * \return 
 *     void pointer to the allocated ImageType
 ************************************************************************
 */
void* allocate_mfc_image_format();


/*!
 ************************************************************************
 * void free_mfc_format(void *mfc_ImageData)
 * \brief 
 *    Free memory for mfc_sdk Image Data 
 * \Param mfc_ImageData
 *     pointer to ImageData to the freed.
 ************************************************************************
 */
void free_mfc_format        (
                            void *mfc_ImageData
                            );


/*!
 ************************************************************************
 * void reset_mfc_imgData(MFC_IMG_DATA *src_image_data,void *pv_img_data)
 * \brief 
 *    Reset a mfc_sdk ImageData
 *
 * \param *src_image_data        
 *   Source ImageData type to be copied to the Destination mfc_sdk IamgeData
 * \param *pv_img_data
 *     pointer to Destination ImageData
 ************************************************************************
 */
void reset_mfc_imgData    (
                            MFC_IMG_DATA    *src_image_data,
                            void            *pv_img_data
                            );

/*!
 ************************************************************************
 *  write_yuv_file_wrapper    (
 *                            FILE        *fp,
 *                            const    void *pv_imgData,
 *                            )
 * \brief 
 *   Write YUV ImageData to a file (Wrapper function)
 *
 * \param *fp        
 *   Output file pointer
 * \param *pv_imgData
 *     ImageData to be written to the file
 ************************************************************************
 */
int write_yuv_file_wrapper  (
                            FILE *fp,
                      const void *pv_imgData
                            );


/*!
 ************************************************************************
 *  void copy_mfc_ImageData (
 *                          void        *pv_dst_image,
 *                          void        *pv_src_image
 *                          )
 * \brief 
 *   Wrapper to  Copy complete imagedata from one Image to another Image (Wrapper function).
 *
 * \param *pv_dst_image        
 *   Output Image Data  pointer
 * \param *pv_src_image        
 *   Source Image Data  pointer
 ************************************************************************
 */
void copy_mfc_ImageData   (
                          void        *pv_dst_image,
                          void        *pv_src_image
                          );
    




#endif

