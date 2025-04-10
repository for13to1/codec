/*
***********************************************************************
* COPYRIGHT AND WARRANTY INFORMATION
*
* Copyright 2001-2016, International Telecommunications Union, Geneva
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
* \file  c3d_utilities_api.h
*
* \brief 
*        MFC SDK  Utilities layer API functions
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_UTILITIES_
#define _C3D_UTILITIES_




/**
 * \fn int32_t read_yuv_image(ImageData *p_img,ImageData *p_img_tmp, FILE *pf_input,uint8_t u8_packedUV);
 *
 * \brief Reads a yuv frame image from a 4:2:0 YUV file
 * 
 *
 * \param[out] p_img        Pointer to the image data struture 
 * \param[out] p_img_tmp    Pointer to the temporary image data struture used for u8_packedUV==1
 * \param[in]  pf_input      File Pointer to the input yuv file
 * \param[in]  u8_packedUV    Packed or Unpacked UV type

 *
 * \return SUCCESS or FAILURE
 */
int32_t read_yuv_image(ImageData *p_img,ImageData *p_img_tmp, FILE *pf_input,uint8_t u8_packedUV);

/**
 * \fn void write_yuv_image(ImageData *p_img,ImageData *p_img_tmp, FILE *pf_output,uint8_t u8_packedUV)
 *
 * \brief Writes a yuv frame image to a 4:2:0 YUV file
 * 
 *
 * \param[in]  p_img        Pointer to the image data struture 
 * \param[out] p_img_tmp    Pointer to the temporary image data struture used for u8_packedUV==1
 * \param[out] pf_output    File Pointer to the output yuv file
 * \param[in]  u8_packedUV    Packed or Unpacked UV type

 *
 * \return SUCCESS or FAILURE
 */
int32_t write_yuv_image(ImageData *p_img,ImageData *p_img_tmp, FILE *pf_output,uint8_t u8_packedUV);

/**
 * \fn int32_t allocate_image(
 *                               ImageData **p_img1,
 *                               ImageData *p_ref_image,
 *                               uint8_t    u8_packed_UV
 *                              )
 *
 * \brief Allocates memory for images by making API calls.
 *
 * \details Information about the frame (like frame width etc) is extracted from the reference image. The function
 * then makes an API call to the RPU kernel to allocate memory for the image data struture.
 *
 * \param[out]    p_img1                    Pointer to store the pointer to image 1
 * \param[in]    p_ref_image                Pointer to the reference image data structure. All the image info is
 *                                        extracted from this.
 * \param[in]    u8_packed_UV            UV packed alternatively or V followed by U
 * \return SUCCESS or FAILURE
 */
int32_t allocate_image(
                        ImageData **p_img1,
                        ImageData *p_ref_image,                       
                        uint8_t    u8_packed_UV
                       );

/**
 * \fn int32_t generate_random_image(ImageData *p_img)
 *
 * \brief Generates a random source image
 *
 * \details Uses the system time to initialse the seed of the random number generator.
 *
 * \param[out] p_img    Pointer to the image data struture that is filled with a random image
 *
 * \return
 */
int32_t generate_random_image(ImageData *p_img);

/**
 * \fn int32_t copy_image_pixelData_only( 
 *                             ImageData* p_dst,
 *                       const ImageData* p_src
 *                      )  
 *
 * \brief Copies the source image to destination.
 *
 * \details Copies the Y,U and V buffers as well as other parameters to destination buffers. It is assumed that the 
 * memory has been allocated.(Doesnt change the destination Image Data Properties)
 *
 * \param[out]    p_dst    Pointer to destination image data structure.
 * \param[in]    p_src    Pointer to source image data structure. 
 *
 * \return SUCCESS or FAILURE
 */
int32_t copy_image_pixeldata_only(
                      ImageData *p_dst,
                const ImageData *p_src);


/**
 * \fn int32_t copy_complete_imageData( 
 *                             ImageData* p_dst,
 *                       const ImageData* p_src
 *                      )  
 *
 * \brief Copies the source image to destination.
 *
 * \details Copies the Y,U and V buffers as well as other parameters to destination buffers. It is assumed that the 
 * memory has been allocated.
 *
 * \param[out]    p_dst    Pointer to destination image data structure.
 * \param[in]    p_src    Pointer to source image data structure. 
 *
 * \return SUCCESS or FAILURE
 */
int32_t copy_complete_ImageData(
                      ImageData *p_dst,
                const ImageData *p_src
               );

/**
 * \fn int32_t pad_ImageData(
 *                      ImageData *p_dst,
 *                      uint16_t u16_orig_width,    
 *                      uint16_t u16_orig_height    
 *               )
 *
 * \brief Copies the source image to destination.
 *
 * \details Pads the X & Y boundaries of Image Data.
 *
 * \param[in/out]    p_dst    Pointer to destination image data structure.
 * \param[in]    u16_orig_width    Last column of the Image that is used to pad the rest of the columns. 
 * \param[in]    u16_orig_height    Last row of the Image that is used to pad the rest of the rows. 
 *
 * \return SUCCESS or FAILURE
 */
int32_t pad_ImageData(
                      ImageData *p_dst,
                      uint16_t u16_original_width,    
                      uint16_t u16_original_height    
               );




/**
 * \fn    int32_t change_image_format(
 *                      ImageData *p_dst,
 *                const ImageData *p_src,
 *                      PICTURE_TYPE e_dst_picture_type 
 *               )
 *
 * \brief Copy Image Format (Dimensions and YUV Pointers) to a destination ImageData based on the picture_type.
 *            picture_type == FRAME ; Copy Image YUV pointers and Format as it is.
 *            picture_type == Top Field ; Copy Image YUV pointers, copy Format by doubling the stride , halving the height, halving the delimiter.
 *            picture_type == Bottom Field ; Copy Image YUV pointers with an offset of Stride , copy Format by doubling the stride , halving the height, halving the delimiter.
 *
 * \param[out]    p_dst    Pointer to destination image data structure.
 * \param[in]    p_src    Pointer to source image data structure.
 * \param[in]    e_dst_picture_type    Destination Picture Tye(Frame/topfield/bottom field)
 *
 * \return SUCCESS or FAILURE
 */
int32_t change_image_format(
                      ImageData *p_dst,
                const ImageData *p_src,
                      PICTURE_TYPE e_dst_picture_type 
               );


/**
 * \fn    int32_t copy_image_dimensions(
 *                      ImageData *p_dst,
 *                const ImageData *p_src                      
 *               )
 *
 * \brief Copy Image Dimensions only from Source Image to Destination Image.
 *
 * \param[out]    p_dst    Pointer to destination image data structure.
 * \param[in]    p_src    Pointer to source image data structure. 
 *
 * \return SUCCESS or FAILURE
 */
int32_t copy_image_dimensions(
                      ImageData *p_dst,
                const ImageData *p_src                      
               );

/**
 * \fn    int32_t print_rpu_header(RPUData *p_rpu_data);
 *
 * \brief Prints the RPU Header Information.
 * 
 * \param[in]    p_rpu_data    Pointer to RPU Header.
 *
 * \return SUCCESS or FAILURE
 */
int32_t print_rpu_header(RPUData *p_rpu_data);

/**
 * \fn uint32_t GetNumberYUVFrames(
 *                    FILE                *pf_yuv_file,
 *                    int                 iWidth,
 *                    int                 iHeight,
 *                    int                 iYUVFormat
 *                    );
 * \brief Find the number of frames in a given yuv file pointer.
 *
 * \param[in]    pf_yuv_file             Point to the start of the YUV file.
 * \param[in]    iWidth                 Width  of the input YUV file. 
 * \param[in]    iHeight                Height of the input YUV file.
 * \param[in]    iYUVFormat             YUV420 - 0 (Only supports YUV420)   
 *
 * \return        None
 */
uint32_t GetNumberYUVFrames(
                    FILE                *pf_yuv_file,
                    int                 iWidth,
                    int                 iHeight,
                    int                 iYUVFormat
                    );

#ifdef PRINT_RPU_INFO
#define PRINT_RPU_DATA(arg) printf arg
#else
#define PRINT_RPU_DATA(arg) 
#endif



#endif /* _C3D_UTILITIES_ */
