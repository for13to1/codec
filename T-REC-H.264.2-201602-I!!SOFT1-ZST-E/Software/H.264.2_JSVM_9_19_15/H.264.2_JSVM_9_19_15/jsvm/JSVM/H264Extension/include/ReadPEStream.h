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

#if !defined(AFX_READPESTREAM_H__DE3116D9_2399_41C8_9C01_E3118DDDE939__INCLUDED_)
#define AFX_READPESTREAM_H__DE3116D9_2399_41C8_9C01_E3118DDDE939__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LargeFile.h"

class H264AVCVIDEOIOLIB_API ReadPEStream
{
protected:
  ReadPEStream();
  virtual ~ReadPEStream();
public:
  ErrVal extractPacket( BinData*& rpcBinData, UInt& uiCts, Bool& rbEOS );
  ErrVal releasePacket( BinData* pcBinData );

  static ErrVal create( ReadPEStream*& rpcReadPEStream );
  ErrVal destroy();

  ErrVal reset();

  ErrVal init( const std::string& rcFileName );
  ErrVal uninit();
protected:
  LargeFile m_cFile;
  UInt   m_uiAULength;
  UChar* m_pucReadPtr;
  UChar* m_pucPayload;
  UInt64 m_uiCts90;
  UInt64 m_uiDts90;
  ErrVal xGetPtsDts( UChar* pucHeader, UInt64& uiTS90);
};

#endif // !defined(AFX_READPESTREAM_H__DE3116D9_2399_41C8_9C01_E3118DDDE939__INCLUDED_)
