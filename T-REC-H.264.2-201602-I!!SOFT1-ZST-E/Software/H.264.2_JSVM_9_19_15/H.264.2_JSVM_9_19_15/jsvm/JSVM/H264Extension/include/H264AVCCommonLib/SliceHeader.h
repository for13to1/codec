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

#if !defined(AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_)
#define AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/SliceHeaderBase.h"
#include "H264AVCCommonLib/CFMO.h"



H264AVC_NAMESPACE_BEGIN

#if defined( WIN32 )
# pragma warning( disable: 4275 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SliceHeader : public SliceHeaderSyntax
{
public:
  SliceHeader();
  SliceHeader( const NalUnitHeader&         rcNalUnitHeader );
  SliceHeader( const PrefixHeader&          rcPrefixHeader );
  SliceHeader( const SequenceParameterSet&  rcSPS,
               const PictureParameterSet&   rcPPS );
  SliceHeader( const SliceHeader&           rcSliceHeader );
  virtual ~SliceHeader();

  ErrVal  init      ( const SequenceParameterSet& rcSPS,
                      const PictureParameterSet&  rcPPS );
  Void    copy      ( const SliceHeader&          rcSliceHeader );
  ErrVal  copyPrefix( const PrefixHeader&         rcPrefixHeader );

  Bool    isFirstSliceOfNextAccessUnit( const SliceHeader* pcLastSliceHeader ) const;

  //>>> remove
  Void          setLayerCGSSNR            ( UInt ui ) { m_uiLayerCGSSNR = ui;}
  Void          setQualityLevelCGSSNR     ( UInt ui ) { m_uiQualityLevelCGSSNR = ui;}
  Void          setBaseLayerCGSSNR        ( UInt ui ) { m_uiBaseLayerCGSSNR = ui;}
  Void          setBaseQualityLevelCGSSNR ( UInt ui ) { m_uiBaseQualityLevelCGSSNR = ui;}
  Void          setBaseLayerId            ( UInt ui ) { m_uiBaseLayerId = ui; }
  UInt          getLayerCGSSNR            ()  const   { return m_uiLayerCGSSNR;}
  UInt          getQualityLevelCGSSNR     ()  const   { return m_uiQualityLevelCGSSNR;}
  UInt          getBaseLayerCGSSNR        ()  const   { return m_uiBaseLayerCGSSNR;}
  UInt          getBaseQualityLevelCGSSNR ()  const   { return m_uiBaseQualityLevelCGSSNR;}
  UInt          getBaseLayerId            ()  const   { return m_uiBaseLayerId; }

  ErrVal  compare           ( const SliceHeader*          pcSH,
                              Bool&                       rbNewPic,
                              Bool&                       rbNewFrame ) const;
  ErrVal  compareRedPic     ( const SliceHeader*          pcSH,
                              Bool&                       rbNewFrame ) const;
  ErrVal  sliceHeaderBackup ( SliceHeader*                pcSH       );
  ErrVal  FMOInit           ();
  ErrVal  FMOUninit         ();
  Int     getNumMbInSlice   ();
  //<<< remove

  Void              getMbPositionFromAddress    ( UInt& ruiMbY, UInt& ruiMbX,                   UInt uiMbAddress ) const;
  Void              getMbPosAndIndexFromAddress ( UInt& ruiMbY, UInt& ruiMbX, UInt& ruiMbIndex, UInt uiMbAddress ) const;
  UInt              getMbIndexFromAddress       (                                               UInt uiMbAddress ) const;
  UInt              getMbAddressFromPosition    ( UInt uiMbY, UInt uiMbX ) const;
  UInt              getMapUnitFromPosition      ( UInt uiMbY, UInt uiMbX ) const;
  Bool              isFieldPair                 ( UInt uiFrameNum, PicType ePicType, Bool bIsRefPic ) const;
  Int               getDistScaleFactorWP        ( const Frame* pcFrameL0, const Frame*  pcFrameL1 ) const;
  const PredWeight& getPredWeight               ( ListIdx eListIdx, UInt uiRefIdx, Bool bFieldFlag ) const;
  PredWeight&       getPredWeight               ( ListIdx eListIdx, UInt uiRefIdx, Bool bFieldFlag );

  UChar           getCbQp                 ( UChar   ucLumaQp )  const { return g_aucChromaScale[ gClipMinMax( ucLumaQp + getPPS().getChromaQpIndexOffset    (), 0, 51 ) ]; }
  UChar           getCrQp                 ( UChar   ucLumaQp )  const { return g_aucChromaScale[ gClipMinMax( ucLumaQp + getPPS().get2ndChromaQpIndexOffset (), 0, 51 ) ]; }
  UChar           getChromaQp             ( UChar   ucLumaQp,
                                            UInt    uiComp   )  const { AOT( uiComp > 1 ); return ( uiComp ? getCrQp( ucLumaQp ) : getCbQp( ucLumaQp ) ); }
  const Bool      isScalingMatrixPresent  ( UInt    uiMatrix )  const { return SliceHeaderSyntax::getScalingMatrix().get( uiMatrix ) != 0; }
  const UChar*    getScalingMatrix        ( UInt    uiMatrix )  const { return SliceHeaderSyntax::getScalingMatrix().get( uiMatrix ); }

  ERROR_CONCEAL getErrorConcealMode       ()                    const { return m_eErrorConcealMode; }
  Bool          isTrueSlice               ()                    const { return m_bTrueSlice; }
  UInt          getMbInPic                ()                    const { AOF( parameterSetsInitialized() ); return getFieldPicFlag() ? getSPS().getMbInFrame() / 2 : getSPS().getMbInFrame(); }
  UInt          getNumMbsInSlice          ()                    const { return m_uiNumMbsInSlice; }
  UInt          getLastMbInSlice          ()                    const { return m_uiLastMbInSlice; }
  Int           getTopFieldPoc            ()                    const { return m_iTopFieldPoc;  }
  Int           getBotFieldPoc            ()                    const { return m_iBotFieldPoc;  }
  RefFrameList* getRefFrameList           ( PicType ePicType,
                                            ListIdx eLstIdx )   const { return m_aapcRefFrameList[ ePicType - 1 ][ eLstIdx ]; }
  UInt          getNumRefIdxUpdate        ( UInt    uiTempLevel,
                                            ListIdx eListIdx )  const { return m_aauiNumRefIdxActiveUpdate[uiTempLevel][eListIdx]; }
  Bool          getSCoeffResidualPredFlag ()                    const { return m_bSCoeffResidualPred; }
  Bool          isReconstructionLayer     ()                    const { return m_bReconstructionLayer; }
  Int           getLongTermFrameIdx       ()                    const { return m_iLongTermFrameIdx; }
  const FMO*    getFMO                    ()                    const { return &m_cFMO;}
  FMO*          getFMO                    ()                          { return &m_cFMO;}
  PicType       getPicType                ()                    const;
  Int           getPoc                    ()                    const;
  Int           getPoc                    ( PicType ePicType )  const;
  Int           getDistScaleFactor        ( PicType eMbPicType,
                                            SChar   sL0RefIdx,
                                            SChar   sL1RefIdx ) const;

  Void          setErrorConcealMode       ( ERROR_CONCEAL       eErrorConcealMode       )   { m_eErrorConcealMode       = eErrorConcealMode; }
  Void          setTrueSlice              ( Bool                bTrueSlice              )   { m_bTrueSlice              = bTrueSlice; }
  Void          setNumMbsInSlice          ( UInt                uiNumMbsInSlice         )   { m_uiNumMbsInSlice         = uiNumMbsInSlice; }
  Void          setLastMbInSlice          ( UInt                uiLastMbInSlice         )   { m_uiLastMbInSlice         = uiLastMbInSlice; }
  Void          setTopFieldPoc            ( Int                 iTopFieldPoc            )   { m_iTopFieldPoc            = iTopFieldPoc;  }
  Void          setBotFieldPoc            ( Int                 iBotFieldPoc            )   { m_iBotFieldPoc            = iBotFieldPoc;  }
  Void          setRefFrameList           ( RefFrameList*       pcRefFrameList,
                                            PicType             ePicType,
                                            ListIdx             eListIdx                )   { m_aapcRefFrameList[ ePicType - 1 ][ eListIdx ]  = pcRefFrameList; }
  Void          setNumRefIdxUpdate        ( UInt                uiTempLevel,
                                            ListIdx             eListIdx,
                                            UInt                uiNumRefIdxActive       )   { m_aauiNumRefIdxActiveUpdate[uiTempLevel][eListIdx] = uiNumRefIdxActive;  }
  Void          setSCoeffResidualPredFlag ( ResizeParameters*   pcResizeParameters      );
  Void          setReconstructionLayer    ( Bool                bReconstructionLayer    )   { m_bReconstructionLayer = bReconstructionLayer; }
  Void          setLongTermFrameIdx       ( Int                 iLongTermFrameIdx       )   { m_iLongTermFrameIdx = iLongTermFrameIdx; }
  Void          setPicType                ( PicType             ePicType                );

  Void          setAdaptiveILPred( Bool b )   { m_bAdaptiveILPred = b; }
  Bool          getAdaptiveILPred() const     { return m_bAdaptiveILPred; }

  const SliceHeader*  getBaseSliceHeader()                          const { return m_pcBaseSliceHeader; }
  Void                setBaseSliceHeader( const SliceHeader* pcSH )       { m_pcBaseSliceHeader = pcSH; } 
  const SliceHeader*  getLastCodedSliceHeader()                     const { return m_pcLastCodedSliceHeader; }
  Void                setLastCodedSliceHeader( const SliceHeader* pcSH )  { m_pcLastCodedSliceHeader = pcSH; } 

private:
  ERROR_CONCEAL	      m_eErrorConcealMode;
  Bool	              m_bTrueSlice;
  UInt                m_uiNumMbsInSlice;
  UInt                m_uiLastMbInSlice;
  Int                 m_iTopFieldPoc;
  Int                 m_iBotFieldPoc;
  Bool                m_bSCoeffResidualPred;      // remove
  FMO                 m_cFMO;
  RefFrameList*       m_aapcRefFrameList[3][2];
  UInt                m_aauiNumRefIdxActiveUpdate[MAX_TEMP_LEVELS][2]; // for MCTF preprocessor
  Bool                m_bReconstructionLayer;
  Int                 m_iLongTermFrameIdx;
  //>>> remove
  UInt                m_uiLayerCGSSNR;
  UInt                m_uiQualityLevelCGSSNR;
  UInt                m_uiBaseLayerCGSSNR;
  UInt                m_uiBaseQualityLevelCGSSNR;
  UInt                m_uiBaseLayerId;
  //<<< remove
  Bool                m_bAdaptiveILPred;
  const SliceHeader*  m_pcBaseSliceHeader;
  const SliceHeader*  m_pcLastCodedSliceHeader;
};



#if defined( WIN32 )
# pragma warning( default: 4251 )
# pragma warning( default: 4275 )
#endif



H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_)
