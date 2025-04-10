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

#include "H264AVCDecoderLib.h"

#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "MbParser.h"
#include "GOPDecoder.h"

#include "SliceReader.h"
#include "DecError.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceReader::SliceReader()
: m_bInitDone ( false )
, m_pcMbParser( 0 )
{
}

SliceReader::~SliceReader()
{
}

ErrVal
SliceReader::create( SliceReader*& rpcSliceReader )
{
  rpcSliceReader = new SliceReader;
  ROT( NULL == rpcSliceReader );
  return Err::m_nOK;
}

ErrVal
SliceReader::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
SliceReader::init( MbParser* pcMbParser )
{
  ROT( m_bInitDone );
  ROF( pcMbParser );

  m_pcMbParser  = pcMbParser;
  m_bInitDone   = true;
  return Err::m_nOK;
}


ErrVal
SliceReader::uninit()
{
  ROF( m_bInitDone );
  m_pcMbParser  = 0;
  m_bInitDone   = false;
  return Err::m_nOK;
}


ErrVal
SliceReader::read( SliceHeader& rcSH,
                   MbDataCtrl*  pcMbDataCtrl,
                   MbStatus*    pacMbStatus,
                   UInt         uiMbInRow,
                   UInt&        ruiMbRead )
{
  ROF( m_bInitDone );

	//====== initialization ======
  UInt  uiMbAddress       = rcSH.getFirstMbInSlice();
  Bool  bEndOfSlice       = false;
  UInt  uiNextSkippedVLC  = 0;

  RNOK( pcMbDataCtrl->initSlice( rcSH, PARSE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
  for( ruiMbRead = 0; !bEndOfSlice; ruiMbRead++ ) //--ICU/ETRI FMO Implementation
  {
    if( !uiNextSkippedVLC )
    {
      DTRACE_NEWMB( uiMbAddress );
    }

    MbDataAccess* pcMbDataAccess      = 0;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress );
    Bool bCropWindowFlag = pcMbDataCtrl->getMbData( uiMbX, uiMbY ).getInCropWindowFlag();

    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    pcMbDataAccess->getMbData().setInCropWindowFlag( bCropWindowFlag );

  	if( rcSH.isMbaffFrame() && uiMbAddress % 2 == 0 )
    {
      pcMbDataAccess->setFieldMode( pcMbDataAccess->getDefaultFieldFlag() );
    }

    RNOK( m_pcMbParser->read( *pcMbDataAccess, ruiMbRead, bEndOfSlice, uiNextSkippedVLC ) );
    UInt      uiMbIndex   = rcSH.getMbIndexFromAddress( uiMbAddress );
    MbStatus& rcMbStatus  = pacMbStatus[ uiMbIndex ];
    RNOK( rcMbStatus.update( *pcMbDataAccess ) );

    if( bEndOfSlice )
    {
      rcSH.setLastMbInSlice(uiMbAddress);
    }
    uiMbAddress  = rcSH.getFMO()->getNextMBNr(uiMbAddress );
  }

  rcSH.setNumMbsInSlice(ruiMbRead);

  int sgId = rcSH.getFMO()->getSliceGroupId(rcSH.getFirstMbInSlice());
  int pocOrder = rcSH.getPicOrderCntLsb();

  rcSH.getFMO()->setCodedSG(sgId, pocOrder);

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
