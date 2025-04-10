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

#if !defined(AFX_DECERROR_H__19223BF1_DA9C_475F_9AEE_D1A55237EB1E__INCLUDED_)
#define AFX_DECERROR_H__19223BF1_DA9C_475F_9AEE_D1A55237EB1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if defined (FAULT_TOLERANT)
 #define DECASSERT(x)
#else
 #define DECASSERT(x) ASSERT(x)
#endif


#define DECROF( exp )         \
{                             \
  if( !( exp ) )              \
  {                           \
    DECASSERT( 0 );           \
    return ERR_CLASS::m_nERR; \
  }                           \
}

#define DECROT( exp )         \
{                             \
  if( ( exp ) )               \
  {                           \
    DECASSERT( 0 );           \
    return ERR_CLASS::m_nERR; \
  }                           \
}


#define DECROFR( exp, retVal )   \
{                             \
  if( !( exp ) )              \
  {                           \
    DECASSERT( 0 );           \
    return retVal;            \
  }                           \
}

#define DECROTR( exp, retVal )   \
{                             \
  if( ( exp ) )               \
  {                           \
    DECASSERT( 0 );           \
    return retVal;            \
  }                           \
}


#define DECROFV( exp )        \
{                             \
  if( !( exp ) )              \
  {                           \
    DECASSERT( 0 );           \
    return;                   \
  }                           \
}

#define DECROTV( exp )        \
{                             \
  if( ( exp ) )               \
  {                           \
    DECASSERT( 0 );           \
    return;                   \
  }                           \
}

#if JVT_U125
#define DECRNOK( exp )            \
{                                 \
  const ERR_VAL nMSysRetVal = ( exp );   \
  if( ERR_CLASS::m_nOK != nMSysRetVal )  \
  {                               \
    return nMSysRetVal;           \
  }                               \
}
#else
#define DECRNOK( exp )            \
{                                 \
  const ERR_VAL nMSysRetVal = ( exp );   \
  if( ERR_CLASS::m_nOK != nMSysRetVal )  \
  {                               \
    DECASSERT( 0 );               \
    return nMSysRetVal;           \
  }                               \
}
#endif

#define DECRNOKR( exp, retVal )     \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    DECASSERT( 0 );                 \
    return retVal;                  \
  }                                 \
}


#define DECRNOKV( exp )             \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    DECASSERT( 0 );                 \
    return;                         \
  }                                 \
}


#endif // !defined(AFX_DECERROR_H__19223BF1_DA9C_475F_9AEE_D1A55237EB1E__INCLUDED_)
