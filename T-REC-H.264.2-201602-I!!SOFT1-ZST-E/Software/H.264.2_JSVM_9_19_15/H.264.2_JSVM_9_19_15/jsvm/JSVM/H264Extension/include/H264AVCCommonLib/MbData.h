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

#if !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)
#define AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/Mv.h"
#include "H264AVCCommonLib/MbMvData.h"
#include "H264AVCCommonLib/MbTransformCoeffs.h"
#include "H264AVCCommonLib/MbDataStruct.h"

H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API MbData :
public MbDataStruct
{
public:
  MbData()
  : m_pcMbTCoeffs( 0 )
  {
    m_apcMbMvdData   [ LIST_0 ] = NULL;
    m_apcMbMvdData   [ LIST_1 ] = NULL;
    m_apcMbMotionData[ LIST_0 ] = NULL;
    m_apcMbMotionData[ LIST_1 ] = NULL;
    m_aabBaseIntra   [ 0 ][ 0 ] = false;
    m_aabBaseIntra   [ 0 ][ 1 ] = false;
    m_aabBaseIntra   [ 1 ][ 0 ] = false;
    m_aabBaseIntra   [ 1 ][ 1 ] = false;
  }
  ~MbData()
  {
  }

  UInt calcMbCbp ( UInt uiStart, UInt uiStop ) const;
  Bool calcBCBP  ( UInt uiStart, UInt uiStop, UInt uiPos ) const;

  Void init(  MbTransformCoeffs*  pcMbTCoeffs,
              MbMvData*           pcMbMvdDataList0,
              MbMvData*           pcMbMvdDataList1,
              MbMotionData*       pcMbMotionDataList0,
              MbMotionData*       pcMbMotionDataList1 )
  {
    m_pcMbTCoeffs           = pcMbTCoeffs;
    m_apcMbMvdData[0]       = pcMbMvdDataList0;
    m_apcMbMvdData[1]       = pcMbMvdDataList1;
    m_apcMbMotionData[0]    = pcMbMotionDataList0;
    m_apcMbMotionData[1]    = pcMbMotionDataList1;
  }

public:
  MbTransformCoeffs&        getMbTCoeffs    ()                          { return *m_pcMbTCoeffs; }
  MbMvData&                 getMbMvdData    ( ListIdx eListIdx )        { return *m_apcMbMvdData   [ eListIdx ]; }
  MbMotionData&             getMbMotionData ( ListIdx eListIdx )        { return *m_apcMbMotionData[ eListIdx ]; }

  const MbTransformCoeffs&  getMbTCoeffs    ()                    const { return *m_pcMbTCoeffs; }
  const MbMvData&           getMbMvdData    ( ListIdx eListIdx )  const { return *m_apcMbMvdData   [ eListIdx ]; }
  const MbMotionData&       getMbMotionData ( ListIdx eListIdx )  const { return *m_apcMbMotionData[ eListIdx ]; }

  operator MbTransformCoeffs& ()                                        { return *m_pcMbTCoeffs; }

  Bool    isBaseIntra(Int iPosX, Int iPosY) const { return m_aabBaseIntra[iPosX][iPosY]; }
  Void    setBaseIntra( Int iPosX, Int iPosY, Bool b ) { m_aabBaseIntra[iPosX][iPosY] = b; }

  Void    copy( const MbData& rcMbData );
  ErrVal  loadAll( FILE* pFile );
  ErrVal  saveAll( FILE* pFile );

  ErrVal  copyMotion    ( const MbData& rcMbData, UInt uiSliceId = MSYS_UINT_MAX );

  // functions for SVC to AVC rewrite
  ErrVal  copyTCoeffs   ( MbData& rcMbData );
  ErrVal  copyIntraPred ( MbData& rcMbData );

protected:
  MbTransformCoeffs*  m_pcMbTCoeffs;
  MbMvData*           m_apcMbMvdData[2];
  MbMotionData*       m_apcMbMotionData[2];
  Bool                m_aabBaseIntra  [2][2];
};


class MbDataBuffer : public MbData
{
public :
  MbDataBuffer()
  {
    MbData::init( &m_cMbTransformCoeffs, m_acMbMvData, m_acMbMvData+1, m_acMbMotionData, m_acMbMotionData + 1);
  }
  virtual ~MbDataBuffer() {}

  MbTransformCoeffs m_cMbTransformCoeffs;
  MbMvData          m_acMbMvData[2];
  MbMotionData      m_acMbMotionData[2];
};


class MbDataAccess;

class H264AVCCOMMONLIB_API MbStatus
{
public:
  MbStatus();
  virtual ~MbStatus();

  Void    reset           ();
  Bool    canBeUpdated    ( const SliceHeader&  rcSliceHeader   );
  ErrVal  update          ( MbDataAccess&       rcMbDataAccess  );

  SliceHeader*        getSliceHeader          ()        { return    m_pcSliceHeader; }
  const SliceHeader*  getSliceHeader          ()  const { return    m_pcSliceHeader; }
  const SliceHeader*  getLastCodedSliceHeader ()  const { return    m_pcLastCodedSliceHeader; }
  UInt                getSliceIdc             ()  const { return    m_uiSliceIdc; }
  UInt                getQ0SliceIdc           ()  const { return    m_uiQ0SliceIdc; }
  UInt                getFirstMbInSlice       ()  const { return    m_uiSliceIdc >> 7; }
  UInt                getDQId                 ()  const { return    m_uiSliceIdc        & 0x7F; }
  UInt                getDependencyId         ()  const { return  ( m_uiSliceIdc >> 4 ) & 0x7; }
  UInt                getQualityId            ()  const { return    m_uiSliceIdc        & 0xF; }
  UInt                getLastCodedDQId        ()  const { return    m_uiLastCodedDQId; }
  UInt                getLastCodedSliceIdc    ()  const { return    m_uiLastCodedSliceIdc; }
  UInt                getMbCbpDQId0           ()  const { return    m_uiMbCbpDQId0; }
  Bool                isCoded                 ()  const { return    m_bIsCoded; }

private:
  UInt          m_uiSliceIdc;
  UInt          m_uiQ0SliceIdc;
  UInt          m_uiLastCodedSliceIdc;
  UInt          m_uiLastCodedDQId;
  UInt          m_uiMbCbpDQId0;
  Bool          m_bIsCoded;
  SliceHeader*  m_pcSliceHeader;
  SliceHeader*  m_pcLastCodedSliceHeader;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)
