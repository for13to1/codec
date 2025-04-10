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
 * \file  nalu.c
 *
 * \brief
 *    Decoder NALU support functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger   <stewe@cs.tu-berlin.de>
 ************************************************************************
 */

#include "global.h"
#include "annexb.h"
#include "nalu.h"
#include "memalloc.h"
#include "rtp.h"
#if MVC_EXTENSION_ENABLE||EXT3D
#include "vlc.h"
#endif


/*!
*************************************************************************************
* \brief
*    Initialize bitstream reading structure
*
* \param
*    p_Vid: Imageparameter information
* \param
*    filemode: 
*
*************************************************************************************
*/

void initBitsFile (VideoParameters *p_Vid, int filemode)
{

  switch (filemode)
  {
  case PAR_OF_ANNEXB:
    if ((p_Vid->bitsfile  =  (BitsFile *) calloc(1, sizeof(BitsFile)))==NULL) 
      no_mem_exit("initBitsFile : p_Vid->bitsfile");

    p_Vid->bitsfile->OpenBitsFile     = OpenAnnexBFile;
    p_Vid->bitsfile->CloseBitsFile    = CloseAnnexBFile;
    p_Vid->bitsfile->GetNALU          = GetAnnexbNALU;
    malloc_annex_b(p_Vid);
    break;
  case PAR_OF_RTP:
    if ((p_Vid->bitsfile  =  (BitsFile *) calloc(1, sizeof(BitsFile)))==NULL) 
      no_mem_exit("initBitsFile : p_Vid->bitsfile");

    p_Vid->bitsfile->OpenBitsFile     = OpenRTPFile;
    p_Vid->bitsfile->CloseBitsFile    = CloseRTPFile;
    p_Vid->bitsfile->GetNALU          = GetRTPNALU;
    break;
  default:
    error ("initBitsFile: Unknown bitstream file mode", 255);
    break;
  }    
}

/*!
 *************************************************************************************
 * \brief
 *    Converts a NALU to an RBSP
 *
 * \param
 *    nalu: nalu structure to be filled
 *
 * \return
 *    length of the RBSP in bytes
 *************************************************************************************
 */

static int NALUtoRBSP (NALU_t *nalu)
{
  assert (nalu != NULL);

  nalu->len = EBSPtoRBSP (nalu->buf, nalu->len, 1) ;

  return nalu->len ;
}



/*!
************************************************************************
* \brief
*    Read the next NAL unit (with error handling)
************************************************************************
*/
int read_next_nalu(VideoParameters *p_Vid, NALU_t *nalu)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int ret;

  ret = p_Vid->bitsfile->GetNALU(p_Vid, nalu);

  if (ret < 0)
  {
    snprintf (errortext, ET_SIZE, "Error while getting the NALU in file format %s, exit\n", p_Inp->FileFormat==PAR_OF_ANNEXB?"Annex B":"RTP");
    error (errortext, 601);
  }
  if (ret == 0)
  {
    //FreeNALU(nalu);
    return 0;
  }

  //In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
  //whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
  CheckZeroByteNonVCL(p_Vid, nalu);

  ret = NALUtoRBSP(nalu);

  if (ret < 0)
    error ("Invalid startcode emulation prevention found.", 602);


  // Got a NALU
  if (nalu->forbidden_bit)
  {
    error ("Found NALU with forbidden_bit set, bit error?", 603);
  }

  return nalu->len;
}

void CheckZeroByteNonVCL(VideoParameters *p_Vid, NALU_t *nalu)
{
  int CheckZeroByte=0;

  //This function deals only with non-VCL NAL units
  if(nalu->nal_unit_type>=1&&nalu->nal_unit_type<=5)
    return;

  //for SPS and PPS, zero_byte shall exist
  if(nalu->nal_unit_type==NALU_TYPE_SPS || nalu->nal_unit_type==NALU_TYPE_PPS)
    CheckZeroByte=1;
  //check the possibility of the current NALU to be the start of a new access unit, according to 7.4.1.2.3
  if(nalu->nal_unit_type==NALU_TYPE_AUD  || nalu->nal_unit_type==NALU_TYPE_SPS ||
    nalu->nal_unit_type==NALU_TYPE_PPS || nalu->nal_unit_type==NALU_TYPE_SEI ||
    (nalu->nal_unit_type>=13 && nalu->nal_unit_type<=18))
  {
    if(p_Vid->LastAccessUnitExists)
    {
      p_Vid->LastAccessUnitExists=0;    //deliver the last access unit to decoder
      p_Vid->NALUCount=0;
    }
  }
  p_Vid->NALUCount++;
  //for the first NAL unit in an access unit, zero_byte shall exists
  if(p_Vid->NALUCount==1)
    CheckZeroByte=1;
  if(CheckZeroByte && nalu->startcodeprefix_len==3)   
  {
    printf("Warning: zero_byte shall exist\n");
    //because it is not a very serious problem, we do not exit here
  }
}

void CheckZeroByteVCL(VideoParameters *p_Vid, NALU_t *nalu)
{
  int CheckZeroByte=0;

  //This function deals only with VCL NAL units
  if(!(nalu->nal_unit_type>=NALU_TYPE_SLICE && nalu->nal_unit_type <= NALU_TYPE_IDR))
    return;

  if(p_Vid->LastAccessUnitExists)
  {
    p_Vid->NALUCount=0;
  }
  p_Vid->NALUCount++;
  //the first VCL NAL unit that is the first NAL unit after last VCL NAL unit indicates
  //the start of a new access unit and hence the first NAL unit of the new access unit.           (sounds like a tongue twister :-)
  if(p_Vid->NALUCount == 1)
    CheckZeroByte = 1;
  p_Vid->LastAccessUnitExists = 1;
  if(CheckZeroByte && nalu->startcodeprefix_len==3)
  {
    printf("warning: zero_byte shall exist\n");
    //because it is not a very serious problem, we do not exit here
  }
}

#if MVC_EXTENSION_ENABLE||EXT3D
void nal_unit_header_mvc_extension(NALUnitHeaderMVCExt_t *NaluHeaderMVCExt, Bitstream *s)
{  
   //to be implemented;  
  NaluHeaderMVCExt->non_idr_flag = u_v (1, "non_idr_flag"            , s);
  NaluHeaderMVCExt->priority_id = u_v (6, "priority_id"            , s);
  NaluHeaderMVCExt->view_id = u_v (10, "view_id"                , s);
  NaluHeaderMVCExt->temporal_id = u_v (3, "temporal_id"            , s);
  NaluHeaderMVCExt->anchor_pic_flag = u_v (1, "anchor_pic_flag"        , s);
  NaluHeaderMVCExt->inter_view_flag = u_v (1, "inter_view_flag"        , s);
  NaluHeaderMVCExt->reserved_one_bit = u_v (1, "reserved_one_bit"        , s);
  if(NaluHeaderMVCExt->reserved_one_bit != 1)
  {
    printf("Nalu Header MVC Extension: reserved_one_bit is not 1!\n");
  }
}

#if EXT3D
void nal_unit_header_3dvc_extension(NALUnitHeader3dVCExt_t *NaluHeader3dVCExt, Bitstream *s)
{  
  NaluHeader3dVCExt->view_id         = u_v (8, "view_id"               , s);
  NaluHeader3dVCExt->depth_flag      = u_v (1, "depth_flag"            , s);  
  NaluHeader3dVCExt->non_idr_flag    = u_v (1, "non_idr_flag"          , s);
  NaluHeader3dVCExt->temporal_id     = u_v (3, "temporal_id"           , s);
  NaluHeader3dVCExt->anchor_pic_flag = u_v (1, "anchor_pic_flag"       , s);
  NaluHeader3dVCExt->inter_view_flag = u_v (1, "inter_view_flag"       , s);
}
#endif
void nal_unit_header_svc_extension( void )
{
  //to be implemented for Annex G;
}

void prefix_nal_unit_svc( void )
{
  //to be implemented for Annex G;
}
#endif



