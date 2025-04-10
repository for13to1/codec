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


#if !defined(AFX_SLICEREADER_H__5B23143A_D267_40C2_908E_164029C1298E__INCLUDED_)
#define AFX_SLICEREADER_H__5B23143A_D267_40C2_908E_164029C1298E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/ControlMngIf.h"

H264AVC_NAMESPACE_BEGIN

class MbParser;
class ParameterSetMng;

class SliceReader
{
protected:
	SliceReader();
	virtual ~SliceReader();

public:
  static ErrVal create( SliceReader*& rpcSliceReader );
  ErrVal destroy();
  ErrVal init(  HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                ParameterSetMng* pcParameterSetMng,
                MbParser* pcMbParser,
                ControlMngIf* pcControlMng );
  ErrVal uninit();

  // JVT-S054 (2) (REPLACE)
  //ErrVal process( const SliceHeader& rcSH, UInt& ruiMbRead );

  ErrVal process( SliceHeader& rcSH, UInt& ruiMbRead );

  
  ErrVal readSliceHeader  ( NalUnitType   eNalUnitType,

                            Bool m_svc_mvc_flag,	
                            Bool bNonIDRFlag, // JVT-W035 
                            Bool bAnchorPicFlag,
                            UInt uiViewId,
							Bool uiInterViewFlag, //JVT-W056 Samsung

                            NalRefIdc     eNalRefIdc,
                            UInt          uiLayerId,
                            UInt          uiTemporalLevel,
                            UInt          uiQualityLevel,
                            SliceHeader*& rpcSH
                            //JVT-P031
                            ,UInt         uiFirstFragSHPPSId
                            ,UInt         uiFirstFragNumMbsInSlice
                            ,Bool         bFirstFragFGSCompSep
                            //~JVT-P031
							,Bool		  UnitAVCFlag	//JVT-S036 lsj
                            );

//Purvin: original function
  ErrVal readSliceHeader  ( NalUnitType   eNalUnitType,
                            NalRefIdc     eNalRefIdc,
                            UInt          uiLayerId,
                            UInt          uiTemporalLevel,
                            UInt          uiQualityLevel,
                            SliceHeader*& rpcSH
                            //JVT-P031
                            ,UInt         uiFirstFragSHPPSId
                            ,UInt         uiFirstFragNumMbsInSlice
                            ,Bool         bFirstFragFGSCompSep
                            //~JVT-P031
                            ,Bool		  UnitAVCFlag	//JVT-S036 
    );
  
  ErrVal readSliceHeaderPrefix( NalUnitType   eNalUnitType,
							 Bool m_svc_mvc_flag,	
							 //JVT-W035
								NalRefIdc     eNalRefIdc,
								UInt		  uiLayerId,
								UInt		  uiQualityLevel,
								SliceHeader*  pcSliceHeader
							  );					//JVT-S036 


  //TMM_EC {{
	ErrVal	readSliceHeaderVirtual(	NalUnitType   eNalUnitType,
		                              SliceHeader	*rpcVeryFirstSliceHeader,
																	UInt	uiDecompositionStages,
																	UInt	uiMaxDecompositionStages,
																	UInt	uiGopSize,
																	UInt	uiMaxGopSize,
																	UInt	uiFrameNum,
																	UInt	uiPocLsb,
																	UInt	uiTemporalLevel,
																	SliceHeader*& rpcSH);
  //TMM_EC }}
  ErrVal  read           ( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           Int             iSpatialScalabilityType,
                           UInt           uiMbInRow,
                           UInt&          ruiMbRead );
//	TMM_EC {{
	ErrVal  readVirtual    ( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlRef,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           Int             iSpatialScalabilityType,
                           UInt           uiMbInRow,
                           UInt&          ruiMbRead,
													 ERROR_CONCEAL      m_eErrorConceal);

protected:
  HeaderSymbolReadIf* m_pcHeaderReadIf;
  ParameterSetMng *m_pcParameterSetMng;
  MbParser* m_pcMbParser;
  ControlMngIf* m_pcControlMng;
  Bool m_bInitDone;
//JVT-S036  start
  UInt KeyPictureFlag;
  Bool uiAdaptiveRefPicMarkingModeFlag;
  MmcoBuffer m_cMmmcoBufferSuffix; 
	UInt PPSId_AVC, SPSId_AVC;
	UInt POC_AVC;
//JVT-S036  end 

};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_SLICEREADER_H__5B23143A_D267_40C2_908E_164029C1298E__INCLUDED_)
