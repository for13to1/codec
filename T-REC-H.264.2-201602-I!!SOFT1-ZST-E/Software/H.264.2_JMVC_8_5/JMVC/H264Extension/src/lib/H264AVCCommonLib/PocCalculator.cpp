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


#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include <math.h>



H264AVC_NAMESPACE_BEGIN



PocCalculator::PocCalculator()
: m_iLastIdrFieldNum    ( 0 )
  , m_iBitsLsb            ( 0 )
  , m_iTop2BotOffset      ( 1 )
  , m_iPrevRefPocMsb      ( 0 )
  , m_iPrevRefPocLsb      ( -1) //--ICU/ETRI FMO Implementation
  , m_iMaxPocLsb          ( 0 )
  , m_iFrameNumOffset     ( 0 )
  , m_iRefOffsetSum       ( 0 )
  , m_iPrevFrameNum       ( 0 )
{
}


ErrVal PocCalculator::create( PocCalculator*& rpcPocCalculator )
{
  rpcPocCalculator = new PocCalculator;

  ROT( NULL == rpcPocCalculator );

  return Err::m_nOK;
}


ErrVal PocCalculator::copy( PocCalculator*& rpcPocCalculator )
{
  rpcPocCalculator = new PocCalculator;

  ROT( NULL == rpcPocCalculator );

  rpcPocCalculator->m_iLastIdrFieldNum = m_iLastIdrFieldNum;
  rpcPocCalculator->m_iBitsLsb         = m_iBitsLsb;
  rpcPocCalculator->m_iTop2BotOffset   = m_iTop2BotOffset;
  rpcPocCalculator->m_iPrevRefPocMsb   = m_iPrevRefPocMsb;
  rpcPocCalculator->m_iPrevRefPocLsb   = m_iPrevRefPocLsb;
  rpcPocCalculator->m_iMaxPocLsb       = m_iMaxPocLsb;
  rpcPocCalculator->m_iFrameNumOffset  = m_iFrameNumOffset;
  rpcPocCalculator->m_iRefOffsetSum    = m_iRefOffsetSum;
  rpcPocCalculator->m_iPrevFrameNum    = m_iPrevFrameNum;

  return Err::m_nOK;
}


ErrVal PocCalculator::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal PocCalculator::initSPS( const SequenceParameterSet& rcSequenceParameterSet )
{
    switch( rcSequenceParameterSet.getPicOrderCntType() )
    {
    case 0:
        {
            m_iPrevRefPocMsb  = 0;
            m_iPrevRefPocLsb  = 0;
            m_iMaxPocLsb      = ( 1 << rcSequenceParameterSet.getLog2MaxPicOrderCntLsb() );
        }
        break;
    case 1:
        {
            m_iFrameNumOffset  = 0;
            m_iRefOffsetSum    = 0;
            for( UInt uiIndex = 0; uiIndex < rcSequenceParameterSet.getNumRefFramesInPicOrderCntCycle(); uiIndex++ )
            {
                m_iRefOffsetSum += rcSequenceParameterSet.getOffsetForRefFrame( uiIndex );
            }
        }
        break;
    case 2:
        {
            m_iFrameNumOffset  = 0;
        }
        break;
    default:
        {
            return Err::m_nERR;
        }
        break;
    }

    return Err::m_nOK;
}

ErrVal PocCalculator::calculatePoc( SliceHeader& rcSliceHeader )
    {
  if( rcSliceHeader.isIdrNalUnit() )
    {
    RNOK( xInitSPS( rcSliceHeader.getSPS() ) );
    }

  switch( rcSliceHeader.getSPS().getPicOrderCntType() )
  {
  case 0:
    {
      //===== POC mode 0 =====
      Int iCurrPocMsb = m_iPrevRefPocMsb;
      Int iCurrPocLsb = rcSliceHeader.getPicOrderCntLsb();
      Int iDiffPocLsb = m_iPrevRefPocLsb - iCurrPocLsb;

      if( rcSliceHeader.isIdrNalUnit() )
      {
/* // JVT-Q065 EIDR
		  iCurrPocMsb   = 0;
		  iCurrPocLsb   = 0;
*/
      }
      else if( iDiffPocLsb >= ( m_iMaxPocLsb >> 1 ) )
      {
        iCurrPocMsb  += m_iMaxPocLsb;
      }
      else if( -iDiffPocLsb > ( m_iMaxPocLsb >> 1 ) )
      {
        iCurrPocMsb  -= m_iMaxPocLsb;
      }
      if( rcSliceHeader.getNalRefIdc() )
      {
        m_iPrevRefPocMsb = iCurrPocMsb;
        m_iPrevRefPocLsb = iCurrPocLsb;
      }

#if JM_MVC_COMPATIBLE
#define DELTA_POCA  DELTA_POC
#else
#define DELTA_POCA  0
#endif

      if( rcSliceHeader.getPicType() & TOP_FIELD )
      {
          rcSliceHeader.setTopFieldPoc( iCurrPocMsb + iCurrPocLsb+DELTA_POCA );

          if( rcSliceHeader.getPicType() == FRAME )
          {
              rcSliceHeader.setBotFieldPoc( rcSliceHeader.getTopFieldPoc() + rcSliceHeader.getDeltaPicOrderCntBottom() +DELTA_POCA);
          }
      }
      else
      {
          rcSliceHeader.setBotFieldPoc( iCurrPocMsb + iCurrPocLsb+DELTA_POCA );
      }
      rcSliceHeader.setPoc( iCurrPocMsb + iCurrPocLsb + DELTA_POCA );
    }
    break;
  case 1:
    {
      //===== POC mode 1 =====
      Int   iExpectedPoc    = 0;
      UInt  uiAbsFrameNum   = 0;

      //--- update parameters, set AbsFrameNum ---
      if( ! rcSliceHeader.isIdrNalUnit() && m_iPrevFrameNum > (Int)rcSliceHeader.getFrameNum() )
      {
        m_iFrameNumOffset  += ( 1 << rcSliceHeader.getSPS().getLog2MaxFrameNum() );
	}
      m_iPrevFrameNum       = rcSliceHeader.getFrameNum();
      if( rcSliceHeader.getSPS().getNumRefFramesInPicOrderCntCycle() )
      {
        uiAbsFrameNum       = m_iFrameNumOffset + m_iPrevFrameNum;
        if( uiAbsFrameNum > 0 && rcSliceHeader.getNalRefIdc() == 0 )
    {
          uiAbsFrameNum--;
        }
      }

      //--- get expected POC ---
      if( uiAbsFrameNum > 0 )
      {
        Int iPocCycleCount  = ( uiAbsFrameNum - 1 ) / rcSliceHeader.getSPS().getNumRefFramesInPicOrderCntCycle();
        Int iFrameNumCycle  = ( uiAbsFrameNum - 1 ) % rcSliceHeader.getSPS().getNumRefFramesInPicOrderCntCycle();
        iExpectedPoc        = iPocCycleCount * m_iRefOffsetSum;
        
        for( Int iIndex = 0; iIndex <= iFrameNumCycle; iIndex++ )
        {
          iExpectedPoc     += rcSliceHeader.getSPS().getOffsetForRefFrame( iIndex );
        } 
      }
      else
      {
          iExpectedPoc = 0;
      }

      if( rcSliceHeader.getNalRefIdc() == 0 )
      {
        iExpectedPoc       += rcSliceHeader.getSPS().getOffsetForNonRefPic();
      }

      //--- set POC ---
      if( rcSliceHeader.getPicType() & TOP_FIELD )
      {
          rcSliceHeader.setTopFieldPoc( iExpectedPoc + rcSliceHeader.getDeltaPicOrderCnt( 0 ) );

          if( rcSliceHeader.getPicType() == FRAME )
          {
              rcSliceHeader.setBotFieldPoc( rcSliceHeader.getTopFieldPoc() + rcSliceHeader.getDeltaPicOrderCnt( 1 ) + rcSliceHeader.getSPS().getOffsetForTopToBottomField() );
          }
      }
      else
      {
          rcSliceHeader.setBotFieldPoc( iExpectedPoc + rcSliceHeader.getDeltaPicOrderCnt( 0 ) + rcSliceHeader.getSPS().getOffsetForTopToBottomField() );
      }
    }
    break;
  case 2:
    {
      //===== POC mode 2 =====
      Int iCurrPoc = 0; // for IDR NAL unit
      
      if( ! rcSliceHeader.isIdrNalUnit() )
      {
        if( (Int)rcSliceHeader.getFrameNum() < m_iPrevFrameNum )
        {
          m_iFrameNumOffset += ( 1 << rcSliceHeader.getSPS().getLog2MaxFrameNum() );
        }
        if( rcSliceHeader.getNalRefIdc() )
        {
          iCurrPoc           = 2 * ( m_iFrameNumOffset + rcSliceHeader.getFrameNum() );
        }
        else
        {
          iCurrPoc           = 2 * ( m_iFrameNumOffset + rcSliceHeader.getFrameNum() ) - 1;
        }
      }
      if( rcSliceHeader.getNalRefIdc() )
      {
        m_iPrevFrameNum = rcSliceHeader.getFrameNum();
      }
      if( rcSliceHeader.getPicType() == FRAME )
      {
          rcSliceHeader.setTopFieldPoc( iCurrPoc );
          rcSliceHeader.setBotFieldPoc( iCurrPoc );
      }
      else if ( rcSliceHeader.getPicType() == TOP_FIELD )
      {
          rcSliceHeader.setTopFieldPoc( iCurrPoc );
      }
      else
      {
          rcSliceHeader.setBotFieldPoc( iCurrPoc );
      }
    }
    break;
  default:
    RERR();
    break;
  }
  return Err::m_nOK;
}


ErrVal
PocCalculator::xInitSPS( const SequenceParameterSet& rcSPS )
{
  switch( rcSPS.getPicOrderCntType() )
  {
  case 0:
    {
      m_iPrevRefPocMsb  = 0;
      m_iPrevRefPocLsb  = 0;
      m_iMaxPocLsb      = ( 1 << rcSPS.getLog2MaxPicOrderCntLsb() );
    }
    break;
  case 1:
    {
      m_iFrameNumOffset = 0;
      m_iRefOffsetSum   = 0;
      for( UInt uiIndex = 0; uiIndex < rcSPS.getNumRefFramesInPicOrderCntCycle(); uiIndex++ )
    {
        m_iRefOffsetSum+= rcSPS.getOffsetForRefFrame( uiIndex );
      }
    }
    break;
  case 2:
    {
      m_iFrameNumOffset = 0;
    }
    break;
  default:
    RERR();
    break;
  }
  return Err::m_nOK;
}


ErrVal PocCalculator::setPoc( SliceHeader&  rcSliceHeader,
                              Int           iContFrameNumber )
{
  ROTRS( iContFrameNumber > ( INT_MAX - 1 ), Err::m_nERR );

//  UInt iCurrFieldNum = ( iContFrameNumber/* << 1*/ ) + (m_iTop2BotOffset?1:0) + ( rcSliceHeader.getBottomFieldFlag() ? m_iTop2BotOffset : 0 );//thBug
UInt iCurrFieldNum=iContFrameNumber;

if( rcSliceHeader.isIdrNalUnit() && !rcSliceHeader.getBottomFieldFlag() )
  {
      m_iBitsLsb          = rcSliceHeader.getSPS().getLog2MaxPicOrderCntLsb();
      m_iLastIdrFieldNum  = iCurrFieldNum;
  }

  Int iCurrPoc = iCurrFieldNum - m_iLastIdrFieldNum;

  if( rcSliceHeader.getPicType() & TOP_FIELD )
  {
      rcSliceHeader.setTopFieldPoc  ( iCurrPoc );

      if( rcSliceHeader.getPicType() == FRAME )
      {
          rcSliceHeader.setBotFieldPoc( rcSliceHeader.getTopFieldPoc() + ( rcSliceHeader.getPPS().getPicOrderPresentFlag() ? m_iTop2BotOffset : 0 ) );
		  if (!rcSliceHeader.getSPS().getFrameMbsOnlyFlag()) // hwsun, fix a bug (add if)
		  {
			rcSliceHeader.setDeltaPicOrderCntBottom ( rcSliceHeader.getBotFieldPoc() - rcSliceHeader.getTopFieldPoc() );
		  }
      }
  }
  else
  {
      rcSliceHeader.setBotFieldPoc  ( iCurrPoc );
  }

  rcSliceHeader.setPicOrderCntLsb ( iCurrPoc & ~ ( -1 << m_iBitsLsb ) );

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END


