/*
********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005-2009, International Telecommunications Union, Geneva

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


#ifndef __H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#include "ReadBitstreamFile.h"
#include "WriteYuvToFile.h"

#define MAX_REFERENCE_FRAMES 15
#define MAX_B_FRAMES         15

#include <algorithm>
#include <list>

#include "DecoderParameter.h"


class H264AVCDecoderTest  
{
protected:
	H264AVCDecoderTest();
	virtual ~H264AVCDecoderTest();

public:
  static ErrVal create( H264AVCDecoderTest*& rpcH264AVCDecoderTest );
  ErrVal init( DecoderParameter *pcDecoderParameter, WriteYuvToFile *pcWriteYuv, ReadBitstreamFile *pcReadBitstreamFile );//TMM_EC
  ErrVal go();
  ErrVal destroy();
  ErrVal setec( UInt uiErrorConceal);//TMM_EC

  ErrVal setCrop();

protected:
  ErrVal xGetNewPicBuffer ( PicBuffer*& rpcPicBuffer, UInt uiSize );
  ErrVal xRemovePicBuffer ( PicBufferList& rcPicBufferUnusedList );

protected:
  h264::CreaterH264AVCDecoder*   m_pcH264AVCDecoder;
  h264::CreaterH264AVCDecoder*   m_pcH264AVCDecoderSuffix; //JVT-S036 lsj
  ReadBitstreamIf*            m_pcReadBitstream;
  WriteYuvIf*                 m_pcWriteYuv;
  WriteYuvIf*				  m_pcWriteSnapShot;   //SEI LSJ
  DecoderParameter*           m_pcParameter;
	
  PicBufferList               m_cActivePicBufferList;
  PicBufferList               m_cUnusedPicBufferList;
};

#endif //__H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
