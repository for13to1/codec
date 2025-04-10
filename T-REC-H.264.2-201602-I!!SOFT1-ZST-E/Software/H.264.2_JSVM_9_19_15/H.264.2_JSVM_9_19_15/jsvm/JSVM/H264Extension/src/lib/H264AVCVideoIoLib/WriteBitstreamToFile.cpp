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

#include "H264AVCVideoIoLib.h"
#include "WriteBitstreamToFile.h"


ErrVal WriteBitstreamToFile::create( WriteBitstreamToFile*& rpcWriteBitstreamToFile )
{
  rpcWriteBitstreamToFile = new WriteBitstreamToFile;
  ROT( NULL == rpcWriteBitstreamToFile );
  return Err::m_nOK;
}



ErrVal WriteBitstreamToFile::init( const std::string& rcFileName, Bool bNewFileOnNewAu )
{
  m_bNewFileOnNewAu = bNewFileOnNewAu;
  m_cFileName = rcFileName;
  if( Err::m_nOK != m_cFile.open( rcFileName, LargeFile::OM_WRITEONLY ) )
  {
    std::cerr << "Failed to create output bitstream " << rcFileName.data() << std::endl;
    return Err::m_nERR;
  }

  m_uiNumber = 0;
  return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::uninit()
{
  if( m_cFile.is_open() )
  {
    RNOK( m_cFile.close() );
  }
  return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::destroy()
{
  ROT( m_cFile.is_open() );
  RNOK( uninit() );
  delete this;
  return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::writePacket( BinData* pcBinData, Bool bNewAU )
{
  BinDataAccessor cBinDataAccessor;
  pcBinData->setMemAccessor( cBinDataAccessor );
  RNOK( writePacket( &cBinDataAccessor, bNewAU ) );
  return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::writePacket( BinDataAccessor* pcBinDataAccessor, Bool bNewAU )
{
  ROTRS( NULL == pcBinDataAccessor, Err::m_nOK );

  if( bNewAU && m_bNewFileOnNewAu )
  {
#if defined MSYS_WIN32
    if( m_cFile.is_open() )
    {
      RNOK( m_cFile.close() );
    }

    std::string cFileName = m_cFileName;
    Int iPos = (Int)cFileName.find_last_of(".");

    Char acBuffer[20];
    itoa( ++m_uiNumber, acBuffer, 10 );
    cFileName.insert( iPos, acBuffer );
    if( Err::m_nOK != m_cFile.open( cFileName, LargeFile::OM_WRITEONLY ) )
    {
      std::cerr << "Failed to create output bitstream " << cFileName.data() << std::endl;
      return Err::m_nERR;
    }
#else
   std::cerr << "multiple output bitstreams only supported in Win32";
   AF();
#endif
  }

  if( 0 != pcBinDataAccessor->size())
  {
    RNOK( m_cFile.write( pcBinDataAccessor->data(), pcBinDataAccessor->size() ) );
  }

  return Err::m_nOK;
}

ErrVal
WriteBitstreamToFile::writePacket( Void* pBuffer, UInt uiLength )
{
  if( uiLength )
  {
    RNOK( m_cFile.write( pBuffer, uiLength ) );
  }
  return Err::m_nOK;
}
