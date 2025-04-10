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
* \file  c3d_generic_filters.h
*
* \brief 
*        MFC SDK  Generic Downsample filtering functions
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


#ifndef _C3D_GENERIC_FILTERS
#define _C3D_GENERIC_FILTERS

/**
 * \fn void vertical_filter_and_downsample(
 *                                     const  RPUFilter       *p_flt,
 *                                     const  RPUFilterParams *p_filter_params,
 *                                     const  img_pel_t       *p_in,
 *                                            uint16_t        u16_input_stride,
 *                                            img_pel_t       *p_out,
 *                                            uint16_t        u16_output_stride, 
 *                                    )
 *
 * \brief vertical_filter_and_downsample.
 *
 * \details * \details Vertically downsample input partition by seperately processing boundary and center pixels.
 *  Independant of packed or unpacked Data
 *
 * \param[in]    p_flt                    Pointer to filter definition. This is typecast to 1D filter.
 * \param[in]    p_filter_params            Pointer to filter params. This contains spatial information about the center and 
 *                                        boundary regions of the partition. 
 * \param[in]    p_in                    Pointer to the input buffer. p_in points to the start of partition.
 * \param[in]    u16_input_stride        Input buffer stride.
 * \param[in]    p_out                    Pointer to the output buffer.
 * \param[in]    u16_output_stride        Output buffer stride. 
 *
 * \return None
 */
void vertical_filter_and_downsample(                                    
                              const RPUFilter    *p_flt,
                              const RPUFilterParams    *p_filter_params,
                              const img_pel_t        *p_in,
                                    uint16_t        u16_input_stride,
                                    img_pel_t        *p_out,
                                    uint16_t        u16_output_stride
                             );

/**
 * \fn void horizontal_filter_and_downsample(
 *                                     const  RPUFilter    *p_flt,
 *                                     const  RPUFilterParams    *p_filter_params,
 *                                     const  img_pel_t        *p_in,
 *                                            uint16_t        u16_input_stride,
 *                                            img_pel_t        *p_out,
 *                                            uint16_t        u16_output_stride
 *                                    )
 *
 * \brief horizontal_filter_and_downsample.
 *
 * \details Filters the input partition with the given 1D filter to obtain the output. It is assumed that the filter is
 * symmetric. The filter processes one line at a time. It calls functions to filter the left, center and right columns.
 * The left and right columns have leading and trailing edges, respectively. Duplication of data is needed for 
 * processing these columns. However, the center column doesn't need any duplication. It has all the required data.
 *
 * \param[in]    p_flt                    Pointer to filter definition. This is typecast to 1D filter.
 * \param[in]    p_filter_params            Pointer to filter params. This contains spatial information about the center and 
 *                                        boundary regions of the partition. 
 * \param[in]    p_in                    Pointer to the input buffer. p_in points to the start of partition.
 * \param[in]    u16_input_stride        Input buffer stride.
 * \param[in]    p_out                    Pointer to the output buffer.
 * \param[in]    u16_output_stride        Output buffer stride. 
 *
 * \return None
 */
void horizontal_filter_and_downsample(                                    
                              const RPUFilter    *p_flt,
                              const RPUFilterParams    *p_filter_params,
                              const img_pel_t        *p_in,
                                    uint16_t        u16_input_stride,
                                    img_pel_t        *p_out,
                                    uint16_t        u16_output_stride,
                                    uint8_t            u8_packed_UV
                             );

#endif /* _C3D_GENERIC_FILTERS */
