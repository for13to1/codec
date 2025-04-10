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

#if !defined(AFX_SCHEDULER_H__4242CFD4_A40A_4FCE_B740_60624D030E86__INCLUDED_)
#define AFX_SCHEDULER_H__4242CFD4_A40A_4FCE_B740_60624D030E86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/Sei.h"
#include "CodingParameter.h"

// h264 namespace begin
H264AVC_NAMESPACE_BEGIN

// #define DEBUG_SCHEDULER
#define DEBUG_SCHEDULER_FILE "scheduler.txt"

class Scheduler
{
  class TimingUnit
  {
  public:
    Int64   m_iDataLength; // in Bytes
    Int64   m_iBitRate;    // in Bits
    Double  m_dInitialArrivalEarliest;
    Double  m_dInitialArrival;
    Double  m_dFinalArrival;
    Double  m_dRemoval;
    UInt    m_uiFirstIrd;
    UInt    m_uiIrd;
    UInt    m_uiIrdOffset;
    Bool    m_bCbr;

    TimingUnit();
    ErrVal  calcTiming( UInt uiSize, Double dTime, Bool bIsIdr);
    ErrVal  calcIrd(Double dTime);
  };


protected:
        Scheduler();
  virtual ~Scheduler() {};

public:
  static ErrVal create( Scheduler*& rpcScheduler );
  ErrVal destroy();

  ErrVal createBufferingSei( SEI::BufferingPeriod*& rpcBufferingPeriod, ParameterSetMng* pcParameterSetMng, UInt uiDQId );
  ErrVal createTimingSei( SEI::PicTiming*& rpcPicTiming, const VUI* pcVui, UInt uiPicNumOffset, SliceHeader &rcSH, UInt uiInputFormat, UInt uiLayerIndex);

  ErrVal calculateTiming( UInt uiVclSize, UInt uiAUSize, Bool bIsIdr, Bool bFieldPicFlag);
//  ErrVal calculateIrd();
  ErrVal init( CodingParameter *pcCodingParameter, UInt uiLayer );
  ErrVal initBuffer( const VUI* pcVui, UInt uiLayerIndex);
  ErrVal uninit();

  Void setLayerBits( UInt uiBits ) { m_uiLayerBits = uiBits; }
  UInt getLayerBits()              { return m_uiLayerBits; }
protected:
  ErrVal xInitHrd (const HRD& rcHrd, const HRD::HrdParamType eHrdParamType);
  ErrVal xCreateBufferingSeiHrd( HRD::HrdParamType eHrdParamType, const HRD &rcHrd, SEI::BufferingPeriod* pcBPSei);
  ErrVal xCalculateTiming( HRD::HrdParamType eHrdParamType, UInt uiSize, Bool bIsIdr );

  Double  m_dFieldTimeDelta;
  Double  m_dOutputFrequency;
  UInt    m_uiOutputTicks;
  Double  m_dClockFrequency;
  Double  m_dActualOutTime;
  Double  m_dActualInTime;
  Double  m_dLastBPTime;

  StatBuf< DynBuf< TimingUnit >,2 >  m_aacTiming;
  const HRD* m_apcHrd[2];

  CodingParameter *m_pcCodingParameter;

  FILE *m_pfFileDebug;
  Bool m_bInitDone;
  UInt m_uiLayerBits;
};

// h264 namespace end
H264AVC_NAMESPACE_END

#endif // !defined(AFX_Scheduler_H__4242CFD4_A40A_4FCE_B740_60624D030E86__INCLUDED_)
