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


#include "MVCScalableTestCode.h"

MVCScalableTestCode::MVCScalableTestCode()
{
}
MVCScalableTestCode::~MVCScalableTestCode()
{
}

ErrVal
MVCScalableTestCode::Destroy()
{
	delete this;
	return Err::m_nOK;
}
ErrVal
MVCScalableTestCode::WriteUVLC( UInt uiValue )
{
	UInt uiLength = 1;
	UInt uiTemp = ++uiValue;
	while( uiTemp != 1 )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}
	RNOK( WriteCode( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableTestCode::SEICode( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableTestCode *pcScalableTestCode )
{
	UInt uiNumOperationPointsMinus1 = pcViewScalInfoSei->getNumOperationPointsMinus1();
	pcScalableTestCode->WriteUVLC( uiNumOperationPointsMinus1 );
	for( UInt uiOpId = 0; uiOpId <= uiNumOperationPointsMinus1; uiOpId++ )
	{
		pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getOperationPointId( uiOpId ) );
		pcScalableTestCode->WriteCode( pcViewScalInfoSei->getPriorityId( uiOpId ), 5 );
		pcScalableTestCode->WriteCode( pcViewScalInfoSei->getTemporalId( uiOpId ), 3 );

		UInt uiNumTargetOutputViewsMinus1 = pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId );//SEI JJ
		pcScalableTestCode->WriteUVLC( uiNumTargetOutputViewsMinus1 );//SEI JJ

		for( UInt j = 0; j <= uiNumTargetOutputViewsMinus1; j++ )//SEI JJ
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getViewId( uiOpId, j ) );

		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) );
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) );
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) );
		if(!pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId ))//SEI JJ
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) );//SEI JJ
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) );//SEI JJ
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag( uiOpId ) );//SEI JJ


		if( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getOpProfileLevelIdc( uiOpId ), 8 );//SEI JJ
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet0Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet1Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet2Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet3Flag( uiOpId ) );
			//bug_fix_chenlulu{
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet4Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet5Flag( uiOpId ) );
			pcScalableTestCode->WriteCode( 0, 2 );
			//pcScalableTestCode->WriteCode( 0, 4 );
			//bug_fix_chenlulu}
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getOpLevelIdc( uiOpId ), 8 );
		}

		if( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getAvgBitrate( uiOpId ), 16 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getMaxBitrate( uiOpId ), 16 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getMaxBitrateCalcWindow( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getConstantFrmRateIdc( uiOpId ), 2 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getAvgFrmRate( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) )//SEI JJ 
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j < pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getDirectlyDependentViewId( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getViewDependencyInfoSrcOpId( uiOpId ) );//SEI JJ 

		if( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) )//SEI JJ 
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getSeqParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ) );//SEI JJ
			for ( UInt j=0;j<=pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ); j++)//SEI JJ
				pcScalableTestCode->WriteUVLC(pcViewScalInfoSei->getSubsetSeqParameterSetIdDelta(uiOpId,j));//SEI JJ
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ) );//SEI JJ
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getPicParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getParameterSetsInfoSrcOpId( uiOpId ) );//SEI JJ 
		if (pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag(uiOpId))
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMotionVectorsOverPicBoundariesFlag(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxBytesPerPicDenom(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxBitsPerMbDenom(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthHorizontal(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthVertical(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumReorderFrames(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxDecFrameBuffering(uiOpId));
		}
	}// for

	return Err::m_nOK;
}

