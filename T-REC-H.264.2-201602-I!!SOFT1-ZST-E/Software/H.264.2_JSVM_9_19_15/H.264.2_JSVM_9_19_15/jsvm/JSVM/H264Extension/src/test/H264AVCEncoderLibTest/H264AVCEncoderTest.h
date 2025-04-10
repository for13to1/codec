/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2009-2012, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/

#ifndef __H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79

#include <algorithm>
#include <list>

#include "WriteBitstreamToFile.h"
#include "ReadYuvFile.h"
#include "WriteYuvToFile.h"



class EncoderCodingParameter;



typedef struct
{
  UInt    uiNumberOfLayers;
  std::string cBitstreamFilename;
  Int     nResult;
  UInt    nFrames;
} EncoderIoParameter;




class H264AVCEncoderTest
{
private:
  H264AVCEncoderTest();
  virtual ~H264AVCEncoderTest();

public:
  static ErrVal create( H264AVCEncoderTest*& rpcH264AVCEncoderTest );

  ErrVal init     ( Int     argc,
                    Char**  argv );
  ErrVal go       ();
  ErrVal destroy  ();
  ErrVal ScalableDealing ();

protected:
  ErrVal  xGetNewPicBuffer( PicBuffer*&             rpcPicBuffer,
                            UInt                    uiLayer,
                            UInt                    uiSize );
  ErrVal  xRemovePicBuffer( PicBufferList&          rcPicBufferUnusedList,
                            UInt                    uiLayer );

  ErrVal  xWrite          ( ExtBinDataAccessorList& rcList,
                            UInt&                   ruiBytesInFrame );
  ErrVal  xRelease        ( ExtBinDataAccessorList& rcList );

  ErrVal  xWrite          ( PicBufferList&          rcList,
                            UInt                    uiLayer );
  ErrVal  xRelease        ( PicBufferList&          rcList,
                            UInt                    uiLayer );
#if DOLBY_ENCMUX_ENABLE
  //Dolby muxing functions
  void sbsMux(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height, Int offset0=0, Int offset1=1, Int iFilterIdx=7);
  ErrVal padBuf(UChar *output, Int iStrideOut, Int width, Int height, Int width_out, Int height_out, Int fillMode);
  void sbsMuxFR(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height);
  void tabMux(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height, Int offset0=0, Int offset1=1, Int iFilterIdx=7);
  void tabMuxFR(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height);
#endif

protected:
  EncoderIoParameter            m_cEncoderIoParameter;
  EncoderCodingParameter*       m_pcEncoderCodingParameter;
  h264::CreaterH264AVCEncoder*  m_pcH264AVCEncoder;
  WriteBitstreamToFile*         m_pcWriteBitstreamToFile;
  WriteYuvToFile*               m_apcWriteYuv           [MAX_LAYERS];
  ReadYuvFile*                  m_apcReadYuv            [MAX_LAYERS];

  PicBufferList                 m_acActivePicBufferList [MAX_LAYERS];
  PicBufferList                 m_acUnusedPicBufferList [MAX_LAYERS];
  UInt                          m_auiLumOffset          [MAX_LAYERS];
  UInt                          m_auiCbOffset           [MAX_LAYERS];
  UInt                          m_auiCrOffset           [MAX_LAYERS];
  UInt                          m_auiHeight             [MAX_LAYERS];
  UInt                          m_auiWidth              [MAX_LAYERS];
  UInt                          m_auiStride             [MAX_LAYERS];
  UInt                          m_aauiCropping          [MAX_LAYERS][4];

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;
  std::string                   m_cWriteToBitFileName;
  std::string                   m_cWriteToBitFileTempName;
};




#endif //__H264AVCENCODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
