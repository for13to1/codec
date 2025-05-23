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


#ifndef __MSYS_MACROS_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
#define __MSYS_MACROS_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79

#if defined( _DEBUG ) || defined( DEBUG )
  #if !defined( _DEBUG )
    #define _DEBUG
  #endif
  #if !defined( DEBUG )
    #define DEBUG
  #endif
#endif

#if !defined( ASSERT )
#define ASSERT assert
#endif

#define VOID_TYPE Void
#define ERR_CLASS Err
#define ERR_VAL   ErrVal


#define RERR( )               \
{                             \
    ASSERT( 0 );              \
    return ERR_CLASS::m_nERR; \
}

#define ROF( exp )            \
{                             \
  if( !( exp ) )              \
  {                           \
    RERR();                   \
  }                           \
}

#define ROT( exp )            \
{                             \
  if( ( exp ) )               \
  {                           \
   RERR();                    \
  }                           \
}

#define RERRS( )               \
{                             \
return ERR_CLASS::m_nERR;     \
}


#define ROFS( exp )           \
{                             \
  if( !( exp ) )              \
  {                           \
    RERRS();                  \
  }                           \
}

#define ROTS( exp )           \
{                             \
  if( ( exp ) )               \
  {                           \
    RERRS();                  \
  }                           \
}


#define RVAL( retVal )        \
{                             \
    ASSERT( 0 );              \
    return retVal;            \
}


#define ROFR( exp, retVal )   \
{                             \
  if( !( exp ) )              \
  {                           \
    RVAL( retVal );           \
  }                           \
}


#define ROTR( exp, retVal )   \
{                             \
  if( ( exp ) )               \
  {                           \
    RVAL( retVal );             \
  }                           \
}



#define ROFRS( exp, retVal )  \
{                             \
  if( !( exp ) )              \
  {                           \
    return retVal;            \
  }                           \
}

#define ROTRS( exp, retVal )  \
{                             \
  if( ( exp ) )               \
  {                           \
    return retVal;            \
  }                           \
}

#define ROFV( exp )           \
{                             \
  if( !( exp ) )              \
  {                           \
    ASSERT( 0 );              \
    return;                   \
  }                           \
}

#define ROTV( exp )           \
{                             \
  if( ( exp ) )               \
  {                           \
    ASSERT( 0 );              \
    return;                   \
  }                           \
}

#define ROFVS( exp )          \
{                             \
  if( !( exp ) )              \
  {                           \
    return;                   \
  }                           \
}

#define ROTVS( exp )          \
{                             \
  if( ( exp ) )               \
  {                           \
    return;                   \
  }                           \
}

#define RNOK( exp )               \
{                                 \
  const ERR_VAL nMSysRetVal = ( exp );   \
  if( ERR_CLASS::m_nOK != nMSysRetVal )  \
  {                               \
    ASSERT( 0 );                  \
    return nMSysRetVal;                  \
  }                               \
}

#define RNOKR( exp, retVal )        \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    ASSERT( 0 );                    \
    return retVal;                  \
  }                                 \
}

#define RNOKS( exp )                \
{                                   \
  const ERR_VAL nMSysRetVal = ( exp );     \
  if( ERR_CLASS::m_nOK != nMSysRetVal )    \
  {                                 \
    return nMSysRetVal;             \
  }                                 \
}

#define RNOKRS( exp, retVal )       \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    return retVal;                  \
  }                                 \
}

#define RNOKV( exp )                \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    ASSERT( 0 );                    \
    return;                         \
  }                                 \
}

#define RNOKVS( exp )               \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    return;                         \
  }                                 \
}


#define AF( )                 \
{                             \
    ASSERT( 0 );              \
}


#define ANOK( exp )                 \
{                                   \
  if( ERR_CLASS::m_nOK != ( exp ) ) \
  {                                 \
    ASSERT( 0 );                    \
  }                                 \
}

#define AOF( exp )                  \
{                                   \
  if( !( exp ) )                    \
  {                                 \
    ASSERT( 0 );                    \
  }                                 \
}

#define AOT( exp )            \
{                             \
  if( ( exp ) )               \
  {                           \
    ASSERT( 0 );              \
  }                           \
}





#if defined( _DEBUG ) || defined( DEBUG )
  #define CHECK( exp )      ASSERT( exp )
  #define AOT_DBG( exp )    AOT( exp )
  #define AOF_DBG( exp )    AOF( exp )
  #define ANOK_DBG( exp )   ANOK( exp )
  #define DO_DBG( exp )     ( exp )
#else  // _DEBUG
  #define CHECK( exp )      ((VOID_TYPE)( exp ))
  #define AOT_DBG( exp )    ((VOID_TYPE)0)
  #define AOF_DBG( exp )    ((VOID_TYPE)0)
  #define ANOK_DBG( exp )   ((VOID_TYPE)0)
  #define DO_DBG( exp )     ((VOID_TYPE)0)
#endif // _DEBUG

#endif //__MACROS_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
