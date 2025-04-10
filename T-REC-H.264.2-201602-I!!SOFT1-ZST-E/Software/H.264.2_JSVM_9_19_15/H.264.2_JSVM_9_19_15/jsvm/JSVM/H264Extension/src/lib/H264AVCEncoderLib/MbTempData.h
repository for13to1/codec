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

#if !defined(AFX_MBTEMPDATA_H__2D7469B9_B5A8_44FE_AA77_6BA0269C8F37__INCLUDED_)
#define AFX_MBTEMPDATA_H__2D7469B9_B5A8_44FE_AA77_6BA0269C8F37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvMbBuffer.h"


H264AVC_NAMESPACE_BEGIN



class IntMbTempData :
public CostData
, public MbData
, public MbTransformCoeffs
, public YuvMbBuffer
{
public:
	IntMbTempData               ();
	virtual ~IntMbTempData      ();

  ErrVal  init                ( MbDataAccess& rcMbDataAccess );

  ErrVal  uninit              ();

  Void    clear               ();
  Void    clearCost           ();

  UInt&   cbp                 ()                { return m_uiMbCbp; }

  Void    copyTo              ( MbDataAccess&   rcMbDataAccess );
  Void    loadChromaData      ( IntMbTempData&  rcMbTempData );

  Void    copyResidualDataTo  ( MbDataAccess&   rcMbDataAccess );

  MbMotionData&       getMbMotionData         ( ListIdx eLstIdx ) { return m_acMbMotionData [ eLstIdx ]; }
  MbMvData&           getMbMvdData            ( ListIdx eLstIdx ) { return m_acMbMvdData    [ eLstIdx ]; }
  YuvMbBuffer&        getTempYuvMbBuffer      ()                  { return m_cTempYuvMbBuffer; }
  YuvMbBuffer&        getTempBLSkipResBuffer  ()                  { return m_cTempBLSkipResBuffer; }
  MbDataAccess&       getMbDataAccess         ()                  { AOF_DBG(m_pcMbDataAccess); return *m_pcMbDataAccess; }
  const SliceHeader&  getSH                   ()            const { AOF_DBG(m_pcMbDataAccess); return m_pcMbDataAccess->getSH(); }
  const CostData&     getCostData             ()            const { return *this; }
  operator MbDataAccess&                      ()                  { AOF_DBG(m_pcMbDataAccess); return *m_pcMbDataAccess; }
  operator YuvMbBuffer*                       ()                  { return this; }

protected:
  MbDataAccess*       m_pcMbDataAccess;
  MbMvData            m_acMbMvdData[2];
  MbMotionData        m_acMbMotionData[2];
  YuvMbBuffer         m_cTempYuvMbBuffer;
  YuvMbBuffer         m_cTempBLSkipResBuffer;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBTEMPDATA_H__2D7469B9_B5A8_44FE_AA77_6BA0269C8F37__INCLUDED_)
