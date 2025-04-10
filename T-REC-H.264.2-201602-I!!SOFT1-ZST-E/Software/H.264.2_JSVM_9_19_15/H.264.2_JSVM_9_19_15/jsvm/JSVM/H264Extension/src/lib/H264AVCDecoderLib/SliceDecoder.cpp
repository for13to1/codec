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

#include "H264AVCDecoderLib.h"
#include "MbDecoder.h"
#include "SliceDecoder.h"
#include "H264AVCCommonLib/SliceHeader.h"
#include "DecError.h"

#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Frame.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceDecoder::SliceDecoder()
: m_pcMbDecoder       ( 0 )
, m_pcControlMng      ( 0 )
, m_bInitDone         ( false )
{
}

SliceDecoder::~SliceDecoder()
{
}

ErrVal
SliceDecoder::create( SliceDecoder*& rpcSliceDecoder )
{
  rpcSliceDecoder = new SliceDecoder;
  ROT( NULL == rpcSliceDecoder );
  return Err::m_nOK;
}

ErrVal
SliceDecoder::destroy()
{
  ROT( m_bInitDone );
  delete this;
  return Err::m_nOK;
}

ErrVal
SliceDecoder::init( MbDecoder*        pcMbDecoder,
                    ControlMngIf*     pcControlMng )
{
  ROT( m_bInitDone );
  ROF( pcMbDecoder );
  ROF( pcControlMng );

  m_pcMbDecoder       = pcMbDecoder;
  m_pcControlMng      = pcControlMng;
  m_bInitDone         = true;
  return Err::m_nOK;
}


ErrVal
SliceDecoder::uninit()
{
  ROF( m_bInitDone );

  m_pcMbDecoder       = 0;
  m_pcControlMng      = 0;
  m_bInitDone         = false;
  return Err::m_nOK;
}


ErrVal
SliceDecoder::decode( SliceHeader&  rcSH,
                      MbDataCtrl*   pcMbDataCtrl,
                      MbDataCtrl*   pcMbDataCtrlBase,
                      Frame*        pcFrame,
                      Frame*        pcResidualLF,
                      Frame*        pcResidualILPred,
                      Frame*        pcBaseLayer,
                      Frame*        pcBaseLayerResidual,
                      RefFrameList* pcRefFrameList0,
                      RefFrameList* pcRefFrameList1,
                      MbDataCtrl*   pcMbDataCtrl0L1,
                      Bool          bReconstructAll )
{
  ROF( m_bInitDone );

  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, pcMbDataCtrl0L1 ) );

  const PicType ePicType = rcSH.getPicType();
	if( ePicType != FRAME )
	{
		if( pcFrame )             RNOK( pcFrame->            addFieldBuffer( ePicType ) );
    if( pcResidualLF )        RNOK( pcResidualLF->       addFieldBuffer( ePicType ) );
    if( pcResidualILPred )    RNOK( pcResidualILPred->   addFieldBuffer( ePicType ) );
		if( pcBaseLayer )         RNOK( pcBaseLayer->        addFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
	}

  //===== loop over macroblocks =====
  UInt uiMbAddress     = rcSH.getFirstMbInSlice();
  UInt uiNumMbsInSlice = rcSH.getNumMbsInSlice();
  for( UInt uiNumMbsDecoded = 0; uiNumMbsDecoded < uiNumMbsInSlice; uiNumMbsDecoded++ )
  {
    MbDataAccess* pcMbDataAccess     = NULL;
    MbDataAccess* pcMbDataAccessBase = NULL;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress                            );

    RNOK( pcMbDataCtrl  ->initMb            (  pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase->initMb        (  pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcControlMng->initMbForDecoding ( *pcMbDataAccess,     uiMbY, uiMbX, false ) );

    if( pcMbDataAccess->getMbData().getBLSkipFlag() && ! m_apabBaseModeFlagAllowedArrays[ ePicType == FRAME ? 0 : 1 ][ uiMbAddress ] )
    {
      printf( "CIU constraint violated (MbAddr=%d) ==> ignore\n", uiMbAddress );
    }

    RNOK( m_pcMbDecoder ->decode            ( *pcMbDataAccess,
                                              pcMbDataAccessBase,
                                              pcFrame                                  ->getPic( ePicType ),
                                              pcResidualLF                             ->getPic( ePicType ),
                                              pcResidualILPred                         ->getPic( ePicType ),
                                              pcBaseLayer         ? pcBaseLayer        ->getPic( ePicType ) : NULL,
                                              pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                                              pcRefFrameList0,
                                              pcRefFrameList1,
                                              bReconstructAll ) );

    uiMbAddress=rcSH.getFMO()->getNextMBNr(uiMbAddress);
  }

  if( ePicType!=FRAME )
	{
		if( pcFrame )             RNOK( pcFrame->            removeFieldBuffer( ePicType ) );
    if( pcResidualLF )        RNOK( pcResidualLF->       removeFieldBuffer( ePicType ) );
    if( pcResidualILPred )    RNOK( pcResidualILPred->   removeFieldBuffer( ePicType ) );
		if( pcBaseLayer )         RNOK( pcBaseLayer->        removeFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
	}

  return Err::m_nOK;
}


ErrVal
SliceDecoder::decodeMbAff( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           MbDataCtrl*    pcMbDataCtrlBaseField,
                           Frame*         pcFrame,
                           Frame*         pcResidualLF,
                           Frame*         pcResidualILPred,
                           Frame*         pcBaseLayer,
                           Frame*         pcBaseLayerResidual,
                           RefFrameList*  pcRefFrameList0,
                           RefFrameList*  pcRefFrameList1,
                           MbDataCtrl*    pcMbDataCtrl0L1,
                           Bool           bReconstructAll )
{
  ROF( m_bInitDone );

  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, pcMbDataCtrl0L1 ) );

  RefFrameList acRefFrameList0[2];
  RefFrameList acRefFrameList1[2];

  RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
  RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

  rcSH.setRefFrameList( &(acRefFrameList0[0]), TOP_FIELD, LIST_0 );
  rcSH.setRefFrameList( &(acRefFrameList0[1]), BOT_FIELD, LIST_0 );
  rcSH.setRefFrameList( &(acRefFrameList1[0]), TOP_FIELD, LIST_1 );
  rcSH.setRefFrameList( &(acRefFrameList1[1]), BOT_FIELD, LIST_1 );

  RefFrameList* apcRefFrameList0[2];
  RefFrameList* apcRefFrameList1[2];

  apcRefFrameList0[0] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
  apcRefFrameList0[1] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
  apcRefFrameList1[0] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
  apcRefFrameList1[1] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];

  Frame* apcFrame            [4] = { NULL, NULL, NULL, NULL };
  Frame* apcResidualLF       [4] = { NULL, NULL, NULL, NULL };
  Frame* apcResidualILPred   [4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayer        [4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayerResidual[4] = { NULL, NULL, NULL, NULL };

	RNOK( gSetFrameFieldArrays( apcFrame,             pcFrame             ) );
  RNOK( gSetFrameFieldArrays( apcResidualLF,        pcResidualLF        ) );
  RNOK( gSetFrameFieldArrays( apcResidualILPred,    pcResidualILPred    ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayer,         pcBaseLayer         ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayerResidual, pcBaseLayerResidual ) );

  //===== loop over macroblocks =====
  Bool bSNR             = rcSH.getTCoeffLevelPredictionFlag() || rcSH.getSCoeffResidualPredFlag();
  UInt uiMbAddress      = rcSH.getFirstMbInSlice();
  UInt uiLastMbAddress  = rcSH.getFirstMbInSlice() + rcSH.getNumMbsInSlice() - 1;
  for( ; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
  {
    for( UInt eP = 0; eP < 2; eP++ )
    {
      UInt uiMbAddressMbAff = uiMbAddress + eP;
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      UInt          uiMbY, uiMbX;

      RefFrameList* pcRefFrameList0F;
      RefFrameList* pcRefFrameList1F;

      rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff                     );

      RNOK( pcMbDataCtrl->      initMb       (  pcMbDataAccess,    uiMbY, uiMbX       ) );
      pcMbDataAccess->setFieldMode( pcMbDataAccess->getMbData().getFieldFlag()          );

      if( ( pcMbDataAccess->getMbPicType()==FRAME || bSNR ) && pcMbDataCtrlBase )
      {
        RNOK( pcMbDataCtrlBase->initMb        ( pcMbDataAccessBase, uiMbY, uiMbX  ) );
      }
      else if( pcMbDataAccess->getMbPicType()<FRAME && !bSNR && pcMbDataCtrlBaseField )
      {
        RNOK( pcMbDataCtrlBaseField->initMb   ( pcMbDataAccessBase, uiMbY, uiMbX  ) );
      }

      if( bSNR && rcSH.getSliceSkipFlag() )
      {
        pcMbDataAccess->setFieldMode( pcMbDataAccessBase->getMbData().getFieldFlag() );
      }

      RNOK( m_pcControlMng->initMbForDecoding( *pcMbDataAccess,    uiMbY, uiMbX, true ) );

      const PicType eMbPicType  = pcMbDataAccess->getMbPicType();
			const UInt    uiLI        = eMbPicType - 1;
      if( FRAME == eMbPicType )
      {
        pcRefFrameList0F = pcRefFrameList0;
        pcRefFrameList1F = pcRefFrameList1;
      }
      else
      {
        pcRefFrameList0F = apcRefFrameList0[eP];
        pcRefFrameList1F = apcRefFrameList1[eP];
      }

      if( pcMbDataAccess->getMbData().getBLSkipFlag() && ! m_apabBaseModeFlagAllowedArrays[ eMbPicType == FRAME ? 0 : 1 ][ uiMbAddressMbAff ] )
      {
        printf( "CIU constraint violated (MbAddr=%d) ==> ignore\n", uiMbAddressMbAff );
      }

      RNOK( m_pcMbDecoder->decode ( *pcMbDataAccess,
                                    pcMbDataAccessBase,
                                    apcFrame            [uiLI],
                                    apcResidualLF       [uiLI],
                                    apcResidualILPred   [uiLI],
                                    apcBaseLayer        [uiLI],
                                    apcBaseLayerResidual[uiLI],
                                    pcRefFrameList0F,
                                    pcRefFrameList1F,
                                    bReconstructAll ) );
    }
  }

  if( pcFrame )             RNOK( pcFrame->            removeFrameFieldBuffer() );
  if( pcResidualLF )        RNOK( pcResidualLF->       removeFrameFieldBuffer() );
  if( pcResidualILPred )    RNOK( pcResidualILPred->   removeFrameFieldBuffer() );
  if( pcBaseLayer )         RNOK( pcBaseLayer->        removeFrameFieldBuffer() );
  if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFrameFieldBuffer() );

	return Err::m_nOK;
}


H264AVC_NAMESPACE_END
