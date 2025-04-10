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


#if !defined(AFX_NALUNITPARSER_H__D5B74729_6F04_42E9_91AE_2E28937F9F3A__INCLUDED_)
#define AFX_NALUNITPARSER_H__D5B74729_6F04_42E9_91AE_2E28937F9F3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



H264AVC_NAMESPACE_BEGIN


class BitReadBuffer;


class NalUnitParser
{
public:
	NalUnitParser                 ();
	virtual ~NalUnitParser        ();

  static ErrVal create          ( NalUnitParser*&   rpcNalUnitParser  );
  
  ErrVal        init            ( BitReadBuffer*    pcBitReadBuffer   );
  ErrVal        destroy         ();

  ErrVal        initNalUnit     ( BinDataAccessor*  pcBinDataAccessor, Bool* KeyPicFlag, 
    UInt& uiNumBytesRemoved, //FIX_FRAG_CAVLC
	  Bool bPreParseHeader = true,
    Bool bConcatenated = false, //FRAG_FIX
		Bool bCheckGap = false); //TMM_EC
  ErrVal        closeNalUnit    ();

  NalUnitType   getNalUnitType  ()      { return m_eNalUnitType;    }

  Bool		getSvcMvcFlag() { return m_svc_mvc_flag;	}
  
  UInt		getViewId() { return m_view_id;}
  UInt		getAvcViewId() { return m_AvcViewId;}
  Bool 		getAnchorPicFlag() { return m_anchor_pic_flag;}
  Bool    getNonIDRFlag      () { return m_bNonIDRFlag; } //JVT-W035 
  UInt    getPriorityID   () { return m_uiSimplePriorityId; } //JVT-W035
	Bool		getInterViewFlag() {return m_inter_view_flag;} //JVT-W056  Samsung

  Void		setSvcMvcFlag(Bool b) { m_svc_mvc_flag = b;	}
  
  Void		setViewId(UInt ui) { m_view_id = ui;}
  Void		setAvcViewId(UInt ui) { m_AvcViewId = ui;}
  Void 		setAnchorPicFlag(Bool b) { m_anchor_pic_flag = b;}
  Void    setNonIDRFlag      (Bool b) { m_bNonIDRFlag =b; }  //JVT-W035 
  Void    setPriorityID   (UInt ui){ m_uiSimplePriorityId=ui;} //JVT-W035
	Void		setInterViewFlag(Bool nal) { m_inter_view_flag = nal;} //JVT-W056  Samsung


//	TMM_EC {{
	Bool          isTrueNalUnit   ()      { return *(int*)m_pucBuffer != 0xdeadface;    }
  ErrVal	      setNalUnitType  (NalUnitType eNalRefUnitType)		{ m_eNalUnitType=eNalRefUnitType; return Err::m_nOK;}
//	TMM_EC }}
  NalRefIdc     getNalRefIdc    ()      { return m_eNalRefIdc;      }
  UInt          getLayerId      ()      { return m_uiLayerId;       }
  UInt          getTemporalLevel()      { return m_uiTemporalLevel; }
  UInt          getQualityLevel ()      { return m_uiQualityLevel;  }
  
  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  Bool			getDiscardableFlag ()	{ return m_bDiscardableFlag;}
  //}}Variable Lengh NAL unit header data with priority and dead substream flag
  /*Void  setSimplePriorityMap ( UInt uiSimplePri, UInt uiTemporalLevel, UInt uiLayer, UInt uiQualityLevel )
                                                                          { m_uiTemporalLevelList[uiSimplePri] = uiTemporalLevel;
                                                                            m_uiDependencyIdList [uiSimplePri] = uiLayer;
                                                                            m_uiQualityLevelList [uiSimplePri] = uiQualityLevel;
                                                                          }
JVT-S036 lsj */
  //JVT-P031
  UInt getBytesLeft();
  UInt getBitsLeft();
  ErrVal initSODBNalUnit( BinDataAccessor* pcBinDataAccessor );
  UInt getNalHeaderSize( BinDataAccessor* pcBinDataAccessor );
  Bool getFragmentedFlag() { return m_bFragmentedFlag;}
  //Bool  getExtensionFlag() { return m_bExtensionFlag;} //BUG_FIX_FT_01_2006_2  //JVT-S036 lsj
    Bool  getReservedZeroBit() { return m_bReservedZeroBit;} //JVT-S036 lsj
  Void setCheckAllNALUs(Bool b) { m_bCheckAllNALUs = b;}
  Void setDecodedLayer( UInt uiLayer) { m_uiDecodedLayer = uiLayer;}
  //~JVT-P031
	ErrVal	readAUDelimiter       ();
  ErrVal  readEndOfSeqence      ();
  ErrVal  readEndOfStream       ();

protected:
  Void    xTrace                ( Bool  bDDIPresent     );
  ErrVal  xConvertPayloadToRBSP ( UInt& ruiPacketLength );
  ErrVal  xConvertRBSPToSODB    ( UInt  iPacketLength,
                                  UInt& ruiBitsInPacket );

protected:
  BitReadBuffer *m_pcBitReadBuffer;
//TMM_EC {{
public:
  UChar         *m_pucBuffer;
protected:
//TMM_EC }}
  NalUnitType   m_eNalUnitType;
  NalRefIdc     m_eNalRefIdc;
  UInt          m_uiLayerId;
  UInt          m_uiTemporalLevel;
  UInt          m_uiQualityLevel;

  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  UInt			m_uiSimplePriorityId;
  Bool			m_bDiscardableFlag;

  Bool			m_bReservedZeroBit;

    Bool		m_svc_mvc_flag;     // u(1)
	  // priority                 // u(6)
    Bool 		m_anchor_pic_flag;  // u(1)
    UInt		m_view_id;          // u(10) 
    UInt		m_AvcViewId;          // u(10) 
    Bool        m_bNonIDRFlag;         // u(1)  
		Bool    m_inter_view_flag; // u(1) JVT-W056 Samsung
    UInt		m_reserved_zero_bits; // u(6)
  UInt		m_reserved_one_bit;   // u(1) // bug fix: prefix NAL (NTT)
  //JVT-P031
  Bool          m_bFragmentedFlag;
  Bool          m_bCheckAllNALUs;
  UInt          m_uiDecodedLayer;
  //~JVT-P031
  UInt          m_uiBitsInPacketSaved; //FRAG_FIX
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_NALUNITPARSER_H__D5B74729_6F04_42E9_91AE_2E28937F9F3A__INCLUDED_)
