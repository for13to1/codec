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

#include "MbTempData.h"

#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"


  
H264AVC_NAMESPACE_BEGIN


ErrVal IntMbTempData::init( MbDataAccess& rcMbDataAccess )
{
  m_pcMbDataAccess = new( m_pcMbDataAccess ) MbDataAccess( rcMbDataAccess, *this );
  clear();

  return Err::m_nOK;
}

ErrVal IntMbTempData::uninit()
{
  return Err::m_nOK;
}


IntMbTempData::IntMbTempData() :
m_pcMbDataAccess( NULL )
{
  m_pcMbDataAccess = NULL;  
  clear();

  MbData::init( this, &m_acMbMvdData[0], &m_acMbMvdData[1], &m_acMbMotionData[0], &m_acMbMotionData[1] );
}


IntMbTempData::~IntMbTempData()
{
  delete m_pcMbDataAccess;
  m_pcMbDataAccess = NULL;
}


Void IntMbTempData::clear()
{
  MbDataStruct::clear();
  CostData::clear();
  MbTransformCoeffs::clear();
}



Void IntMbTempData::clearCost()
{
  CostData::clear();
}



Void IntMbTempData::copyTo( MbDataAccess& rcMbDataAccess )
{
  rcMbDataAccess.getMbData()            .copyFrom( *this );
  rcMbDataAccess.getMbTCoeffs()         .copyFrom( *this );

  rcMbDataAccess.getMbMvdData(LIST_0)   .copyFrom( m_acMbMvdData[LIST_0] );
  rcMbDataAccess.getMbMotionData(LIST_0).copyFrom( m_acMbMotionData[LIST_0] );

  if( rcMbDataAccess.getSH().isInterB() )
  {
    rcMbDataAccess.getMbMvdData(LIST_1)   .copyFrom( m_acMbMvdData[LIST_1] );
    rcMbDataAccess.getMbMotionData(LIST_1).copyFrom( m_acMbMotionData[LIST_1] );
  }

}


Void IntMbTempData::copyResidualDataTo( MbDataAccess& rcMbDataAccess )
{
  rcMbDataAccess.getMbData    ().setBCBP              ( getBCBP             () );
  rcMbDataAccess.getMbData    ().setMbExtCbp          ( getMbExtCbp         () );
  rcMbDataAccess.getMbData    ().setQp                ( getQp               () );
  rcMbDataAccess.getMbTCoeffs ().copyFrom             ( *this                  );
  rcMbDataAccess.getMbData    ().setTransformSize8x8  ( isTransformSize8x8  () );

  rcMbDataAccess.getMbData    ().setResidualPredFlags ( getResidualPredFlags() );
}


Void IntMbTempData::loadChromaData( IntMbTempData& rcMbTempData )
{
  ::memcpy( get(CIdx(0)), rcMbTempData.get(CIdx(0)), sizeof(TCoeff)*128);
  setChromaPredMode( rcMbTempData.getChromaPredMode() );
  IntYuvMbBuffer::loadChroma( rcMbTempData );
  distU()  = rcMbTempData.distU();
  distV()  = rcMbTempData.distV();
  getTempYuvMbBuffer().loadChroma( rcMbTempData.getTempYuvMbBuffer() );
}


H264AVC_NAMESPACE_END
