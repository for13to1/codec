/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2016, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TComPic.h
    \brief    picture class (header)
*/

#ifndef __TCOMPIC__
#define __TCOMPIC__

// Include files
#include "CommonDef.h"
#include "TComPicSym.h"
#include "TComPicYuv.h"
#include "TComBitStream.h"
#if AVC_BASE
#include <fstream>
#endif

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// picture class (symbol + YUV buffers)

class TComPic
{
public:
  typedef enum { PIC_YUV_ORG=0, PIC_YUV_REC=1, PIC_YUV_TRUE_ORG=2, NUM_PIC_YUV=3 } PIC_YUV_T;
     // TRUE_ORG is the input file without any pre-encoder colour space conversion (but with possible bit depth increment)
  TComPicYuv*   getPicYuvTrueOrg()        { return  m_apcPicYuv[PIC_YUV_TRUE_ORG]; }

private:
  UInt                  m_uiTLayer;               //  Temporal layer
  Bool                  m_bUsedByCurr;            //  Used by current picture
  Bool                  m_bIsLongTerm;            //  IS long term picture
  TComPicSym            m_picSym;                 //  Symbol
  TComPicYuv*           m_apcPicYuv[NUM_PIC_YUV];

  TComPicYuv*           m_pcPicYuvPred;           //  Prediction
  TComPicYuv*           m_pcPicYuvResi;           //  Residual
  Bool                  m_bReconstructed;
  Bool                  m_bNeededForOutput;
  UInt                  m_uiCurrSliceIdx;         // Index of current slice
  Bool                  m_bCheckLTMSB;

  Bool                  m_isTop;
  Bool                  m_isField;

  std::vector<std::vector<TComDataCU*> > m_vSliceCUDataLink;

  SEIMessages  m_SEIs; ///< Any SEI messages that have been received.  If !NULL we own the object.

#if SVC_EXTENSION
  UInt                  m_layerId;                         //  Layer ID
  Bool                  m_requireResampling[MAX_LAYERS];   // whether current layer requires resampling
  TComPicYuv*           m_pcFullPelBaseRec[MAX_LAYERS];    // upsampled base layer recontruction for difference domain inter prediction
  Bool                  m_equalPictureSizeAndOffsetFlag[MAX_LAYERS];
  Int*                  m_mvScalingFactor[2];
  Int*                  m_posScalingFactor[2];
#if CGS_3D_ASYMLUT
  Int                   m_nFrameBit;
#endif
  Bool                  m_currAuFlag;
#endif
public:
  TComPic();
  virtual ~TComPic();

#if REDUCED_ENCODER_MEMORY
#if SVC_EXTENSION
  Void          create( const TComSPS &sps, const TComPPS &pps, const Bool bCreateEncoderSourcePicYuv, const Bool bCreateForImmediateReconstruction, const UInt layerId );
#else
  Void          create( const TComSPS &sps, const TComPPS &pps, const Bool bCreateEncoderSourcePicYuv, const Bool bCreateForImmediateReconstruction );
#endif
  Void          prepareForEncoderSourcePicYuv();
  Void          prepareForReconstruction();
  Void          releaseReconstructionIntermediateData();
  Void          releaseAllReconstructionData();
  Void          releaseEncoderSourceImageData();
#else
#if SVC_EXTENSION
  Void          create( const TComSPS &sps, const TComPPS &pps, const Bool bIsVirtual /*= false*/, const UInt layerId );
#else
  Void          create( const TComSPS &sps, const TComPPS &pps, const Bool bIsVirtual /*= false*/ );
#endif
#endif

  virtual Void  destroy();

  UInt          getTLayer() const               { return m_uiTLayer;   }
  Void          setTLayer( UInt uiTLayer ) { m_uiTLayer = uiTLayer; }

  Bool          getUsedByCurr() const            { return m_bUsedByCurr; }
  Void          setUsedByCurr( Bool bUsed ) { m_bUsedByCurr = bUsed; }
  Bool          getIsLongTerm() const            { return m_bIsLongTerm; }
  Void          setIsLongTerm( Bool lt ) { m_bIsLongTerm = lt; }
  Void          setCheckLTMSBPresent     (Bool b ) {m_bCheckLTMSB=b;}
  Bool          getCheckLTMSBPresent     () { return m_bCheckLTMSB;}

  TComPicSym*   getPicSym()                        { return  &m_picSym;    }
  const TComPicSym* getPicSym() const              { return  &m_picSym;    }
  TComSlice*    getSlice(Int i)                    { return  m_picSym.getSlice(i);  }
  const TComSlice* getSlice(Int i) const           { return  m_picSym.getSlice(i);  }
  Int           getPOC() const                     { return  m_picSym.getSlice(m_uiCurrSliceIdx)->getPOC();  }
  TComDataCU*   getCtu( UInt ctuRsAddr )           { return  m_picSym.getCtu( ctuRsAddr ); }
  const TComDataCU* getCtu( UInt ctuRsAddr ) const { return  m_picSym.getCtu( ctuRsAddr ); }

  TComPicYuv*   getPicYuvOrg()        { return  m_apcPicYuv[PIC_YUV_ORG]; }
  TComPicYuv*   getPicYuvRec()        { return  m_apcPicYuv[PIC_YUV_REC]; }

  TComPicYuv*   getPicYuvPred()       { return  m_pcPicYuvPred; }
  TComPicYuv*   getPicYuvResi()       { return  m_pcPicYuvResi; }
  Void          setPicYuvPred( TComPicYuv* pcPicYuv )       { m_pcPicYuvPred = pcPicYuv; }
  Void          setPicYuvResi( TComPicYuv* pcPicYuv )       { m_pcPicYuvResi = pcPicYuv; }

  UInt          getNumberOfCtusInFrame() const     { return m_picSym.getNumberOfCtusInFrame(); }
  UInt          getNumPartInCtuWidth() const       { return m_picSym.getNumPartInCtuWidth();   }
  UInt          getNumPartInCtuHeight() const      { return m_picSym.getNumPartInCtuHeight();  }
  UInt          getNumPartitionsInCtu() const      { return m_picSym.getNumPartitionsInCtu();  }
  UInt          getFrameWidthInCtus() const        { return m_picSym.getFrameWidthInCtus();    }
  UInt          getFrameHeightInCtus() const       { return m_picSym.getFrameHeightInCtus();   }
  UInt          getMinCUWidth() const              { return m_picSym.getMinCUWidth();          }
  UInt          getMinCUHeight() const             { return m_picSym.getMinCUHeight();         }

  Int           getStride(const ComponentID id) const          { return m_apcPicYuv[PIC_YUV_REC]->getStride(id); }
  Int           getComponentScaleX(const ComponentID id) const    { return m_apcPicYuv[PIC_YUV_REC]->getComponentScaleX(id); }
  Int           getComponentScaleY(const ComponentID id) const    { return m_apcPicYuv[PIC_YUV_REC]->getComponentScaleY(id); }
  ChromaFormat  getChromaFormat() const                           { return m_apcPicYuv[PIC_YUV_REC]->getChromaFormat(); }
  Int           getNumberValidComponents() const                  { return m_apcPicYuv[PIC_YUV_REC]->getNumberValidComponents(); }

  Void          setReconMark (Bool b) { m_bReconstructed = b;     }
  Bool          getReconMark () const      { return m_bReconstructed;  }
  Void          setOutputMark (Bool b) { m_bNeededForOutput = b;     }
  Bool          getOutputMark () const      { return m_bNeededForOutput;  }

  Void          compressMotion();
  UInt          getCurrSliceIdx() const           { return m_uiCurrSliceIdx;                }
  Void          setCurrSliceIdx(UInt i)      { m_uiCurrSliceIdx = i;                   }
  UInt          getNumAllocatedSlice() const      {return m_picSym.getNumAllocatedSlice();}
  Void          allocateNewSlice()           {m_picSym.allocateNewSlice();         }
  Void          clearSliceBuffer()           {m_picSym.clearSliceBuffer();         }

  const Window& getConformanceWindow() const { return m_picSym.getSPS().getConformanceWindow(); }
  Window        getDefDisplayWindow() const  { return m_picSym.getSPS().getVuiParametersPresentFlag() ? m_picSym.getSPS().getVuiParameters()->getDefaultDisplayWindow() : Window(); }

  Bool          getSAOMergeAvailability(Int currAddr, Int mergeAddr);

  UInt          getSubstreamForCtuAddr(const UInt ctuAddr, const Bool bAddressInRaster, TComSlice *pcSlice);

  /* field coding parameters*/

   Void              setTopField(Bool b)                  {m_isTop = b;}
   Bool              isTopField()                         {return m_isTop;}
   Void              setField(Bool b)                     {m_isField = b;}
   Bool              isField()                            {return m_isField;}

  /** transfer ownership of seis to this picture */
  Void setSEIs(SEIMessages& seis) { m_SEIs = seis; }

  /**
   * return the current list of SEI messages associated with this picture.
   * Pointer is valid until this->destroy() is called */
  SEIMessages& getSEIs() { return m_SEIs; }

  /**
   * return the current list of SEI messages associated with this picture.
   * Pointer is valid until this->destroy() is called */
  const SEIMessages& getSEIs() const { return m_SEIs; }

#if SVC_EXTENSION
  Void          setLayerId(UInt layerId)                                    { m_layerId = layerId;                                       }
  UInt          getLayerId() const                                          { return m_layerId;                                          }
  UInt          getLayerIdx() const                                         { return m_picSym.getSlice(0)->getVPS()->getLayerIdxInVps(m_layerId);     }
  Bool          requireResampling(UInt refLayerIdc) const                   { return m_requireResampling[refLayerIdc];                   }
  Void          setRequireResamplingFlag (UInt refLayerIdc, Bool b)         { m_requireResampling[refLayerIdc] = b;                      }
  Void          setFullPelBaseRec   (UInt refLayerIdc, TComPicYuv* p)       { m_pcFullPelBaseRec[refLayerIdc] = p;                       }
  TComPicYuv*   getFullPelBaseRec   (UInt refLayerIdc) const                { return  m_pcFullPelBaseRec[refLayerIdc];                   }
  Bool          isILR( UInt currLayerId ) const                             { return ( m_bIsLongTerm && m_layerId < currLayerId );       }
  Bool          equalPictureSizeAndOffsetFlag(UInt refLayerIdc) const       { return m_equalPictureSizeAndOffsetFlag[refLayerIdc];       }
  Void          setEqualPictureSizeAndOffsetFlag(UInt refLayerIdc, Bool b)  { m_equalPictureSizeAndOffsetFlag[refLayerIdc] = b;          }
  Void          copyUpsampledMvField(UInt refLayerIdc, Int** mvScalingFactor, Int** posScalingFactor);
  Void          initUpsampledMvField();
  Bool          checkSameRefInfo();
  Void          copyUpsampledPictureYuv(TComPicYuv* pcPicYuvIn, TComPicYuv* pcPicYuvOut); 
  Void          setMvScalingFactor(UInt refLayerIdc, Int compX, Int compY)  { m_mvScalingFactor[0][refLayerIdc] = compX; m_mvScalingFactor[1][refLayerIdc] = compY;   }
  Void          setPosScalingFactor(UInt refLayerIdc, Int compX, Int compY) { m_posScalingFactor[0][refLayerIdc] = compX; m_posScalingFactor[1][refLayerIdc] = compY; }
  Int           getMvScalingFactor(UInt refLayerIdc, UChar comp) const      { return m_mvScalingFactor[comp][refLayerIdc];  }
  Int           getPosScalingFactor(UInt refLayerIdc, UChar comp) const     { return m_posScalingFactor[comp][refLayerIdc]; }
  Int**         getPosScalingFactor()                                       { return m_posScalingFactor; }
  Int**         getMvScalingFactor()                                        { return m_mvScalingFactor;  }
  Void          createMvScalingFactor(UInt numOfILRPs);
  Void          createPosScalingFactor(UInt numOfILRPs);
#if CGS_3D_ASYMLUT
  Void          setFrameBit( Int n )                                        { m_nFrameBit = n;     }
  Int           getFrameBit() const                                         { return m_nFrameBit;  }
#endif
  Bool          isCurrAu() const                                            { return m_currAuFlag; }
  Void          setCurrAuFlag(Bool x)                                       { m_currAuFlag = x;    }
#endif //SVC_EXTENSION
};// END CLASS DEFINITION TComPic

//! \}

#endif // __TCOMPIC__
