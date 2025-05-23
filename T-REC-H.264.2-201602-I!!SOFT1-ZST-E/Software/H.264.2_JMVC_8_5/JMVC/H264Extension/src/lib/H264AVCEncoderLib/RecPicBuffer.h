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


#if !defined  _REC_PIC_BUFFER_INCLUDED_
#define       _REC_PIC_BUFFER_INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/IntFrame.h"


H264AVC_NAMESPACE_BEGIN


// Possible prediciton directions for multiview referencs.  FORWARD
// references get put into list 0 for P/B frames while BACKWARD
// references get put into list 1 only for B frames.  See Multiview.h/.cpp 
// for more details.  Each RecPicBufUnit must indicate whether it
// is not a multiview reference o indicate the prediction direction.

enum MultiviewReferenceDirection { NOT_MULTIVIEW=0, FORWARD, BACKWARD };

class PicEncoder;//JVT-W056


class RecPicBufUnit
{
protected:
  RecPicBufUnit             ();
  virtual ~RecPicBufUnit    ();
  
public:
  static ErrVal create          ( RecPicBufUnit*&             rpcRecPicBufUnit,
                                  YuvBufferCtrl&              rcYuvBufferCtrlFullPel,
                                  YuvBufferCtrl&              rcYuvBufferCtrlHalfPel,
                                  const SequenceParameterSet& rcSPS );  
  ErrVal        destroy         ();

  ErrVal        init            ( SliceHeader*                pcSliceHeader,
                                  PicBuffer*                  pcPicBuffer );
  ErrVal        initNonEx       ( Int                         iPoc,
                                  UInt                        uiFrameNum );
  ErrVal        uninit          ();

  ErrVal        markNonRef      ();
  ErrVal        markOutputted   ();

  Int           getPoc          ()  const { return m_iPoc; }
  UInt          getFrameNum     ()  const { return m_uiFrameNum; }
  Bool          isExisting      ()  const { return m_bExisting; }
  Bool          isNeededForRef  ()  const { return m_bNeededForReference; }
  Bool          isOutputted     ()  const { return m_bOutputted; }
  IntFrame*     getRecFrame     ()        { return m_pcReconstructedFrame; }
  MbDataCtrl*   getMbDataCtrl   ()        { return m_pcMbDataCtrl; }
  PicBuffer*    getPicBuffer    ()        { return m_pcPicBuffer; }

  Int           getPicNum       ( UInt uiCurrFrameNum,
                                  UInt uiMaxFrameNum )  const
  {
    if( m_uiFrameNum > uiCurrFrameNum )
    {
      return (Int)m_uiFrameNum - (Int)uiMaxFrameNum;
    }
    return (Int)m_uiFrameNum;
  }

  UInt          getViewId       ()  const { return m_uiViewId; }
  Void  setViewId( UInt uiViewId ) { m_uiViewId = uiViewId; }



  bool IsMultiviewReference() const {
    return NOT_MULTIVIEW != m_multiviewRefDirection;
  }

  MultiviewReferenceDirection GetMultiviewReferenceDirection() const {
    return m_multiviewRefDirection;
  }

  void SetMultiviewReferenceDirection(const MultiviewReferenceDirection dir) {
    m_multiviewRefDirection = dir;
  }


private:
  Int           m_iPoc;
  UInt          m_uiFrameNum;
  Bool          m_bExisting;
  Bool          m_bNeededForReference;
  Bool          m_bOutputted;
  IntFrame*     m_pcReconstructedFrame;
  MbDataCtrl*   m_pcMbDataCtrl;
  PicBuffer*    m_pcPicBuffer;
  MultiviewReferenceDirection     m_multiviewRefDirection;
  UInt          m_uiViewId;

};


typedef MyList<RecPicBufUnit*>  RecPicBufUnitList;



class RecPicBuffer
{
protected:
  RecPicBuffer          ();
  virtual ~RecPicBuffer ();

public:
  static ErrVal   create                ( RecPicBuffer*&              rpcRecPicBuffer );
  ErrVal          destroy               ();
  ErrVal          init                  ( YuvBufferCtrl*              pcYuvBufferCtrlFullPel,
                                          YuvBufferCtrl*              pcYuvBufferCtrlHalfPel );
  ErrVal          initSPS               ( const SequenceParameterSet& rcSPS );
  ErrVal          uninit                ();
  ErrVal          clear                 ( PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );

  ErrVal          initCurrRecPicBufUnit ( RecPicBufUnit*&             rpcCurrRecPicBufUnit,
                                          PicBuffer*                  pcPicBuffer,
                                          SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList 
					  ,MultiviewReferenceDirection refDirection = NOT_MULTIVIEW							  
										  );
  ErrVal          store2                ( Bool isRef );       // just in order to trigger sliding window. -Dong
  ErrVal          store                 ( RecPicBufUnit*              pcRecPicBufUnit,
                                          SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList 
					  ,MultiviewReferenceDirection = NOT_MULTIVIEW			  
										  );

  ErrVal          getRefLists           ( RefFrameList&               rcList0,
                                          RefFrameList&               rcList1,
                                          SliceHeader&                rcSliceHeader,
										  QuarterPelFilter* pcQuarterPelFilter);

  ErrVal
				xFieldList(    SliceHeader&   rcSliceHeader,
                           RefFrameList&  rcList,
                           RefFrameList&  rcListTemp );//lufeng
ErrVal
                ProcessRef(SliceHeader&   rcSliceHeader, RefFrameList&  rcList ,RefFrameList&  rcListTemp,
							QuarterPelFilter* pcQuarterPelFilter);//lufeng

  RecPicBufUnit*  getLastUnit           ();
  RecPicBufUnit*  getCurrUnit           ();
  RecPicBufUnit*  getRecPicBufUnit      ( Int                         iPoc );


  ErrVal AddMultiviewRef (RecPicBufUnitList& recPicBufUnitList,
			                    RefFrameList& rcList, const int maxListSize,
			                    const MultiviewReferenceDirection refDirection, SliceHeader&   rcSliceHeader
								,QuarterPelFilter* pcQuarterPelFilter); //JVT-W056  Samsung

  void RemoveMultiviewRef(RecPicBufUnit* pcRecPicBufUnitToRemove);

  YuvBufferCtrl*      m_pcYuvBufferCtrlFullPel;
  YuvBufferCtrl*      m_pcYuvBufferCtrlHalfPel;

  //--- V-frames, See comment in Multiview.h for description of how 
  //              V-Frames work.
  void    SetCodeAsVFrameFlag(const bool flag) { m_codeAsVFrame = flag; }
  bool    CodeAsVFrameP() const {return m_codeAsVFrame; }
  void		SetPictureEncoder(PicEncoder * picencoder) { m_pcPicEncoder = picencoder;}  //JVT-W056  Samsung

private:
  ErrVal          xCreateData           ( UInt                        uiMaxFramesInDPB,
                                          const SequenceParameterSet& rcSPS );
  ErrVal          xDeleteData           ();


  //===== memory management =====
  ErrVal          xCheckMissingPics     ( SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          xStorePicture         ( RecPicBufUnit*              pcRecPicBufUnit,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList,
                                          SliceHeader*                pcSliceHeader,
                                          Bool                        bTreatAsIdr );
  ErrVal          xOutput               ( PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          xClearOutputAll       ( PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          xUpdateMemory         ( SliceHeader*                pcSliceHeader );
  ErrVal          xClearBuffer          ();
  ErrVal          xSlidingWindow        (int bFirstField);
  ErrVal          xMMCO                 ( SliceHeader*                pcSliceHeader );
  ErrVal          xMarkShortTermUnused  ( RecPicBufUnit*              pcCurrentRecPicBufUnit,
                                          UInt                        uiDiffOfPicNums );

  //===== reference picture lists =====

  ErrVal          xInitRefListPSlice    ( RefFrameList&               rcList,
											PicType        ePicType,
											Bool bRef);
  ErrVal          xInitRefListsBSlice   ( RefFrameList&               rcList0,
                                          RefFrameList&               rcList1,
											PicType        ePicType,
											Bool bRef);

  ErrVal          xRefListRemapping     ( RefFrameList&               rcList,
                                          ListIdx                     eListIdx,
                                          SliceHeader*                pcSliceHeader );
  ErrVal          xAdaptListSize        ( RefFrameList&               rcList,
                                          ListIdx                     eListIdx,
                                          SliceHeader&                rcSliceHeader );

  //--- debug ---
  ErrVal          xDumpRecPicBuffer     ();
  ErrVal          xDumpRefList          ( RefFrameList&               rcList,
                                          ListIdx                     eListIdx );

private:
  Bool                m_bInitDone;
  UInt                m_uiNumRefFrames;
  UInt                m_uiMaxFrameNum;
  UInt                m_uiLastRefFrameNum;
  RecPicBufUnitList   m_cUsedRecPicBufUnitList;
  RecPicBufUnitList   m_cFreeRecPicBufUnitList;
  RecPicBufUnit*      m_pcCurrRecPicBufUnit;
  PicEncoder*					m_pcPicEncoder;
  Bool                m_codeAsVFrame;

};


H264AVC_NAMESPACE_END


#endif // _REC_PIC_BUFFER_INCLUDED_

