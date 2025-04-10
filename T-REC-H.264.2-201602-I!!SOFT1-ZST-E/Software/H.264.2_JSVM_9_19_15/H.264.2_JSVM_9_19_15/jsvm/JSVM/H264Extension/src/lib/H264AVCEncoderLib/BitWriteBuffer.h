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

#if !defined(AFX_BITWRITEBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_)
#define AFX_BITWRITEBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BitWriteBufferIf.h"

H264AVC_NAMESPACE_BEGIN

class BitWriteBuffer :
public BitWriteBufferIf
{
protected:
  BitWriteBuffer();
  virtual ~BitWriteBuffer();

public:

  BitWriteBufferIf* getNextBitWriteBuffer( Bool bStartNewBitstream );
  Bool nextBitWriteBufferActive() { return NULL != m_pucNextStreamPacket; }
  UChar* getNextBuffersPacket() { return m_pucNextStreamPacket; }

  static ErrVal create( BitWriteBuffer*& rpcBitWriteBuffer );
  ErrVal destroy();

  ErrVal init();
  ErrVal uninit() { return init(); }

  ErrVal initPacket( UInt* pulBits, UInt uiPacketLength );

  ErrVal write( UInt uiBits, UInt uiNumberOfBits = 1);

  UInt getNumberOfWrittenBits() { return  m_uiBitsWritten; }

  ErrVal pcmSamples( const TCoeff* pCoeff, UInt uiNumberOfSamples );

  ErrVal flushBuffer();
  ErrVal writeAlignZero();
  ErrVal writeAlignOne();

  ErrVal getLastByte(UChar &uiLastByte, UInt &uiLastBitPos);//FIX_FRAG_CAVLC
	//JVT-X046 {
	UInt   getDWordsLeft(void)  { return m_uiDWordsLeft;   }
	UInt   getBitsWritten(void) { return m_uiBitsWritten;  }
	Int    getValidBits(void)   { return m_iValidBits;     }
	UInt	 getCurrentBits(void) { return m_ulCurrentBits;  }
	UInt*  getStreamPacket(void){ return m_pulStreamPacket;}
	void loadBitWriteBuffer(BitWriteBufferIf* pcBitWriteBufferIf)
	{
		BitWriteBuffer* pcBitWriteBuffer = (BitWriteBuffer*)(pcBitWriteBufferIf);
		m_uiDWordsLeft = pcBitWriteBuffer->getDWordsLeft();
		m_uiBitsWritten = pcBitWriteBuffer->getBitsWritten();
		m_iValidBits = pcBitWriteBuffer->getValidBits();
		m_ulCurrentBits = pcBitWriteBuffer->getCurrentBits();
		m_pulStreamPacket = pcBitWriteBuffer->getStreamPacket();
	}
	void loadBitCounter(BitWriteBufferIf* pcBitWriteBufferIf){}
	UInt getBitsWriten() { return m_uiBitsWritten; }
	//JVT-X046 }
protected:
  UInt  xSwap( UInt ul )
  {
    // heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
    return ul;
#else
    UInt ul2;

    ul2  = ul>>24;
    ul2 |= (ul>>8) & 0x0000ff00;
    ul2 |= (ul<<8) & 0x00ff0000;
    ul2 |= ul<<24;

    return ul2;
#endif
  }

protected:
  UInt   m_uiDWordsLeft;
  UInt   m_uiBitsWritten;
  Int    m_iValidBits;
  UInt   m_ulCurrentBits;
  UInt*  m_pulStreamPacket;

private:
  UInt   m_uiInitPacketLength;
  BitWriteBuffer * m_pcNextBitWriteBuffer;
  UChar *m_pucNextStreamPacket;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_BITWRITEBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_)
