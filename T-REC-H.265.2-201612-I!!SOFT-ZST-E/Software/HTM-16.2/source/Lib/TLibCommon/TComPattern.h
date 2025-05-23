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

/** \file     TComPattern.h
    \brief    neighbouring pixel access classes (header)
*/

#ifndef __TCOMPATTERN__
#define __TCOMPATTERN__

// Include files
#include "CommonDef.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class TComDataCU;
class TComTU;

/// neighbouring pixel access class for one component
class TComPatternParam
{
private:
  Pel*  m_piROIOrigin;

public:
  Int   m_iROIWidth;
  Int   m_iROIHeight;
  Int   m_iPatternStride;
  Int   m_bitDepth;

  /// return starting position of ROI (ROI = &pattern[AboveOffset][LeftOffset])
  __inline Pel*  getROIOrigin()
  {
    return  m_piROIOrigin;
  }
  __inline const Pel*  getROIOrigin() const
  {
    return  m_piROIOrigin;
  }

  /// set parameters from Pel buffer for accessing neighbouring pixels
  Void setPatternParamPel( Pel* piTexture, Int iRoiWidth, Int iRoiHeight, Int iStride, Int bitDepth );
};

/// neighbouring pixel access class for all components
class TComPattern
{
private:
  TComPatternParam  m_cPatternY;
//  TComPatternParam  m_cPatternCb;
  //TComPatternParam  m_cPatternCr;
#if NH_3D_IC
  Bool              m_bICFlag;
#endif
#if NH_3D_SDC_INTER
  Bool              m_bSDCMRSADFlag;
#endif


public:

  // ROI & pattern information, (ROI = &pattern[AboveOffset][LeftOffset])
  Pel*  getROIY()                 { return m_cPatternY.getROIOrigin();    }
  const Pel*  getROIY() const     { return m_cPatternY.getROIOrigin();    }
  Int   getROIYWidth() const      { return m_cPatternY.m_iROIWidth;       }
  Int   getROIYHeight() const     { return m_cPatternY.m_iROIHeight;      }
  Int   getPatternLStride() const { return m_cPatternY.m_iPatternStride;  }
  Int   getBitDepthY() const      { return m_cPatternY.m_bitDepth;        }

#if NH_3D_IC
  Bool  getICFlag()               const { return m_bICFlag; }
  Void  setICFlag( Bool bICFlag ) { m_bICFlag = bICFlag; }
#endif
#if NH_3D_SDC_INTER
  Bool  getSDCMRSADFlag()         const { return m_bSDCMRSADFlag; }
  Void  setSDCMRSADFlag( Bool bSDCMRSADFlag )    { m_bSDCMRSADFlag = bSDCMRSADFlag; }
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // initialization functions
  // -------------------------------------------------------------------------------------------------------------------

  /// set parameters from Pel buffers for accessing neighbouring pixels
  Void initPattern( Pel* piY, Int iRoiWidth, Int iRoiHeight, Int iStride, Int bitDepthLuma );
};

//! \}

#endif // __TCOMPATTERN__
