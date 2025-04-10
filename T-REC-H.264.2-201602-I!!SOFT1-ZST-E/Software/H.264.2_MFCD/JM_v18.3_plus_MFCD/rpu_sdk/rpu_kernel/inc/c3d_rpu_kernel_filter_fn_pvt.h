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
* \file  c3d_rpu_kernel_filter_fn_pvt.h
*
* \brief 
*        MFC SDK  rpu internal filtering functions
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_RPU_KERNEL_FILTER_FN_PVT_H_
#define _C3D_RPU_KERNEL_FILTER_FN_PVT_H_

/** FUNCTION DECLARATIONS */




/**
 * \fn int32_t sbs_om_down_filter(
 *                                void        *pv_rpu_handle,
 *                          const img_pel_t   *p_src,
 *                                uint16_t     u16_src_stride,
 *                                img_pel_t   *p_dst,
 *                                uint16_t     u16_dst_stride,
 *                                uint16_t     u16_view_delimiter_sbs,
 *                                uint16_t     u16_view_delimiter_ou,
 *                                uint8_t      u8_packed_uv
 *                         )
 *
 * \brief Calls the appropriate OM vertical downsampling function .
 *
 * \details The SBS input image is vertically downsampled in the 1st stage of OM.Left and right regions are handled
 *  sepearely in this process.
 *
 * \param[in]    pv_rpu_handle           Pointer to the RPU handle. It contains the filter definitions.
 * \param[in]    p_src                   Pointer to the source buffer. It points to the input frame's origin.
 * \param[in]    u16_src_stride          Source buffer stride.
 * \param[out]   p_dst                   Pointer to the destination buffer. It points to the output frame's origin.
 * \param[in]    u16_dst_stride          Destination buffer stride.
  * \param[in]   u16_view_delimiter_sbs  SBS view delimiter.
 * \param[in]    u16_view_delimiter_ou   OU view delimiter.
 * \param[in]    u8_packed_uv            Packed or Unpacked UV.
 *
 * \return Success (0) or Failure (1)
 */
int32_t sbs_om_down_filter(
              void       *pv_rpu_handle,
        const img_pel_t  *p_src,
              uint16_t   u16_src_stride,
              img_pel_t  *p_dst,
              uint16_t   u16_dst_stride,
              uint16_t   u16_view_delimiter_sbs,
              uint16_t   u16_view_delimiter_ou,
              uint8_t    u8_packed_uv
              );

/**
 * \fn int32_t sbs_om_up_filter(
 *                                void        *pv_rpu_handle,
 *                          const img_pel_t   *p_src,
 *                                uint16_t    u16_src_stride,
 *                                img_pel_t   *p_dst,
 *                                uint16_t    u16_dst_stride,
 *                                uint16_t    u16_view_delimiter_sbs,
 *                                uint16_t    u16_view_delimiter_ou,
 *                                uint32_t    u8_packed_uv,
 *                                uint8_t     u8_view_grid_offset_v0,
 *                                uint8_t     u8_view_grid_offset_v1
 *                         )
 *
 * \brief Calls the appropriate OM horizontal upsampling function.
 *
 * \details The vertically downsampled SBS image in OM stage 1 is horizontally upsampled in stage 2 of OM.
 *   Left and right regions are handled sepearely in this process.
 *
 * \param[in]    pv_rpu_handle            Pointer to the RPU handle. It contains the filter definitions.
 * \param[in]    p_src                    Pointer to the source buffer. It points to the input frame's origin.
 * \param[in]    u16_src_stride           Source buffer stride.
 * \param[out]   p_dst                    Pointer to the destination buffer. It points to the output frame's origin.
 * \param[in]    u16_dst_stride           Destination buffer stride.
 * \param[in]    u16_view_delimiter_sbs   SBS view delimiter.
 * \param[in]    u16_view_delimiter_ou    OU view delimiter.
 * \param[in]    u8_packed_uv             Packed or Unpacked UV.
 * \param[in]    u8_view_grid_offset_v0   Horizontal Offset of View 0.
 * \param[in]    u8_view_grid_offset_v1   Horizontal Offset of View 1.
 *
 * \return Success (0) or Failure (1)
 */
int32_t sbs_om_up_filter(
              void       *pv_rpu_handle,
        const img_pel_t  *p_src,
              uint16_t   u16_src_stride,
              img_pel_t  *p_dst,
              uint16_t   u16_dst_stride,
              uint16_t   u16_view_delimiter_sbs,
              uint16_t   u16_view_delimiter_ou,
              uint8_t    u8_packed_uv,
              uint8_t    u8_view_grid_offset_v0,
              uint8_t    u8_view_grid_offset_v1
              );


/**
 * \fn int32_t ou_om_down_filter(
 *                                void        *pv_rpu_handle,
 *                         const  img_pel_t   *p_src,
 *                                uint16_t    u16_src_stride,
 *                                img_pel_t   *p_dst,
 *                                uint16_t    u16_dst_stride,
 *                                uint16_t    u16_view_delimiter_sbs,
 *                                uint16_t    u16_view_delimiter_ou,
 *                                uint32_t    u8_packed_uv
 *                         )
 *
 * \brief Calls the appropriate OM horizontal downsampling function.
 *
 * \details The OU input image is horizontally downsampled in the 1st stage of OM.Left and right regions are handled
 *  sepearely in this process.
 * 
 * \param[in]    pv_rpu_handle            Pointer to the RPU handle. It contains the filter definitions.
 * \param[in]    p_src                    Pointer to the source buffer. It points to the input frame's origin.
 * \param[in]    u16_src_stride           Source buffer stride.
 * \param[out]   p_dst                    Pointer to the destination buffer. It points to the output frame's origin.
 * \param[in]    u16_dst_stride           Destination buffer stride.
 * \param[in]    u16_view_delimiter_sbs   SBS view delimiter.
 * \param[in]    u16_view_delimiter_ou    OU view delimiter.
 * \param[in]    u8_packed_uv             Packed or Unpacked UV.
 *
 * \return Success (1) or Failure (0)
 */
int32_t ou_om_down_filter(
              void        *pv_rpu_handle,
        const img_pel_t   *p_src,
              uint16_t    u16_src_stride,
              img_pel_t   *p_dst,
              uint16_t    u16_dst_stride,
              uint16_t    u16_view_delimiter_sbs,
              uint16_t    u16_view_delimiter_ou,
              uint8_t     u8_packed_uv
                 );
/**
 * \fn int32_t ou_om_up_filter(
 *                                void        *pv_rpu_handle,
 *                         const  img_pel_t   *p_src,
 *                                uint16_t    u16_src_stride,
 *                                img_pel_t   *p_dst,
 *                                uint16_t    u16_dst_stride,
 *                                uint16_t    u16_view_delimiter_sbs,
 *                                uint16_t    u16_view_delimiter_ou,
 *                                uint32_t    u8_packed_uv,
 *                                uint8_t     u8_view_grid_offset_v0,
 *                                uint8_t     u8_view_grid_offset_v1
 *                         )
 *
 * \brief Calls the appropriate OM vertical upsampling function.
 *
 * \details The horizontally downsampled OU image in OM stage 1 is vertically upsampled in stage 2 of OM.
 *   Left and right regions are handled sepearely in this process.
 *
 * \param[in]    pv_rpu_handle            Pointer to the RPU handle. It contains the filter definitions.
 * \param[in]    p_src                    Pointer to the source buffer. It points to the input frame's origin.
 * \param[in]    u16_src_stride           Source buffer stride.
 * \param[out]    p_dst                   Pointer to the destination buffer. It points to the output frame's origin.
 * \param[in]    u16_dst_stride           Destination buffer stride.
 * \param[in]    u16_view_delimiter_sbs   SBS view delimiter.
 * \param[in]    u16_view_delimiter_ou    OU view delimiter.
 * \param[in]    u8_packed_uv             Packed or Unpacked UV.
 * \param[in]    u8_view_grid_offset_v0   Vertical Offset of View 0.
 * \param[in]    u8_view_grid_offset_v1   Vertical Offset of View 1.
 *
 * \return Success (1) or Failure (0)
 */
int32_t ou_om_up_filter(
              void       *pv_rpu_handle,
        const img_pel_t  *p_src,
              uint16_t   u16_src_stride,
              img_pel_t  *p_dst,
              uint16_t   u16_dst_stride,
              uint16_t   u16_view_delimiter_sbs,
              uint16_t   u16_view_delimiter_ou,
              uint8_t    u8_packed_uv,
              uint8_t    u8_view_grid_offset_v0,
              uint8_t    u8_view_grid_offset_v1
                 );
/**
 * \fn void om_rpu_downsample_vertical_filter(
  *                                    const  RPUFilter         *p_flt,
 *                                     const  RPUFilterParams   *p_filter_params,
 *                                     const  img_pel_t         *p_in,
 *                                            uint16_t           u16_input_stride,
 *                                            img_pel_t         *p_out,
 *                                            uint16_t           u16_output_stride, 
 *                                            uint8_t            u8_packed_UV
 *                                    )
 *
 * \brief  OM Vertical downsampling function.
 *
 * \details 
 *
 
 * \param[in]    p_flt                    Pointer to filter definition. This is typecast to 1D filter.
 * \param[in]    p_filter_params          Pointer to filter params. This contains spatial information about the center and 
 *                                        boundary regions of the partition. 
 * \param[in]    p_in                     Pointer to the input buffer. p_in points to the start of partition.
 * \param[in]    u16_input_stride         Input buffer stride.
 * \param[in]    p_out                    Pointer to the output buffer.
 * \param[in]    u16_output_stride        Output buffer stride. 
 * \param[in]    u8_packed_uv             Packed or Unpacked UV.

 *
 * \return None
 */
void om_rpu_downsample_vertical_filter(                                    
          const RPUFilter      *p_flt,
          const RPUFilterParams *p_filter_params,
          const img_pel_t       *p_in,
                uint16_t        u16_input_stride,
                img_pel_t       *p_out,
                uint16_t        u16_output_stride,
                uint8_t         u8_packed_UV
                             );



/**
 * \fn void om_rpu_downsample_horizontal_filter(
 *                                     const  RPUFilter         *p_flt,
 *                                     const  RPUFilterParams   *p_filter_params,
 *                                     const  img_pel_t         *p_in,
 *                                            uint16_t           u16_input_stride,
 *                                            img_pel_t         *p_out,
 *                                            uint16_t           u16_output_stride, 
 *                                            uint8_t            u8_packed_UV
 *                                    )
 *
 * \brief   OM Horizontal downsampling function.
 *
 * \details 
 *
 * \param[in]    p_flt                    Pointer to filter definition. This is typecast to 1D filter.
 * \param[in]    p_filter_params          Pointer to filter params. This contains spatial information about the center and 
 *                                        boundary regions of the partition. 
 * \param[in]    p_in                     Pointer to the input buffer. p_in points to the start of partition.
 * \param[in]    u16_input_stride         Input buffer stride.
 * \param[in]    p_out                    Pointer to the output buffer.
 * \param[in]    u16_output_stride        Output buffer stride. 
 * \param[in]    u8_packed_uv             Packed or Unpacked UV.
 * \return None
 */
void om_rpu_downsample_horizontal_filter(
        const RPUFilter         *p_flt,
        const RPUFilterParams   *p_filter_params,
        const img_pel_t         *p_in,
              uint16_t           u16_input_stride,
              img_pel_t         *p_out,
              uint16_t           u16_output_stride,
              uint8_t            u8_packed_UV
                             );



/**
 * \fn void om_rpu_upsample_horizontal_filter(
 *                                     const  RPUFilter         *p_flt,
 *                                     const  RPUFilterParams   *p_filter_params,
 *                                     const  img_pel_t         *p_in,
 *                                            uint16_t           u16_input_stride,
 *                                            img_pel_t         *p_out,
 *                                            uint16_t           u16_output_stride, 
 *                                            uint8_t            u8_packed_UV, 
 *                                    )
 *
 * \brief OM RPU Horizontal Upsampling function.
 *
 * \details 
 *
 * \param[in]    p_flt                    Pointer to filter definition. This is typecast to 1D filter.
 * \param[in]    p_filter_params          Pointer to filter params. This contains spatial information about the center and 
 *                                        boundary regions of the partition. 
 * \param[in]    p_in                     Pointer to the input buffer. p_in points to the start of partition.
 * \param[in]    u16_input_stride         Input buffer stride.
 * \param[in]    p_out                    Pointer to the output buffer.
 * \param[in]    u16_output_stride        Output buffer stride. 
 * \param[in]    u8_packed_uv             Packed or Unpacked UV.
 *
 * \return None
 */
void om_rpu_upsample_horizontal_filter(
        const RPUFilter         *p_flt,
        const RPUFilterParams   *p_filter_params,
        const img_pel_t         *p_in,
              uint16_t           u16_input_stride,
              img_pel_t         *p_out,
              uint16_t           u16_output_stride,
              uint8_t            u8_packed_UV                                    
                             );



/**
 * \fn void om_rpu_upsample_vertical_filter(
 *                                     const  RPUFilter    *p_flt,
 *                                     const  RPUFilterParams    *p_filter_params,
 *                                     const  img_pel_t        *p_in,
 *                                            uint16_t        u16_input_stride,
 *                                            img_pel_t        *p_out,
 *                                            uint16_t        u16_output_stride
 *                                    )
 *
  * \brief OM RPU Vertical Upsampling function.
 *
 * \details 
 *
 * \param[in]    p_flt                    Pointer to filter definition. This is typecast to 1D filter.
 * \param[in]    p_filter_params          Pointer to filter params. This contains spatial information about the center and 
 *                                        boundary regions of the partition. 
 * \param[in]    p_in                     Pointer to the input buffer. p_in points to the start of partition.
 * \param[in]    u16_input_stride         Input buffer stride.
 * \param[in]    p_out                    Pointer to the output buffer.
 * \param[in]    u16_output_stride        Output buffer stride. 
 *
 * \return None
 */
void om_rpu_upsample_vertical_filter(
                              const RPUFilter    *p_flt,
                              const RPUFilterParams  *p_filter_params,
                              const img_pel_t        *p_in,
                                    uint16_t         u16_input_stride,
                                    img_pel_t        *p_out,
                                    uint16_t         u16_output_stride
                             );












/**
 * \fn int32_t init_om_up_down_filters(
 *                               RPUHandle           *p_rpu_handle 
 *                              )
 *
 * \brief Initialises the OM Up and Down filters
 *
 * \details 
 *
 * \param[out]   p_rpu_handle    Pointer to the RPU handle. 
 *
 * \return Success (0) or Failure (1)
 */
int32_t init_om_up_down_filters(
              RPUHandle            *p_rpu_handle
              );








/**
 * \fn int32_t om_rpu_process(
 *                                 void       *pv_rpu_handle,
 *                           const ImageData *p_bl,
 *                                 ImageData *p_pred_el,
 *                           const RPUData   *p_rpu_data
 *                          )
 *
 * \brief API function to filter the input base layer to obtain the predicted enhancement layer (OM Method).
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
int32_t om_rpu_process(
                          void      *pv_rpu_handle,
                    const ImageData *p_bl,
                          ImageData *p_pred_el,
                    const RPUData   *p_rpu_data
                   );

#endif /* _C3D_RPU_KERNEL_FILTER_FN_PVT_H_ */
