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

#if !defined(AFX_MBTRANSFORMCOEFFS_H__B5704512_C23C_497A_A794_36691A0D01BE__INCLUDED_)
#define AFX_MBTRANSFORMCOEFFS_H__B5704512_C23C_497A_A794_36691A0D01BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN

class YuvMbBuffer;

class H264AVCCOMMONLIB_API MbTransformCoeffs
{
public:
  ErrVal copyPredictionFrom( YuvMbBuffer &rcPred );
  ErrVal copyPredictionTo( YuvMbBuffer &rcPred );
  ErrVal clearPrediction();

  MbTransformCoeffs() ;
  TCoeff*       get   ( LumaIdx   cLumaIdx )          { return &m_aaiLevel[cLumaIdx     ][0]; }
  const TCoeff* get   ( LumaIdx   cLumaIdx )    const { return &m_aaiLevel[cLumaIdx     ][0]; }

  TCoeff*       get   ( ChromaIdx cChromaIdx )        { return &m_aaiLevel[16+cChromaIdx][0]; }
  const TCoeff* get   ( ChromaIdx cChromaIdx )  const { return &m_aaiLevel[16+cChromaIdx][0]; }

  TCoeff*       get8x8( B8x8Idx   c8x8Idx )           { return &m_aaiLevel[4*c8x8Idx.b8x8Index()][0]; }
  const TCoeff* get8x8( B8x8Idx   c8x8Idx )     const { return &m_aaiLevel[4*c8x8Idx.b8x8Index()][0]; }

  TCoeff*       getTCoeffBuffer()                     { return &m_aaiLevel[0][0]; }
  const TCoeff* getTCoeffBuffer()               const { return &m_aaiLevel[0][0]; }

  UInt calcCoeffCount( LumaIdx cLumaIdx, Bool bIs8x8, Bool bInterlaced, UInt uiStart, UInt uiStop ) const;
  UInt calcCoeffCount( ChromaIdx cChromaIdx, const UChar *pucScan, UInt uiStart, UInt uiStop ) const;
  UInt getCoeffCount( LumaIdx cLumaIdx )                   const { return m_aaucCoeffCount[cLumaIdx]; }
  UInt getCoeffCount( ChromaIdx cChromaIdx )               const { return m_aaucCoeffCount[16+cChromaIdx]; }
  Void setCoeffCount( LumaIdx cLumaIdx, UInt uiCoeffCount )      { m_aaucCoeffCount[cLumaIdx] = uiCoeffCount; }
  Void setCoeffCount( ChromaIdx cChromaIdx, UInt uiCoeffCount )  { m_aaucCoeffCount[16+cChromaIdx] = uiCoeffCount; }
  Void clear();
  Void setAllCoeffCount( UChar ucCoeffCountValue = 0 );
  Void copyFrom( const MbTransformCoeffs& rcMbTransformCoeffs );
  Void copyFrom( MbTransformCoeffs& rcMbTransformCoeffs, ChromaIdx cChromaIdx );
  Void copyCoeffCounts( const MbTransformCoeffs& rcMbTransformCoeffs );
  Void reset() {}

	Void dump ( FILE* hFile ) const;

  ErrVal  load( FILE* pFile );
  ErrVal  save( FILE* pFile );

  Void  clearAcBlk                 ( ChromaIdx cChromaIdx );
  Void  clearLumaLevels            ();
  Void  clearChromaLevels          ();
  Void  clearLumaLevels4x4         ( LumaIdx c4x4Idx );
  Void  clearLumaLevels8x8         ( B8x8Idx c8x8Idx );
  Void  clearLumaLevels8x8Block    ( B8x8Idx c8x8Idx );
  Void  clearNewLumaLevels         ( MbTransformCoeffs& rcBaseMbTCoeffs );
  Void  clearNewLumaLevels8x8      ( B8x8Idx c8x8Idx, MbTransformCoeffs& rcBaseMbTCoeffs );
  Void  clearNewLumaLevels8x8Block ( B8x8Idx c8x8Idx, MbTransformCoeffs& rcBaseMbTCoeffs );

  Void storeLevelData              ();
  Void switchLevelCoeffData        ();
  Void add                         ( MbTransformCoeffs* pcCoeffs, Bool bLuma = true, Bool bChroma = true );

  Bool  allCoeffsZero               ()  const;
  Bool  allLevelsZero               ()  const;
  Bool  allLevelsAndPredictionsZero ()  const;

protected:
  TCoeff m_aaiLevel[24][16];
  UChar  m_aaucCoeffCount[24];
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBTRANSFORMCOEFFS_H__B5704512_C23C_497A_A794_36691A0D01BE__INCLUDED_)
