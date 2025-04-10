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


#ifndef __EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#define MAX_PACKET_SIZE 1000000
// JVT-S080 LMI
// If defined to 1, the initial scalability_info SEI should be updated after an extraction;
// Otherwise, it's unchanged and followed by a layers_not_present scalable SEI. 
#define UPDATE_SCALABLE_SEI 1
#include "H264AVCCommonLib/Sei.h"

#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "ExtractorParameter.h"

#define MAX_ROIS			5


enum NalUnitType
{
  NAL_UNIT_EXTERNAL                 = 0,
  NAL_UNIT_CODED_SLICE              = 1,
  NAL_UNIT_CODED_SLICE_DATAPART_A   = 2,
  NAL_UNIT_CODED_SLICE_DATAPART_B   = 3,
  NAL_UNIT_CODED_SLICE_DATAPART_C   = 4,
  NAL_UNIT_CODED_SLICE_IDR          = 5,
  NAL_UNIT_SEI                      = 6,
  NAL_UNIT_SPS                      = 7,
  NAL_UNIT_PPS                      = 8,
  NAL_UNIT_ACCESS_UNIT_DELIMITER    = 9,
  NAL_UNIT_END_OF_SEQUENCE          = 10,
  NAL_UNIT_END_OF_STREAM            = 11,
  NAL_UNIT_FILLER_DATA              = 12,
  NAL_UNIT_SUBSET_SPS               = 15,

  NAL_UNIT_CODED_SLICE_SCALABLE     = 20,
  NAL_UNIT_CODED_SLICE_IDR_SCALABLE = 21
};


class Extractor  
{
protected:
	Extractor();
	virtual ~Extractor();

public:
  static ErrVal create              ( Extractor*&         rpcExtractor );
  ErrVal        init                ( ExtractorParameter* pcExtractorParameter );
  ErrVal        go                  ();
  ErrVal        destroy             ();


  ErrVal	    xWriteViewScalSEIToBuffer(h264::SEI::ViewScalabilityInfoSei *pcViewScalSei, BinData *pcBinData);

protected:
  ErrVal		xExtractOperationPoints ();
  ErrVal		xDisplayOperationPoints ();


  // ROI ICU/ETRI
  ErrVal		xChangeViewScalSEIMessage( BinData *pcBinData, h264::SEI::SEIMessage* pcSEIMessage, UInt uiOpId, UInt*& uiViewId, UInt* uiNewNumViews );


protected:
  ReadBitstreamIf*              m_pcReadBitstream;
  WriteBitstreamIf*             m_pcWriteBitstream;
  ExtractorParameter*           m_pcExtractorParameter;
  h264::H264AVCPacketAnalyzer*  m_pcH264AVCPacketAnalyzer;

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;

};
class ExtractStop{};
#endif //__EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79

