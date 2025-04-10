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

#ifndef _SCALABLE_MODIFY_CODE_
#define _SCALABLE_MODIFY_CODE_

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Sei.h"

class ScalableSEIModifyCode
{
protected:
	ScalableSEIModifyCode();
	~ScalableSEIModifyCode()	{}

public:
  static ErrVal Create         ( ScalableSEIModifyCode*& rpcScalableSEIModifyCode );
	ErrVal Destroy               ();
	ErrVal init                  ( UInt* pulStream, UInt uiBytes );
	ErrVal Uninit                ();
  ErrVal Write                 ( UInt uiBits, UInt uiNumberOfBits );
	ErrVal WriteUVLC             ( UInt uiValue );
	ErrVal WriteFlag             ( Bool bFlag );
	ErrVal WriteCode             ( UInt uiValue, UInt uiLength );
	ErrVal WriteAlignZero        ();
	ErrVal flushBuffer           ();
	UInt	 getNumberOfWrittenBits() { return m_uiBitsWritten; }
	ErrVal WritePayloadHeader    ( enum h264::SEI::MessageType eType, UInt uiSize );
	ErrVal WriteTrailingBits     ();
	ErrVal ConvertRBSPToPayload  ( UChar* m_pucBuffer, UChar pucStreamPacket[], UInt& uiBits, UInt uiHeaderBytes );

	ErrVal SEICode	( h264::SEI::ScalableSei* pcScalableSei, ScalableSEIModifyCode *pcScalableCodeIf );
	ErrVal SEICode	( h264::SEI::ScalableSeiLayersNotPresent* pcScalableSeiLayersNotPresent, ScalableSEIModifyCode *pcScalableCodeIf );
	ErrVal SEICode	( h264::SEI::ScalableSeiDependencyChange* pcScalableSeiDependencyChange, ScalableSEIModifyCode *pcScalableCodeIf );

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

private:
	BinData *m_pcBinData;
	UInt *m_pulStreamPacket;
	UInt m_uiBitCounter;
	UInt m_uiPosCounter;
	UInt m_uiDWordsLeft;
	UInt m_uiBitsWritten;
	UInt m_iValidBits;
	UInt m_ulCurrentBits;
	UInt m_uiCoeffCost;
};

#endif




