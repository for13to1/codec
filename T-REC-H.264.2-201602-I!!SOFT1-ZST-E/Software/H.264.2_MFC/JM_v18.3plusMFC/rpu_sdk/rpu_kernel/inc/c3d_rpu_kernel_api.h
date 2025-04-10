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
* \file  c3d_rpu_kernel_api.h
*
* \brief 
*        MFC SDK  RPU kernel API functions and structures 
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*         - Santosh Chilkunda     (santosh.chilkunda@dolby.com)
*
*************************************************************************************
*/


#ifndef _C3D_RPU_KERNEL_API_
#define _C3D_RPU_KERNEL_API_

/* User defined header files */
#include "c3d_rpu_kernel.h"
#include "c3d_rpu_kernel_filter.h"

/** STRUCTURES */


/**
 * RPU data structure
 * This structure contains the RPU configuration parameters.
 */
typedef struct rpu_data
{
    
    /*
     * Prediction process format that will be used (given the RPU type) when processing the video data for prediction 
     * and/or final reconstruction. When RPU type is 0, RPU format is 1 for SBS and 2 for OU.
     */
    RPU_PROCESS_FORMAT e_rpu_process_format;

    
    /* Rpu Filter Flag
     * 0 - RPU Filters not used , Output of RPU 
     *     is filled with  consant value of 128
     * 1 - RPU Filters used
     */
    uint8_t    u8_rpu_filter_enabled_flag;


    /* View 0 and view 1 grid positions */
    uint8_t    u8_default_grid_position_flag;
    uint8_t    u8_view0_grid_position_x;
    uint8_t    u8_view0_grid_position_y;
    uint8_t    u8_view1_grid_position_x;
    uint8_t    u8_view1_grid_position_y;


    /*
     * Flag to indicate if the UV is packed or unpacked. When Flag is enable it denotes U & V components are arranged alternatively.
     * when Flat is diabled it denoted V components are followed after U components.
     *
     */
    uint8_t u8_packed_UV; 
 
} RPUData;





/** API FUNCTION DECLARATIONS */

/**
 * \fn void* rpu_init()
 *
 * \brief API function to initialize an instance of RPU kernel
 *
 * \details This is the first call to the RPU kernel. It performs initializations needed to process the data in 
 * subsequent calls. The function loads the implicit filter definitions.
 * 
 *
 * \return Returns the handle to RPU kernel as a void pointer.
 */
void* rpu_init();



/**
 * \fn int32_t rpu_process(
 *                                 void       *pv_rpu_handle,
 *                           const ImageData *p_bl,
 *                                 ImageData *p_pred_el,
 *                           const RPUData   *p_rpu_data
 *                          )
 *
 * \brief API function calls the corresponding rpu_process .
 *
 * \details The partitions listed in the p_rpu_data structure are filtered to obtain the enhancement layer prediction.
 * 
 * \param[in]    pv_rpu_handle    Handle to the RPU context.
 * \param[in]    p_bl            Base layer picture. It can correspond to the reconstructed base layer in case of encoder or
 *                                decoded base layer in case of decoder.
 * \param[out]    p_pred_el        Predicted enhancement layer output by the RPU.
 * \param[in]    p_rpu_data        It contains information such as number of partitions, spatial information and filter to be
 *                                used to process each partition.
 *
 * \return Success (0) or Failure (1)
 */
int32_t rpu_process(
                          void        *pv_rpu_handle,
                    const ImageData *p_bl,
                          ImageData *p_pred_el,
                    const RPUData   *p_rpu_data
                   );

/**
 * \fn int32_t rpu_close(
 *                         void *pv_rpu_handle
 *                        )
 *
 * \brief API function to close RPU kernel instance
 *
 * \details This is the final call to the RPU kernel. It de-allocates the resources allocated to the RPU kernel.
 * 
 * \param[in]    pv_rpu_handle    Handle to the RPU kernel.
 *
 * \return Success (0) or Failure (1)
 */
int32_t rpu_close(
                  void *pv_rpu_handle
                 );

/** MEMORY MANAGEMENT FUNCTION DECLARATIONS */

/**
 * \fn void* allocate_memory(
 *                             uint32_t u32_memory_size
 *                            )
 *
 * \brief Function to allocate memory
 *
 * \details The function allocates the requested number of bytes and returns the pointer to the buffer as void.
 * 
 * \param[in]    u32_memory_size    Size of buffer.
 *
 * \return Pointer to the allocated buffer.
 */
void* allocate_memory(
                      uint32_t u32_memory_size
                     );

/**
 * \fn void set_memory(
 *                       void        *pv_buffer,
 *                       uint32_t u32_size,
 *                       uint32_t    u32_val
 *                      )
 *
 * \brief Function to set the memory with given value.
 *
 * \details The function sets the value in the buffer.
 * 
 * \param[in]    pv_buffer    Pointer to the buffer.
 * \param[in]    u32_size    Size of the buffer.
 * \param[in]    u32_val        value to set in the buffer.
 *
 * \return None.
 */
void set_memory(
                void     *pv_buffer,
                uint32_t u32_size,
                uint32_t u32_val
               );

/**
 * \fn void copy_memory( 
 *                        const void *pv_dst,
 *                        void       *pv_src,
 *                        uint32_t   u32_width
 *                       )
 *
 * \brief Copies a line of memory from source to destination.
 *
 * \details Copies a line of memory from source to destination.
 * 
 * \param[out]    pv_dst        Pointer to the destination buffer.
 * \param[in]    pv_src        Pointer to the source buffer. 
 * \param[in]    u32_width    Line width.
 *
 * \return None.
 */
void copy_memory(
                 void        *pv_dst,             
                 const void *pv_src,                 
                 uint32_t   u32_width
                );

/**
 * \fn int32_t compare_memory( 
 *                              const void     *pv_buf1,
 *                              const void     *pv_buf2,
 *                                    uint32_t u32_width
 *                             )
 *
 * \brief Compares buffer 1 and buffer 2. Returns -1 if they match.
 *
 * \details Compares u32_width number of bytes of pv_buf1 and pv_buf2. The buffers are type cast to unsigned char
 * before comparing. The function returns a -1 if the buffers match, else returns the byte at which they mismatch.
 * 
 * \param[in]    pv_buf1        Pointer to first buffer.
 * \param[in]    pv_buf2        Pointer to second buffer.
 * \param[in]    u32_width    Line width.
 *
 * \return -1 if buffers match, else the byte at which they mismatch.
 */
int32_t compare_memory(
                       const void     *pv_buf1,             
                       const void     *pv_buf2,
                       uint32_t           u32_width
                      );

/**
 * \fn void free_memory(
 *                        void *pv_buffer
 *                       )
 *
 * \brief Function to free the allocated memory.
 *
 * \details The function frees the allocated memory.
 * 
 * \param[in]    pv_buffer    Pointer to the buffer.
 *
 * \return None.
 */
void free_memory(
                 void *pv_buffer
                );

/**
 * \fn ImageData* alloc_image_mem(
 *                                  uint16_t        u16_frame_width_y,
 *                                  uint16_t      u16_frame_height_y,
 *                                  uint16_t      u16_frame_stride_y,
 *                                  CHROMA_FORMAT e_yuv_chroma_format, 
 *                                    uint8_t        u8_packed_UV
 *                                 )
 *
 * \brief Initialises image data buffer.
 *
 * \details Allocates memory for image data structure and also for the Y, U, V buffers. The frame dimensions are 
 * assumed to be MB aligned.
 *
 * \param[in]    u16_frame_width_y    Width of the Y buffer.
 * \param[in]    u16_frame_height_y    Height of the Y buffer.
 * \param[in]    u16_frame_stride_y    Stride of the Y buffer.
 * \param[in]    e_yuv_chroma_format    YUV color format. 
 * \param[in]    u8_packed_UV            UV packed alternatively or V followed by U
 * \return Pointer to image data structure
 */
ImageData* alloc_image_mem(
                           uint16_t      u16_frame_width_y,
                           uint16_t      u16_frame_height_y,
                           uint16_t      u16_frame_stride_y,
                           CHROMA_FORMAT e_yuv_chroma_format,                           
                           uint8_t         u8_packed_UV
                          );



/**
 * \fn int32_t compare_images( 
 *                              const ImageData* p_img1,
 *                              const ImageData* p_img2,
 *                                    uint32_t   *pu32_mismatch_x,
 *                                    uint32_t   *pu32_mismatch_y,
 *                                    uint8_t       *pu8_component
 *                             )  
 *
 * \brief Compares two images and returns the location of mismatch (if any).
 *
 * \details Compares the two images img1 and img2. In case they match, SUCCESS is returned. In case they mismatch, 
 * FAILURE is returned along with the x- and y- location.
 *
 * \param[in]    p_img1            Pointer to image 1.
 * \param[in]    p_img2            Pointer to image 2.
 * \param[out]    pu32_mismatch_x    x co-ordinate of mismatch location.
 * \param[out]    pu32_mismatch_y    y co-ordinate of mismatch location.
 * \param[out]    pu8_component    Y, U or V
 *
 * \return Success (0) or Failure (1)
 */
int32_t compare_images( 
                        const ImageData* p_img1,
                         const ImageData* p_img2,
                              uint32_t   *pu32_mismatch_x,
                              uint32_t   *pu32_mismatch_y,
                             uint8_t    *pu8_component
                       );

/**
 * \fn void free_image_mem(
 *                           ImageData* p_img
 *                          )
 *
 * \brief Deallocates the memory allocated for the image
 *
 * \details Deallocates the memory allocated for the Y,U,V buffers and then deallocates the image data structure.
 *
 * \param[in]    p_img    Pointer to image data structure.
 *
 * \return None
 */
void free_image_mem(
                    ImageData *p_img
                   );

/**
 * \fn void reset_image_mem(
 *                           ImageData* p_img
 *                          )
 *
 * \brief resets the memory allocated for the image to 0
 *
 * \details 
 *
 * \param[in]    p_img    Pointer to image data structure.
 *
 * \return None
 */
void reset_image_mem(
                    ImageData *p_img
                   );



/**
 * \fn void pack_frame(uint8_t *dst, const uint8_t *src, uint16_t width, uint16_t height, uint16_t stride)
 *
 * \brief pack planar format UV data to pseudo planar format
 *
 * \param[in]    dst        byte ptr - pseudo-planar output frame
 * \param[in]    src        byte ptr - planar input frame
 * \param[in]    width    image width of unpacked frame src
 * \param[in]    height    image height of unpacked frame src
 * \param[in]    stride    line stride of unpacked frame src
 *
 * \return        None
 */

void pack_frame(uint8_t *dst, const uint8_t *src, uint16_t width, uint16_t height, uint16_t stride);

/**
 * \fn void unpack_frame(uint8_t *dst, const uint8_t *src, uint16_t width, uint16_t height, uint16_t stride)
 *
 * \brief unpack pseudo planar format UV data to planar format
 *
 * \param[in]    dst        byte ptr - planar output frame
 * \param[in]    src        byte ptr - pseudo planar input frame
 * \param[in]    width    image width of packed frame src
 * \param[in]    height    image height of packed frame src
 * \param[in]    stride    line stride of packed frame src
 *
 * \return        None
 */
void unpack_frame(uint8_t *dst, const uint8_t *src, uint16_t width, uint16_t height, uint16_t stride);

/** FUNCTION DEFINITIONS */



/**
 * \fn int32_t  upSampleSBSImage( 
 *                        ImageData                *p_view0_image,
 *                        ImageData                *p_view1_image,
 *                const   ImageData                *p_muxed_image,
 *                const   RPUFilter                *p_demux_filter,
 *                        uint8_t                    u8_view0_offset,
 *                        uint8_t                    u8_view1_offset,
 *                        uint8_t                    u8_packed_UV            
 *                        )
 *
 * \brief API function to upsample a side-by-side image to view0 and view1.
 *
 * \details Horizontal upsampling function is used to upsample the two halves of a side-by-sie image.
 * 
 * \param[out]   p_view0_image    Pointer to Output View 0 Image Data.
 * \param[out]   p_view1_image    Pointer to Output View 1 Image Data.
 * \param[in]    p_muxed_image    Pointer to Input  SBS Image Data.
 * \param[in]    p_demux_filter   Filter used for upsampling.
 * \param[in]    u8_view0_offset  View 0 Offset.
 * \param[in]    u8_view1_offset  View 1 Offset.
 * \param[in]    u8_packed_UV     Packed or Planar UV data.
 *
 * \return Success (0) or Failure (1)
 */
int32_t  upSampleSBSImage( 
                        ImageData                *p_view0_image,
                        ImageData                *p_view1_image,
                const   ImageData                *p_muxed_image,
                const   RPUFilter                *p_mux_filter,
                        uint8_t                   u8_view0_offset,
                        uint8_t                   u8_view1_offset,
                        uint8_t                   u8_packed_uv            
                        );

/**
 * \fn int32_t  upSampleOUImage( 
 *                        ImageData                *p_view0_image,
 *                        ImageData                *p_view1_image,
 *                const   ImageData                *p_muxed_image,
 *                const   RPUFilter                *p_demux_filter,
 *                        uint8_t                   u8_view0_offset,
 *                        uint8_t                   u8_view1_offset,
 *                        uint8_t                   u8_packed_UV            
 *                        )
 *
 * \brief API function to upsample a over-under to view0 and view1.
 *
 * \details Vertical upsampling function is used to upsample the two halves of a side-by-sie image.
 * 
 * \param[out]   p_view0_image    Pointer to Output View 0 Image Data.
 * \param[out]   p_view1_image    Pointer to Output View 1 Image Data.
 * \param[in]    p_muxed_image    Pointer to Input  OU Image Data.
 * \param[in]    p_demux_filter   Filter used for upsampling.
 * \param[in]    u8_view0_offset  View 0 Offset.
 * \param[in]    u8_view1_offset  View 1 Offset.
 * \param[in]    u8_packed_UV     Packed or Planar UV data.
 *
 * \return Success (0) or Failure (1)
 */
int32_t  upSampleOUImage( 
                        ImageData                *p_view0_image,
                        ImageData                *p_view1_image,
                const   ImageData                *p_muxed_image,
                const   RPUFilter                *p_mux_filter,
                        uint8_t                   u8_view0_offset,
                        uint8_t                   u8_view1_offset,
                        uint8_t                   u8_packed_uv            
                        );

/**
 * \fn int32_t  add_diff_image_with_offset( 
 *                        ImageData               *p_dst_image,
 *                const   ImageData               *p_src_image1,
 *                const   ImageData               *p_src_image2,
 *                        uint8_t                  u8_offset,
 *                        uint8_t                  u8_mode
 *                        );
 *
 * \brief API function to find Sum / Difference of two Images with offset.
 *
 * \details Based on the Mode , function can sum/difference two image apply an offset to get the final image.
 *             if Mode==0 , p_dst_image = p_src_image1 - p_src_image2 + u8_offset
 *             if Mode==1 , p_dst_image = p_src_image1 + p_src_image2 - u8_offset
 * \param[out]    p_dst_image        Pointer to Output  Image Data. 
 * \param[in]    p_src_image1    Pointer to Src1 Image Data.
 * \param[in]    p_src_image2    Pointer to Src2 Image Data.
 * \param[in]    u8_offset        Offset scalar to add/subtract.
 * \param[in]    u8_mode            (1-Sum) , (0-Difference) 
 *
 * \return Success (0) or Failure (1)
 */
int32_t  add_diff_image_with_offset( 
                        ImageData                *p_dst_image,
                const    ImageData                *p_src_image1,
                const    ImageData                *p_src_image2,
                        uint8_t                     u8_offset,
                        uint8_t                     u8_mode
                        );

/**
 *    int32_t pad_image_based_on_delimiter(
 *                      ImageData *p_dst
 *               )
 *
 * \brief Pad Images outside the 2*delimiter boundaries
 *
 * \details Pads the X & Y boundaries of Image Data.
 *
 * \param[in/out]    p_dst    Pointer to destination image data structure.
 *
 * \return None
 */
int32_t pad_image_based_on_delimiter(
                      ImageData *p_dst
               );


/**
 * \fn int32_t  init_demuxing_filter(RPUFilter *p_flt);
 *
 * \brief Initialises the demuxing/upsampling filter
 *
 *
 * \param[out]    p_flt    Pointer to upsampling filter
 *
 * \return Success (0) or Failure (1)
 */
int32_t init_demuxing_filter(RPUFilter *p_flt);



/**
 * int setComponent_to_Dc(
 *                        ImageData    *p_dst,
 *                        uint16_t     u16_component_id,
 *                        uint16_t     u16_dc_value)
 *
 * \brief set a particular component values to dc value specified.
 * 
 * \param[out]    p_dst               Pointer to Image Data
 * \param[in]     u16_component_id    Component Id 0-luma,1-U,2-V;
 * \param[out]    u16_dc_value        Fixed DC value to set;
 *
 * \return SUCESS or FAILURE
 */
int32_t set_component_to_dc(
                        ImageData    *p_dst,
                        uint16_t     u16_component_id,
                        uint16_t     u16_dc_value);

#endif /* _C3D_RPU_KERNEL_API_ */
