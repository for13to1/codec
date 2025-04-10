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


#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/MbTransformCoeffs.h"

#include <stdio.h>

H264AVC_NAMESPACE_BEGIN



ErrVal
MbTransformCoeffs::save( FILE* pFile )
{
  ROF( pFile );

  UInt uiSave  = ::fwrite( this, sizeof(MbTransformCoeffs), 1, pFile );

  ROF( uiSave == 1 );

  return Err::m_nOK;
}


ErrVal
MbTransformCoeffs::load( FILE* pFile )
{
  ROF( pFile );

  UInt uiRead  = ::fread( this, sizeof(MbTransformCoeffs), 1, pFile );

  ROF( uiRead == 1 );

  return Err::m_nOK;
}



Void MbTransformCoeffs::clear()
{
  for (Int i=0; i<24; i++)
    ::memset( &(m_aaiLevel[i][0]), 0, 16*sizeof(TCoeff) );
  
  ::memset( m_aaiLevel, 0, sizeof( m_aaiLevel ) );
  ::memset( m_aaucCoeffCount, 0, sizeof(m_aaucCoeffCount));
}

Void MbTransformCoeffs::clearAcBlk( ChromaIdx cChromaIdx )
{
  ::memset( &m_aaiLevel[16+cChromaIdx][1], 0, sizeof( TCoeff) * 15 );
}



Void
MbTransformCoeffs::clearLumaLevels()
{
  ::memset( &m_aaiLevel[0][0], 0, sizeof(TCoeff)*16*16 );
}

Void
MbTransformCoeffs::clearLumaLevels8x8( B8x8Idx c8x8Idx )
{
  UInt uiIndex = c8x8Idx.b8x8();
  ::memset( &m_aaiLevel[uiIndex  ][0], 0, sizeof(TCoeff)*16 );
  ::memset( &m_aaiLevel[uiIndex+1][0], 0, sizeof(TCoeff)*16 );
  ::memset( &m_aaiLevel[uiIndex+4][0], 0, sizeof(TCoeff)*16 );
  ::memset( &m_aaiLevel[uiIndex+5][0], 0, sizeof(TCoeff)*16 );
}

Void
MbTransformCoeffs::clearLumaLevels8x8Block( B8x8Idx c8x8Idx )
{
  ::memset( &m_aaiLevel[4*c8x8Idx.b8x8Index()][0], 0, sizeof(TCoeff)*64 );
}



Void MbTransformCoeffs::setAllCoeffCount( UChar ucCoeffCountValue )
{
  ::memset( m_aaucCoeffCount, ucCoeffCountValue, sizeof(m_aaucCoeffCount));
}

Void MbTransformCoeffs::copyFrom( MbTransformCoeffs& rcMbTransformCoeffs )
{
  ::memcpy( m_aaiLevel, rcMbTransformCoeffs.m_aaiLevel, sizeof( m_aaiLevel ) );
  ::memcpy( m_aaucCoeffCount, rcMbTransformCoeffs.m_aaucCoeffCount, sizeof( m_aaucCoeffCount ) );
}


MbTransformCoeffs::MbTransformCoeffs()
{
  clear();  
}







Void MbTransformCoeffs::clearNewAcBlk( ChromaIdx          cChromaIdx,
                                       MbTransformCoeffs& rcBaseMbTCoeffs )
{
  TCoeff* piCoeff     = m_aaiLevel[16+cChromaIdx];
  TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[16+cChromaIdx];

  for( UInt ui = 1; ui < 16; ui++ )
  {
    if( ! piCoeffBase[ui] )
    {
      piCoeff[ui] = 0;
    }
  }
}



Void
MbTransformCoeffs::clearNewLumaLevels( MbTransformCoeffs& rcBaseMbTCoeffs )
{
  TCoeff* piCoeff     = m_aaiLevel[0];
  TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[0];

  for( UInt ui = 0; ui < 16*16; ui++ )
  {
    if( ! piCoeffBase[ui] )
    {
      piCoeff[ui] = 0;
    }
  }
}

Void
MbTransformCoeffs::clearNewLumaLevels8x8( B8x8Idx             c8x8Idx,
                                          MbTransformCoeffs&  rcBaseMbTCoeffs )
{
  UInt auiOffset[4] = { 0, 1, 4, 5 };
  UInt uiIndex      = c8x8Idx.b8x8();

  for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
  {
    TCoeff* piCoeff     = m_aaiLevel[uiIndex+auiOffset[uiBlk]];
    TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[uiIndex+auiOffset[uiBlk]];

    for( UInt ui = 0; ui < 16; ui++ )
    {
      if( ! piCoeffBase[ui] )
      {
        piCoeff[ui] = 0;
      }
    }
  }
}

Void
MbTransformCoeffs::clearNewLumaLevels8x8Block( B8x8Idx            c8x8Idx,
                                               MbTransformCoeffs& rcBaseMbTCoeffs )
{
  TCoeff* piCoeff     = m_aaiLevel[4*c8x8Idx.b8x8Index()];
  TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[4*c8x8Idx.b8x8Index()];

  for( UInt ui = 0; ui < 64; ui++ )
  {
    if( ! piCoeffBase[ui] )
    {
      piCoeff[ui] = 0;
    }
  }
}



H264AVC_NAMESPACE_END
