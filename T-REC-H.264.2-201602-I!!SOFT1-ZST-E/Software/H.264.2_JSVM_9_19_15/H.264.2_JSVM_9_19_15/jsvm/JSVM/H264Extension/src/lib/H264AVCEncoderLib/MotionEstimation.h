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

#if !defined(AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_)
#define AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "MotionEstimationCost.h"
#include "Distortion.h"
#include "CodingParameter.h"
#include "H264AVCCommonLib/MotionCompensation.h"



H264AVC_NAMESPACE_BEGIN


class MotionVectorCalculation;
class RateDistortionIf;
class CodingParameter;
class QuarterPelFilter;


class MotionEstimation
: public MotionCompensation
, public MotionEstimationCost
{
public:
  class MEBiSearchParameters
  {
  public:
    YuvMbBuffer*      pcAltRefPelData;    // the prediction signal for the opposite list (not weighted)
    UInt              uiL1Search;         // 1 if current search is L1 search, else false
    const Frame*      pcAltRefFrame;      // the reference frame of the opposite list
    const PredWeight* apcWeight[2];       // { list 0 prediction weight, list 1 prediction weight }
  };

protected:
  typedef struct
  {
    Void init( Short sLimit, Mv& rcMvPel, Mv cMin, Mv cMax )
    {
      cMin >>= 2;
      cMax >>= 2;
      Short sPosV   = cMax.getVer() - rcMvPel.getVer();
      Short sNegV   = rcMvPel.getVer() - cMin.getVer();
      iNegVerLimit  = gMin( sLimit, sNegV) - rcMvPel.getVer();
      iPosVerLimit  = gMin( sLimit, sPosV) + rcMvPel.getVer();
      Short sPosH   = cMax.getHor() - rcMvPel.getHor();
      Short sNegH   = rcMvPel.getHor() - cMin.getHor();
      iNegHorLimit  = gMin( sLimit, sNegH) - rcMvPel.getHor();
      iPosHorLimit  = gMin( sLimit, sPosH) + rcMvPel.getHor();
    }
    Int iNegVerLimit;
    Int iPosVerLimit;
    Int iNegHorLimit;
    Int iPosHorLimit;
  }
  SearchRect;

  typedef struct
  {
    XPel* pucYRef;
    XPel* pucURef;
    XPel* pucVRef;
    Int   iYStride;
    Int   iCStride;
    Int   iBestX;
    Int   iBestY;
    UInt  uiBestRound;
    UInt  uiBestDistance;
    UInt  uiBestSad;
    UChar ucPointNr;
  }
  IntTZSearchStrukt;

protected:
	MotionEstimation();
	virtual ~MotionEstimation();

public:
  ErrVal destroy();
  virtual ErrVal uninit();

  SampleWeighting* getSW() { return m_pcSampleWeighting; }

  ErrVal initMb( UInt uiMbPosY, UInt uiMbPosX, MbDataAccess& rcMbDataAccess );

  virtual ErrVal init(  XDistortion*      pcXDistortion,
                        CodingParameter*  pcCodingParameter,
                        RateDistortionIf* pcRateDistortionIf,
                        QuarterPelFilter* pcQuarterPelFilter,
                        Transform*        pcTransform,
                        SampleWeighting*  pcSampleWeighting );

  UInt    getRateCost   ( UInt uiBits, Bool bSad  ) { xSetMEPars( 0, bSad ); return xGetCost( uiBits ); }
  Bool    getELSearch   () const                    { return m_cParams.getELSearch(); }
  Void    setEL         ( Bool b)                   { m_bELWithBLMv = b; }

  ErrVal  estimateBlockWithStart( const MbDataAccess&         rcMbDataAccess,
                                  const Frame&                rcRefFrame,
                                  Mv&                         rcMv, // start and result
                                  const Mv&                   rcMvPred,
                                  UInt&                       ruiBits,
                                  UInt&                       ruiCost,
                                  const UInt                  uiBlk,
                                  const UInt                  uiMode,
                                  const UInt                  uiSearchRange,
                                  const PredWeight*           pcPW,
                                  const MEBiSearchParameters* pcBSP = 0 );

  virtual ErrVal compensateBlock( YuvMbBuffer *pcRecPelData, UInt uiBlk, UInt uiMode, YuvMbBuffer *pcRefPelData2 = NULL ) = 0;

protected:
  __inline Void xTZCheckPoint         ( IntTZSearchStrukt& rcStrukt, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance );
  __inline Void xTZ2PointSearch       ( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect );
  __inline Void xTZ8PointSquareSearch ( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect, const Int iStartX, const Int iStartY, const Int iDist );
  __inline Void xTZ8PointDiamondSearch( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect, const Int iStartX, const Int iStartY, const Int iDist );

  Void          xPelBlockSearch ( YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD,                              UInt uiSearchRange = 0 );
  Void          xPelSpiralSearch( YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD,                              UInt uiSearchRange = 0 );
  Void          xPelLogSearch   ( YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, Bool bFme,  UInt uiStep = 4, UInt uiSearchRange = 0 );
  Void          xTZSearch       ( YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, Bool bEL,                    Int  iSearchRange  = 0 );
  virtual Void  xSubPelSearch   ( YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiBlk, UInt uiMode ) = 0;

protected:
  QuarterPelFilter*         m_pcQuarterPelFilter;
  MotionVectorSearchParams  m_cParams;
  Int                       m_iMaxLogStep;
  Mv*                       m_pcMvSpiralSearch;
  UInt                      m_uiSpiralSearchEntries;
  std::vector<Mv>           m_acMvCandList;
  XDistortion*              m_pcXDistortion;
  XDistSearchStruct         m_cXDSS;
  Bool                      m_bELWithBLMv;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_)
