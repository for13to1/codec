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

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/ScalingMatrix.h"



H264AVC_NAMESPACE_BEGIN



ScalingMatrix::ScalingMatrix()
{
}


const UChar*
ScalingMatrix::get( UInt uiMatrix ) const
{
  if( uiMatrix < 6)
  {
    return  m_acScalingMatrix4x4.get(uiMatrix  ).getMatrix();
  }
  return    m_acScalingMatrix8x8.get(uiMatrix-6).getMatrix();
}


ErrVal
ScalingMatrix::write( HeaderSymbolWriteIf*  pcWriteIf,
                      Bool                  bWrite8x8 ) const
{
  for( Int i4x4 = 0; i4x4 < 6; i4x4++ )
  {
    RNOK( m_acScalingMatrix4x4[i4x4].write( pcWriteIf, g_aucFrameScan   ) );
  }
  ROTRS( ! bWrite8x8, Err::m_nOK );

  for( Int i8x8 = 0; i8x8 < 2; i8x8++ )
  {
    RNOK( m_acScalingMatrix8x8[i8x8].write( pcWriteIf, g_aucFrameScan64 ) );
  }

  return Err::m_nOK;
}


ErrVal
ScalingMatrix::read( HeaderSymbolReadIf*  pcReadIf,
                     Bool                 bRead8x8 )
{
  for( Int i4x4 = 0; i4x4 < 6; i4x4++ )
  {
    RNOK( m_acScalingMatrix4x4.get(i4x4).read( pcReadIf, g_aucFrameScan   ) );
  }
  ROTRS( ! bRead8x8, Err::m_nOK );

  RNOK  ( m_acScalingMatrix8x8.get(0   ).read( pcReadIf, g_aucFrameScan64 ) );
  RNOK  ( m_acScalingMatrix8x8.get(1   ).read( pcReadIf, g_aucFrameScan64 ) );

  return Err::m_nOK;
}

ErrVal
ScalingMatrix::init( const UChar* pucScaleMatrixBuffer )
{
  m_acScalingMatrix4x4.get( 0 ).init( pucScaleMatrixBuffer+0*64, 0 );
  m_acScalingMatrix4x4.get( 1 ).init( pucScaleMatrixBuffer+1*64, pucScaleMatrixBuffer+0*64 );
  m_acScalingMatrix4x4.get( 2 ).init( pucScaleMatrixBuffer+2*64, pucScaleMatrixBuffer+1*64 );

  m_acScalingMatrix4x4.get( 3 ).init( pucScaleMatrixBuffer+3*64, 0 );
  m_acScalingMatrix4x4.get( 4 ).init( pucScaleMatrixBuffer+4*64, pucScaleMatrixBuffer+3*64 );
  m_acScalingMatrix4x4.get( 5 ).init( pucScaleMatrixBuffer+5*64, pucScaleMatrixBuffer+4*64 );

  m_acScalingMatrix8x8.get( 0 ).init( pucScaleMatrixBuffer+6*64, 0 );
  m_acScalingMatrix8x8.get( 1 ).init( pucScaleMatrixBuffer+7*64, 0 );

  return Err::m_nOK;
}


Bool
ScalingMatrix::isSame( const ScalingMatrix&  rcSM )  const
{
  for( UInt ui = 0; ui < 6; ui++ )
  {
    ROFRS( m_acScalingMatrix4x4[ ui ].isSame( rcSM.m_acScalingMatrix4x4[ ui ] ),  false );
  }
  for( UInt ui = 0; ui < 2; ui++ )
  {
    ROFRS( m_acScalingMatrix8x8[ ui ].isSame( rcSM.m_acScalingMatrix8x8[ ui ] ),  false );
  }
  return true;
}


H264AVC_NAMESPACE_END



