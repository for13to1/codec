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

#if !defined(AFX_CABAENCODER_H__19223BF1_DA9C_475F_9AEE_D1A55237EB1E__INCLUDED_)
#define AFX_CABAENCODER_H__19223BF1_DA9C_475F_9AEE_D1A55237EB1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BitWriteBufferIf.h"


H264AVC_NAMESPACE_BEGIN


class CabacContextModel;

class CabaEncoder
{
protected:
  CabaEncoder();
  virtual ~CabaEncoder();

public:
  ErrVal start();
  ErrVal getLastByte(UChar &uiLastByte, UInt &uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal setFirstBits(UChar ucByte,UInt uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal init( BitWriteBufferIf* pcBitWriteBufferIf );
  ErrVal uninit();

  ErrVal writeEPSymbol( UInt uiSymbol );
  ErrVal writeSymbol( UInt uiSymbol, CabacContextModel& rcCCModel );
  ErrVal writeUnaryMaxSymbol( UInt uiSymbol, CabacContextModel* pcCCModel, Int iOffset, UInt uiMaxSymbol );
  ErrVal writeUnarySymbol( UInt uiSymbol, CabacContextModel* pcCCModel, Int iOffset );

  ErrVal writeExGolombLevel( UInt uiSymbol, CabacContextModel& rcCCModel  );
  ErrVal writeEpExGolomb( UInt uiSymbol, UInt uiCount );
  ErrVal writeExGolombMvd( UInt uiSymbol, CabacContextModel* pcCCModel, UInt uiMaxBin );
  ErrVal writeTerminatingBit( UInt uiBit );
  ErrVal finish();
  UInt   getWrittenBits()  { return m_pcBitWriteBufferIf->getNumberOfWrittenBits() + 8 + m_uiBitsToFollow - m_uiBitsLeft + 1; } //JVT-P031

  Void   setStates  (CabaEncoder* pcExtEncoder )
  {
    m_pcBitWriteBufferIf  = pcExtEncoder->m_pcBitWriteBufferIf;
    m_uiRange             = pcExtEncoder->m_uiRange;
    m_uiLow               = pcExtEncoder->m_uiLow;
    m_uiByte              = pcExtEncoder->m_uiByte;
    m_uiBitsLeft          = pcExtEncoder->m_uiBitsLeft;
    m_uiBitsToFollow      = pcExtEncoder->m_uiBitsToFollow;
  }
  Void   getStates  (CabaEncoder* pcExtEncoder )
  {
    pcExtEncoder->m_pcBitWriteBufferIf  = m_pcBitWriteBufferIf;
    pcExtEncoder->m_uiRange             = m_uiRange;
    pcExtEncoder->m_uiLow               = m_uiLow;
    pcExtEncoder->m_uiByte              = m_uiByte;
    pcExtEncoder->m_uiBitsLeft          = m_uiBitsLeft;
    pcExtEncoder->m_uiBitsToFollow      = m_uiBitsToFollow;
  }
	BitWriteBufferIf* getBitWriteBufferIf(void){return m_pcBitWriteBufferIf;}//JVT-X046

private:
  __inline ErrVal xWriteBit( UInt uiBit);
  __inline ErrVal xWriteBitAndBitsToFollow( UInt uiBit);

protected:
  BitWriteBufferIf* m_pcBitWriteBufferIf;

  UInt m_uiRange;
  UInt m_uiLow;
  UInt m_uiByte;
  UInt m_uiBitsLeft;
  UInt m_uiBitsToFollow;
  Bool m_bTraceEnable;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CABAENCODER_H__19223BF1_DA9C_475F_9AEE_D1A55237EB1E__INCLUDED_)
