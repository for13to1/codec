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


#include "H264AVCEncoderLib.h"
#include "BitWriteBuffer.h"
#include "NalUnitEncoder.h"


H264AVC_NAMESPACE_BEGIN


NalUnitEncoder::NalUnitEncoder()
: m_bIsUnitActive         ( false )
, m_pcBitWriteBuffer      ( 0 )
, m_pcHeaderSymbolWriteIf ( 0 )
, m_pcHeaderSymbolTestIf  ( 0 )
, m_pcBinDataAccessor     ( 0 )
, m_pucBuffer             ( 0 )
, m_pucTempBuffer         ( 0 )
, m_uiPacketLength        ( MSYS_UINT_MAX )
, m_eNalUnitType          ( NAL_UNIT_EXTERNAL )
{
}


NalUnitEncoder::~NalUnitEncoder()
{
}


ErrVal
NalUnitEncoder::create( NalUnitEncoder*& rpcNalUnitEncoder )
{
  rpcNalUnitEncoder = new NalUnitEncoder;
  ROT( NULL == rpcNalUnitEncoder );
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::init( BitWriteBuffer*       pcBitWriteBuffer,
                      HeaderSymbolWriteIf*  pcHeaderSymbolWriteIf,
                      HeaderSymbolWriteIf*  pcHeaderSymbolTestIf )
{
  ROT( NULL == pcBitWriteBuffer );
  ROT( NULL == pcHeaderSymbolWriteIf );
  ROT( NULL == pcHeaderSymbolTestIf );

  m_pcBitWriteBuffer      = pcBitWriteBuffer;
  m_pcHeaderSymbolTestIf  = pcHeaderSymbolTestIf;
  m_pcHeaderSymbolWriteIf = pcHeaderSymbolWriteIf;
  m_bIsUnitActive         = false;
  m_pucBuffer             = NULL;
  m_pucTempBuffer         = NULL;
  m_uiPacketLength        = MSYS_UINT_MAX;
  m_eNalUnitType          = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::uninit()
{
  delete [] m_pucTempBuffer;

  m_pcBitWriteBuffer      = NULL;
  m_pcHeaderSymbolWriteIf = NULL;
  m_pcHeaderSymbolTestIf  = NULL;
  m_bIsUnitActive         = false;
  m_pucBuffer             = NULL;
  m_pucTempBuffer         = NULL;
  m_uiPacketLength        = MSYS_UINT_MAX;
  m_eNalUnitType          = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::destroy()
{
  uninit();
  delete this;
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::initNalUnit( BinDataAccessor* pcBinDataAccessor )
{
  ROT( m_bIsUnitActive );
  ROF( pcBinDataAccessor );
  ROT( pcBinDataAccessor->size() < 1 );
  
  m_bIsUnitActive     = true;
  m_pcBinDataAccessor = pcBinDataAccessor;
  m_pucBuffer         = pcBinDataAccessor->data();

  if( m_uiPacketLength != m_pcBinDataAccessor->size() )
  {
    delete [] m_pucTempBuffer;

    m_uiPacketLength = m_pcBinDataAccessor->size();
    m_pucTempBuffer  = new UChar[ m_uiPacketLength ];
    ROF( m_pucTempBuffer );
  }

  RNOK( m_pcBitWriteBuffer->initPacket( (UInt32*)(m_pucTempBuffer), m_uiPacketLength-1 ) );

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::closeNalUnit( UInt& ruiBits )
{
  ROF( m_bIsUnitActive );

  //===== write trailing bits =====
  if( NAL_UNIT_END_OF_SEQUENCE != m_eNalUnitType &&
      NAL_UNIT_END_OF_STREAM      != m_eNalUnitType &&
      NAL_UNIT_CODED_SLICE_PREFIX != m_eNalUnitType )   // bug fix: no trailing bit for prefix NAL (NTT)
  {
    RNOK ( xWriteTrailingBits() );
  }
  RNOK( m_pcBitWriteBuffer->flushBuffer() );

  //===== convert to payload and add header =====
  Bool  bDDIPresent   = ( m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
                          m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE );
  UInt  uiHeaderBytes = ( bDDIPresent ? 2 : 1 );
  UInt  uiBits        = m_pcBitWriteBuffer->getNumberOfWrittenBits();
  uiBits              = ( uiBits >> 3 ) + ( 0 != ( uiBits & 0x07 ) );
  RNOK( xConvertRBSPToPayload( uiBits, uiHeaderBytes ) );
  RNOK( m_pcBinDataAccessor->decreaseEndPos( m_pcBinDataAccessor->size() - uiBits ) );
  ruiBits             = 8*uiBits;

  //==== reset parameters =====
  m_bIsUnitActive     = false;
  m_pucBuffer         = NULL;
  m_pcBinDataAccessor = NULL;
  m_eNalUnitType      = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::xConvertRBSPToPayload( UInt& ruiBytesWritten,
                                       UInt  uiHeaderBytes )
{
  UInt uiZeroCount    = 0;
  UInt uiReadOffset   = uiHeaderBytes;
  UInt uiWriteOffset  = uiHeaderBytes;

  //===== NAL unit header =====
  for( UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++ )
  {
    m_pucBuffer[uiIndex] = m_pucTempBuffer[uiIndex];
  }

  //===== NAL unit payload =====
  for( ; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++ )
  {
    ROT( uiWriteOffset >= m_uiPacketLength );

    if( 2 == uiZeroCount && 0 == ( m_pucTempBuffer[uiReadOffset] & 0xfc ) )
    {
      uiZeroCount                   = 0;
      m_pucBuffer[uiWriteOffset++]  = 0x03;
    }

    m_pucBuffer[uiWriteOffset] = m_pucTempBuffer[uiReadOffset];

    if( 0 == m_pucTempBuffer[uiReadOffset] )
    {
      uiZeroCount++;
    }
    else
    {
      uiZeroCount = 0;
    }
  }
  if( ( 0x00 == m_pucBuffer[uiWriteOffset-1] ) && ( 0x00 == m_pucBuffer[uiWriteOffset-2] ) )
  {
    ROT( uiWriteOffset >= m_uiPacketLength );
    m_pucBuffer[uiWriteOffset++] = 0x03;
  }
  ruiBytesWritten = uiWriteOffset;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::xWriteTrailingBits( UInt uiFixedNumberOfBits )
{
  if( uiFixedNumberOfBits )
  {
    RNOK( m_pcBitWriteBuffer->write( 1 << ( uiFixedNumberOfBits - 1 ), uiFixedNumberOfBits ) );
    return Err::m_nOK;
  }

  RNOK( m_pcBitWriteBuffer->write( 1 ) );
  RNOK( m_pcBitWriteBuffer->writeAlignZero() );

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const SequenceParameterSet& rcSPS )
{
  RNOK( rcSPS.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcSPS.getNalUnitType();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const PictureParameterSet& rcPPS )
{
  RNOK( rcPPS.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcPPS.getNalUnitType();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const SliceHeader& rcSH )
{
  RNOK( rcSH.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcSH.getNalUnitType();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::write( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );

  m_eNalUnitType  = NAL_UNIT_SEI;
  return Err::m_nOK;
}

//SEI LSJ{
ErrVal
NalUnitEncoder::writeNesting( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::writeNesting( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );
  m_eNalUnitType = NAL_UNIT_SEI;
  return Err::m_nOK;
}
//SEI LSJ}

H264AVC_NAMESPACE_END
