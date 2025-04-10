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

#ifndef _MCTF_PRE_PROCESSOR_TEST_H
#define _MCTF_PRE_PROCESSOR_TEST_H


#include "Typedefs.h"
#include "H264AVCEncoderLib.h"
#include "H264AVCVideoIoLib.h"
#include "MCTFPreProcessor.h"
#include "PreProcessorParameter.h"
#include "WriteYuvToFile.h"
#include "ReadYuvFile.h"
#include "CodingParameter.h"


class MCTFPreProcessorTest
{
public:
  MCTFPreProcessorTest    ();
  ~MCTFPreProcessorTest   ();

  static ErrVal   create  ( MCTFPreProcessorTest*&  rpcMCTFPreProcessorTest );
  ErrVal          destroy ();
  ErrVal          init    ( PreProcessorParameter*  pcPreProcessorParameter );
  ErrVal          go      ();

protected:
  ErrVal  xInitCodingParameter  ();
  ErrVal  xGetNewPicBuffer      ( PicBuffer*&     rpcPicBuffer,
                                  UInt            uiSize );
  ErrVal  xRemovePicBuffer      ( PicBufferList&  rcList );
  ErrVal  xWrite                ( PicBufferList&  rcList );

private:
  PreProcessorParameter*        m_pcParameter;
  h264::CodingParameter         m_cCodingParameter;
  h264::MCTFPreProcessor*       m_pcMCTFPreProcessor;
  WriteYuvToFile*               m_pcWriteYuv;
  ReadYuvFile*                  m_pcReadYuv;
  PicBufferList                 m_cActivePicBufferList;
  PicBufferList                 m_cUnusedPicBufferList;
  UInt                          m_uiPicSize;
  UInt                          m_uiLumOffset;
  UInt                          m_uiCbOffset;
  UInt                          m_uiCrOffset;
  UInt                          m_uiHeight;
  UInt                          m_uiWidth;
  UInt                          m_uiStride;
};


#endif // _MCTF_PRE_PROCESSOR_TEST_H

