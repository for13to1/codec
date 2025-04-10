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


#include <cstdio>
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/TraceFile.h"



#ifdef MSYS_WIN32
#define snprintf _snprintf
#endif


H264AVC_NAMESPACE_BEGIN



TraceFile::TraceFile()
{
}


TraceFile::~TraceFile()
{
}


UInt  TraceFile::sm_uiLayer;
UInt  TraceFile::sm_uiViewId;
FILE **TraceFile::sm_fTrace;
UInt  *TraceFile::sm_uiFrameNum;
UInt  *TraceFile::sm_uiSliceNum;
UInt  *TraceFile::sm_uiPosCounter;
Char  TraceFile::sm_acLine      [MAX_LINE_LENGTH] ;
Char  TraceFile::sm_acType      [9];
Char  TraceFile::sm_acPos       [9];
Char  TraceFile::sm_acCode      [6];
Char  TraceFile::sm_acBits      [35];
Bool  TraceFile::sm_bEncoder;
UInt  TraceFile::sm_uiNumOfViews;
Bool TraceFile::Initialized;

ErrVal
TraceFile::initTrace(Bool b, UInt uiNumOfViews)
{
  sm_uiLayer    = 0;
  sm_uiViewId   = 0;
  sm_acLine[0]  = '\0';
  sm_acType[0]  = '\0';
  sm_acPos [0]  = '\0';
  sm_acCode[0]  = '\0';
  sm_acBits[0]  = '\0';
  sm_bEncoder   = b;
  sm_uiNumOfViews = uiNumOfViews;

  sm_fTrace = new FILE*[uiNumOfViews];
  sm_uiFrameNum  = new UInt[uiNumOfViews];
  sm_uiSliceNum  = new UInt[uiNumOfViews];
  sm_uiPosCounter= new UInt[uiNumOfViews];

  for( UInt ui = 0; ui < uiNumOfViews; ui++ )
  {
    sm_fTrace      [ui] = NULL;
    sm_uiFrameNum  [ui] = 0;
    sm_uiSliceNum  [ui] = 0;
    sm_uiPosCounter[ui] = 0;
  }

  Initialized = true;
  return Err::m_nOK;
}


ErrVal
TraceFile::openTrace( Char* pucBaseFilename, UInt uiViewId )
{
  for( UInt ui = 0; ui < sm_uiNumOfViews; ui++ )
  {
    Char file[1000];
    if(sm_bEncoder)
    {
      ::snprintf( file, 1000, "%s_%d.txt", pucBaseFilename, uiViewId );
      ROT( NULL == ( sm_fTrace[uiViewId] = ::fopen( file, "wt" ) ) );
      break;
    }
    else
      ::snprintf( file, 1000, "%s_%d.txt", pucBaseFilename, ui );

    ROT( NULL == ( sm_fTrace[ui] = ::fopen( file, "wt" ) ) );
  }

  return Err::m_nOK;
}


ErrVal
TraceFile::closeTrace ()
{
  for( UInt ui = 0; ui < sm_uiNumOfViews; ui++ )
  {
    if( sm_fTrace[ui] )
    {
      ::fclose( sm_fTrace[ui] );
    }
  }
  delete [] sm_fTrace;
  delete [] sm_uiFrameNum;
  delete [] sm_uiSliceNum;
  delete [] sm_uiPosCounter;

  return Err::m_nOK;
}


ErrVal
TraceFile::setLayer( UInt uiLayerId )
{
  if (IsInitialized())
	sm_uiLayer = uiLayerId;
  return Err::m_nOK;
}

ErrVal
TraceFile::setViewId( UInt uiViewId )
{
  if (IsInitialized())
	sm_uiViewId = uiViewId;
  return Err::m_nOK;
}


ErrVal
TraceFile::startNalUnit()
{
  if (IsInitialized())
    RNOK( printHeading("Nal Unit") );
  return Err::m_nOK;
}


ErrVal
TraceFile::startFrame()
{
  if (IsInitialized())
  {
	sm_uiFrameNum[sm_uiViewId]++;
	sm_uiSliceNum[sm_uiViewId]=0;
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::startSlice()
{
  if (IsInitialized())
  {
	Char acSliceHead[100];
	::snprintf( acSliceHead, 100, "Slice # %d Frame # %d", sm_uiSliceNum[sm_uiViewId], sm_uiFrameNum[sm_uiViewId] );
	  
	RNOK( printHeading( acSliceHead ) );
	sm_uiSliceNum[sm_uiViewId]++;
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::startMb( Int iMbAddress )
{
  if (IsInitialized())
  {
	Char acMbHead[100];
	::snprintf( acMbHead, 100, "MB # %d", iMbAddress );
	RNOK( printHeading( acMbHead ) );
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::printHeading( const Char* pcString )
{
  if (IsInitialized())
  {
	if( ! sm_fTrace[sm_uiViewId] )
	{
		sm_acLine[0] = '\0';
		return Err::m_nOK;
	}

	::snprintf( sm_acLine, MAX_LINE_LENGTH, "-------------------- %s --------------------\n", pcString );
	::fprintf ( sm_fTrace[sm_uiViewId], "%s", sm_acLine );
	::fflush  ( sm_fTrace[sm_uiViewId] );
	sm_acLine[0] = '\0';
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::countBits( UInt uiBitCount )
{
  if (IsInitialized())
	sm_uiPosCounter[sm_uiViewId] += uiBitCount;
  return Err::m_nOK;
}



ErrVal
TraceFile::printPos()
{
  if (IsInitialized())		
	::snprintf( sm_acPos, 8, "@%d", sm_uiPosCounter[sm_uiViewId] );
  return Err::m_nOK;
}


ErrVal
TraceFile::printString( const Char* pcString )
{
  if (IsInitialized())
  ::strncat( sm_acLine, pcString, MAX_LINE_LENGTH-strlen(pcString) );
  return Err::m_nOK;
}


ErrVal
TraceFile::printType( Char* pcString )
{
  if (IsInitialized())
   ::snprintf( sm_acType, 8, "%s", pcString);
  return Err::m_nOK;
}


ErrVal
TraceFile::printVal( UInt uiVal )
{
  Char tmp[8];
  if (IsInitialized()) {
	::snprintf( tmp, 8, "%3u", uiVal);
	::strncat( sm_acLine, tmp, MAX_LINE_LENGTH-8 );
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::printVal( Int iVal )
{
  Char tmp[8];
  if (IsInitialized())
  {
	::snprintf( tmp, 8, "%3i", iVal );
	::strncat( sm_acLine, tmp, MAX_LINE_LENGTH-8 );
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::printXVal( UInt uiVal )
{
  Char tmp[8];
  if (IsInitialized())
  {
	::snprintf( tmp, 8, "0x%04x", uiVal );
	::strncat( sm_acLine, tmp, MAX_LINE_LENGTH-8);
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::addBits( UInt uiVal,
                    UInt uiLength )
{
  if (IsInitialized())
  {
	ROT( uiLength > 32 );

	Char acBuffer[33];
	UInt i;
	for ( i = 0; i < uiLength; i++ )
	{
		acBuffer[i] = '0' + ( ( uiVal & ( 1 << (uiLength - i - 1) ) ) >> (uiLength - i - 1 ) );
	}
	acBuffer[uiLength] = '\0';

	i = strlen( sm_acBits );
	if( i < 2 )
	{
		sm_acBits[0] = '[';
		sm_acBits[1] = '\0';
	}
	else
	{
		sm_acBits[i-1] = '\0';
	}
	strncat( sm_acBits, acBuffer, 34 );
	strncat( sm_acBits, "]",      34 );
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::printBits( UInt uiVal,
                      UInt uiLength )
{
  if (IsInitialized()) 
  {
	sm_acBits[0] = '[';
	sm_acBits[1] = ']';
	sm_acBits[2] = '\0';
	RNOK( addBits( uiVal, uiLength ) );
  }
  return Err::m_nOK;
}


ErrVal
TraceFile::printCode( UInt uiVal )
{
  if (IsInitialized())
	::snprintf( sm_acCode, 5, "%u", uiVal );
  return Err::m_nOK;
}


ErrVal
TraceFile::printCode(Int iVal)
{
  if (IsInitialized())	
	::snprintf( sm_acCode, 5, "%i", iVal );
  return Err::m_nOK;
}


ErrVal
TraceFile::newLine()
{
  if (IsInitialized())
  {
	if( ! sm_fTrace[sm_uiViewId] )
	{
		sm_acLine[0] = '\0';
		sm_acType[0] ='\0';
		sm_acCode[0] ='\0';
		sm_acBits[0] ='\0';
		sm_acPos [0] ='\0';
		return Err::m_nOK;
	}

	::fprintf( sm_fTrace[sm_uiViewId], "%-6s",   sm_acPos  );
	::fprintf( sm_fTrace[sm_uiViewId], " %-50s", sm_acLine );
	::fprintf( sm_fTrace[sm_uiViewId], " %-8s",  sm_acType );
	::fprintf( sm_fTrace[sm_uiViewId], " %5s",   sm_acCode );
	::fprintf( sm_fTrace[sm_uiViewId], " %s",    sm_acBits );
	::fprintf( sm_fTrace[sm_uiViewId], "\n");
	::fflush ( sm_fTrace[sm_uiViewId] );

	sm_acLine[0] ='\0';
	sm_acType[0] ='\0';
	sm_acCode[0] ='\0';
	sm_acBits[0] ='\0';
	sm_acPos [0] ='\0';
  }
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
