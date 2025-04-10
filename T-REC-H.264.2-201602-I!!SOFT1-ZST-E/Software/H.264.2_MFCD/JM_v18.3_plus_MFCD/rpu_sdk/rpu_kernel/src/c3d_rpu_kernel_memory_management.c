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
* \file  c3d_rpu_kernel_memory_management.c
*
* \brief 
*        MFC SDK  RPU Kernal memeory management functions
*
* \author 
*    Main contributors
*        - Hariharan Ganapathy   (hari.ganapathy@dolby.com)
*
*
*************************************************************************************
*/


/* User defined header files */
#include "c3d_rpu_kernel_api.h"


void* allocate_memory(
                      uint32_t u32_memory_size
                     )
{
    void *pv_buffer = NULL;

    pv_buffer = malloc(u32_memory_size);

    /* Reset to 0 */
    memset(pv_buffer, 0, u32_memory_size);

    return pv_buffer;
} /* End of allocate_memory() function */


void set_memory(
                void     *pv_buffer,
                uint32_t u32_size,
                uint32_t u32_val
               )
{
    memset(pv_buffer, u32_val, u32_size);
} /* End of set_memory() function */

void copy_memory(
                 void        *pv_dst,             
                 const void *pv_src,                 
                 uint32_t   u32_width
                )
{
    memcpy(pv_dst, pv_src, u32_width);    
} /* End of copy_memory() function */


int32_t compare_memory(
                       const void     *pv_buf1,             
                       const void     *pv_buf2,
                             uint32_t u32_width
                      )
{
    int32_t i32_result = -1;
    uint32_t u32_i;

    uint8_t *pi8_buf1 = (uint8_t *) pv_buf1;
    uint8_t *pi8_buf2 = (uint8_t *) pv_buf2;

    for(u32_i = 0; u32_i < u32_width; u32_i++)
    {
        if(pi8_buf1[u32_i] != pi8_buf2[u32_i])
        {
            i32_result = u32_i;
            break;
        } /* if(pi8_buf1[u32_i] != pi8_buf2[u32_i]) */

    } /* for(u32_i = 0; u32_i < u32_width; u32_i++) */

    return i32_result;
} /* End of copy_memory() function */


void free_memory(
                 void *pv_buffer
                )
{
    free(pv_buffer);
} /* End of free_memory() function */
