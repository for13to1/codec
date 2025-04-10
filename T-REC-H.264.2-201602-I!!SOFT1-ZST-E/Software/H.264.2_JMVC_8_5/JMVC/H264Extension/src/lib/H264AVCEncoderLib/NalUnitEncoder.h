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


#if !defined(AFX_NALUNITENCODER_H__DA2EE2CC_46F5_4F11_B046_FA18CD441B65__INCLUDED_)
#define AFX_NALUNITENCODER_H__DA2EE2CC_46F5_4F11_B046_FA18CD441B65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/Sei.h"



H264AVC_NAMESPACE_BEGIN


class BitWriteBuffer;


class NalUnitEncoder
{
protected:
	NalUnitEncoder          ();
	virtual ~NalUnitEncoder ();

public:
  static ErrVal create    ( NalUnitEncoder*&            rpcNalUnitEncoder );
  ErrVal        destroy   ();

  ErrVal init             ( BitWriteBuffer*             pcBitWriteBuffer,
                            HeaderSymbolWriteIf*        pcHeaderSymbolWriteIf,
                            HeaderSymbolWriteIf*        pcHeaderSymbolTestIf  );
  ErrVal uninit           ();

  ErrVal initNalUnit      ( BinDataAccessor*            pcBinDataAccessor );
  ErrVal closeNalUnit     ( UInt&                       ruiBits );

  ErrVal write            ( const SequenceParameterSet& rcSPS );
  ErrVal write            ( const PictureParameterSet&  rcPPS );
  ErrVal write            ( const SliceHeader&          rcSH  );
  ErrVal write            ( SEI::MessageList&           rcSEIMessageList );

  ErrVal writeNesting     ( SEI::MessageList&           rcSEIMessageList );//SEI LSJ
protected:
  ErrVal xConvertRBSPToPayload( UInt& ruiBytesWritten,
                                UInt  uiHeaderBytes );
  ErrVal xWriteTrailingBits   ( UInt  uiFixedNumberOfBits = 0);

protected:
  Bool                  m_bIsUnitActive;
  BitWriteBuffer*       m_pcBitWriteBuffer;
  HeaderSymbolWriteIf*  m_pcHeaderSymbolWriteIf;
  HeaderSymbolWriteIf*  m_pcHeaderSymbolTestIf;
  BinDataAccessor*      m_pcBinDataAccessor;
  UChar*                m_pucBuffer;
  UChar*                m_pucTempBuffer;
  UInt                  m_uiPacketLength;
  NalUnitType           m_eNalUnitType;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_NALUNITENCODER_H__DA2EE2CC_46F5_4F11_B046_FA18CD441B65__INCLUDED_)
