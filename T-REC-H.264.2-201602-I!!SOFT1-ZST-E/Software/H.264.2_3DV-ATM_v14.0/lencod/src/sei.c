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
 *  \file
 *     sei.c
 *  \brief
 *     implementation of SEI related functions
 *  \author(s)
 *      - Dong Tian                             <tian@cs.tut.fi>
 *      - Athanasios Leontaris                  <aleon@dolby.com>  
 *
 ************************************************************************
 */

#include "global.h"
#include "memalloc.h"
#include "rtp.h"
#include "mbuffer.h"
#include "sei.h"
#include "vlc.h"
#include "header.h"

#if EXT3D
#include "configfile.h"
#endif

static void InitRandomAccess           (SEIParameters *p_SEI);
static void CloseRandomAccess          (SEIParameters *p_SEI);
static void InitToneMapping            (SEIParameters *p_SEI, InputParameters *p_Inp);
static void CloseToneMapping           (SEIParameters *p_SEI);
static void CloseBufferingPeriod       (SEIParameters *p_SEI);
static void InitPicTiming              (SEIParameters *p_SEI);
static void ClosePicTiming             (SEIParameters *p_SEI);
static void InitDRPMRepetition         (SEIParameters *p_SEI);
static void CloseDRPMRepetition        (SEIParameters *p_SEI);
static void FinalizeDRPMRepetition     (VideoParameters *p_Vid);
static void InitSparePicture           (SEIParameters *p_SEI);
static void CloseSparePicture          (SEIParameters *p_SEI);
static void FinalizeSpareMBMap         (VideoParameters *p_Vid);
static void InitSubseqChar             (VideoParameters *p_Vid);
static void ClearSubseqCharPayload     (SEIParameters *p_SEI);
static void CloseSubseqChar            (SEIParameters *p_SEI);
static void InitSubseqLayerInfo        (SEIParameters *p_SEI);
static void InitPanScanRectInfo        (SEIParameters *p_SEI);
static void ClearPanScanRectInfoPayload(SEIParameters *p_SEI);
static void ClosePanScanRectInfo       (SEIParameters *p_SEI);
static void InitUser_data_unregistered (SEIParameters *p_SEI);
static void InitUser_data_registered_itu_t_t35(SEIParameters *p_SEI);
static void ClearUser_data_unregistered(SEIParameters *p_SEI);
static void CloseUser_data_unregistered(SEIParameters *p_SEI);
static void ClearUser_data_registered_itu_t_t35(SEIParameters *p_SEI);
static void CloseUser_data_registered_itu_t_t35(SEIParameters *p_SEI);
static void InitPostFilterHints        (SEIParameters *p_SEI);
static void ClearPostFilterHints       (SEIParameters *p_SEI);
static void ClosePostFilterHints       (SEIParameters *p_SEI);
static void InitFramePackingArrangement(VideoParameters *p_Vid);
static void CloseFramePackingArrangement(SEIParameters *p_SEI);
#if EXT3D
static void InitMultiviewAcquisitionInfo(SEIParameters *p_SEI, VideoParameters *p_Vid);
static void CloseMultiviewAcquisitionInfo(SEIParameters *p_SEI);
static void InitDepthRepresentationInfo(SEIParameters *p_SEI, VideoParameters *p_Vid);
static void CloseDepthRepresentationInfo(SEIParameters *p_SEI);
static void Init3DVCScalableNesting(SEIParameters *p_SEI);
static void Close3DVCScalableNesting(SEIParameters *p_SEI);
static void InitReferenceDisplayInfo(SEIParameters *p_SEI, VideoParameters *p_Vid);
static void CloseReferenceDisplayInfo(SEIParameters *p_SEI);
#endif

void init_sei(SEIParameters *p_SEI)
{
  p_SEI->seiHasTemporal_reference=FALSE;
  p_SEI->seiHasClock_timestamp=FALSE;
  p_SEI->seiHasPanscan_rect=FALSE;
  p_SEI->seiHasHrd_picture=FALSE;
  p_SEI->seiHasFiller_payload=FALSE;
  p_SEI->seiHasUser_data_registered_itu_t_t35=FALSE;
  p_SEI->seiHasUser_data_unregistered=FALSE;
  p_SEI->seiHasRecoveryPoint_info=FALSE;
  p_SEI->seiHasRef_pic_buffer_management_repetition=FALSE;
  p_SEI->seiHasSpare_picture=FALSE;
  p_SEI->seiHasBufferingPeriod_info=FALSE;
  p_SEI->seiHasPicTiming_info=FALSE;
  p_SEI->seiHasSceneInformation=FALSE;
  p_SEI->seiHasSubseq_information=FALSE;
  p_SEI->seiHasSubseq_layer_characteristics=FALSE;
  p_SEI->seiHasSubseq_characteristics=FALSE;
  p_SEI->seiHasTone_mapping=FALSE;
  p_SEI->seiHasPostFilterHints_info=FALSE;
  p_SEI->seiHasDRPMRepetition_info=FALSE;
  p_SEI->seiHasSparePicture = FALSE;
  p_SEI->seiHasSubseqChar = FALSE;
  p_SEI->seiHasSubseqInfo = FALSE;
  p_SEI->seiHasSubseqLayerInfo = FALSE;
  p_SEI->seiHasPanScanRectInfo = FALSE;
#if EXT3D
  p_SEI->seiHasMultiviewAcquisitionInfo = FALSE;
  p_SEI->seiHas3DVCScalableNesting = FALSE;
  p_SEI->seiHasDepthRepresentationInfo = FALSE;
  p_SEI->seiHasReferenceDisplayInfo = FALSE;
#endif
}

/*
 ************************************************************************
 *  \basic functions on supplemental enhancement information
 *  \brief
 *     The implementations are based on FCD
 ************************************************************************
 */
void InitSEIMessages(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  int i;
  for (i=0; i<2; i++)
  {
    p_SEI->sei_message[i].data = malloc(MAXRTPPAYLOADLEN);
    if( p_SEI->sei_message[i].data == NULL ) no_mem_exit("InitSEIMessages: sei_message[i].data");
    p_SEI->sei_message[i].subPacketType = SEI_PACKET_TYPE;
    clear_sei_message(p_SEI, i);
  }

  // init sei messages
  p_SEI->seiSparePicturePayload.data = NULL;
  InitSparePicture(p_SEI);  
  if (p_Inp->NumFramesInELSubSeq != 0)
  {
    InitSubseqLayerInfo(p_SEI);
    InitSubseqChar(p_Vid);
  }
  InitSceneInformation(p_SEI);
  // init panscanrect sei message
  InitPanScanRectInfo(p_SEI);
  // init user_data_unregistered
  InitUser_data_unregistered(p_SEI);
  // init user_data_unregistered
  InitUser_data_registered_itu_t_t35(p_SEI);
  // init user_RandomAccess
  InitRandomAccess(p_SEI);
  // Init tone_mapping
  InitToneMapping(p_SEI, p_Inp);
  // init post_filter_hints
  InitPostFilterHints(p_SEI);
  // init BufferingPeriod
  InitBufferingPeriod(p_Vid);
  // init PicTiming
  InitPicTiming(p_SEI);
  // init DRPM Repetition
  InitDRPMRepetition(p_SEI);
  // init Frame Packing Arrangement
  InitFramePackingArrangement(p_Vid);
#if EXT3D
  // init Depth Representation Info
  InitDepthRepresentationInfo(p_SEI, p_Vid);
  // init Multiview Acquisition Info
  InitMultiviewAcquisitionInfo(p_SEI, p_Vid);
  // init 3DVC Scalable Nesting
  Init3DVCScalableNesting(p_SEI);
  InitReferenceDisplayInfo(p_SEI, p_Vid);
#endif
}

void CloseSEIMessages(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  int i;

  if (p_Inp->NumFramesInELSubSeq != 0)
    CloseSubseqLayerInfo();

  CloseSubseqChar(p_SEI);
  CloseSparePicture(p_SEI);
  CloseSceneInformation(p_SEI);
  ClosePanScanRectInfo(p_SEI);
  CloseUser_data_unregistered(p_SEI);
  CloseUser_data_registered_itu_t_t35(p_SEI);
  CloseRandomAccess(p_SEI);
  CloseToneMapping(p_SEI);
  ClosePostFilterHints(p_SEI);
  CloseBufferingPeriod(p_SEI);
  ClosePicTiming(p_SEI);
  CloseDRPMRepetition(p_SEI);
  CloseFramePackingArrangement(p_SEI);
#if EXT3D
  CloseDepthRepresentationInfo(p_SEI);
  CloseMultiviewAcquisitionInfo(p_SEI);
  Close3DVCScalableNesting(p_SEI);
  CloseReferenceDisplayInfo(p_SEI);
#endif

  for (i=0; i<MAX_LAYER_NUMBER; i++)
  {
    if ( p_SEI->sei_message[i].data ) 
      free( p_SEI->sei_message[i].data );
    p_SEI->sei_message[i].data = NULL;
  }
}

Boolean HaveAggregationSEI(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;

#if EXT3D
  if (p_SEI->sei_message[AGGREGATION_SEI].available)
    return TRUE;

  // New code proposed by Nokia, NOT tested, @DT
//  if (p_SEI->seiHasMultiviewAcquisitionInfo)
//    return TRUE;
//  if (p_SEI->seiHasDepthRepresentationInfo)
//    return TRUE;

//   
//     // @DT: The following code seems need to be kept, originally removed by Nokia macro: NOKIA_DEPTH_SEI_C0162
//     //      TODO: Double check with Nokia
//   
//
#else
  
  if (p_SEI->sei_message[AGGREGATION_SEI].available && p_Vid->type != B_SLICE)
    return TRUE;
  if (p_SEI->seiHasSubseqInfo)
    return TRUE;
  if (p_SEI->seiHasSubseqLayerInfo && p_Vid->number == 0)
    return TRUE;
  if (p_SEI->seiHasSubseqChar)
    return TRUE;
  if (p_SEI->seiHasSceneInformation)
    return TRUE;
  if (p_SEI->seiHasPanScanRectInfo)
    return TRUE;
  if (p_SEI->seiHasUser_data_unregistered_info)
    return TRUE;
  if (p_SEI->seiHasUser_data_registered_itu_t_t35_info)
    return TRUE;
  if (p_SEI->seiHasRecoveryPoint_info)
    return TRUE;
  if (p_SEI->seiHasTone_mapping)
    return TRUE;
  if (p_SEI->seiHasPostFilterHints_info)
    return TRUE;
  if (p_SEI->seiHasBufferingPeriod_info)
    return TRUE;
  if (p_SEI->seiHasPicTiming_info)
    return TRUE;
  if (p_SEI->seiHasDRPMRepetition_info)
    return TRUE;

#endif

  return FALSE;
//  return p_Inp->SparePictureOption && ( seiHasSpare_picture || seiHasSubseq_information ||
//    seiHasSubseq_layer_characteristics || seiHasSubseq_characteristics );
}

/*!
 ************************************************************************
 *  \brief
 *     write one sei payload to the sei message
 *  \param p_SEI
 *    SEI message
 *  \param id
 *    0, if this is the normal packet\n
 *    1, if this is a aggregation packet
 *  \param payload
 *    a pointer that point to the sei payload. Note that the bitstream
 *    should have be byte aligned already.
 *  \param payload_size
 *    the size of the sei payload
 *  \param payload_type
 *    the type of the sei payload
 *  \par Output
 *    the content of the sei message (sei_message[id]) is updated.
 ************************************************************************
 */
static void write_sei_message(SEIParameters *p_SEI, int id, byte* payload, int payload_size, int payload_type)
{
  int offset, type, size;
  assert(payload_type >= 0 && payload_type < SEI_MAX_ELEMENTS);

  type = payload_type;
  size = payload_size;
  offset = p_SEI->sei_message[id].payloadSize;

  while ( type > 254 )
  {
    p_SEI->sei_message[id].data[offset++] = 0xFF;
    type = type - 255;
  }
  p_SEI->sei_message[id].data[offset++] = (byte) type;

  while ( size > 254 )
  {
    p_SEI->sei_message[id].data[offset++] = 0xFF;
    size = size - 255;
  }
  p_SEI->sei_message[id].data[offset++] = (byte) size;

  memcpy(p_SEI->sei_message[id].data + offset, payload, payload_size);
  offset += payload_size;

  p_SEI->sei_message[id].payloadSize = offset;
}

/*!
 ************************************************************************
 *  \brief
 *     write rbsp_trailing_bits to the sei message
 *  \param p_SEI
 *    SEI message
 *  \param id
 *    0, if this is the normal packet \n
 *    1, if this is a aggregation packet
 *  \par Output
 *    the content of the sei message is updated and ready for packetisation
 ************************************************************************
 */
static void finalize_sei_message(SEIParameters *p_SEI, int id)
{
  int offset = p_SEI->sei_message[id].payloadSize;

  p_SEI->sei_message[id].data[offset] = 0x80;
  p_SEI->sei_message[id].payloadSize++;

  p_SEI->sei_message[id].available = TRUE;
}

/*!
 ************************************************************************
 *  \brief
 *     empty the sei message buffer
 *  \param p_SEI
 *    the SEI message to be cleared
 *  \param id
 *    0, if this is the normal packet \n
 *    1, if this is a aggregation packet
 *  \par Output
 *    the content of the sei message is cleared and ready for storing new
 *      messages
 ************************************************************************
 */
void clear_sei_message(SEIParameters *p_SEI, int id)
{
  memset( p_SEI->sei_message[id].data, 0, MAXRTPPAYLOADLEN);
  p_SEI->sei_message[id].payloadSize       = 0;
  p_SEI->sei_message[id].available         = FALSE;
}

/*!
 ************************************************************************
 *  \brief
 *     copy the bits from one bitstream buffer to another one
 *  \param dest
 *    pointer to the dest bitstream buffer
 *  \param source
 *    pointer to the source bitstream buffer
 *  \par Output
 *    the content of the dest bitstream is changed.
 ************************************************************************
 */
void AppendTmpbits2Buf( Bitstream* dest, Bitstream* source )
{
  int i, j;
  byte mask;
  int bits_in_last_byte;

  // copy the first bytes in source buffer
  for (i=0; i<source->byte_pos; i++)
  {
    mask = 0x80;
    for (j=0; j<8; j++)
    {
      dest->byte_buf <<= 1;
      if (source->streamBuffer[i] & mask)
        dest->byte_buf |= 1;
      dest->bits_to_go--;
      mask >>= 1;
      if (dest->bits_to_go==0)
      {
        dest->bits_to_go = 8;
        dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
        dest->byte_buf = 0;
      }
    }
  }
  // copy the last byte, there are still (8-source->bits_to_go) bits in the source buffer
  bits_in_last_byte = 8-source->bits_to_go;
  if ( bits_in_last_byte > 0 )
  {
    mask = (byte) (1 << (bits_in_last_byte-1));
    for (j=0; j<bits_in_last_byte; j++)
    {
      dest->byte_buf <<= 1;
      if (source->byte_buf & mask)
        dest->byte_buf |= 1;
      dest->bits_to_go--;
      mask >>= 1;
      if (dest->bits_to_go==0)
      {
        dest->bits_to_go = 8;
        dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
        dest->byte_buf = 0;
      }
    }
  }
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on spare pictures
 *  \brief
 *     implementation of Spare Pictures related functions based on
 *      JVT-D100
 *  \author
 *      Dong Tian                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*!
 ************************************************************************
 *  \brief
 *      Init the global variables for spare picture information
 ************************************************************************
 */
static void InitSparePicture(SEIParameters *p_SEI)
{
  if ( p_SEI->seiSparePicturePayload.data != NULL ) 
    CloseSparePicture(p_SEI);

  p_SEI->seiSparePicturePayload.data = malloc( sizeof(Bitstream) );
  if ( p_SEI->seiSparePicturePayload.data == NULL ) no_mem_exit("InitSparePicture: seiSparePicturePayload.data");
  p_SEI->seiSparePicturePayload.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if ( p_SEI->seiSparePicturePayload.data->streamBuffer == NULL ) no_mem_exit("InitSparePicture: seiSparePicturePayload.data->streamBuffer");
  memset( p_SEI->seiSparePicturePayload.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiSparePicturePayload.num_spare_pics = 0;
  p_SEI->seiSparePicturePayload.target_frame_num = 0;

  p_SEI->seiSparePicturePayload.data->bits_to_go  = 8;
  p_SEI->seiSparePicturePayload.data->byte_pos    = 0;
  p_SEI->seiSparePicturePayload.data->byte_buf    = 0;
}

/*!
 ************************************************************************
 *  \brief
 *      Close the global variables for spare picture information
 ************************************************************************
 */
static void CloseSparePicture(SEIParameters *p_SEI)
{
  if (p_SEI->seiSparePicturePayload.data->streamBuffer)
    free(p_SEI->seiSparePicturePayload.data->streamBuffer);
  p_SEI->seiSparePicturePayload.data->streamBuffer = NULL;
  if (p_SEI->seiSparePicturePayload.data)
    free(p_SEI->seiSparePicturePayload.data);
  p_SEI->seiSparePicturePayload.data = NULL;
  p_SEI->seiSparePicturePayload.num_spare_pics = 0;
  p_SEI->seiSparePicturePayload.target_frame_num = 0;
}

/*!
 ************************************************************************
 *  \brief
 *     Calculate the spare picture info, save the result in map_sp
 *      then compose the spare picture information.
 *  \par Output
 *      the spare picture payload is available in *seiSparePicturePayload*
 *      the syntax elements in the loop (see FCD), excluding the two elements
 *      at the beginning.
 ************************************************************************
 */
void CalculateSparePicture()
{
  /*
  int i, j, tmp, i0, j0, m;
  byte **map_sp;
  int delta_spare_frame_num;
  Bitstream *tmpBitstream;

  int num_of_mb=(p_Vid->height >> 4) * (p_Vid->width >> 4);
  int threshold1 = 16*16*p_Inp->SPDetectionThreshold;
  int threshold2 = num_of_mb * p_Inp->SPPercentageThreshold / 100;
  int ref_area_indicator;
  int CandidateSpareFrameNum, SpareFrameNum;
  int possible_spare_pic_num;

  // define it for debug purpose
  #define WRITE_MAP_IMAGE

#ifdef WRITE_MAP_IMAGE
  byte **y;
  int k;
  FILE* fp;
  static int first = 1;
  char map_file_name[255]="map.yuv";
#endif

  // basic check
  if (fb->picbuf_short[0]->used==0 || fb->picbuf_short[1]->used==0)
  {
#ifdef WRITE_MAP_IMAGE
    fp = fopen( map_file_name, "wb" );
    assert( fp != NULL );
    // write the map image
    for (i=0; i < p_Vid->height; i++)
      for (j=0; j < p_Vid->width; j++)
        fputc(0, fp);

    for (k=0; k < 2; k++)
      for (i=0; i < p_Vid->height >> 1; i++)
        for (j=0; j < p_Vid->width >> 1; j++)
          fputc(128, fp);
    fclose( fp );
#endif
    seiHasSparePicture = FALSE;
    return;
  }
  seiHasSparePicture = TRUE;

  // set the global bitstream memory.
  InitSparePicture();
  seiSparePicturePayload.target_frame_num = p_Vid->number % MAX_FN;
  // init the local bitstream memory.
  tmpBitstream = malloc(sizeof(Bitstream));
  if ( tmpBitstream == NULL ) no_mem_exit("CalculateSparePicture: tmpBitstream");
  tmpBitstream->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if ( tmpBitstream->streamBuffer == NULL ) no_mem_exit("CalculateSparePicture: tmpBitstream->streamBuffer");
  memset( tmpBitstream->streamBuffer, 0, MAXRTPPAYLOADLEN);

#ifdef WRITE_MAP_IMAGE
  if ( first )
  {
    fp = fopen( map_file_name, "wb" );
    first = 0;
  }
  else
    fp = fopen( map_file_name, "ab" );
  get_mem2D(&y, p_Vid->height, p_Vid->width);
#endif
  get_mem2D(&map_sp, p_Vid->height >> 4, p_Vid->width >> 4);

  if (fb->picbuf_short[2]->used!=0) possible_spare_pic_num = 2;
  else possible_spare_pic_num = 1;
  // loop over the spare pictures
  for (m=0; m<possible_spare_pic_num; m++)
  {
    // clear the temporal bitstream buffer
    tmpBitstream->bits_to_go  = 8;
    tmpBitstream->byte_pos    = 0;
    tmpBitstream->byte_buf    = 0;
    memset( tmpBitstream->streamBuffer, 0, MAXRTPPAYLOADLEN);

    // set delta_spare_frame_num
    // the order of the following lines cannot be changed.
    if (m==0)
      CandidateSpareFrameNum = seiSparePicturePayload.target_frame_num - 1; // TargetFrameNum - 1;
    else
      CandidateSpareFrameNum = SpareFrameNum - 1;
    if ( CandidateSpareFrameNum < 0 ) CandidateSpareFrameNum = MAX_FN - 1;
    SpareFrameNum = fb->picbuf_short[m+1]->frame_num_256;
    delta_spare_frame_num = CandidateSpareFrameNum - SpareFrameNum;
    assert( delta_spare_frame_num == 0 );

    // calculate the spare macroblock map of one spare picture
    // the results are stored into map_sp[][]
    for (i=0; i < p_Vid->height >> 4; i++)
      for (j=0; j < p_Vid->width >> 4; j++)
      {
        tmp = 0;
        for (i0=0; i0<16; i0++)
          for (j0=0; j0<16; j0++)
            tmp+=iabs(fb->picbuf_short[m+1]->Refbuf11[(i*16+i0)*p_Vid->width+j*16+j0]-
                       fb->picbuf_short[0]->Refbuf11[(i*16+i0)*p_Vid->width+j*16+j0]);
        tmp = (tmp<=threshold1? 255 : 0);
        map_sp[i][j] = (tmp==0? 1 : 0);
#ifdef WRITE_MAP_IMAGE
//        if (m==0)
        {
        for (i0=0; i0<16; i0++)
          for (j0=0; j0<16; j0++)
            y[i*16+i0][j*16+j0]=tmp;
        }
#endif
      }

    // based on map_sp[][], compose the spare picture information
    // and write the spare picture information to a temp bitstream
    tmp = 0;
    for (i=0; i < p_Vid->height >> 4; i++)
      for (j=0; j < p_Vid->width >> 4; j++)
        if (map_sp[i][j]==0) tmp++;
    if ( tmp > threshold2 )
      ref_area_indicator = 0;
    else if ( !CompressSpareMBMap(p_Vid, map_sp, tmpBitstream) )
      ref_area_indicator = 1;
    else
      ref_area_indicator = 2;

//    printf( "ref_area_indicator = %d\n", ref_area_indicator );

#ifdef WRITE_MAP_IMAGE
    // write the map to a file
//    if (m==0)
    {
      // write the map image
      for (i=0; i < p_Vid->height; i++)
        for (j=0; j < p_Vid->width; j++)
        {
          if ( ref_area_indicator == 0 ) fputc(255, fp);
          else fputc(y[i][j], fp);
        }

      for (k=0; k < 2; k++)
        for (i=0; i < p_Vid->height >> 1; i++)
          for (j=0; j < p_Vid->width >> 1; j++)
            fputc(128, fp);
    }
#endif

    // Finnally, write the current spare picture information to
    // the global variable: seiSparePicturePayload
    ComposeSparePictureMessage(p_SEI, delta_spare_frame_num, ref_area_indicator, tmpBitstream);
    seiSparePicturePayload.num_spare_pics++;
  }  // END for (m=0; m<2; m++)

  free_mem2D( map_sp );
  free( tmpBitstream->streamBuffer );
  free( tmpBitstream );

#ifdef WRITE_MAP_IMAGE
  free_mem2D( y );
  fclose( fp );
#undef WRITE_MAP_IMAGE
#endif
  */
}

/*!
 ************************************************************************
 *  \brief
 *      compose the spare picture information.
 *  \param p_SEI
 *      SEI message 
 *  \param delta_spare_frame_num
 *      see FCD
 *  \param ref_area_indicator
 *      Indicate how to represent the spare mb map
 *  \param tmpBitstream
 *      pointer to a buffer to save the payload
 *  \par Output
 *      bitstream: the composed spare picture payload are
 *        ready to put into the sei_message.
 ************************************************************************
 */
void ComposeSparePictureMessage(SEIParameters *p_SEI, int delta_spare_frame_num, int ref_area_indicator, Bitstream *tmpBitstream)
{
  Bitstream *bitstream = p_SEI->seiSparePicturePayload.data;
  SyntaxElement sym;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  sym.value1 = delta_spare_frame_num;
  writeSyntaxElement2Buf_UVLC(&sym, bitstream);
  sym.value1 = ref_area_indicator;
  writeSyntaxElement2Buf_UVLC(&sym, bitstream);

  AppendTmpbits2Buf( bitstream, tmpBitstream );
}

/*!
 ************************************************************************
 *  \brief
 *      test if the compressed spare mb map will occupy less mem and
 *      fill the payload buffer.
 *  \param p_Vid
 *      Image parameters for current picture coding
 *  \param map_sp
 *      in which the spare picture information are stored.
 *  \param bitstream
 *      pointer to a buffer to save the payload
 *  \return
 *      TRUE: If it is compressed version, \n
 *             FALSE: If it is not compressed.
 ************************************************************************
 */
Boolean CompressSpareMBMap(VideoParameters *p_Vid, unsigned char **map_sp, Bitstream *bitstream)
{
  int j, k;
  int noc, bit0, bitc; // bit1
  SyntaxElement sym;
  int x, y, left, right, bottom, top, directx, directy;

  // this is the size of the uncompressed mb map:
  int size_uncompressed = (p_Vid->height >> 4) * (p_Vid->width >> 4);
  int size_compressed   = 0;
  Boolean ret;

  // initialization
  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;
  noc = 0;
  bit0 = 0;
  //bit1 = 1;
  bitc = bit0;

  // compress the map, the result goes to the temporal bitstream buffer
  x = ( (p_Vid->width >> 4) - 1 ) / 2;
  y = ( (p_Vid->height >> 4) - 1 ) / 2;
  left = right = x;
  top = bottom = y;
  directx = 0;
  directy = 1;
  for (j=0; j<p_Vid->height >> 4; j++)
    for (k=0; k<p_Vid->width >> 4; k++)
    {
      // check current mb
      if ( map_sp[y][x] == bitc ) noc++;
      else
      {
        sym.value1 = noc;
        size_compressed += writeSyntaxElement2Buf_UVLC(&sym, bitstream);    // the return value indicate the num of bits written
        noc=0;
      }
      // go to the next mb:
      if ( directx == -1 && directy == 0 )
      {
        if (x > left) x--;
        else if (x == 0)
        {
          y = bottom + 1;
          bottom++;
          directx = 1;
          directy = 0;
        }
        else if (x == left)
        {
          x--;
          left--;
          directx = 0;
          directy = 1;
        }
      }
      else if ( directx == 1 && directy == 0 )
      {
        if (x < right) x++;
        else if (x == (p_Vid->width >> 4) - 1)
        {
          y = top - 1;
          top--;
          directx = -1;
          directy = 0;
        }
        else if (x == right)
        {
          x++;
          right++;
          directx = 0;
          directy = -1;
        }
      }
      else if ( directx == 0 && directy == -1 )
      {
        if ( y > top) y--;
        else if (y == 0)
        {
          x = left - 1;
          left--;
          directx = 0;
          directy = 1;
        }
        else if (y == top)
        {
          y--;
          top--;
          directx = -1;
          directy = 0;
        }
      }
      else if ( directx == 0 && directy == 1 )
      {
        if (y < bottom) y++;
        else if (y == (p_Vid->height >> 4) - 1)
        {
          x = right+1;
          right++;
          directx = 0;
          directy = -1;
        }
        else if (y == bottom)
        {
          y++;
          bottom++;
          directx = 1;
          directy = 0;
        }
      }
    }
  if (noc!=0)
  {
    sym.value1 = noc;
    size_compressed += writeSyntaxElement2Buf_UVLC(&sym, bitstream);
  }

  ret = (size_compressed<size_uncompressed? TRUE : FALSE);
  if ( !ret ) // overwrite the streambuffer with the original mb map
  {
    // write the mb map to payload bit by bit
    bitstream->byte_buf = 0;
    bitstream->bits_to_go = 8;
    bitstream->byte_pos = 0;
    for (j=0; j<p_Vid->height >> 4; j++)
    {
      for (k=0; k<p_Vid->width >> 4; k++)
      {
        bitstream->byte_buf <<= 1;
        if (map_sp[j][k]) bitstream->byte_buf |= 1;
        bitstream->bits_to_go--;
        if (bitstream->bits_to_go==0)
        {
          bitstream->bits_to_go = 8;
          bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
          bitstream->byte_buf = 0;
        }
      }
    }
  }

  return ret;
}

/*!
 ************************************************************************
 *  \brief
 *      Finalize the spare picture SEI payload.
 *        The spare picture paylaod will be ready for encapsulation, and it
 *        should be called before current picture packetized.
 *  \par Input
 *      seiSparePicturePayload.data: points to the payload starting from
 *        delta_spare_frame_num. (See FCD)
 *  \par Output
 *      seiSparePicturePayload.data is updated, pointing to the whole spare
 *        picture information: spare_picture( PayloadSize ) (See FCD)
 *        Make sure it is byte aligned.
 ************************************************************************
 */
static void FinalizeSpareMBMap(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  int CurrFrameNum = p_Vid->number % MAX_FN;
  int delta_frame_num;
  SyntaxElement sym;
  Bitstream *dest, *source;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  source = p_SEI->seiSparePicturePayload.data;
  dest = malloc(sizeof(Bitstream));
  if ( dest == NULL ) no_mem_exit("FinalizeSpareMBMap: dest");
  dest->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if ( dest->streamBuffer == NULL ) no_mem_exit("FinalizeSpareMBMap: dest->streamBuffer");
  dest->bits_to_go  = 8;
  dest->byte_pos    = 0;
  dest->byte_buf    = 0;
  memset( dest->streamBuffer, 0, MAXRTPPAYLOADLEN);

  //    delta_frame_num
  delta_frame_num = CurrFrameNum - p_SEI->seiSparePicturePayload.target_frame_num;
  if ( delta_frame_num < 0 ) delta_frame_num += MAX_FN;
  sym.value1 = delta_frame_num;
  writeSyntaxElement2Buf_UVLC(&sym, dest);

  // num_spare_pics_minus1
  sym.value1 = p_SEI->seiSparePicturePayload.num_spare_pics - 1;
  writeSyntaxElement2Buf_UVLC(&sym, dest);

  // copy the other bits
  AppendTmpbits2Buf( dest, source);

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiSparePicturePayload.payloadSize = dest->byte_pos;

  // the payload is ready now
  p_SEI->seiSparePicturePayload.data = dest;
  free( source->streamBuffer );
  free( source );
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on subseq information sei messages
 *  \brief
 *      JVT-D098
 *  \author
 *      Dong Tian                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*!
 ************************************************************************
 *  \brief
 *      init subseqence info
 ************************************************************************
 */
void InitSubseqInfo(SEIParameters *p_SEI, int currLayer)
{
  static uint16 id = 0;

  p_SEI->seiHasSubseqInfo = TRUE;
  p_SEI->seiSubseqInfo[currLayer].subseq_layer_num = currLayer;
  p_SEI->seiSubseqInfo[currLayer].subseq_id = id++;
  p_SEI->seiSubseqInfo[currLayer].last_picture_flag = 0;
  p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt = (unsigned int) -1;
  p_SEI->seiSubseqInfo[currLayer].payloadSize = 0;

  p_SEI->seiSubseqInfo[currLayer].data = malloc( sizeof(Bitstream) );
  if ( p_SEI->seiSubseqInfo[currLayer].data == NULL ) no_mem_exit("InitSubseqInfo: p_SEI->seiSubseqInfo[currLayer].data");
  p_SEI->seiSubseqInfo[currLayer].data->streamBuffer = malloc( MAXRTPPAYLOADLEN );
  if ( p_SEI->seiSubseqInfo[currLayer].data->streamBuffer == NULL ) no_mem_exit("InitSubseqInfo: p_SEI->seiSubseqInfo[currLayer].data->streamBuffer");
  p_SEI->seiSubseqInfo[currLayer].data->bits_to_go  = 8;
  p_SEI->seiSubseqInfo[currLayer].data->byte_pos    = 0;
  p_SEI->seiSubseqInfo[currLayer].data->byte_buf    = 0;
  memset( p_SEI->seiSubseqInfo[currLayer].data->streamBuffer, 0, MAXRTPPAYLOADLEN );
}

/*!
 ************************************************************************
 *  \brief
 *      update subsequence info
 ************************************************************************
 */
void UpdateSubseqInfo(VideoParameters *p_Vid, InputParameters *p_Inp, int currLayer)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  if (p_Vid->type != B_SLICE)
  {
    p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt ++;
    p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt = p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt % MAX_FN;
  }

  if ( currLayer == 0 )
  {
    if ( p_Vid->number == p_Inp->no_frames - 1 )
      p_SEI->seiSubseqInfo[currLayer].last_picture_flag = 1;
    else
      p_SEI->seiSubseqInfo[currLayer].last_picture_flag = 0;
  }
  if ( currLayer == 1 )
  {
    if ( (((p_Vid->curr_frm_idx - p_Vid->last_idr_code_order) % (p_Inp->NumFramesInELSubSeq + 1) == 0) && (p_Inp->NumberBFrames != 0) && ((p_Vid->curr_frm_idx - p_Vid->last_idr_code_order) > 0)) || // there are B frames
      (((p_Vid->curr_frm_idx - p_Vid->last_idr_code_order) % (p_Inp->NumFramesInELSubSeq + 1) == p_Inp->NumFramesInELSubSeq) && (p_Inp->NumberBFrames==0))  // there are no B frames
      )
      p_SEI->seiSubseqInfo[currLayer].last_picture_flag = 1;
    else
      p_SEI->seiSubseqInfo[currLayer].last_picture_flag = 0;
  }
}

/*!
 ************************************************************************
 *  \brief
 *      Finalize subseqence info
 ************************************************************************
 */
static void FinalizeSubseqInfo(SEIParameters *p_SEI, int currLayer)
{
  SyntaxElement sym;
  Bitstream *dest = p_SEI->seiSubseqInfo[currLayer].data;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  sym.value1 = p_SEI->seiSubseqInfo[currLayer].subseq_layer_num;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.value1 = p_SEI->seiSubseqInfo[currLayer].subseq_id;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.bitpattern = p_SEI->seiSubseqInfo[currLayer].last_picture_flag;
  sym.len = 1;
  writeSyntaxElement2Buf_Fixed(&sym, dest);
  sym.value1 = p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt;
  writeSyntaxElement2Buf_UVLC(&sym, dest);

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiSubseqInfo[currLayer].payloadSize = dest->byte_pos;

//  printf("layer %d, last picture %d, stored_cnt %d\n", currLayer, p_SEI->seiSubseqInfo[currLayer].last_picture_flag, p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt );
}

/*!
 ************************************************************************
 *  \brief
 *      Clear the payload buffer
 ************************************************************************
 */
static void ClearSubseqInfoPayload(SEIParameters *p_SEI, int currLayer)
{
  p_SEI->seiSubseqInfo[currLayer].data->bits_to_go  = 8;
  p_SEI->seiSubseqInfo[currLayer].data->byte_pos    = 0;
  p_SEI->seiSubseqInfo[currLayer].data->byte_buf    = 0;
  memset( p_SEI->seiSubseqInfo[currLayer].data->streamBuffer, 0, MAXRTPPAYLOADLEN );
  p_SEI->seiSubseqInfo[currLayer].payloadSize = 0;
}

/*!
 ************************************************************************
 *  \brief
 *      Close the global variables for spare picture information
 ************************************************************************
 */
void CloseSubseqInfo(SEIParameters *p_SEI, int currLayer)
{
  p_SEI->seiSubseqInfo[currLayer].stored_frame_cnt = (unsigned int) -1;
  p_SEI->seiSubseqInfo[currLayer].payloadSize = 0;

  free( p_SEI->seiSubseqInfo[currLayer].data->streamBuffer );
  free( p_SEI->seiSubseqInfo[currLayer].data );
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on subseq layer characteristic sei messages
 *  \brief
 *      JVT-D098
 *  \author
 *      Dong Tian                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

/*!
 ************************************************************************
 *  \brief
 *      Init the global variables for spare picture information
 ************************************************************************
 */
static void InitSubseqLayerInfo(SEIParameters *p_SEI)
{
  int i;
  p_SEI->seiHasSubseqLayerInfo = TRUE;
  p_SEI->seiSubseqLayerInfo.layer_number = 0;
  for (i=0; i<MAX_LAYER_NUMBER; i++)
  {
    p_SEI->seiSubseqLayerInfo.bit_rate[i] = 0;
    p_SEI->seiSubseqLayerInfo.frame_rate[i] = 0;
    p_SEI->seiSubseqLayerInfo.layer_number++;
  }
}

/*!
 ************************************************************************
 *  \brief
 *
 ************************************************************************
 */
void CloseSubseqLayerInfo()
{
}

/*!
 ************************************************************************
 *  \brief
 *      Write the data to buffer, which is byte aligned
 ************************************************************************
 */
static void FinalizeSubseqLayerInfo(SEIParameters *p_SEI)
{
  int i, pos;
  pos = 0;
  p_SEI->seiSubseqLayerInfo.payloadSize = 0;
  for (i=0; i<p_SEI->seiSubseqLayerInfo.layer_number; i++)
  {
    *((uint16*)&(p_SEI->seiSubseqLayerInfo.data[pos])) = p_SEI->seiSubseqLayerInfo.bit_rate[i];
    pos += 2;
    *((uint16*)&(p_SEI->seiSubseqLayerInfo.data[pos])) = p_SEI->seiSubseqLayerInfo.frame_rate[i];
    pos += 2;
    p_SEI->seiSubseqLayerInfo.payloadSize += 4;
  }
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on subseq characteristic sei messages
 *  \brief
 *      JVT-D098
 *  \author
 *      Dong Tian                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

void InitSubseqChar(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  p_SEI->seiSubseqChar.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiSubseqChar.data == NULL ) no_mem_exit("InitSubseqChar: p_SEI->seiSubseqChar.data");
  p_SEI->seiSubseqChar.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiSubseqChar.data->streamBuffer == NULL ) no_mem_exit("InitSubseqChar: p_SEI->seiSubseqChar.data->streamBuffer");
  ClearSubseqCharPayload(p_SEI);

  p_SEI->seiSubseqChar.subseq_layer_num = p_Vid->layer;
  p_SEI->seiSubseqChar.subseq_id = p_SEI->seiSubseqInfo[p_Vid->layer].subseq_id;
  p_SEI->seiSubseqChar.duration_flag = 0;
  p_SEI->seiSubseqChar.average_rate_flag = 0;
  p_SEI->seiSubseqChar.num_referenced_subseqs = 0;
}

static void ClearSubseqCharPayload(SEIParameters *p_SEI)
{
  memset( p_SEI->seiSubseqChar.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiSubseqChar.data->bits_to_go  = 8;
  p_SEI->seiSubseqChar.data->byte_pos    = 0;
  p_SEI->seiSubseqChar.data->byte_buf    = 0;
  p_SEI->seiSubseqChar.payloadSize       = 0;

  p_SEI->seiHasSubseqChar = FALSE;
}

void UpdateSubseqChar(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;

  p_SEI->seiSubseqChar.subseq_layer_num = p_Vid->layer;
  p_SEI->seiSubseqChar.subseq_id = p_SEI->seiSubseqInfo[p_Vid->layer].subseq_id;
  p_SEI->seiSubseqChar.duration_flag = 0;
  p_SEI->seiSubseqChar.average_rate_flag = 0;
  p_SEI->seiSubseqChar.average_bit_rate = 100;
  p_SEI->seiSubseqChar.average_frame_rate = 30;
  p_SEI->seiSubseqChar.num_referenced_subseqs = 0;
  p_SEI->seiSubseqChar.ref_subseq_layer_num[0] = 1;
  p_SEI->seiSubseqChar.ref_subseq_id[0] = 2;
  p_SEI->seiSubseqChar.ref_subseq_layer_num[1] = 3;
  p_SEI->seiSubseqChar.ref_subseq_id[1] = 4;

  p_SEI->seiHasSubseqChar = TRUE;
}

static void FinalizeSubseqChar(SEIParameters *p_SEI)
{
  int i;
  SyntaxElement sym;
  Bitstream *dest = p_SEI->seiSubseqChar.data;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  sym.value1 = p_SEI->seiSubseqChar.subseq_layer_num;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.value1 = p_SEI->seiSubseqChar.subseq_id;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.bitpattern = p_SEI->seiSubseqChar.duration_flag;
  sym.len = 1;
  writeSyntaxElement2Buf_Fixed(&sym, dest);
  if ( p_SEI->seiSubseqChar.duration_flag )
  {
    sym.bitpattern = p_SEI->seiSubseqChar.subseq_duration;
    sym.len = 32;
    writeSyntaxElement2Buf_Fixed(&sym, dest);
  }
  sym.bitpattern = p_SEI->seiSubseqChar.average_rate_flag;
  sym.len = 1;
  writeSyntaxElement2Buf_Fixed(&sym, dest);
  if ( p_SEI->seiSubseqChar.average_rate_flag )
  {
    sym.bitpattern = p_SEI->seiSubseqChar.average_bit_rate;
    sym.len = 16;
    writeSyntaxElement2Buf_Fixed(&sym, dest);
    sym.bitpattern = p_SEI->seiSubseqChar.average_frame_rate;
    sym.len = 16;
    writeSyntaxElement2Buf_Fixed(&sym, dest);
  }
  sym.value1 = p_SEI->seiSubseqChar.num_referenced_subseqs;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  for (i=0; i<p_SEI->seiSubseqChar.num_referenced_subseqs; i++)
  {
    sym.value1 = p_SEI->seiSubseqChar.ref_subseq_layer_num[i];
    writeSyntaxElement2Buf_UVLC(&sym, dest);
    sym.value1 = p_SEI->seiSubseqChar.ref_subseq_id[i];
    writeSyntaxElement2Buf_UVLC(&sym, dest);
  }

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiSubseqChar.payloadSize = dest->byte_pos;
}

static void CloseSubseqChar(SEIParameters *p_SEI)
{
  if (p_SEI->seiSubseqChar.data)
  {
    free(p_SEI->seiSubseqChar.data->streamBuffer);
    free(p_SEI->seiSubseqChar.data);
  }
  p_SEI->seiSubseqChar.data = NULL;
}


// JVT-D099
/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on scene information SEI message
 *  \brief
 *      JVT-D099
 *  \author
 *      Ye-Kui Wang                 <wyk@ieee.org>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void InitSceneInformation(SEIParameters *p_SEI)
{
  p_SEI->seiHasSceneInformation = TRUE;

  p_SEI->seiSceneInformation.scene_id = 0;
  p_SEI->seiSceneInformation.scene_transition_type = 0;
  p_SEI->seiSceneInformation.second_scene_id = -1;

  p_SEI->seiSceneInformation.data = malloc( sizeof(Bitstream) );
  if(p_SEI-> seiSceneInformation.data == NULL ) no_mem_exit("InitSceneInformation: seiSceneInformation.data");
  p_SEI->seiSceneInformation.data->streamBuffer = malloc( MAXRTPPAYLOADLEN );
  if( p_SEI->seiSceneInformation.data->streamBuffer == NULL ) no_mem_exit("InitSceneInformation: seiSceneInformation.data->streamBuffer");
  p_SEI->seiSceneInformation.data->bits_to_go  = 8;
  p_SEI->seiSceneInformation.data->byte_pos    = 0;
  p_SEI->seiSceneInformation.data->byte_buf    = 0;
  memset( p_SEI->seiSceneInformation.data->streamBuffer, 0, MAXRTPPAYLOADLEN );
}

void CloseSceneInformation(SEIParameters *p_SEI)
{
  if (p_SEI->seiSceneInformation.data)
  {
    free(p_SEI->seiSceneInformation.data->streamBuffer);
    free(p_SEI->seiSceneInformation.data);
  }
  p_SEI->seiSceneInformation.data = NULL;
}

void FinalizeSceneInformation(SEIParameters *p_SEI)
{
  SyntaxElement sym;
  Bitstream *dest = p_SEI->seiSceneInformation.data;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  sym.bitpattern = p_SEI->seiSceneInformation.scene_id;
  sym.len = 8;
  writeSyntaxElement2Buf_Fixed(&sym, dest);

  sym.value1 = p_SEI->seiSceneInformation.scene_transition_type;
  writeSyntaxElement2Buf_UVLC(&sym, dest);

  if(p_SEI->seiSceneInformation.scene_transition_type > 3)
  {
    sym.bitpattern = p_SEI->seiSceneInformation.second_scene_id;
    sym.len = 8;
    writeSyntaxElement2Buf_Fixed(&sym, dest);
  }

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiSceneInformation.payloadSize = dest->byte_pos;
}

// HasSceneInformation: To include a scene information SEI into the next slice/DP,
//      set HasSceneInformation to be TRUE when calling this function. Otherwise,
//      set HasSceneInformation to be FALSE.
void UpdateSceneInformation(SEIParameters *p_SEI, Boolean HasSceneInformation, int sceneID, int sceneTransType, int secondSceneID)
{
  p_SEI->seiHasSceneInformation = HasSceneInformation;

  assert (sceneID < 256);
  p_SEI->seiSceneInformation.scene_id = sceneID;

  assert (sceneTransType <= 6 );
  p_SEI->seiSceneInformation.scene_transition_type = sceneTransType;

  if(sceneTransType > 3)
  {
    assert (secondSceneID < 256);
    p_SEI->seiSceneInformation.second_scene_id = secondSceneID;
  }
}
// End JVT-D099


/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on Pan Scan messages
 *  \brief
 *      Based on FCD
 *  \author
 *      Shankar Regunathan                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


static void InitPanScanRectInfo(SEIParameters *p_SEI)
{
  p_SEI->seiPanScanRectInfo.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiPanScanRectInfo.data == NULL ) no_mem_exit("InitPanScanRectInfo: p_SEI->seiPanScanRectInfo.data");
  p_SEI->seiPanScanRectInfo.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiPanScanRectInfo.data->streamBuffer == NULL ) no_mem_exit("InitPanScanRectInfo: p_SEI->seiPanScanRectInfo.data->streamBuffer");
  ClearPanScanRectInfoPayload(p_SEI);

  p_SEI->seiPanScanRectInfo.pan_scan_rect_left_offset = 0;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_right_offset = 0;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_top_offset = 0;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_bottom_offset = 0;

}


static void ClearPanScanRectInfoPayload(SEIParameters *p_SEI)
{
  memset( p_SEI->seiPanScanRectInfo.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiPanScanRectInfo.data->bits_to_go  = 8;
  p_SEI->seiPanScanRectInfo.data->byte_pos    = 0;
  p_SEI->seiPanScanRectInfo.data->byte_buf    = 0;
  p_SEI->seiPanScanRectInfo.payloadSize       = 0;

  p_SEI->seiHasPanScanRectInfo = FALSE;
}

void UpdatePanScanRectInfo(SEIParameters *p_SEI)
{
  p_SEI->seiPanScanRectInfo.pan_scan_rect_id = 3;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_left_offset = 10;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_right_offset = 40;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_top_offset = 20;
  p_SEI->seiPanScanRectInfo.pan_scan_rect_bottom_offset =32;
  p_SEI->seiHasPanScanRectInfo = TRUE;
}

void FinalizePanScanRectInfo(SEIParameters *p_SEI)
{
  SyntaxElement sym;
  Bitstream *dest = p_SEI->seiPanScanRectInfo.data;


  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  sym.value1 = p_SEI->seiPanScanRectInfo.pan_scan_rect_id;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.value1 = p_SEI->seiPanScanRectInfo.pan_scan_rect_left_offset;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.value1 = p_SEI->seiPanScanRectInfo.pan_scan_rect_right_offset;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.value1 = p_SEI->seiPanScanRectInfo.pan_scan_rect_top_offset;
  writeSyntaxElement2Buf_UVLC(&sym, dest);
  sym.value1 = p_SEI->seiPanScanRectInfo.pan_scan_rect_bottom_offset;
  writeSyntaxElement2Buf_UVLC(&sym, dest);

// #define PRINT_PAN_SCAN_RECT
#ifdef PRINT_PAN_SCAN_RECT
  printf("Pan Scan Id %d Left %d Right %d Top %d Bottom %d \n", p_SEI->seiPanScanRectInfo.pan_scan_rect_id, p_SEI->seiPanScanRectInfo.pan_scan_rect_left_offset, p_SEI->seiPanScanRectInfo.pan_scan_rect_right_offset, p_SEI->seiPanScanRectInfo.pan_scan_rect_top_offset, p_SEI->seiPanScanRectInfo.pan_scan_rect_bottom_offset);
#endif
#ifdef PRINT_PAN_SCAN_RECT
#undef PRINT_PAN_SCAN_RECT
#endif
  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiPanScanRectInfo.payloadSize = dest->byte_pos;
}


void ClosePanScanRectInfo(SEIParameters *p_SEI)
{
  if (p_SEI->seiPanScanRectInfo.data)
  {
    free(p_SEI->seiPanScanRectInfo.data->streamBuffer);
    free(p_SEI->seiPanScanRectInfo.data);
  }
  p_SEI->seiPanScanRectInfo.data = NULL;
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on arbitrary (unregistered) data
 *  \brief
 *      Based on FCD
 *  \author
 *      Shankar Regunathan                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
static void InitUser_data_unregistered(SEIParameters *p_SEI)
{

  p_SEI->seiUser_data_unregistered.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiUser_data_unregistered.data == NULL ) no_mem_exit("InitUser_data_unregistered: p_SEI->seiUser_data_unregistered.data");
  p_SEI->seiUser_data_unregistered.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiUser_data_unregistered.data->streamBuffer == NULL ) no_mem_exit("InitUser_data_unregistered: p_SEI->seiUser_data_unregistered.data->streamBuffer");
  p_SEI->seiUser_data_unregistered.byte = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiUser_data_unregistered.byte == NULL ) no_mem_exit("InitUser_data_unregistered: p_SEI->seiUser_data_unregistered.byte");
  ClearUser_data_unregistered(p_SEI);

}


static void ClearUser_data_unregistered(SEIParameters *p_SEI)
{
  memset( p_SEI->seiUser_data_unregistered.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiUser_data_unregistered.data->bits_to_go  = 8;
  p_SEI->seiUser_data_unregistered.data->byte_pos    = 0;
  p_SEI->seiUser_data_unregistered.data->byte_buf    = 0;
  p_SEI->seiUser_data_unregistered.payloadSize       = 0;

  memset( p_SEI->seiUser_data_unregistered.byte, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiUser_data_unregistered.total_byte = 0;

  p_SEI->seiHasUser_data_unregistered_info = FALSE;
}

void UpdateUser_data_unregistered(SEIParameters *p_SEI)
{
  int i, temp_data;
  int total_byte;


  total_byte = 7;
  for(i = 0; i < total_byte; i++)
  {
    temp_data = i * 4;
    p_SEI->seiUser_data_unregistered.byte[i] = (char) iClip3(0, 255, temp_data);
  }
  p_SEI->seiUser_data_unregistered.total_byte = total_byte;
}

static void FinalizeUser_data_unregistered(SEIParameters *p_SEI)
{
  int i;
  SyntaxElement sym;
  Bitstream *dest = p_SEI->seiUser_data_unregistered.data;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

// #define PRINT_USER_DATA_UNREGISTERED_INFO
  for( i = 0; i < p_SEI->seiUser_data_unregistered.total_byte; i++)
  {
    sym.bitpattern = p_SEI->seiUser_data_unregistered.byte[i];
    sym.len = 8; // b (8)
    writeSyntaxElement2Buf_Fixed(&sym, dest);
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
    printf("Unreg data payload_byte = %d\n", p_SEI->seiUser_data_unregistered.byte[i]);
#endif
  }
#ifdef PRINT_USER_DATA_UNREGISTERED_INFO
#undef PRINT_USER_DATA_UNREGISTERED_INFO
#endif
  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiUser_data_unregistered.payloadSize = dest->byte_pos;
}

static void CloseUser_data_unregistered(SEIParameters *p_SEI)
{
  if (p_SEI->seiUser_data_unregistered.data)
  {
    free(p_SEI->seiUser_data_unregistered.data->streamBuffer);
    free(p_SEI->seiUser_data_unregistered.data);
  }
  p_SEI->seiUser_data_unregistered.data = NULL;
  if(p_SEI->seiUser_data_unregistered.byte)
  {
    free(p_SEI->seiUser_data_unregistered.byte);
  }
}


/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on registered ITU_T_T35 user data
 *  \brief
 *      Based on FCD
 *  \author
 *      Shankar Regunathan                 <tian@cs.tut.fi>
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void InitUser_data_registered_itu_t_t35(SEIParameters *p_SEI)
{

  p_SEI->seiUser_data_registered_itu_t_t35.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiUser_data_registered_itu_t_t35.data == NULL ) no_mem_exit("InitUser_data_unregistered: p_SEI->seiUser_data_registered_itu_t_t35.data");
  p_SEI->seiUser_data_registered_itu_t_t35.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiUser_data_registered_itu_t_t35.data->streamBuffer == NULL ) no_mem_exit("InitUser_data_unregistered: p_SEI->seiUser_data_registered_itu_t_t35.data->streamBuffer");
  p_SEI->seiUser_data_registered_itu_t_t35.byte = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiUser_data_registered_itu_t_t35.data == NULL ) no_mem_exit("InitUser_data_unregistered: p_SEI->seiUser_data_registered_itu_t_t35.byte");
  ClearUser_data_registered_itu_t_t35(p_SEI);

}


static void ClearUser_data_registered_itu_t_t35(SEIParameters *p_SEI)
{
  memset( p_SEI->seiUser_data_registered_itu_t_t35.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiUser_data_registered_itu_t_t35.data->bits_to_go  = 8;
  p_SEI->seiUser_data_registered_itu_t_t35.data->byte_pos    = 0;
  p_SEI->seiUser_data_registered_itu_t_t35.data->byte_buf    = 0;
  p_SEI->seiUser_data_registered_itu_t_t35.payloadSize       = 0;

  memset( p_SEI->seiUser_data_registered_itu_t_t35.byte, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiUser_data_registered_itu_t_t35.total_byte = 0;
  p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code = 0;
  p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code_extension_byte = 0;

  p_SEI->seiHasUser_data_registered_itu_t_t35_info = FALSE;
}

void UpdateUser_data_registered_itu_t_t35(SEIParameters *p_SEI)
{
  int i, temp_data;
  int total_byte;
  int country_code = 82; // Country_code for India

  if(country_code < 0xFF)
  {
    p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code = country_code;
  }
  else
  {
    p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code = 0xFF;
    p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code_extension_byte = country_code - 0xFF;
  }

  total_byte = 7;
  for(i = 0; i < total_byte; i++)
  {
    temp_data = i * 3;
    p_SEI->seiUser_data_registered_itu_t_t35.byte[i] = (char) iClip3(0, 255, temp_data);
  }
  p_SEI->seiUser_data_registered_itu_t_t35.total_byte = total_byte;
}

static void FinalizeUser_data_registered_itu_t_t35(SEIParameters *p_SEI)
{
  int i;
  SyntaxElement sym;
  Bitstream *dest = p_SEI->seiUser_data_registered_itu_t_t35.data;

  sym.type = SE_HEADER;
  sym.mapping = ue_linfo;

  sym.bitpattern = p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code;
  sym.len = 8;
  writeSyntaxElement2Buf_Fixed(&sym, dest);

// #define PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
  printf(" ITU_T_T35_COUNTRTY_CODE %d \n", p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code);
#endif

  if(p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code == 0xFF)
  {
    sym.bitpattern = p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code_extension_byte;
    sym.len = 8;
    writeSyntaxElement2Buf_Fixed(&sym, dest);
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
    printf(" ITU_T_T35_COUNTRTY_CODE_EXTENSION_BYTE %d \n", p_SEI->seiUser_data_registered_itu_t_t35.itu_t_t35_country_code_extension_byte);
#endif
  }

  for( i = 0; i < p_SEI->seiUser_data_registered_itu_t_t35.total_byte; i++)
  {
    sym.bitpattern = p_SEI->seiUser_data_registered_itu_t_t35.byte[i];
    sym.len = 8; // b (8)
    writeSyntaxElement2Buf_Fixed(&sym, dest);
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
    printf("itu_t_t35 payload_byte = %d\n", p_SEI->seiUser_data_registered_itu_t_t35.byte[i]);
#endif
  }
#ifdef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#undef PRINT_USER_DATA_REGISTERED_ITU_T_T35_INFO
#endif
  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( dest->bits_to_go != 8 )
  {
    (dest->byte_buf) <<= 1;
    dest->byte_buf |= 1;
    dest->bits_to_go--;
    if ( dest->bits_to_go != 0 ) (dest->byte_buf) <<= (dest->bits_to_go);
    dest->bits_to_go = 8;
    dest->streamBuffer[dest->byte_pos++]=dest->byte_buf;
    dest->byte_buf = 0;
  }
  p_SEI->seiUser_data_registered_itu_t_t35.payloadSize = dest->byte_pos;
}

void CloseUser_data_registered_itu_t_t35(SEIParameters *p_SEI)
{
  if (p_SEI->seiUser_data_registered_itu_t_t35.data)
  {
    free(p_SEI->seiUser_data_registered_itu_t_t35.data->streamBuffer);
    free(p_SEI->seiUser_data_registered_itu_t_t35.data);
  }
  p_SEI->seiUser_data_registered_itu_t_t35.data = NULL;
  if(p_SEI->seiUser_data_registered_itu_t_t35.byte)
  {
    free(p_SEI->seiUser_data_registered_itu_t_t35.byte);
  }
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on random access message
 *  \brief
 *      Based on FCD
 *  \author
 *      Shankar Regunathan
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
static void InitRandomAccess(SEIParameters *p_SEI)
{
  p_SEI->seiRecoveryPoint.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiRecoveryPoint.data == NULL ) no_mem_exit("InitRandomAccess: seiRandomAccess.data");
  p_SEI->seiRecoveryPoint.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiRecoveryPoint.data->streamBuffer == NULL ) no_mem_exit("InitRandomAccess: seiRandomAccess.data->streamBuffer");
  ClearRandomAccess(p_SEI);
}


void ClearRandomAccess(SEIParameters *p_SEI)
{
  memset( p_SEI->seiRecoveryPoint.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiRecoveryPoint.data->bits_to_go  = 8;
  p_SEI->seiRecoveryPoint.data->byte_pos    = 0;
  p_SEI->seiRecoveryPoint.data->byte_buf    = 0;
  p_SEI->seiRecoveryPoint.payloadSize       = 0;

  p_SEI->seiRecoveryPoint.recovery_frame_cnt = 0;
  p_SEI->seiRecoveryPoint.broken_link_flag = 0;
  p_SEI->seiRecoveryPoint.exact_match_flag = 0;
  p_SEI->seiRecoveryPoint.changing_slice_group_idc = 0;

  p_SEI->seiHasRecoveryPoint_info = FALSE;
}

void UpdateRandomAccess(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  if(p_Vid->type == I_SLICE)
  {
    p_SEI->seiRecoveryPoint.recovery_frame_cnt = 0;
    p_SEI->seiRecoveryPoint.exact_match_flag = 1;
    p_SEI->seiRecoveryPoint.broken_link_flag = 0;
    p_SEI->seiRecoveryPoint.changing_slice_group_idc = 0;
    p_SEI->seiHasRecoveryPoint_info = TRUE;
  }
  else
  {
    p_SEI->seiHasRecoveryPoint_info = FALSE;
  }
}

static void FinalizeRandomAccess(SEIParameters *p_SEI)
{
  Bitstream *bitstream = p_SEI->seiRecoveryPoint.data;

  ue_v(   "SEI: recovery_frame_cnt",       p_SEI->seiRecoveryPoint.recovery_frame_cnt,       bitstream);
  u_1 (   "SEI: exact_match_flag",         p_SEI->seiRecoveryPoint.exact_match_flag,         bitstream);
  u_1 (   "SEI: broken_link_flag",         p_SEI->seiRecoveryPoint.broken_link_flag,         bitstream);
  u_v (2, "SEI: changing_slice_group_idc", p_SEI->seiRecoveryPoint.changing_slice_group_idc, bitstream);


// #define PRINT_RECOVERY_POINT
#ifdef PRINT_RECOVERY_POINT
  printf(" recovery_frame_cnt %d \n",       p_SEI->seiRecoveryPoint.recovery_frame_cnt);
  printf(" exact_match_flag %d \n",         p_SEI->seiRecoveryPoint.exact_match_flag);
  printf(" broken_link_flag %d \n",         p_SEI->seiRecoveryPoint.broken_link_flag);
  printf(" changing_slice_group_idc %d \n", p_SEI->seiRecoveryPoint.changing_slice_group_idc);
  printf(" %d %d \n", bitstream->byte_pos, bitstream->bits_to_go);

#undef PRINT_RECOVERY_POINT
#endif
  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiRecoveryPoint.payloadSize = bitstream->byte_pos;
}

void CloseRandomAccess(SEIParameters *p_SEI)
{
  if (p_SEI->seiRecoveryPoint.data)
  {
    free(p_SEI->seiRecoveryPoint.data->streamBuffer);
    free(p_SEI->seiRecoveryPoint.data);
  }
  p_SEI->seiRecoveryPoint.data = NULL;
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on HDR tone-mapping messages
 *  \brief
 *      Based on JVT-T060
 *  \author
 *      Jane Zhao, sharp labs of america
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
static int ParseToneMappingConfigFile(SEIParameters *p_SEI, InputParameters *p_Inp, ToneMappingSEI *pSeiToneMapping)
{
  int i;
  int ret;
  FILE* fp;
  char buf[1024];
  unsigned int tmp;

  printf ("Parsing Tone mapping cfg file %s ..........\n\n", p_Inp->ToneMappingFile);
  if ((fp = fopen(p_Inp->ToneMappingFile, "r")) == NULL) 
  {
    fprintf(stderr, "Tone mapping config file %s is not found, disable tone mapping SEI\n", p_Inp->ToneMappingFile);
    p_SEI->seiHasTone_mapping=FALSE;

    return 1;
  }

  //read the tone mapping config file
  while (fscanf(fp, "%s", buf)!=EOF) 
  {
    ret = 1;
    if (strcmp(buf, "tone_map_id")==0) 
    {
      ret = fscanf(fp, " = %ud\n", &(pSeiToneMapping->tone_map_id));
    }
    else if (strcmp(buf, "tone_map_cancel_flag")==0) 
    {
      ret = fscanf(fp, " = %ud\n", &tmp);
      pSeiToneMapping->tone_map_cancel_flag = (unsigned char) (tmp ? 1 : 0);
    }
    else if (strcmp(buf, "tone_map_repetition_period")==0) 
    {
      ret = fscanf(fp, " = %ud\n", &(pSeiToneMapping->tone_map_repetition_period));
    }
    else if (strcmp(buf, "coded_data_bit_depth")==0) 
    {
      ret = fscanf(fp, " = %ud\n", &tmp);
      pSeiToneMapping->coded_data_bit_depth = (unsigned char) tmp;
    }
    else if (strcmp(buf, "sei_bit_depth")==0) 
    {
      ret = fscanf(fp, " = %ud\n", &tmp);
      pSeiToneMapping->sei_bit_depth =  (unsigned char) tmp;
    }
    else if (strcmp(buf, "model_id")==0) 
    {
      ret = fscanf(fp, " = %ud\n", &(pSeiToneMapping->model_id));
    }
    //else if (model_id ==0) 
    else if (strcmp(buf, "min_value")==0) 
    {
      ret = fscanf(fp, " = %d\n", &(pSeiToneMapping->min_value));
    }
    else if (strcmp(buf, "max_value")==0) 
    {
      ret = fscanf(fp, " = %d\n", &(pSeiToneMapping->max_value));
    }
    //(model_id == 1)
    else if (strcmp(buf, "sigmoid_midpoint")==0) 
    {
      ret = fscanf(fp, " = %d\n", &(pSeiToneMapping->sigmoid_midpoint));
    }
    else if (strcmp(buf, "sigmoid_width")==0) 
    {
      ret = fscanf(fp, " = %d\n", &(pSeiToneMapping->sigmoid_width));
    }
    // (model_id == 2) 
    else if (strcmp(buf, "start_of_coded_interval")==0) 
    {
      int max_output_num = 1<<(pSeiToneMapping->sei_bit_depth);
      ret = fscanf(fp, " = ");
      if (ret!=0)
      {
        error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
      }
      for (i=0; i < max_output_num; i++)
      {
        ret = fscanf(fp, "%d\n", &(pSeiToneMapping->start_of_coded_interval[i]));
        if (ret!=1)
        {
          error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
        }
      }
    }
    //(model_id == 3)
    else if (strcmp(buf, "num_pivots")==0) 
    {
      ret = fscanf(fp, " = %d\n", &(pSeiToneMapping->num_pivots));
    }

    else if (strcmp(buf, "coded_pivot_value")==0) 
    {
      ret = fscanf(fp, " = ");
      if (ret!=0)
      {
        error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
      }
      for (i=0; i < pSeiToneMapping->num_pivots; i++)
      {
        ret = fscanf(fp, "%d\n", &(pSeiToneMapping->coded_pivot_value[i]));
        if (ret!=1)
        {
          error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
        }
      }
    }
    else if (strcmp(buf, "sei_pivot_value")==0) 
    {
      ret = fscanf(fp, " = ");
      if (ret!=0)
      {
        error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
      }
      for (i=0; i < pSeiToneMapping->num_pivots; i++)
      {
        ret = fscanf(fp, "%d\n", &(pSeiToneMapping->sei_pivot_value[i]));
        if (ret!=1)
        {
          error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
        }
      }
    }
    else
    {
      // read till the line end 
      if (NULL == fgets(buf, sizeof(buf), fp))
      {
        error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
      }
    }
    if (ret!=1)
    {
      error ("ParseToneMappingConfigFile: error parsing tone mapping config file",500);
    }
  }

  fclose(fp);

  return 0;
}

static void InitToneMapping(SEIParameters *p_SEI, InputParameters *p_Inp) 
{
  if (p_Inp->ToneMappingSEIPresentFlag == 0)
  {
    p_SEI->seiHasTone_mapping = FALSE;
    return;
  }
  else
    p_SEI->seiHasTone_mapping = TRUE;

  p_SEI->seiToneMapping.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiToneMapping.data == NULL ) no_mem_exit("InitToneMapping: seiToneMapping.data");
  p_SEI->seiToneMapping.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiToneMapping.data->streamBuffer == NULL ) no_mem_exit("InitToneMapping: seiToneMapping.data->streamBuffer");
  memset( p_SEI->seiToneMapping.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiToneMapping.data->bits_to_go  = 8;
  p_SEI->seiToneMapping.data->byte_pos    = 0;
  p_SEI->seiToneMapping.data->byte_buf    = 0;
  p_SEI->seiToneMapping.payloadSize       = 0;

  // read tone mapping config from file
  ParseToneMappingConfigFile(p_SEI, p_Inp, &p_SEI->seiToneMapping);
}

static void FinalizeToneMapping(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;

  Bitstream *bitstream = p_SEI->seiToneMapping.data;  
  int i;

  ue_v("SEI: tone_map_id"               , p_SEI->seiToneMapping.tone_map_id,             bitstream);
  u_1("SEI: tone_map_cancel_flag"       , p_SEI->seiToneMapping.tone_map_cancel_flag,    bitstream);

#ifdef PRINT_TONE_MAPPING
  printf("frame %d: Tone-mapping SEI message\n", p_Vid->frame_num);
  printf("tone_map_id = %d\n", p_SEI->seiToneMapping.tone_map_id);
  printf("tone_map_cancel_flag = %d\n", p_SEI->seiToneMapping.tone_map_cancel_flag);
#endif
  if (!p_SEI->seiToneMapping.tone_map_cancel_flag) 
  {
    ue_v(  "SEI: tone_map_repetition_period", p_SEI->seiToneMapping.tone_map_repetition_period, bitstream);
    u_v (8,"SEI: coded_data_bit_depth"      , p_SEI->seiToneMapping.coded_data_bit_depth,       bitstream);
    u_v (8,"SEI: sei_bit_depth"             , p_SEI->seiToneMapping.sei_bit_depth,              bitstream);
    ue_v(  "SEI: model_id"                  , p_SEI->seiToneMapping.model_id,                   bitstream);

#ifdef PRINT_TONE_MAPPING
    printf("tone_map_repetition_period = %d\n", p_SEI->seiToneMapping.tone_map_repetition_period);
    printf("coded_data_bit_depth = %d\n", p_SEI->seiToneMapping.coded_data_bit_depth);
    printf("sei_bit_depth = %d\n", p_SEI->seiToneMapping.sei_bit_depth);
    printf("model_id = %d\n", p_SEI->seiToneMapping.model_id);
#endif
    if (p_SEI->seiToneMapping.model_id == 0) 
    { // linear mapping
      u_v (32,"SEI: min_value", p_SEI->seiToneMapping.min_value, bitstream);
      u_v (32,"SEI: min_value", p_SEI->seiToneMapping.max_value, bitstream);
#ifdef PRINT_TONE_MAPPING
      printf("min_value = %d, max_value = %d\n", p_SEI->seiToneMapping.min_value, p_SEI->seiToneMapping.max_value);
#endif
    }
    else if (p_SEI->seiToneMapping.model_id == 1) 
    { // sigmoidal mapping
      u_v (32,"SEI: sigmoid_midpoint", p_SEI->seiToneMapping.sigmoid_midpoint,   bitstream);
      u_v (32,"SEI: sigmoid_width", p_SEI->seiToneMapping.sigmoid_width,         bitstream);
#ifdef PRINT_TONE_MAPPING
      printf("sigmoid_midpoint = %d, sigmoid_width = %d\n", p_SEI->seiToneMapping.sigmoid_midpoint, p_SEI->seiToneMapping.sigmoid_width);
#endif
    }
    else if (p_SEI->seiToneMapping.model_id == 2) 
    { // user defined table mapping
      int bit_depth_val = 1 << p_SEI->seiToneMapping.sei_bit_depth;
      for (i=0; i<bit_depth_val; i++) 
      {
        u_v((((p_SEI->seiToneMapping.coded_data_bit_depth+7)>>3)<<3), "SEI: start_of_coded_interval", p_SEI->seiToneMapping.start_of_coded_interval[i], bitstream);
#ifdef PRINT_TONE_MAPPING
        //printf("start_of_coded_interval[%d] = %d\n", i, p_SEI->seiToneMapping.start_of_coded_interval[i]);
#endif
      }
    }
    else if (p_SEI->seiToneMapping.model_id == 3) 
    {  // piece-wise linear mapping
      u_v (16,"SEI: num_pivots", p_SEI->seiToneMapping.num_pivots, bitstream);
#ifdef PRINT_TONE_MAPPING
      printf("num_pivots = %d\n", p_SEI->seiToneMapping.num_pivots);
#endif
      for (i=0; i < p_SEI->seiToneMapping.num_pivots; i++) 
      {
        u_v( (((p_SEI->seiToneMapping.coded_data_bit_depth+7)>>3)<<3), "SEI: coded_pivot_value",  p_SEI->seiToneMapping.coded_pivot_value[i], bitstream);
        u_v( (((p_SEI->seiToneMapping.sei_bit_depth+7)>>3)<<3), "SEI: sei_pivot_value",           p_SEI->seiToneMapping.sei_pivot_value[i],   bitstream);
#ifdef PRINT_TONE_MAPPING
        printf("coded_pivot_value[%d] = %d, sei_pivot_value[%d] = %d\n", i, p_SEI->seiToneMapping.coded_pivot_value[i], i, p_SEI->seiToneMapping.sei_pivot_value[i]);
#endif
      }
    }
  } // end !tone_map_cancel_flag

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 ) 
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiToneMapping.payloadSize = bitstream->byte_pos;
}


void UpdateToneMapping(SEIParameters *p_SEI) 
{
  UNREFERENCED_PARAMETER(p_SEI);
  // return;

  // you may manually generate some test case here
}

static void ClearToneMapping(SEIParameters *p_SEI) 
{
  memset( p_SEI->seiToneMapping.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiToneMapping.data->bits_to_go  = 8;
  p_SEI->seiToneMapping.data->byte_pos    = 0;
  p_SEI->seiToneMapping.data->byte_buf    = 0;
  p_SEI->seiToneMapping.payloadSize       = 0;

  p_SEI->seiHasTone_mapping=FALSE;
}

static void CloseToneMapping(SEIParameters *p_SEI) 
{

  if (p_SEI->seiToneMapping.data)
  {
    free(p_SEI->seiToneMapping.data->streamBuffer);
    free(p_SEI->seiToneMapping.data);
  }
  p_SEI->seiToneMapping.data = NULL;
  p_SEI->seiHasTone_mapping = FALSE;
}

/*
 ************************************************************************
 *  \functions on post-filter message
 *  \brief
 *      Based on JVT-U035
 *  \author
 *      Steffen Wittmann <steffen.wittmann@eu.panasonic.com>
 ************************************************************************
 */
static void InitPostFilterHints(SEIParameters *p_SEI)
{
  p_SEI->seiPostFilterHints.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiPostFilterHints.data == NULL ) no_mem_exit("InitPostFilterHints: p_SEI->seiPostFilterHints.data");
  p_SEI->seiPostFilterHints.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiPostFilterHints.data->streamBuffer == NULL ) no_mem_exit("InitPostFilterHints: p_SEI->seiPostFilterHints.data->streamBuffer");
  ClearPostFilterHints(p_SEI);
}

static void ClearPostFilterHints(SEIParameters *p_SEI)
{
  memset( p_SEI->seiPostFilterHints.data->streamBuffer, 0, MAXRTPPAYLOADLEN);
  p_SEI->seiPostFilterHints.data->bits_to_go  = 8;
  p_SEI->seiPostFilterHints.data->byte_pos    = 0;
  p_SEI->seiPostFilterHints.data->byte_buf    = 0;
  p_SEI->seiPostFilterHints.payloadSize       = 0;

  p_SEI->seiPostFilterHints.filter_hint_size_y        = 0;
  p_SEI->seiPostFilterHints.filter_hint_size_x        = 0;
  p_SEI->seiPostFilterHints.filter_hint_type          = 0;
  p_SEI->seiPostFilterHints.additional_extension_flag = 0;
}

void UpdatePostFilterHints(SEIParameters *p_SEI)
{
  unsigned int color_component, cx, cy;
  p_SEI->seiPostFilterHints.filter_hint_type = 0; //define filter_hint_type here
  p_SEI->seiPostFilterHints.filter_hint_size_y = p_SEI->seiPostFilterHints.filter_hint_size_x = 5; //define filter_hint_size here
  get_mem3Dint(&p_SEI->seiPostFilterHints.filter_hint, 3, p_SEI->seiPostFilterHints.filter_hint_size_y, p_SEI->seiPostFilterHints.filter_hint_size_x);

  for (color_component = 0; color_component < 3; color_component ++)
    for (cy = 0; cy < p_SEI->seiPostFilterHints.filter_hint_size_y; cy ++)
      for (cx = 0; cx < p_SEI->seiPostFilterHints.filter_hint_size_x; cx ++)
        p_SEI->seiPostFilterHints.filter_hint[color_component][cy][cx] = 1; //define filter_hint here

  p_SEI->seiPostFilterHints.additional_extension_flag = 0;
}

static void FinalizePostFilterHints(SEIParameters *p_SEI)
{
  Bitstream *bitstream = p_SEI->seiPostFilterHints.data;
  unsigned int color_component, cx, cy;

  ue_v(  "SEI: post_filter_hint_size_y", p_SEI->seiPostFilterHints.filter_hint_size_y, bitstream);
  ue_v(  "SEI: post_filter_hint_size_x", p_SEI->seiPostFilterHints.filter_hint_size_x, bitstream);
  u_v (2,"SEI: post_filter_hint_type",   p_SEI->seiPostFilterHints.filter_hint_type,   bitstream);

  for (color_component = 0; color_component < 3; color_component ++)
    for (cy = 0; cy < p_SEI->seiPostFilterHints.filter_hint_size_y; cy ++)
      for (cx = 0; cx < p_SEI->seiPostFilterHints.filter_hint_size_x; cx ++)
        se_v("SEI: post_filter_hints", p_SEI->seiPostFilterHints.filter_hint[color_component][cy][cx], bitstream);

  u_1 ("SEI: post_filter_additional_extension_flag", p_SEI->seiPostFilterHints.additional_extension_flag, bitstream);

// #define PRINT_POST_FILTER_HINTS
#ifdef PRINT_POST_FILTER_HINTS
  printf(" post_filter_hint_size_y %d \n", p_SEI->seiPostFilterHints.filter_hint_size_y);
  printf(" post_filter_hint_size_x %d \n", p_SEI->seiPostFilterHints.filter_hint_size_x);
  printf(" post_filter_hint_type %d \n",   p_SEI->seiPostFilterHints.filter_hint_type);
  for (color_component = 0; color_component < 3; color_component ++)
    for (cy = 0; cy < p_SEI->seiPostFilterHints.filter_hint_size_y; cy ++)
      for (cx = 0; cx < p_SEI->seiPostFilterHints.filter_hint_size_x; cx ++)
        printf(" post_filter_hint[%d][%d][%d] %d \n", color_component, cy, cx, filter_hint[color_component][cy][cx]);

  printf(" additional_extension_flag %d \n", p_SEI->seiPostFilterHints.additional_extension_flag);

#undef PRINT_POST_FILTER_HINTS
#endif
  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 ) 
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiPostFilterHints.payloadSize = bitstream->byte_pos;
}

static void ClosePostFilterHints(SEIParameters *p_SEI)
{
  if (p_SEI->seiPostFilterHints.data)
  {
    free(p_SEI->seiPostFilterHints.data->streamBuffer);
    free(p_SEI->seiPostFilterHints.data);  
    if (p_SEI->seiPostFilterHints.filter_hint)
      free_mem3Dint(p_SEI->seiPostFilterHints.filter_hint);
  }
  p_SEI->seiPostFilterHints.data = NULL;
}

/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*  \functions to write SEI message into NAL
*  \brief     
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int Write_SEI_NALU(VideoParameters *p_Vid, int len)
{  
  NALU_t *nalu = NULL;
  int RBSPlen = 0;
  //int NALUlen;
  byte *rbsp;

  if (HaveAggregationSEI(p_Vid))
  {
    SEIParameters *p_SEI = p_Vid->p_SEI;

    nalu = AllocNALU(MAXNALUSIZE);
    rbsp = p_SEI->sei_message[AGGREGATION_SEI].data;
    RBSPlen = p_SEI->sei_message[AGGREGATION_SEI].payloadSize;
    //NALUlen = 
    RBSPtoNALU (rbsp, nalu, RBSPlen, NALU_TYPE_SEI, NALU_PRIORITY_DISPOSABLE, 1);
    nalu->startcodeprefix_len = 4;

    len += p_Vid->WriteNALU (p_Vid, nalu);
    FreeNALU (nalu);
  }  

  return len;
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on buffering period SEI message
 *  \brief
 *      Based on final Recommendation
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void InitBufferingPeriod(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  p_SEI->seiBufferingPeriod.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiBufferingPeriod.data == NULL ) 
    no_mem_exit("InitBufferingPeriod: seiBufferingPeriod.data");

  p_SEI->seiBufferingPeriod.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiBufferingPeriod.data->streamBuffer == NULL ) 
    no_mem_exit("InitBufferingPeriod: seiBufferingPeriod.data->streamBuffer");

  ClearBufferingPeriod(p_SEI, p_Vid->active_sps);
}

void ClearBufferingPeriod(SEIParameters *p_SEI, seq_parameter_set_rbsp_t *active_sps)
{
  unsigned int SchedSelIdx;
  memset( p_SEI->seiBufferingPeriod.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiBufferingPeriod.data->bits_to_go  = 8;
  p_SEI->seiBufferingPeriod.data->byte_pos    = 0;
  p_SEI->seiBufferingPeriod.data->byte_buf    = 0;
  p_SEI->seiBufferingPeriod.payloadSize       = 0;

  p_SEI->seiBufferingPeriod.seq_parameter_set_id = active_sps->seq_parameter_set_id;
  if ( active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag )
  {
    for ( SchedSelIdx = 0; SchedSelIdx <= active_sps->vui_seq_parameters.nal_hrd_parameters.cpb_cnt_minus1; SchedSelIdx++ )
    {
      p_SEI->seiBufferingPeriod.nal_initial_cpb_removal_delay[SchedSelIdx] = 0;
      p_SEI->seiBufferingPeriod.nal_initial_cpb_removal_delay_offset[SchedSelIdx] = 0;
    }
  }
  if ( active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag )
  {
    for ( SchedSelIdx = 0; SchedSelIdx <= active_sps->vui_seq_parameters.vcl_hrd_parameters.cpb_cnt_minus1; SchedSelIdx++ )
    {
      p_SEI->seiBufferingPeriod.vcl_initial_cpb_removal_delay[SchedSelIdx] = 0;
      p_SEI->seiBufferingPeriod.vcl_initial_cpb_removal_delay_offset[SchedSelIdx] = 0;
    }
  }

  p_SEI->seiHasBufferingPeriod_info = FALSE;
}

void UpdateBufferingPeriod(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  p_SEI->seiHasBufferingPeriod_info = FALSE;
  UNREFERENCED_PARAMETER(p_Inp);
}

static void FinalizeBufferingPeriod(SEIParameters *p_SEI, seq_parameter_set_rbsp_t *active_sps)
{
  unsigned int SchedSelIdx;
  Bitstream *bitstream = p_SEI->seiBufferingPeriod.data;

  ue_v(   "SEI: seq_parameter_set_id",     p_SEI->seiBufferingPeriod.seq_parameter_set_id,   bitstream);
  if ( active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag )
  {
    for ( SchedSelIdx = 0; SchedSelIdx <= active_sps->vui_seq_parameters.nal_hrd_parameters.cpb_cnt_minus1; SchedSelIdx++ )
    {
      u_v( active_sps->vui_seq_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length_minus1 + 1,
        "SEI: initial_cpb_removal_delay",     p_SEI->seiBufferingPeriod.nal_initial_cpb_removal_delay[SchedSelIdx],   bitstream);
      u_v( active_sps->vui_seq_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length_minus1 + 1,
        "SEI: initial_cpb_removal_delay_offset",     p_SEI->seiBufferingPeriod.nal_initial_cpb_removal_delay_offset[SchedSelIdx],   bitstream);
    }
  }
  if ( active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag )
  {
    for ( SchedSelIdx = 0; SchedSelIdx <= active_sps->vui_seq_parameters.vcl_hrd_parameters.cpb_cnt_minus1; SchedSelIdx++ )
    {
      u_v( active_sps->vui_seq_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length_minus1 + 1,
        "SEI: initial_cpb_removal_delay",     p_SEI->seiBufferingPeriod.vcl_initial_cpb_removal_delay[SchedSelIdx],   bitstream);
      u_v( active_sps->vui_seq_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length_minus1 + 1,
        "SEI: initial_cpb_removal_delay_offset",     p_SEI->seiBufferingPeriod.vcl_initial_cpb_removal_delay_offset[SchedSelIdx],   bitstream);
    }
  }

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiBufferingPeriod.payloadSize = bitstream->byte_pos;
}

static void CloseBufferingPeriod(SEIParameters *p_SEI)
{
  if (p_SEI->seiBufferingPeriod.data)
  {
    free(p_SEI->seiBufferingPeriod.data->streamBuffer);
    free(p_SEI->seiBufferingPeriod.data);
  }
  p_SEI->seiBufferingPeriod.data = NULL;
}

/*
 ************************************************************************
 * \brief
 *    Initialize Picture Timing SEI data 
 ************************************************************************
 */
void InitPicTiming(SEIParameters *p_SEI)
{
  p_SEI->seiPicTiming.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiPicTiming.data == NULL ) 
    no_mem_exit("InitPicTiming: seiPicTiming.data");

  p_SEI->seiPicTiming.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiPicTiming.data->streamBuffer == NULL ) 
    no_mem_exit("InitPicTiming: seiPicTiming.data->streamBuffer");

  ClearPicTiming(p_SEI);
}

/*
 ************************************************************************
 * \brief
 *    Clear Picture Timing SEI data
 ************************************************************************
 */
void ClearPicTiming(SEIParameters *p_SEI)
{
  memset( p_SEI->seiPicTiming.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiPicTiming.data->bits_to_go  = 8;
  p_SEI->seiPicTiming.data->byte_pos    = 0;
  p_SEI->seiPicTiming.data->byte_buf    = 0;
  p_SEI->seiPicTiming.payloadSize       = 0;

  // initialization
  p_SEI->seiPicTiming.cpb_removal_delay = 0;
  p_SEI->seiPicTiming.dpb_output_delay = 0;
  p_SEI->seiPicTiming.pic_struct = 0;
  memset(p_SEI->seiPicTiming.clock_timestamp_flag, 0, MAX_PIC_STRUCT_VALUE * sizeof(Boolean) ); // 0 == FALSE
  p_SEI->seiPicTiming.ct_type = 0;
  p_SEI->seiPicTiming.nuit_field_based_flag = FALSE;
  p_SEI->seiPicTiming.counting_type = 0;
  p_SEI->seiPicTiming.full_timestamp_flag = FALSE;
  p_SEI->seiPicTiming.discontinuity_flag = FALSE;
  p_SEI->seiPicTiming.cnt_dropped_flag = FALSE;
  p_SEI->seiPicTiming.n_frames = 0;
  p_SEI->seiPicTiming.seconds_value = 0;
  p_SEI->seiPicTiming.minutes_value = 0;
  p_SEI->seiPicTiming.hours_value = 0;
  p_SEI->seiPicTiming.seconds_flag = FALSE;
  p_SEI->seiPicTiming.minutes_flag = FALSE;
  p_SEI->seiPicTiming.hours_flag = FALSE;
  p_SEI->seiPicTiming.time_offset = 0;  

  p_SEI->seiHasPicTiming_info = FALSE;
}

/*
 ************************************************************************
 * \brief
 *    Update Picture Timing SEI data
 ************************************************************************
 */
void UpdatePicTiming(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  p_SEI->seiHasPicTiming_info = FALSE;
  UNREFERENCED_PARAMETER(p_Inp);
}

/*
 ************************************************************************
 * \brief
 *    Finalize Picture Timing SEI data
 ************************************************************************
 */
static void FinalizePicTiming(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  Bitstream *bitstream = p_SEI->seiPicTiming.data;
  // CpbDpbDelaysPresentFlag can also be set "by some means not specified in this Recommendation | International Standard"
  Boolean CpbDpbDelaysPresentFlag =  (Boolean) (active_sps->vui_parameters_present_flag
                              && (   (active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag != 0)
                                   ||(active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag != 0)));
  hrd_parameters_t *hrd = NULL;

  assert( active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag || active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag );
  if (active_sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    hrd = &(active_sps->vui_seq_parameters.vcl_hrd_parameters);
  else if (active_sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
    hrd = &(active_sps->vui_seq_parameters.nal_hrd_parameters);
  else // this should never happen
    error ("HRD structures not properly created.",-1);

  if ( CpbDpbDelaysPresentFlag )
  {
    u_v( hrd->cpb_removal_delay_length_minus1 + 1, "SEI: cpb_removal_delay", p_SEI->seiPicTiming.cpb_removal_delay, bitstream);
    u_v( hrd->dpb_output_delay_length_minus1  + 1, "SEI: dpb_output_delay",  p_SEI->seiPicTiming.dpb_output_delay,  bitstream);
  }
  if ( active_sps->vui_seq_parameters.pic_struct_present_flag )
  {
    int NumClockTS = 0, i;
    //int bottom_field_flag = (p_Vid->structure == BOTTOM_FIELD);

    u_v( 4, "SEI: pic_struct", p_SEI->seiPicTiming.pic_struct, bitstream);
    // interpret pic_struct
    switch( p_SEI->seiPicTiming.pic_struct )
    {
    case 0:
    default:
      // frame
      assert( p_Vid->fld_flag == FALSE );
      NumClockTS = 1;
      break;
    case 1:
      // top field
      //assert( (p_Vid->fld_flag == TRUE) && (bottom_field_flag == FALSE) );
      assert( (p_Vid->fld_flag == TRUE) && (p_Vid->structure != BOTTOM_FIELD) );
      NumClockTS = 1;
      break;
    case 2:
      // bottom field
      //assert( (p_Vid->fld_flag == TRUE) && (bottom_field_flag == TRUE) );
      assert( (p_Vid->fld_flag == TRUE) && (p_Vid->structure == BOTTOM_FIELD) );
      NumClockTS = 1;
      break;
    case 3:
      // top field, bottom field, in that order
    case 4:
      // bottom field, top field, in that order
      assert( p_Vid->fld_flag == FALSE );
      NumClockTS = 2;
      break;
    case 5:
      // top field, bottom field, top field repeated, in that order
    case 6:
      // bottom field, top field, bottom field repeated, in that order
      assert( p_Vid->fld_flag == FALSE );
      NumClockTS = 3;
      break;
    case 7:
      // frame doubling
      assert( (p_Vid->fld_flag == FALSE) && active_sps->vui_seq_parameters.fixed_frame_rate_flag == 1 );
      NumClockTS = 2;
      break;
    case 8:
      // frame tripling
      assert( (p_Vid->fld_flag == FALSE) && active_sps->vui_seq_parameters.fixed_frame_rate_flag == 1 );
      NumClockTS = 3;
      break;
    }
    for ( i = 0; i < NumClockTS; i++ )
    {
      u_1( "SEI: clock_timestamp_flag", p_SEI->seiPicTiming.clock_timestamp_flag[i], bitstream);
      if ( p_SEI->seiPicTiming.clock_timestamp_flag[i] )
      {
        u_v( 2, "SEI: ct_type", p_SEI->seiPicTiming.ct_type, bitstream);
        u_1( "SEI: nuit_field_based_flag", p_SEI->seiPicTiming.nuit_field_based_flag, bitstream);
        u_v( 5, "SEI: counting_type", p_SEI->seiPicTiming.counting_type, bitstream);
        u_1( "SEI: full_timestamp_flag", p_SEI->seiPicTiming.full_timestamp_flag, bitstream);
        u_1( "SEI: discontinuity_flag", p_SEI->seiPicTiming.discontinuity_flag, bitstream);
        u_1( "SEI: cnt_dropped_flag", p_SEI->seiPicTiming.cnt_dropped_flag, bitstream);
        u_v( 8, "SEI: n_frames", p_SEI->seiPicTiming.n_frames, bitstream);

        if ( p_SEI->seiPicTiming.full_timestamp_flag )
        {      
          u_v( 6, "SEI: seconds_value", p_SEI->seiPicTiming.seconds_value, bitstream);
          u_v( 6, "SEI: minutes_value", p_SEI->seiPicTiming.minutes_value, bitstream);
          u_v( 5, "SEI: hours_value",   p_SEI->seiPicTiming.hours_value, bitstream);
        }
        else
        {            
          u_1( "SEI: seconds_flag", p_SEI->seiPicTiming.seconds_flag, bitstream);
          if (p_SEI->seiPicTiming.seconds_flag)
          {
            u_v( 6, "SEI: seconds_value", p_SEI->seiPicTiming.seconds_value, bitstream);
            u_1( "SEI: minutes_flag", p_SEI->seiPicTiming.minutes_flag, bitstream);
            if (p_SEI->seiPicTiming.minutes_flag)
            {
              u_v( 6, "SEI: minutes_value", p_SEI->seiPicTiming.minutes_value, bitstream);
              u_1( "SEI: hours_flag", p_SEI->seiPicTiming.hours_flag, bitstream);
              if (p_SEI->seiPicTiming.hours_flag)
              {
                u_v( 5, "SEI: hours_value",   p_SEI->seiPicTiming.hours_value, bitstream);
              }
            }
          }
        }
        if ( hrd->time_offset_length )
        {
          u_v( hrd->time_offset_length, "SEI: time_offset", p_SEI->seiPicTiming.time_offset, bitstream);
        }
      }
    }
  }

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiPicTiming.payloadSize = bitstream->byte_pos;
}

/*
 ************************************************************************
 * \brief
 *    Close Picture Timing SEI data
 ************************************************************************
 */
static void ClosePicTiming(SEIParameters *p_SEI)
{
  if (p_SEI->seiPicTiming.data)
  {
    free(p_SEI->seiPicTiming.data->streamBuffer);
    free(p_SEI->seiPicTiming.data);
  }
  p_SEI->seiPicTiming.data = NULL;
}

/*
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *  \functions on frame packing arrangement SEI message
 *  \brief
 *      Based on pre-published Recommendation
 **++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

//! Frame packing arrangement Information
static void InitFramePackingArrangement(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;

  p_SEI->seiFramePackingArrangement.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiFramePackingArrangement.data == NULL ) 
    no_mem_exit("InitFramePackingArrangement: seiFramePackingArrangement.data");

  p_SEI->seiFramePackingArrangement.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiFramePackingArrangement.data->streamBuffer == NULL ) 
    no_mem_exit("InitFramePackingArrangement: seiFramePackingArrangement.data->streamBuffer");

  ClearFramePackingArrangement(p_SEI);
}

void ClearFramePackingArrangement(SEIParameters *p_SEI)
{
  memset( p_SEI->seiFramePackingArrangement.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiFramePackingArrangement.data->bits_to_go  = 8;
  p_SEI->seiFramePackingArrangement.data->byte_pos    = 0;
  p_SEI->seiFramePackingArrangement.data->byte_buf    = 0;
  p_SEI->seiFramePackingArrangement.payloadSize       = 0;

  p_SEI->seiFramePackingArrangement.frame_packing_arrangement_id = 0;
  p_SEI->seiFramePackingArrangement.frame_packing_arrangement_cancel_flag = FALSE;
  p_SEI->seiFramePackingArrangement.frame_packing_arrangement_type = 0;
  p_SEI->seiFramePackingArrangement.quincunx_sampling_flag = FALSE;
  p_SEI->seiFramePackingArrangement.content_interpretation_type = 0;
  p_SEI->seiFramePackingArrangement.spatial_flipping_flag = FALSE;
  p_SEI->seiFramePackingArrangement.frame0_flipped_flag = FALSE;
  p_SEI->seiFramePackingArrangement.field_views_flag = FALSE;
  p_SEI->seiFramePackingArrangement.current_frame_is_frame0_flag = FALSE;
  p_SEI->seiFramePackingArrangement.frame0_self_contained_flag = FALSE;
  p_SEI->seiFramePackingArrangement.frame1_self_contained_flag = FALSE;
  p_SEI->seiFramePackingArrangement.frame0_grid_position_x = 0;
  p_SEI->seiFramePackingArrangement.frame0_grid_position_y = 0;
  p_SEI->seiFramePackingArrangement.frame1_grid_position_x = 0;
  p_SEI->seiFramePackingArrangement.frame1_grid_position_y = 0;
  p_SEI->seiFramePackingArrangement.frame_packing_arrangement_reserved_byte = 0;
  p_SEI->seiFramePackingArrangement.frame_packing_arrangement_repetition_period = 0;
  p_SEI->seiFramePackingArrangement.frame_packing_arrangement_extension_flag = FALSE;

  p_SEI->seiHasFramePackingArrangement_info = FALSE;
}

void UpdateFramePackingArrangement(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  p_SEI->seiHasFramePackingArrangement_info = FALSE;
  UNREFERENCED_PARAMETER(p_Inp);
}

static void FinalizeFramePackingArrangement(SEIParameters *p_SEI)
{
  Bitstream *bitstream = p_SEI->seiFramePackingArrangement.data;

  ue_v( "SEI: frame_packing_arrangement_id", (int)p_SEI->seiFramePackingArrangement.frame_packing_arrangement_id, bitstream );
  u_1( "SEI: frame_packing_arrangement_cancel_flag", (int)p_SEI->seiFramePackingArrangement.frame_packing_arrangement_cancel_flag, bitstream );
  if ( p_SEI->seiFramePackingArrangement.frame_packing_arrangement_cancel_flag == FALSE )
  {
    u_v( 7, "SEI: frame_packing_arrangement_type", (int)p_SEI->seiFramePackingArrangement.frame_packing_arrangement_type, bitstream );
    u_1( "SEI: quincunx_sampling_flag", (int)p_SEI->seiFramePackingArrangement.quincunx_sampling_flag, bitstream );
    u_v( 6, "SEI: content_interpretation_type", (int)p_SEI->seiFramePackingArrangement.content_interpretation_type, bitstream );
    u_1( "SEI: spatial_flipping_flag", (int)p_SEI->seiFramePackingArrangement.spatial_flipping_flag, bitstream );
    u_1( "SEI: frame0_flipped_flag", (int)p_SEI->seiFramePackingArrangement.frame0_flipped_flag, bitstream );
    u_1( "SEI: field_views_flag", (int)p_SEI->seiFramePackingArrangement.field_views_flag, bitstream );
    u_1( "SEI: current_frame_is_frame0_flag", (int)p_SEI->seiFramePackingArrangement.current_frame_is_frame0_flag, bitstream );
    u_1( "SEI: frame0_self_contained_flag", (int)p_SEI->seiFramePackingArrangement.frame0_self_contained_flag, bitstream );
    u_1( "SEI: frame1_self_contained_flag", (int)p_SEI->seiFramePackingArrangement.frame1_self_contained_flag, bitstream );
    if ( p_SEI->seiFramePackingArrangement.quincunx_sampling_flag == FALSE && p_SEI->seiFramePackingArrangement.frame_packing_arrangement_type != 5 )
    {
      u_v( 4, "SEI: frame0_grid_position_x", (int)p_SEI->seiFramePackingArrangement.frame0_grid_position_x, bitstream );
      u_v( 4, "SEI: frame0_grid_position_y", (int)p_SEI->seiFramePackingArrangement.frame0_grid_position_y, bitstream );
      u_v( 4, "SEI: frame1_grid_position_x", (int)p_SEI->seiFramePackingArrangement.frame1_grid_position_x, bitstream );
      u_v( 4, "SEI: frame1_grid_position_y", (int)p_SEI->seiFramePackingArrangement.frame1_grid_position_y, bitstream );
    }
    u_v( 8, "SEI: frame_packing_arrangement_reserved_byte", (int)p_SEI->seiFramePackingArrangement.frame_packing_arrangement_reserved_byte, bitstream );
    ue_v( "SEI: frame_packing_arrangement_repetition_period", (int)p_SEI->seiFramePackingArrangement.frame_packing_arrangement_repetition_period, bitstream );
  }
  u_1( "SEI: frame_packing_arrangement_extension_flag", (int)p_SEI->seiFramePackingArrangement.frame_packing_arrangement_extension_flag, bitstream );

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiFramePackingArrangement.payloadSize = bitstream->byte_pos;
}

static void CloseFramePackingArrangement(SEIParameters *p_SEI)
{
  if (p_SEI->seiFramePackingArrangement.data)
  {
    free(p_SEI->seiFramePackingArrangement.data->streamBuffer);
    free(p_SEI->seiFramePackingArrangement.data);
  }
  p_SEI->seiFramePackingArrangement.data = NULL;
}

/*
 ************************************************************************
 * \brief
 *    Initialize dec_ref_pic_marking Repetition SEI data 
 ************************************************************************
 */
static void InitDRPMRepetition(SEIParameters *p_SEI)
{
  p_SEI->seiDRPMRepetition.data = malloc( sizeof(Bitstream) );
  if( p_SEI->seiDRPMRepetition.data == NULL ) 
    no_mem_exit("InitDRPMRepetition: p_SEI->seiDRPMRepetition.data");

  p_SEI->seiDRPMRepetition.data->streamBuffer = malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiDRPMRepetition.data->streamBuffer == NULL ) 
    no_mem_exit("InitDRPMRepetition: p_SEI->seiDRPMRepetition.data->streamBuffer");

  ClearDRPMRepetition(p_SEI);
}

/*
 ************************************************************************
 * \brief
 *    Clear dec_ref_pic_marking Repetition SEI data
 ************************************************************************
 */
void ClearDRPMRepetition(SEIParameters *p_SEI)
{
  memset( p_SEI->seiDRPMRepetition.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiDRPMRepetition.data->bits_to_go = 8;
  p_SEI->seiDRPMRepetition.data->byte_pos   = 0;
  p_SEI->seiDRPMRepetition.data->byte_buf   = 0;
  p_SEI->seiDRPMRepetition.payloadSize      = 0;

  p_SEI->seiDRPMRepetition.original_bottom_field_flag = FALSE;
  p_SEI->seiDRPMRepetition.original_field_pic_flag    = FALSE;
  p_SEI->seiDRPMRepetition.original_frame_num         = 0;
  p_SEI->seiDRPMRepetition.original_idr_flag          = FALSE;
  if ( p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved != NULL )
  {
    free_drpm_buffer( p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved );
    p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved = NULL;
  }

  p_SEI->seiHasDRPMRepetition_info = FALSE;
}

/*
 ************************************************************************
 * \brief
 *    Update dec_ref_pic_marking Repetition data
 ************************************************************************
 */
void UpdateDRPMRepetition(SEIParameters *p_SEI)
{
  if ( p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved != NULL )
  {
    p_SEI->seiHasDRPMRepetition_info = TRUE;
  }
  else
  {
    p_SEI->seiHasDRPMRepetition_info = FALSE;
  }
}

/*
 ************************************************************************
 * \brief
 *    Finalize dec_ref_pic_marking Repetition SEI data
 ************************************************************************
 */
static void FinalizeDRPMRepetition(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  Bitstream *bitstream = p_SEI->seiDRPMRepetition.data;

  // SEI message bits
  u_1( "SEI: original_idr_flag", p_SEI->seiDRPMRepetition.original_idr_flag, bitstream);
  ue_v( "SEI: original_frame_num", p_SEI->seiDRPMRepetition.original_frame_num, bitstream);
  if ( !active_sps->frame_mbs_only_flag )
  {
    u_1( "SEI: original_field_pic_flag", p_SEI->seiDRPMRepetition.original_field_pic_flag, bitstream);
    if ( p_SEI->seiDRPMRepetition.original_field_pic_flag )
    {
      u_1( "SEI: original_bottom_field_flag", p_SEI->seiDRPMRepetition.original_bottom_field_flag, bitstream);
    }
  }
  // now repeat dec_ref_pic_marking_buffer info
  dec_ref_pic_marking( bitstream, 
    p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved, 
    p_SEI->seiDRPMRepetition.original_idr_flag, 0, 0);

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiDRPMRepetition.payloadSize = bitstream->byte_pos;
}

/*
 ************************************************************************
 * \brief
 *    Close dec_ref_pic_marking Repetition SEI data
 ************************************************************************
 */
static void CloseDRPMRepetition(SEIParameters *p_SEI)
{
  if (p_SEI->seiDRPMRepetition.data)
  {
    free(p_SEI->seiDRPMRepetition.data->streamBuffer);
    free(p_SEI->seiDRPMRepetition.data);
  }
  p_SEI->seiDRPMRepetition.data = NULL;
  if ( p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved != NULL )
  {
    free_drpm_buffer( p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved );
    p_SEI->seiDRPMRepetition.dec_ref_pic_marking_buffer_saved = NULL;
  }
}


#if EXT3D

static void InitDepthRepresentationInfo(SEIParameters *p_SEI, VideoParameters *p_Vid)
{
  int  i=0;
  UNREFERENCED_PARAMETER(p_Vid);

  p_SEI->seiDepthRepresentationInfo.data = (Bitstream*)malloc( sizeof(Bitstream) );
  if( p_SEI->seiDepthRepresentationInfo.data == NULL ) 
    no_mem_exit("InitDepthRepresentationInfo: seiDepthRepresentationInfo.data");

  p_SEI->seiDepthRepresentationInfo.data->streamBuffer = (byte*)malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiDepthRepresentationInfo.data->streamBuffer == NULL ) 
    no_mem_exit("InitDepthRepresentationInfo: seiDepthRepresentationInfo.data->streamBuffer");

  p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei=(DepthAcquisitionInfoSEI*)calloc(MAX_CODEVIEW,sizeof(DepthAcquisitionInfoSEI));
  if(p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei==NULL)
    no_mem_exit("p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei");

  for(i=0;i<MAX_CODEVIEW;i++)
  {
    init_depth_acquisition_info_SEI(&p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i]);
  }

  ClearDepthRepresentationInfo(p_SEI);
}

void ClearDepthRepresentationInfo(SEIParameters *p_SEI)
{
  memset( p_SEI->seiDepthRepresentationInfo.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiDepthRepresentationInfo.data->bits_to_go  = 8;
  p_SEI->seiDepthRepresentationInfo.data->byte_pos    = 0;
  p_SEI->seiDepthRepresentationInfo.data->byte_buf    = 0;
  p_SEI->seiDepthRepresentationInfo.payloadSize       = 0;

  p_SEI->seiDepthRepresentationInfo.all_views_equal_flag = 0;
  p_SEI->seiDepthRepresentationInfo.num_views_minus1     = 0;

  p_SEI->seiHasDepthRepresentationInfo = FALSE;
}

static void calcElement(ThreeDVAESEI* curr_3dv_ae){
  if(curr_3dv_ae->original>=0)
    curr_3dv_ae->element.sign=0;
  else
    curr_3dv_ae->element.sign=1;

  get_exponent_mantissa_SEI(curr_3dv_ae);
  get_rec_double_type_SEI(curr_3dv_ae);
}

static void prepareDepthAcquisitionInfo(SEIParameters *p_SEI, VideoParameters *p_TextVid, VideoParameters *p_DepthVid, int voidx)
{
  //double camera_scaleText  = 0.0;
  DepthAcquisitionInfoSEI* p_acquisition_info;
  InputParameters* p_DepthInp=p_DepthVid->p_Inp;
  int frm_no_in_file=p_TextVid->frm_no_in_file;

  p_acquisition_info = &p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei[voidx];

  p_acquisition_info->depth_near_ae.mantissa_length  = p_DepthInp->MantissaLengthDepthRange;
  p_acquisition_info->depth_far_ae.mantissa_length   = p_DepthInp->MantissaLengthDepthRange;

  p_acquisition_info->depth_near_ae.original        = (p_DepthInp->AcquisitionIdx==2)?p_DepthInp->ZNears[voidx][frm_no_in_file]:p_DepthInp->ZNears[voidx][0];
  p_acquisition_info->depth_far_ae.original         = (p_DepthInp->AcquisitionIdx==2)?p_DepthInp->ZFars[voidx][frm_no_in_file] :p_DepthInp->ZFars[voidx][0];

  calcElement(&p_acquisition_info->depth_near_ae);
  calcElement(&p_acquisition_info->depth_far_ae);
}

void UpdateDepthRepresentationInfo(VideoParameters *p_Vid)
{
  SEIParameters   *p_SEI = p_Vid->p_SEI;
  InputParameters *p_Inp = p_Vid->p_Inp;
  int i;
  int  voidx=GetVOIdx(p_Inp,p_Vid->view_id);


  if(p_Vid->is_depth==1 && voidx==p_Vid->CodingDepthView[0])
  {
    VideoParameters *p_TextVid  = p_Vid->p_DualVid;
    VideoParameters *p_DepthVid = p_Vid;

    if (p_DepthVid->p_Inp->AcquisitionIdx==2 || (p_DepthVid->p_Inp->AcquisitionIdx==1 && p_TextVid->frame_no==0))
    {
      p_SEI->seiDepthRepresentationInfo.all_views_equal_flag     = FALSE;
      p_SEI->seiDepthRepresentationInfo.num_views_minus1         = p_Vid->p_Inp->NumOfViews-1;

      p_SEI->seiDepthRepresentationInfo.z_near_flag = 1;
      p_SEI->seiDepthRepresentationInfo.z_far_flag  = 1;
      p_SEI->seiDepthRepresentationInfo.z_axis_equal_flag     = TRUE;
      p_SEI->seiDepthRepresentationInfo.common_z_axis_reference_view = p_Inp->ViewCodingOrder[0];

      p_SEI->seiDepthRepresentationInfo.d_min_flag  = 0;
      p_SEI->seiDepthRepresentationInfo.d_max_flag  = 0;

      p_SEI->seiDepthRepresentationInfo.depth_representation_type = (p_DepthVid->NonlinearDepthNum > 0) ? 3 : 0;

      for(i=0;i<p_SEI->seiDepthRepresentationInfo.num_views_minus1+1;i++)
      {
        p_SEI->seiDepthRepresentationInfo.depth_info_view_id[i] = p_Inp->ViewCodingOrder[i];
        p_SEI->seiDepthRepresentationInfo.z_axis_reference_view[i] = p_Inp->ViewCodingOrder[0];
        p_SEI->seiDepthRepresentationInfo.disparity_reference_view[i]  = p_Inp->ViewCodingOrder[0];

        prepareDepthAcquisitionInfo(p_SEI, p_TextVid, p_DepthVid,i);
      }

      p_SEI->seiHasDepthRepresentationInfo = TRUE;
    }
    else
    {
      p_SEI->seiHasDepthRepresentationInfo = FALSE;
    }
  }
  else
  {
    p_SEI->seiHasDepthRepresentationInfo = FALSE;
  }
}

static void depth_acquisition_sei_element(ThreeDVAESEI* curr_3dv_ae, Bitstream *bitstream)
{
  u_1("SEI:sign0", curr_3dv_ae->element.sign, bitstream);
  u_v(curr_3dv_ae->exponent_size, "SEI:exponent0", curr_3dv_ae->element.exponent, bitstream);

  if(curr_3dv_ae->pred_mode==0){
    u_v(5, "SEI: mantissa_length", curr_3dv_ae->element.mantissa_bits-1, bitstream);
  }

  u_v(curr_3dv_ae->element.mantissa_bits, "SEI:mantissa0", curr_3dv_ae->element.mantissa, bitstream);
}

static void FinalizeDepthRepresentationInfo(SEIParameters *p_SEI, VideoParameters *p_Vid)
{
  int i,k;
  int numViews;

  Bitstream *bitstream = p_SEI->seiDepthRepresentationInfo.data;
  VideoParameters *p_DepthVid = p_Vid;

  u_1 (   "SEI: all_views_equal_flag",          p_SEI->seiDepthRepresentationInfo.all_views_equal_flag,      bitstream);
  if(p_SEI->seiDepthRepresentationInfo.all_views_equal_flag==0){
    ue_v (   "SEI: num_views_minus1",           p_SEI->seiDepthRepresentationInfo.num_views_minus1,          bitstream);
    numViews = p_SEI->seiDepthRepresentationInfo.num_views_minus1+1;
  }else{
    numViews = 1;
  }

  u_1 (   "SEI: z_near_flag",                 p_SEI->seiDepthRepresentationInfo.z_near_flag,                   bitstream);
  u_1 (   "SEI: z_far_flag",                  p_SEI->seiDepthRepresentationInfo.z_far_flag,                    bitstream);

  if (p_SEI->seiDepthRepresentationInfo.z_near_flag || p_SEI->seiDepthRepresentationInfo.z_far_flag)
  {
    u_1 (   "SEI: z_axis_equal_flag",                 p_SEI->seiDepthRepresentationInfo.z_axis_equal_flag,                   bitstream);
    if (p_SEI->seiDepthRepresentationInfo.z_axis_equal_flag)
    {
      ue_v(   "SEI: common_z_axis_reference_view",      p_SEI->seiDepthRepresentationInfo.common_z_axis_reference_view,           bitstream);
    }
  }

  u_1 (   "SEI: d_min_flag",                  p_SEI->seiDepthRepresentationInfo.d_min_flag,                    bitstream);
  u_1 (   "SEI: d_max_flag",                  p_SEI->seiDepthRepresentationInfo.d_max_flag,                    bitstream);
  ue_v(   "SEI: depth_representation_type",   p_SEI->seiDepthRepresentationInfo.depth_representation_type,     bitstream);

  for(i=0;i<numViews;i++)
  {
    ue_v(   "SEI: depth_info_view_id",        p_SEI->seiDepthRepresentationInfo.depth_info_view_id[i],           bitstream);
    if ((p_SEI->seiDepthRepresentationInfo.z_near_flag || p_SEI->seiDepthRepresentationInfo.z_far_flag) && p_SEI->seiDepthRepresentationInfo.z_axis_equal_flag == 0)
    {
      ue_v(   "SEI: z_axis_reference_view",        p_SEI->seiDepthRepresentationInfo.z_axis_reference_view[i],           bitstream);
    }
    if (p_SEI->seiDepthRepresentationInfo.d_min_flag || p_SEI->seiDepthRepresentationInfo.d_max_flag)
    {
      ue_v(   "SEI: disparity_reference_view",        p_SEI->seiDepthRepresentationInfo.disparity_reference_view[i],           bitstream);
    }

    if(p_SEI->seiDepthRepresentationInfo.z_near_flag)
    {
      depth_acquisition_sei_element(&p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].depth_near_ae, bitstream);
    }
    if(p_SEI->seiDepthRepresentationInfo.z_far_flag)
    {
      depth_acquisition_sei_element(&p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].depth_far_ae, bitstream);
    }
    if(p_SEI->seiDepthRepresentationInfo.d_min_flag)
    {
      depth_acquisition_sei_element(&p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].d_min_ae, bitstream);
    }
    if(p_SEI->seiDepthRepresentationInfo.d_max_flag)
    {
      depth_acquisition_sei_element(&p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei[i].d_max_ae, bitstream);
    }
  }

#if EXT3D
  if ( p_SEI->seiDepthRepresentationInfo.depth_representation_type == 3 ) 
  {  // actual NDR SEI coding:
    ue_v("SEI: depth_nonlinear_representation_num_minus1", p_DepthVid->NonlinearDepthNum - 1,   bitstream);
    for (k=1; k<=p_DepthVid->NonlinearDepthNum; ++k)
    {
      ue_v("SEI: depth_nonlinear_representation_model", p_DepthVid->NonlinearDepthPoints[k],                 bitstream);
    }
  }
#endif
  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiDepthRepresentationInfo.payloadSize = bitstream->byte_pos;
}

static void CloseDepthRepresentationInfo(SEIParameters *p_SEI)
{
  if (p_SEI->seiDepthRepresentationInfo.data)
  {
    free(p_SEI->seiDepthRepresentationInfo.data->streamBuffer);
    free(p_SEI->seiDepthRepresentationInfo.data);
  }

  if(p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei)
  {
    free(p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei);
  }

  p_SEI->seiDepthRepresentationInfo.data = NULL;
}

static void InitMultiviewAcquisitionInfo(SEIParameters *p_SEI, VideoParameters *p_Vid)
{
  int  i=0;
  UNREFERENCED_PARAMETER(p_Vid);

  p_SEI->seiMultiviewAcquisitionInfo.data = (Bitstream*)malloc( sizeof(Bitstream) );
  if( p_SEI->seiMultiviewAcquisitionInfo.data == NULL ) 
    no_mem_exit("InitMultiviewAcquisitionInfo: seiMultiviewAcquisitionInfo.data");

  p_SEI->seiMultiviewAcquisitionInfo.data->streamBuffer = (byte*)malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiMultiviewAcquisitionInfo.data->streamBuffer == NULL ) 
    no_mem_exit("InitMultiviewAcquisitionInfo: seiMultiviewAcquisitionInfo.data->streamBuffer");

  p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei=(MultiviewAcquisitionInfoSEI*)calloc(MAX_CODEVIEW,sizeof(MultiviewAcquisitionInfoSEI));
  if(p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei==NULL)
    no_mem_exit("p_SEI->seiDepthRepresentationInfo.depth_acquisition_update_info_sei");

  for(i=0;i<MAX_CODEVIEW;i++)
  {
    init_multiview_acquisition_info_SEI(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i]);
  }

  ClearMultiviewAcquisitionInfo(p_SEI);
}

void ClearMultiviewAcquisitionInfo(SEIParameters *p_SEI)
{
  memset( p_SEI->seiMultiviewAcquisitionInfo.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiMultiviewAcquisitionInfo.data->bits_to_go  = 8;
  p_SEI->seiMultiviewAcquisitionInfo.data->byte_pos    = 0;
  p_SEI->seiMultiviewAcquisitionInfo.data->byte_buf    = 0;
  p_SEI->seiMultiviewAcquisitionInfo.payloadSize       = 0;

  p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1     = 0;

  p_SEI->seiHasMultiviewAcquisitionInfo = FALSE;
}

static void prepareMultiviewAcquisitionInfo(SEIParameters *p_SEI, VideoParameters *p_TextVid, VideoParameters *p_DepthVid, int voidx)
{
  int i,j;
  double camera_scaleText  = 0.0;
  MultiviewAcquisitionInfoSEI* p_acquisition_info;
  InputParameters* p_DepthInp=p_DepthVid->p_Inp;
  int frm_no_in_file=p_TextVid->frm_no_in_file;

#if EXT3D
  camera_scaleText  = p_TextVid->width /(double)p_TextVid->width_ori;
#else 
  camera_scaleText  = 1.0;
#endif

  p_acquisition_info = &p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[voidx];

  p_acquisition_info->focal_length_x_ae.precision    = p_DepthInp->PrecFocalLength;
  p_acquisition_info->focal_length_y_ae.precision    = p_DepthInp->PrecFocalLength;
  p_acquisition_info->principal_point_x_ae.precision = p_DepthInp->PrecPrincipalPoint;
  p_acquisition_info->principal_point_y_ae.precision = p_DepthInp->PrecPrincipalPoint;
  p_acquisition_info->skew_factor_ae.precision       = p_DepthInp->PrecSkewFactor;

  p_acquisition_info->focal_length_x_ae.original    = camera_scaleText*p_DepthInp->IntrinsicMatrix[voidx][0][0];
  p_acquisition_info->focal_length_y_ae.original    = camera_scaleText*p_DepthInp->IntrinsicMatrix[voidx][1][1];
  p_acquisition_info->principal_point_x_ae.original = camera_scaleText*p_DepthInp->IntrinsicMatrix[voidx][0][2];
  p_acquisition_info->principal_point_y_ae.original = camera_scaleText*p_DepthInp->IntrinsicMatrix[voidx][1][2];
  p_acquisition_info->skew_factor_ae.original       = camera_scaleText*p_DepthInp->IntrinsicMatrix[voidx][0][1];

  calcElement(&p_acquisition_info->focal_length_x_ae);
  calcElement(&p_acquisition_info->focal_length_y_ae);
  calcElement(&p_acquisition_info->principal_point_x_ae);
  calcElement(&p_acquisition_info->principal_point_y_ae);
  calcElement(&p_acquisition_info->skew_factor_ae);

  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
    {
      p_acquisition_info->rotation_ae[i][j].precision = p_DepthInp->PrecRotation;
      p_acquisition_info->rotation_ae[i][j].original  = p_DepthInp->ExtrinsicMatrix[voidx][i][j];
      calcElement(&p_acquisition_info->rotation_ae[i][j]);
    }
    for (i=0; i<3; i++)
    {
      p_acquisition_info->translation_ae[i].precision = p_DepthInp->PrecTranslation;
      p_acquisition_info->translation_ae[i].original     = (p_DepthInp->AcquisitionIdx==2)?p_DepthInp->TranslationVector[voidx][i][frm_no_in_file]:p_DepthInp->TranslationVector[voidx][i][0];
      calcElement(&p_acquisition_info->translation_ae[i]);
    }
}

static Boolean check_element_equal(ThreeDVAESEI* first_3dv_ae, ThreeDVAESEI* second_3dv_ae)
{
  if (first_3dv_ae->element.exponent == second_3dv_ae->element.exponent 
    && first_3dv_ae->element.mantissa == second_3dv_ae->element.mantissa 
    && first_3dv_ae->element.sign == second_3dv_ae->element.sign)
  {
    return TRUE;
  }
  else
    return FALSE;
}

static int check_intrinsic_params_equal(multiview_acquisition_info* p_info)
{
  int NumOfViews = p_info->num_views_minus1 + 1;
  int i;

  MultiviewAcquisitionInfoSEI* p_acquisition_info = p_info->multiview_acquisition_update_info_sei;
  for (i=1; i<NumOfViews; i++)
  {
    if (check_element_equal(&p_acquisition_info[i].focal_length_x_ae, &p_acquisition_info[0].focal_length_x_ae) == FALSE) return 0;
    if (check_element_equal(&p_acquisition_info[i].focal_length_y_ae, &p_acquisition_info[0].focal_length_y_ae) == FALSE) return 0;
    if (check_element_equal(&p_acquisition_info[i].principal_point_x_ae, &p_acquisition_info[0].principal_point_x_ae) == FALSE) return 0;
    if (check_element_equal(&p_acquisition_info[i].principal_point_y_ae, &p_acquisition_info[0].principal_point_y_ae) == FALSE) return 0;
    if (check_element_equal(&p_acquisition_info[i].skew_factor_ae, &p_acquisition_info[0].skew_factor_ae) == FALSE) return 0;
  }
  return 1;
}

void UpdateMultiviewAcquisitionInfo(VideoParameters *p_Vid)
{
  SEIParameters   *p_SEI = p_Vid->p_SEI;
  InputParameters *p_Inp = p_Vid->p_Inp;
  int i=0, j=0;
  int  voidx=GetVOIdx(p_Inp,p_Vid->view_id);
#if EXT3D
  //Saving the previous translation parameters of multiview acquisition for reference display info sei
  if(p_Vid->type == I_SLICE && p_Vid->p_Inp->ReferenceDisplayInfoFlag)
  {
    for( i = 0; i < p_Vid->p_Inp->NumOfViews; i++ ) 
    {
      for ( j = 0; j <= 2; j++) 
      { 
        p_Vid->seiAcquisitionInfo[i].translation_ae[j].original = p_Vid->p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].translation_ae[j].original;
      }
    }
  }
#endif
  if(p_Inp->ThreeDVCoding&&p_Vid->is_depth==0&&voidx==0)
  {
    VideoParameters *p_TextVid  = p_Vid;
    VideoParameters *p_DepthVid = p_Vid->p_DualVid;

    if (p_DepthVid->p_Inp->AcquisitionIdx==2 || (p_DepthVid->p_Inp->AcquisitionIdx==1 && p_TextVid->frame_no==0))
    {
      p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1         = p_Vid->p_Inp->NumOfViews-1;

      p_SEI->seiMultiviewAcquisitionInfo.intrinsic_param_flag = (p_TextVid->frame_no==0) ? 1 : 0;
      p_SEI->seiMultiviewAcquisitionInfo.extrinsic_param_flag = 1;

      for(i=0;i<p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1+1;i++)
      {
        prepareMultiviewAcquisitionInfo(p_SEI, p_TextVid, p_DepthVid,i);
      }

      p_SEI->seiMultiviewAcquisitionInfo.intrinsic_params_equal = check_intrinsic_params_equal( &p_SEI->seiMultiviewAcquisitionInfo );

      p_SEI->seiHasMultiviewAcquisitionInfo = TRUE;
    }
    else
    {
      p_SEI->seiHasMultiviewAcquisitionInfo = FALSE;
    }
  }
  else
  {
    p_SEI->seiHasMultiviewAcquisitionInfo = FALSE;
  }
}

static void FinalizeMultiviewAcquisitionInfo(SEIParameters *p_SEI, VideoParameters *p_Vid)
{
  int i,j,k;
  int num_of_param_sets;

  Bitstream *bitstream = p_SEI->seiMultiviewAcquisitionInfo.data;
  VideoParameters *p_DepthVid = p_Vid->p_DualVid;
  InputParameters* p_DepthInp = p_DepthVid->p_Inp;

  ue_v (   "SEI: num_views_minus1",           p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1,          bitstream);

  u_1 (   "SEI: intrinsic_param_flag",        p_SEI->seiMultiviewAcquisitionInfo.intrinsic_param_flag,      bitstream);
  u_1 (   "SEI: extrinsic_param_flag",        p_SEI->seiMultiviewAcquisitionInfo.extrinsic_param_flag,      bitstream);

  if (p_SEI->seiMultiviewAcquisitionInfo.intrinsic_param_flag)
  {
    u_1  (   "SEI: intrinsic_params_equal",       p_SEI->seiMultiviewAcquisitionInfo.intrinsic_params_equal,      bitstream);
    ue_v (   "SEI: prec_focal_length",            p_DepthInp->PrecFocalLength,                                    bitstream);
    ue_v (   "SEI: prec_principal_point",         p_DepthInp->PrecPrincipalPoint,                                 bitstream);
    ue_v (   "SEI: prec_skew_factor",             p_DepthInp->PrecSkewFactor,                                     bitstream);
    if (p_SEI->seiMultiviewAcquisitionInfo.intrinsic_params_equal)
      num_of_param_sets = 1;
    else
      num_of_param_sets = p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1 + 1;

    for( i = 0; i < num_of_param_sets; i++ ) 
    {
      depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].focal_length_x_ae, bitstream);
      depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].focal_length_y_ae, bitstream);
      depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].principal_point_x_ae, bitstream);
      depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].principal_point_y_ae, bitstream);
      depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].skew_factor_ae, bitstream);
    }
  }

  if (p_SEI->seiMultiviewAcquisitionInfo.extrinsic_param_flag)
  {
    ue_v (   "SEI: prec_rotation_param",            p_DepthInp->PrecRotation,                                    bitstream);
    ue_v (   "SEI: prec_translation_param",         p_DepthInp->PrecTranslation,                                 bitstream);
    for( i = 0; i <= p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1; i++ ) 
    {
      for ( j = 0; j <= 2; j++) { /* row */
        for ( k = 0; k <= 2; k++) { /* column */
          depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].rotation_ae[j][k], bitstream);
        }
        depth_acquisition_sei_element(&p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].translation_ae[j], bitstream);
      }
    }
  }

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiMultiviewAcquisitionInfo.payloadSize = bitstream->byte_pos;
}

static void CloseMultiviewAcquisitionInfo(SEIParameters *p_SEI)
{
  if (p_SEI->seiMultiviewAcquisitionInfo.data)
  {
    free(p_SEI->seiMultiviewAcquisitionInfo.data->streamBuffer);
    free(p_SEI->seiMultiviewAcquisitionInfo.data);
  }

  if(p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei)
  {
    free(p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei);
  }

  p_SEI->seiMultiviewAcquisitionInfo.data = NULL;
}

static void Init3DVCScalableNesting(SEIParameters *p_SEI)
{
  //int  i=0;

  p_SEI->sei3DVCScalableNesting.data = (Bitstream*)malloc( sizeof(Bitstream) );
  if( p_SEI->sei3DVCScalableNesting.data == NULL ) 
    no_mem_exit("Init3DVCScalableNesting: sei3DVCScalableNesting.data");

  p_SEI->sei3DVCScalableNesting.data->streamBuffer = (byte*)malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->sei3DVCScalableNesting.data->streamBuffer == NULL ) 
    no_mem_exit("Init3DVCScalableNesting: sei3DVCScalableNesting.data->streamBuffer");

  Clear3DVCScalableNesting(p_SEI);
}

void Clear3DVCScalableNesting(SEIParameters *p_SEI)
{
  memset( p_SEI->sei3DVCScalableNesting.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->sei3DVCScalableNesting.data->bits_to_go  = 8;
  p_SEI->sei3DVCScalableNesting.data->byte_pos    = 0;
  p_SEI->sei3DVCScalableNesting.data->byte_buf    = 0;
  p_SEI->sei3DVCScalableNesting.payloadSize       = 0;


  p_SEI->seiHas3DVCScalableNesting = FALSE;
}

void Update3DVCScalableNesting(VideoParameters *p_Vid)
{
  SEIParameters   *p_SEI = p_Vid->p_SEI;

  p_SEI->sei3DVCScalableNesting.operation_point_flag = FALSE;
  p_SEI->sei3DVCScalableNesting.all_view_components_in_au_flag = TRUE;

  p_SEI->seiHas3DVCScalableNesting = p_SEI->seiHasMultiviewAcquisitionInfo;
}

static void Finalize3DVCScalableNesting(SEIParameters *p_SEI, VideoParameters *p_Vid)
{
  int i;

  Bitstream *bitstream = p_SEI->sei3DVCScalableNesting.data;
  UNREFERENCED_PARAMETER(p_Vid);

  u_1 (   "SEI: operation_point_flag",           p_SEI->sei3DVCScalableNesting.operation_point_flag,          bitstream);

  if ( !p_SEI->sei3DVCScalableNesting.operation_point_flag )
  {
    u_1 (   "SEI: all_view_components_in_au_flag",           p_SEI->sei3DVCScalableNesting.all_view_components_in_au_flag,          bitstream);
    if( !p_SEI->sei3DVCScalableNesting.all_view_components_in_au_flag )
    {
      ue_v (   "SEI: num_view_components_minus1",           p_SEI->sei3DVCScalableNesting.num_view_components_minus1,          bitstream);
      for( i = 0; i <= p_SEI->sei3DVCScalableNesting.num_view_components_minus1; i++ )
      {
        ue_v (   "SEI: sei_view_id",           p_SEI->sei3DVCScalableNesting.sei_view_id[i],          bitstream);
        u_1 (   "SEI: sei_view_applicability_flag",           p_SEI->sei3DVCScalableNesting.sei_view_applicability_flag[i],          bitstream);
      }
    }
  }
  else
  {
    u_1 (   "SEI: sei_op_texture_only_flag",           p_SEI->sei3DVCScalableNesting.sei_op_texture_only_flag,          bitstream);
    ue_v (   "SEI: num_view_components_op_minus1",           p_SEI->sei3DVCScalableNesting.num_view_components_op_minus1,          bitstream);
    for( i = 0; i <= p_SEI->sei3DVCScalableNesting.num_view_components_op_minus1; i++ )
    {
      ue_v (   "SEI: sei_op_view_id",           p_SEI->sei3DVCScalableNesting.sei_op_view_id[i],          bitstream);
    }
    ue_v (   "SEI: sei_op_temporal_id",           p_SEI->sei3DVCScalableNesting.sei_op_temporal_id,          bitstream);
  }

  // make sure the payload is byte aligned, stuff bits are 0..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->sei3DVCScalableNesting.payloadSize = bitstream->byte_pos;
}

static void Close3DVCScalableNesting(SEIParameters *p_SEI)
{
  if (p_SEI->sei3DVCScalableNesting.data)
  {
    free(p_SEI->sei3DVCScalableNesting.data->streamBuffer);
    free(p_SEI->sei3DVCScalableNesting.data);
  }

  p_SEI->sei3DVCScalableNesting.data = NULL;
}

static void InitReferenceDisplayInfo(SEIParameters *p_SEI, VideoParameters *p_Vid)
{
  int  i=0;
  UNREFERENCED_PARAMETER(p_Vid);

  p_SEI->seiReferenceDisplayInfo.data = (Bitstream*)malloc( sizeof(Bitstream) );
  if( p_SEI->seiReferenceDisplayInfo.data == NULL ) 
    no_mem_exit("InitReferenceDisplayInfo: seiReferenceDisplayInfo.data");

  p_SEI->seiReferenceDisplayInfo.data->streamBuffer = (byte*)malloc(MAXRTPPAYLOADLEN);
  if( p_SEI->seiReferenceDisplayInfo.data->streamBuffer == NULL ) 
    no_mem_exit("InitDepthRepresentationInfo: seiReferenceDisplayInfo.data->streamBuffer");

  p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei=(ReferenceDisplayInfoSEI*)calloc(MAX_DISPLAYS,sizeof(ReferenceDisplayInfoSEI));
  if(p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei==NULL)
    no_mem_exit("p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei");

  for(i=0;i<MAX_DISPLAYS;i++)
  { 
    init_ref_display_info_SEI(&p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei[i]);
  }

  ClearReferenceDisplayInfo(p_SEI);
}
void ClearReferenceDisplayInfo(SEIParameters *p_SEI)
{
  memset( p_SEI->seiReferenceDisplayInfo.data->streamBuffer, 0, MAXRTPPAYLOADLEN);

  p_SEI->seiReferenceDisplayInfo.data->bits_to_go  = 8;
  p_SEI->seiReferenceDisplayInfo.data->byte_pos    = 0;
  p_SEI->seiReferenceDisplayInfo.data->byte_buf    = 0;
  p_SEI->seiReferenceDisplayInfo.payloadSize       = 0;

  p_SEI->seiHasReferenceDisplayInfo = FALSE;
}

static void prepareReferenceDisplayInfo(SEIParameters *p_SEI, VideoParameters *p_TextVid, int num_ref_displays, double scaleFactor)
{
  ReferenceDisplayInfoSEI* p_reference_display_info;

  p_reference_display_info = &p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei[num_ref_displays];

  p_reference_display_info->ref_baseline.precision         = p_TextVid->p_Inp->BaselinePrecision;
  p_reference_display_info->ref_display_width.precision    = p_TextVid->p_Inp->WidthPrecsion;
  p_reference_display_info->ref_viewing_distance.precision = p_TextVid->p_Inp->ViewingDistancePrecision;


  p_reference_display_info->ref_baseline.original         = p_TextVid->p_Inp->ReferenceBaseline[num_ref_displays]*scaleFactor;
  p_reference_display_info->ref_display_width.original    = p_TextVid->p_Inp->ReferenceWidth[num_ref_displays];
  p_reference_display_info->ref_viewing_distance.original = p_TextVid->p_Inp->ReferenceViewingDistance[num_ref_displays];

  calcElement(&p_reference_display_info->ref_baseline);
  calcElement(&p_reference_display_info->ref_display_width);
  calcElement(&p_reference_display_info->ref_viewing_distance);
}

void UpdateReferenceDisplayInfo(VideoParameters *p_Vid)
{
  SEIParameters   *p_SEI = p_Vid->p_SEI;
  int i=0;

  double *TranslationFrame0 =  malloc((p_Vid->p_DualInp->NumOfViews)*sizeof(double));
  double *TranslationFrameN =  malloc((p_Vid->p_DualInp->NumOfViews)*sizeof(double));
  double scaleFactor;

  for (i = 0; i < p_SEI->seiMultiviewAcquisitionInfo.num_views_minus1 + 1; i++)
  {
    TranslationFrame0[i] = p_Vid->seiAcquisitionInfo[i].translation_ae[0].original; //Translation params of all views of previous Multiviewacquisiton sei
    TranslationFrameN[i] = p_SEI->seiMultiviewAcquisitionInfo.multiview_acquisition_update_info_sei[i].translation_ae[0].original;//Translation params of all views of current Multiviewacquisiton sei
  }
  if(TranslationFrame0[0] != TranslationFrame0[1] && TranslationFrameN[0] != TranslationFrameN[1] )
    scaleFactor = fabs(TranslationFrameN[0]-TranslationFrameN[1])/fabs(TranslationFrame0[0]-TranslationFrame0[1]);
  else
    scaleFactor = 1;

  if(p_Vid->p_Inp->NumberOfDiplays > 0)
  {
    VideoParameters *p_TextVid  = p_Vid;
    p_SEI->seiReferenceDisplayInfo.num_ref_displays_minus1 = p_Vid->p_Inp->NumberOfDiplays - 1;

    p_SEI->seiReferenceDisplayInfo.ref_viewing_distance_flag = p_Vid->p_Inp->ReferenceViewingDistanceFlag;

    for(i = 0; i < p_SEI->seiReferenceDisplayInfo.num_ref_displays_minus1 + 1; i++)
    {
      prepareReferenceDisplayInfo(p_SEI, p_TextVid, i, scaleFactor);
      p_SEI->seiReferenceDisplayInfo.additional_shift_present_flag[i] = p_Vid->p_Inp->NumSampleShiftFlag[i];
      if (p_SEI->seiReferenceDisplayInfo.additional_shift_present_flag[i])
        p_SEI->seiReferenceDisplayInfo.num_sample_shift_plus512[i] = (int32)p_Vid->p_Inp->NumSampleShift[i]*scaleFactor;
    }
    p_SEI->seiReferenceDisplayInfo.three_dimensional_extension_flag = 0;
    p_SEI->seiHasReferenceDisplayInfo = TRUE;
  }
  else
  {
    p_SEI->seiHasReferenceDisplayInfo = FALSE;
  }
}

static void FinalizeReferenceDisplayInfo(SEIParameters *p_SEI)
{
  int i;

  Bitstream *bitstream = p_SEI->seiReferenceDisplayInfo.data;

  ue_v (   "SEI: prec_ref_baseline",           p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei->ref_baseline.precision,           bitstream);
  ue_v (   "SEI: prec_ref_display_width",      p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei->ref_display_width.precision,      bitstream);
  u_1  (   "SEI: ref_viewing_distance_flag",   p_SEI->seiReferenceDisplayInfo.ref_viewing_distance_flag,      bitstream);

  if (p_SEI->seiReferenceDisplayInfo.ref_viewing_distance_flag)
    ue_v (   "SEI: prec_ref_viewing_dist",     p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei->ref_viewing_distance.precision,      bitstream);

  ue_v (   "SEI: num_ref_displays_minus1",     p_SEI->seiReferenceDisplayInfo.num_ref_displays_minus1,      bitstream);

  for( i = 0; i < p_SEI->seiReferenceDisplayInfo.num_ref_displays_minus1 + 1; i++ ) 
  {
    ThreeDVAESEI* curr_3dv_ae = &p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_baseline;
    u_v(curr_3dv_ae->exponent_size,         "SEI:exponent_ref_baseline[ i ]",     curr_3dv_ae->element.exponent, bitstream);
    u_v(curr_3dv_ae->element.mantissa_bits, "SEI:mantissa_ref_baseline[ i ]",     curr_3dv_ae->element.mantissa, bitstream);

    curr_3dv_ae = &p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_display_width;   
    u_v(curr_3dv_ae->exponent_size,         "SEI:exponent_ref_width[ i ]",     curr_3dv_ae->element.exponent, bitstream);
    u_v(curr_3dv_ae->element.mantissa_bits, "SEI:mantissa_ref_width[ i ]",     curr_3dv_ae->element.mantissa, bitstream);

    curr_3dv_ae = &p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei[i].ref_viewing_distance;
    if (p_SEI->seiReferenceDisplayInfo.ref_viewing_distance_flag)
    {
      u_v(curr_3dv_ae->exponent_size,         "SEI:exponent_ref_viewing_distance[ i ]",     curr_3dv_ae->element.exponent, bitstream);
      u_v(curr_3dv_ae->element.mantissa_bits, "SEI:mantissa_ref_viewing_distance[ i ]",     curr_3dv_ae->element.mantissa, bitstream);
    }
    
    u_1  (   "SEI: additional_shift_present_flag[ i ]",   p_SEI->seiReferenceDisplayInfo.additional_shift_present_flag[i], bitstream);
    if (p_SEI->seiReferenceDisplayInfo.additional_shift_present_flag[i])
      u_v(10,"SEI:num_sample_shift_plus512[ i ]",     p_SEI->seiReferenceDisplayInfo.num_sample_shift_plus512[i],      bitstream);    
  }
  u_1  (   "SEI: three_dimensional_reference_displays_extension_flag",   p_SEI->seiReferenceDisplayInfo.three_dimensional_extension_flag,      bitstream);

  // make sure the payload is byte aligned, stuff bits are 10..0
  if ( bitstream->bits_to_go != 8 )
  {
    (bitstream->byte_buf) <<= 1;
    bitstream->byte_buf |= 1;
    bitstream->bits_to_go--;
    if ( bitstream->bits_to_go != 0 )
      (bitstream->byte_buf) <<= (bitstream->bits_to_go);
    bitstream->bits_to_go = 8;
    bitstream->streamBuffer[bitstream->byte_pos++]=bitstream->byte_buf;
    bitstream->byte_buf = 0;
  }
  p_SEI->seiReferenceDisplayInfo.payloadSize = bitstream->byte_pos;
}


static void CloseReferenceDisplayInfo(SEIParameters *p_SEI)
{
  if (p_SEI->seiReferenceDisplayInfo.data)
  {
    free(p_SEI->seiReferenceDisplayInfo.data->streamBuffer);
    free(p_SEI->seiReferenceDisplayInfo.data);
  }

  if(p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei)
  {
    free(p_SEI->seiReferenceDisplayInfo.reference_display_update_info_sei);
  }

  p_SEI->seiReferenceDisplayInfo.data = NULL;
}
#endif
  
/*!
 *****************************************************************************
 * \brief
 *    Prepare the aggregation sei message.
 *
 * \date
 *    September 10, 2002
 *
 * \author
 *    Dong Tian   tian@cs.tut.fi
 *****************************************************************************/
void PrepareAggregationSEIMessage(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  Boolean has_aggregation_sei_message = FALSE;

  clear_sei_message(p_SEI, AGGREGATION_SEI);

  // prepare the sei message here
  // write the spare picture sei payload to the aggregation sei message
  if (p_SEI->seiHasSparePicture && p_Vid->type != B_SLICE)
  {
    FinalizeSpareMBMap(p_Vid);
    assert(p_SEI->seiSparePicturePayload.data->byte_pos == p_SEI->seiSparePicturePayload.payloadSize);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiSparePicturePayload.data->streamBuffer, p_SEI->seiSparePicturePayload.payloadSize, SEI_SPARE_PIC);
    has_aggregation_sei_message = TRUE;
  }
  // write the sub sequence information sei paylaod to the aggregation sei message
  if (p_SEI->seiHasSubseqInfo)
  {
    FinalizeSubseqInfo(p_SEI, p_Vid->layer);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiSubseqInfo[p_Vid->layer].data->streamBuffer, p_SEI->seiSubseqInfo[p_Vid->layer].payloadSize, SEI_SUB_SEQ_INFO);
    ClearSubseqInfoPayload(p_SEI, p_Vid->layer);
    has_aggregation_sei_message = TRUE;
  }
  // write the sub sequence layer information sei paylaod to the aggregation sei message
  if (p_SEI->seiHasSubseqLayerInfo && p_Vid->number == 0)
  {
    FinalizeSubseqLayerInfo(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiSubseqLayerInfo.data, p_SEI->seiSubseqLayerInfo.payloadSize, SEI_SUB_SEQ_LAYER_CHARACTERISTICS);
    p_SEI->seiHasSubseqLayerInfo = FALSE;
    has_aggregation_sei_message = TRUE;
  }
  // write the sub sequence characteristics payload to the aggregation sei message
  if (p_SEI->seiHasSubseqChar)
  {
    FinalizeSubseqChar(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiSubseqChar.data->streamBuffer, p_SEI->seiSubseqChar.payloadSize, SEI_SUB_SEQ_CHARACTERISTICS);
    ClearSubseqCharPayload(p_SEI);
    has_aggregation_sei_message = TRUE;
  }
  // write the pan scan rectangle info sei playload to the aggregation sei message
  if (p_SEI->seiHasPanScanRectInfo)
  {
    FinalizePanScanRectInfo(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiPanScanRectInfo.data->streamBuffer, p_SEI->seiPanScanRectInfo.payloadSize, SEI_PAN_SCAN_RECT);
    ClearPanScanRectInfoPayload(p_SEI);
    has_aggregation_sei_message = TRUE;
  }
  // write the arbitrary (unregistered) info sei playload to the aggregation sei message
  if (p_SEI->seiHasUser_data_unregistered_info)
  {
    FinalizeUser_data_unregistered(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiUser_data_unregistered.data->streamBuffer, p_SEI->seiUser_data_unregistered.payloadSize, SEI_USER_DATA_UNREGISTERED);
    ClearUser_data_unregistered(p_SEI);
    has_aggregation_sei_message = TRUE;
  }
  // write the arbitrary (unregistered) info sei playload to the aggregation sei message
  if (p_SEI->seiHasUser_data_registered_itu_t_t35_info)
  {
    FinalizeUser_data_registered_itu_t_t35(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiUser_data_registered_itu_t_t35.data->streamBuffer, p_SEI->seiUser_data_registered_itu_t_t35.payloadSize, SEI_USER_DATA_REGISTERED_ITU_T_T35);
    ClearUser_data_registered_itu_t_t35(p_SEI);
    has_aggregation_sei_message = TRUE;
  }  
  // more aggregation sei payload is written here...

  // write the scene information SEI payload
  if (p_SEI->seiHasSceneInformation)
  {
    FinalizeSceneInformation(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiSceneInformation.data->streamBuffer, p_SEI->seiSceneInformation.payloadSize, SEI_SCENE_INFO);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasTone_mapping)
  {
    FinalizeToneMapping(p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiToneMapping.data->streamBuffer, p_SEI->seiToneMapping.payloadSize, SEI_TONE_MAPPING);
    ClearToneMapping(p_SEI);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasPostFilterHints_info)
  {
    FinalizePostFilterHints(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiPostFilterHints.data->streamBuffer, p_SEI->seiPostFilterHints.payloadSize, SEI_POST_FILTER_HINTS);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasBufferingPeriod_info)
  {
    FinalizeBufferingPeriod(p_SEI, p_Vid->active_sps);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiBufferingPeriod.data->streamBuffer, p_SEI->seiBufferingPeriod.payloadSize, SEI_BUFFERING_PERIOD);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasPicTiming_info)
  {
    FinalizePicTiming(p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiPicTiming.data->streamBuffer, p_SEI->seiPicTiming.payloadSize, SEI_PIC_TIMING);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasRecoveryPoint_info)
  {
    FinalizeRandomAccess(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiRecoveryPoint.data->streamBuffer, p_SEI->seiRecoveryPoint.payloadSize, SEI_RECOVERY_POINT);
    ClearRandomAccess(p_SEI);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasDRPMRepetition_info)
  {
    FinalizeDRPMRepetition(p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiDRPMRepetition.data->streamBuffer, p_SEI->seiDRPMRepetition.payloadSize, SEI_DEC_REF_PIC_MARKING_REPETITION);
    has_aggregation_sei_message = TRUE;
    ClearDRPMRepetition(p_SEI); // to reset the drpm buffer into NULL
  }

  if (p_SEI->seiHasFramePackingArrangement_info)
  {
    FinalizeFramePackingArrangement(p_SEI);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiFramePackingArrangement.data->streamBuffer, p_SEI->seiFramePackingArrangement.payloadSize, SEI_FRAME_PACKING_ARRANGEMENT);
    has_aggregation_sei_message = TRUE;
  }

#if EXT3D
  if (p_SEI->seiHasMultiviewAcquisitionInfo)
  {
    FinalizeMultiviewAcquisitionInfo(p_SEI, p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiMultiviewAcquisitionInfo.data->streamBuffer, p_SEI->seiMultiviewAcquisitionInfo.payloadSize, SEI_MULTIVIEW_ACQUISITION_INFO);
    ClearMultiviewAcquisitionInfo(p_SEI);
    has_aggregation_sei_message = TRUE;
  }

  if (p_SEI->seiHasDepthRepresentationInfo)
  {
    FinalizeDepthRepresentationInfo(p_SEI, p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiDepthRepresentationInfo.data->streamBuffer, p_SEI->seiDepthRepresentationInfo.payloadSize, SEI_3DV_DEPTH_REPRESENTATION_INFO);
    ClearDepthRepresentationInfo(p_SEI);
    has_aggregation_sei_message = TRUE;
  }
#endif

  // after all the sei payload is written
  if (has_aggregation_sei_message)
  {
    finalize_sei_message(p_SEI, AGGREGATION_SEI);
  }
}

#if EXT3D
void PrepareAggregationNestingSEIMessage(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  Boolean has_aggregation_sei_message = FALSE;

  clear_sei_message(p_SEI, AGGREGATION_SEI);

  // prepare the sei message here
  if (p_SEI->seiHas3DVCScalableNesting)
  {
    Finalize3DVCScalableNesting(p_SEI, p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->sei3DVCScalableNesting.data->streamBuffer, p_SEI->sei3DVCScalableNesting.payloadSize, SEI_3DVC_SCALABLE_NESTING);
    Clear3DVCScalableNesting(p_SEI);
    has_aggregation_sei_message = TRUE;
  }
  if (p_SEI->seiHasMultiviewAcquisitionInfo)
  {
    FinalizeMultiviewAcquisitionInfo(p_SEI, p_Vid);
    write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiMultiviewAcquisitionInfo.data->streamBuffer, p_SEI->seiMultiviewAcquisitionInfo.payloadSize, SEI_MULTIVIEW_ACQUISITION_INFO);
    ClearMultiviewAcquisitionInfo(p_SEI);
    has_aggregation_sei_message = TRUE;
  }

  // after all the sei payload is written
  if (has_aggregation_sei_message)
  {
    finalize_sei_message(p_SEI, AGGREGATION_SEI);
  }
}

void PrepareReferenceDisplaySEIMessage(VideoParameters *p_Vid)
{
  SEIParameters *p_SEI = p_Vid->p_SEI;
  Boolean has_aggregation_sei_message = FALSE;

  clear_sei_message(p_SEI, AGGREGATION_SEI);
  if(p_SEI->seiHasMultiviewAcquisitionInfo)
  {
    if (p_SEI->seiHasReferenceDisplayInfo)
    {
      FinalizeReferenceDisplayInfo(p_SEI);
      write_sei_message(p_SEI, AGGREGATION_SEI, p_SEI->seiReferenceDisplayInfo.data->streamBuffer, p_SEI->seiReferenceDisplayInfo.payloadSize, SEI_3DV_REFERENCE_DISPLAY_INFO);
      ClearReferenceDisplayInfo(p_SEI);
      has_aggregation_sei_message = TRUE;
    }
  }

  // after all the sei payload is written
  if (has_aggregation_sei_message)
  {
    finalize_sei_message(p_SEI, AGGREGATION_SEI);
  }
}
#endif
/*!
 ************************************************************************
 * \brief
 *    Free dec_ref_pic_marking_bufffer structure
 ************************************************************************
 */
void free_drpm_buffer( DecRefPicMarking_t *pDRPM )
{
  DecRefPicMarking_t *pTmp = pDRPM, *pPrev;

  if (pTmp == NULL)
    return;

  while( pTmp->Next != NULL )
  {
    pPrev = pTmp;
    pTmp = pTmp->Next;
    free( pPrev );
  }
  free( pTmp );
}

