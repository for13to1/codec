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
 *************************************************************************************
 * \file annexb.c
 *
 * \brief
 *    Annex B Byte Stream format NAL Unit writing routines
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger                  <stewe@cs.tu-berlin.de>
 *************************************************************************************
 */

#include "global.h"
#include "nalucommon.h"

/*!
 ********************************************************************************************
 * \brief
 *    Writes a NALU to the Annex B Byte Stream
 *
 * \return
 *    number of bits written
 *
 ********************************************************************************************
*/
int WriteAnnexbNALU (VideoParameters *p_Vid, NALU_t *n)
{
  int BitsWritten = 0;
  int offset = 0;
  int length = 4;
  static const byte startcode[] = {0,0,0,1};
  byte first_byte;

  assert (n != NULL);
  assert (n->forbidden_bit == 0);
  assert (p_Vid->f_annexb != NULL);
  assert (n->startcodeprefix_len == 3 || n->startcodeprefix_len == 4);

// printf ("WriteAnnexbNALU: writing %d bytes w/ startcode_len %d\n", n->len+1, n->startcodeprefix_len);
  if (n->startcodeprefix_len < 4)
  {
    offset = 1;
    length = 3;
  }

  if ( length != (int) fwrite (startcode+offset, 1, length, p_Vid->f_annexb))
  {
    printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", length);
    exit (-1);
  }

  BitsWritten = (length) << 3;

  first_byte = (unsigned char) ((n->forbidden_bit << 7) | (n->nal_reference_idc << 5) | n->nal_unit_type);

  if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
  {
    printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
    exit (-1);
  }

  BitsWritten += 8;

  // printf ("First Byte %x, nal_ref_idc %x, nal_unit_type %d\n", first_byte, n->nal_reference_idc, n->nal_unit_type);
#if EXT3D
  if(n->nal_unit_type==NALU_TYPE_PREFIX 
    || n->nal_unit_type==NALU_TYPE_SLC_EXT
    ||(n->nal_unit_type==NALU_TYPE_3DV_EXT && n->svc_extension_flag==0))
  {
    int view_id = n->view_id;

    first_byte = (unsigned char) ((n->svc_extension_flag << 7) | (n->non_idr_flag << 6) | n->priority_id);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;

    first_byte = (unsigned char) (view_id >> 2);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;

    first_byte = (unsigned char) (((view_id&3) << 6) | (n->temporal_id << 3) | (n->anchor_pic_flag << 2) | (n->inter_view_flag << 1) | n->reserved_one_bit);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;
  }

  else if (n->nal_unit_type==NALU_TYPE_3DV_EXT && n->svc_extension_flag==1)
  {
    int view_id = n->view_id;
    first_byte = (unsigned char) ((n->svc_extension_flag << 7) | (view_id >> 1));
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;
    first_byte = (unsigned char) (((view_id&1) << 7) | (n->depth_flag << 6) | (n->non_idr_flag << 5) | (n->temporal_id << 2) | (n->anchor_pic_flag << 1) | n->inter_view_flag);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;
  }

#else
#if (MVC_EXTENSION_ENABLE)
  if(n->nal_unit_type==NALU_TYPE_PREFIX || n->nal_unit_type==NALU_TYPE_SLC_EXT)
  {
    int view_id = p_Vid->p_Inp->MVCFlipViews ? !(n->view_id) : n->view_id;

    first_byte = (unsigned char) ((n->svc_extension_flag << 7) | (n->non_idr_flag << 6) | n->priority_id);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;

    first_byte = (unsigned char) (view_id >> 2);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;

    first_byte = (unsigned char) (((view_id&3) << 6) | (n->temporal_id << 3) | (n->anchor_pic_flag << 2) | (n->inter_view_flag << 1) | n->reserved_one_bit);
    if ( 1 != fwrite (&first_byte, 1, 1, p_Vid->f_annexb))
    {
      printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", 1);
      exit (-1);
    }
    BitsWritten += 8;
  }
#endif
#endif

  if (n->len != fwrite (n->buf, 1, n->len, p_Vid->f_annexb))
  {
    printf ("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", n->len);
    exit (-1);
  }
  BitsWritten += n->len * 8;

  fflush (p_Vid->f_annexb);
#if TRACE
  //fprintf (p_Enc->p_trace, "\nAnnex B NALU w/ %s startcode, len %d, forbidden_bit %d, nal_reference_idc %d, nal_unit_type %d\n\n\n",
  //  n->startcodeprefix_len == 4?"long":"short", n->len + 1, n->forbidden_bit, n->nal_reference_idc, n->nal_unit_type);
  fprintf (p_Enc->p_trace, "\nAnnex B NALU w/ %s startcode, len %d,", n->startcodeprefix_len == 4?"long":"short", n->len + 1);
  fprintf (p_Enc->p_trace, "\n                forbidden_bit       %d,", n->forbidden_bit);
  fprintf (p_Enc->p_trace, "\n                nal_reference_idc   %d,", n->nal_reference_idc);
  fprintf (p_Enc->p_trace, "\n                nal_unit_type       %d ", n->nal_unit_type);
#if (MVC_EXTENSION_ENABLE)
  if(n->nal_unit_type==NALU_TYPE_PREFIX || n->nal_unit_type==NALU_TYPE_SLC_EXT)
  {
    fprintf (p_Enc->p_trace, "\n                svc_extension_flag  %d ", n->svc_extension_flag);
    fprintf (p_Enc->p_trace, "\n                non_idr_flag        %d ", n->non_idr_flag);
    fprintf (p_Enc->p_trace, "\n                priority_id         %d ", n->priority_id);

    fprintf (p_Enc->p_trace, "\n                view_id             %d ", p_Vid->p_Inp->MVCFlipViews ? !(n->view_id) : n->view_id);
    fprintf (p_Enc->p_trace, "\n                temporal_id         %d ", n->temporal_id);
    fprintf (p_Enc->p_trace, "\n                anchor_pic_flag     %d ", n->anchor_pic_flag);
    fprintf (p_Enc->p_trace, "\n                inter_view_flag     %d ", n->inter_view_flag);
    fprintf (p_Enc->p_trace, "\n                reserved_one_bit    %d ", n->reserved_one_bit);
  }
#elif EXT3D
  if(n->nal_unit_type==NALU_TYPE_PREFIX 
    || n->nal_unit_type==NALU_TYPE_SLC_EXT
    ||(n->nal_unit_type==NALU_TYPE_3DV_EXT && n->svc_extension_flag==0))
  {
    fprintf (p_Enc->p_trace, "\n                svc_extension_flag  %d ", n->svc_extension_flag);
    fprintf (p_Enc->p_trace, "\n                non_idr_flag        %d ", n->non_idr_flag);
    fprintf (p_Enc->p_trace, "\n                priority_id         %d ", n->priority_id);

    fprintf (p_Enc->p_trace, "\n                view_id             %d ", n->view_id);
    fprintf (p_Enc->p_trace, "\n                temporal_id         %d ", n->temporal_id);
    fprintf (p_Enc->p_trace, "\n                anchor_pic_flag     %d ", n->anchor_pic_flag);
    fprintf (p_Enc->p_trace, "\n                inter_view_flag     %d ", n->inter_view_flag);
    fprintf (p_Enc->p_trace, "\n                reserved_one_bit    %d ", n->reserved_one_bit);
  }
  else if (n->nal_unit_type==NALU_TYPE_3DV_EXT && n->svc_extension_flag==1)
  {
    fprintf (p_Enc->p_trace, "\n                svc_extension_flag  %d ", n->svc_extension_flag);
    fprintf (p_Enc->p_trace, "\n                view_id             %d ", n->view_id);
    fprintf (p_Enc->p_trace, "\n                depth_flag          %d ", n->depth_flag);
    fprintf (p_Enc->p_trace, "\n                non_idr_flag        %d ", n->non_idr_flag);
    fprintf (p_Enc->p_trace, "\n                temporal_id         %d ", n->temporal_id);
    fprintf (p_Enc->p_trace, "\n                anchor_pic_flag     %d ", n->anchor_pic_flag);
    fprintf (p_Enc->p_trace, "\n                inter_view_flag     %d ", n->inter_view_flag);
  }
#endif
  fprintf (p_Enc->p_trace, "\n----------------------------------------------------------------------------\n\n\n");
  fflush (p_Enc->p_trace);
#endif
  return BitsWritten;
}


/*!
 ********************************************************************************************
 * \brief
 *    Opens the output file for the bytestream
 *
 * \param p_Vid
 *    VideoParameters structure
 * \param Filename
 *    The filename of the file to be opened
 *
 * \return
 *    none.  Function terminates the program in case of an error
 *
 ********************************************************************************************
*/
void OpenAnnexbFile (VideoParameters *p_Vid, char *Filename)
{
  if ((p_Vid->f_annexb = fopen (Filename, "wb")) == NULL)
  {
    printf ("Fatal: cannot open Annex B bytestream file '%s', exit (-1)\n", Filename);
    exit (-1);
  }
}


/*!
 ********************************************************************************************
 * \brief
 *    Closes the output bit stream file
 *
 * \return
 *    none.  Funtion trerminates the program in case of an error
 ********************************************************************************************
*/
void CloseAnnexbFile(VideoParameters *p_Vid) 
{
#if EXT3D
  if (p_Vid->f_annexb)
  {
#endif
  if (fclose (p_Vid->f_annexb))
  {
    printf ("Fatal: cannot close Annex B bytestream file, exit (-1)\n");
    exit (-1);
  }
#if EXT3D
  }
#endif
}

