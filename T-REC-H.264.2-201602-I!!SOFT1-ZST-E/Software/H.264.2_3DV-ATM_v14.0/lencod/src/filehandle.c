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
 *    filehandle.c
 * \brief
 *    Start and terminate sequences
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Thomas Stockhammer            <stockhammer@ei.tum.de>
 *      - Detlev Marpe                  <marpe@hhi.de>
 ***************************************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "enc_statistics.h"

#include "rtp.h"
#include "annexb.h"
#include "parset.h"
#include "mbuffer.h"


/*!
 ************************************************************************
 * \brief
 *    Error handling procedure. Print error message to stderr and exit
 *    with supplied code.
 * \param text
 *    Error message
 * \param code
 *    Exit code
 ************************************************************************
 */
void error(char *text, int code)
{
  fprintf(stderr, "%s\n", text);
#if EXT3D
  flush_dpb(p_Enc->p_VidText->p_Dpb, &p_Enc->p_InpText->output);
  flush_dpb(p_Enc->p_VidDepth->p_Dpb, &p_Enc->p_InpDepth->output);
#else
  flush_dpb(p_Enc->p_Vid->p_Dpb, &p_Enc->p_Inp->output);
#endif
  exit(code);
}

/*!
 ************************************************************************
 * \brief
 *     This function generates and writes the PPS
 *
 ************************************************************************
 */
int write_PPS(VideoParameters *p_Vid, int len, int PPS_id)
{
  NALU_t *nalu;
  nalu = NULL;
  nalu = GeneratePic_parameter_set_NALU (p_Vid, PPS_id);
  len += p_Vid->WriteNALU (p_Vid, nalu);
  FreeNALU (nalu);

  return len;
}

/*!
 ************************************************************************
 * \brief
 *    This function opens the output files and generates the
 *    appropriate sequence header
 ************************************************************************
 */
int start_sequence(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  int i,len=0, total_pps = (p_Inp->GenerateMultiplePPS) ? 3 : 1;
  NALU_t *nalu;

  switch(p_Inp->of_mode)
  {
    case PAR_OF_ANNEXB:
#if EXT3D
    if(!p_Vid->is_depth)
    {
        OpenAnnexbFile (p_Vid, p_Inp->outfile);
        p_Vid->WriteNALU = WriteAnnexbNALU;
    }
    else
    {
        p_Vid->f_annexb=p_Vid->p_DualVid->f_annexb;
        p_Vid->WriteNALU=WriteAnnexbNALU;
    }
#else
      OpenAnnexbFile (p_Vid, p_Inp->outfile);
      p_Vid->WriteNALU = WriteAnnexbNALU;
#endif
      break;
    case PAR_OF_RTP:
      OpenRTPFile (p_Vid, p_Inp->outfile);
      p_Vid->WriteNALU = WriteRTPNALU;
      break;
    default:
      snprintf(errortext, ET_SIZE, "Output File Mode %d not supported", p_Inp->of_mode);
      error(errortext,1);
  }

  // Access Unit Delimiter NALU
  if ( p_Inp->SendAUD )
  {
    len += Write_AUD_NALU(p_Vid);
  }

  //! As a sequence header, here we write both sequence and picture
  //! parameter sets.  As soon as IDR is implemented, this should go to the
  //! IDR part, as both parsets have to be transmitted as part of an IDR.
  //! An alternative may be to consider this function the IDR start function.

#if EXT3D
  if(!p_Vid->is_depth)
  {
#endif
  nalu = NULL;
  nalu = GenerateSeq_parameter_set_NALU (p_Vid);
  len += p_Vid->WriteNALU (p_Vid, nalu);
  FreeNALU (nalu);
#if EXT3D
  }
#endif

#if EXT3D
  if(p_Inp->NumOfViews>1)
  {
    int bits;
    nalu = NULL;
    nalu = GenerateSubsetSeq_parameter_set_NALU (p_Vid);
    bits = p_Vid->WriteNALU (p_Vid, nalu);
    len += bits;
    //p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = bits;
    p_Vid->p_Stats->bit_3dv_update_info += bits;    // ZTE bug-fix
    FreeNALU (nalu);

    if(p_Vid->is_depth && p_Vid->p_Inp->AcquisitionIdx==0 && p_Inp->ThreeDVCoding && p_Inp->CompatibilityCategory) // @DT: Prevent DPS in MVC+D
    {
      nalu = NULL;
      nalu = GenerateDep_parameter_set_NALU(p_Vid);
      bits = p_Vid->WriteNALU (p_Vid, nalu);
      len += bits;
      p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = bits;
      FreeNALU (nalu);
    }  
  }
  else
  {
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = 0;
  }
#else
#if (MVC_EXTENSION_ENABLE)
  if(p_Inp->num_of_views==2)
  {
    int bits;
    nalu = NULL;
    nalu = GenerateSubsetSeq_parameter_set_NALU (p_Vid);
    bits = p_Vid->WriteNALU (p_Vid, nalu);
    len += bits;
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = bits;
    FreeNALU (nalu);
  }
  else
  {
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = 0;
  }
#endif
#endif

  //! Lets write now the Picture Parameter sets. Output will be equal to the total number of bits spend here.
  for (i=0;i<total_pps;i++)
  {
     len = write_PPS(p_Vid, len, i);
  }

  if (p_Inp->GenerateSEIMessage)
  {
    nalu = NULL;
    nalu = GenerateSEImessage_NALU(p_Inp);
    len += p_Vid->WriteNALU (p_Vid, nalu);
    FreeNALU (nalu);
  }

  p_Vid->p_Stats->bit_ctr_parametersets_n = len;
#if EXT3D
  if(p_Inp->NumOfViews>1)
  {
    //Wenyi>>

    //!<I am not sure this is correct for multi-view(>3),what is your opinion?
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[0] = len - p_Vid->p_Stats->bit_ctr_parametersets_n_v[1];
  }
#else
#if (MVC_EXTENSION_ENABLE)
  if(p_Inp->num_of_views==2)
  {
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[0] = len - p_Vid->p_Stats->bit_ctr_parametersets_n_v[1];
  }
#endif
#endif

  return 0;
}

int end_of_stream(VideoParameters *p_Vid)
{
  int bits;
  NALU_t *nalu;

  nalu = AllocNALU(MAXNALUSIZE);
  nalu->startcodeprefix_len = 4;
  nalu->forbidden_bit       = 0;  
  nalu->nal_reference_idc   = 0;
  nalu->nal_unit_type       = NALU_TYPE_EOSTREAM;
  nalu->len = 0;
  bits = p_Vid->WriteNALU (p_Vid, nalu);
  
  p_Vid->p_Stats->bit_ctr_parametersets += bits;
  FreeNALU (nalu);
  return bits;
}

/*!
 ************************************************************************
 * \brief
 *    This function opens the output files and generates the
 *    appropriate sequence header
 ************************************************************************
 */
int rewrite_paramsets(VideoParameters *p_Vid)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int i,len=0, total_pps = (p_Inp->GenerateMultiplePPS) ? 3 : 1;
  NALU_t *nalu;

  // Access Unit Delimiter NALU
  if ( p_Inp->SendAUD )
  {
    len += Write_AUD_NALU(p_Vid);
  }

  //! As a sequence header, here we write both sequence and picture
  //! parameter sets.  As soon as IDR is implemented, this should go to the
  //! IDR part, as both parsets have to be transmitted as part of an IDR.
  //! An alternative may be to consider this function the IDR start function.

 #if EXT3D
  if(!p_Vid->is_depth)
  {
    nalu = NULL;
    nalu = GenerateSeq_parameter_set_NALU (p_Vid);
    len += p_Vid->WriteNALU (p_Vid, nalu);
    FreeNALU (nalu);
  }

  if(p_Inp->NumOfViews>1)
  {
    int bits;
    nalu = GenerateSubsetSeq_parameter_set_NALU (p_Vid);
    bits = p_Vid->WriteNALU (p_Vid, nalu);
    len += bits;
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = bits;
    FreeNALU (nalu);

    if(p_Vid->is_depth && p_Vid->p_Inp->AcquisitionIdx==0 && p_Inp->ThreeDVCoding && p_Inp->CompatibilityCategory) // @DT: Prevent DPS in MVC+D
    {
      nalu = NULL;
      nalu = GenerateDep_parameter_set_NALU(p_Vid);
      bits = p_Vid->WriteNALU (p_Vid, nalu);
      len += bits;
      p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = bits;
      FreeNALU (nalu);
    }  
  }
#else
  nalu = NULL;
  nalu = GenerateSeq_parameter_set_NALU (p_Vid);
  len += p_Vid->WriteNALU (p_Vid, nalu);
  FreeNALU (nalu);
#if (MVC_EXTENSION_ENABLE)
  if(p_Inp->num_of_views==2)
  {
    int bits;
    nalu = GenerateSubsetSeq_parameter_set_NALU (p_Vid);
    bits = p_Vid->WriteNALU (p_Vid, nalu);
    len += bits;
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[1] = bits;
    FreeNALU (nalu);
  }
#endif
#endif

  //! Lets write now the Picture Parameter sets. Output will be equal to the total number of bits spend here.
  for (i=0;i<total_pps;i++)
  {
    len = write_PPS(p_Vid, len, i);
  }

  if (p_Inp->GenerateSEIMessage)
  {
    nalu = NULL;
    nalu = GenerateSEImessage_NALU(p_Inp);
    len += p_Vid->WriteNALU (p_Vid, nalu);
    FreeNALU (nalu);
  }

  p_Vid->p_Stats->bit_ctr_parametersets_n = len;
#if EXT3D
  if(p_Inp->NumOfViews>1)
  {
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[0] = len - p_Vid->p_Stats->bit_ctr_parametersets_n_v[1];
  }
#else
#if (MVC_EXTENSION_ENABLE)
  if(p_Inp->num_of_views==2)
  {
    p_Vid->p_Stats->bit_ctr_parametersets_n_v[0] = len - p_Vid->p_Stats->bit_ctr_parametersets_n_v[1];
  }
#endif
#endif
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *     This function terminates the sequence and closes the
 *     output files
 ************************************************************************
 */
int terminate_sequence(VideoParameters *p_Vid, InputParameters *p_Inp)
{
//  Bitstream *currStream;

  // Mainly flushing of everything
  // Add termination symbol, etc.
#if EXT3D
  if(p_Vid->is_depth)
    return 1 ;
#endif

  switch(p_Inp->of_mode)
  {
  case PAR_OF_ANNEXB:
    CloseAnnexbFile(p_Vid);
    break;
  case PAR_OF_RTP:
    CloseRTPFile(p_Vid);
    return 0;
  default:
    snprintf(errortext, ET_SIZE, "Output File Mode %d not supported", p_Inp->of_mode);
    error(errortext,1);
  }
  return 1;   // make lint happy
}

