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


#ifndef _RESIZE_PARAMETERS_H_
#define _RESIZE_PARAMETERS_H_

#include "Typedefs.h"
#include "H264AVCCommonIf.h"

#define MAX_PICT_PARAM_NB          128
#define	MAX_REFLIST_SIZE		   32
//H264AVC_NAMESPACE_BEGIN

#define INTRA_UPSAMPLING_TYPE_DEFAULT     2

#define SST_RATIO_1   0
#define SST_RATIO_2   1
#define SST_RATIO_3_2 2
#define SST_RATIO_X   3

#define ESS_NONE      0
#define ESS_SEQ       1
#define ESS_PICT      2

struct PictureParameters {
    Int  m_iPosX;          // Position     Xorig
    Int  m_iPosY;          //              Yorig
    Int  m_iOutWidth;      // Size of the upsampled baselayer
    Int  m_iOutHeight;     //  
    Int  m_iBaseChromaPhaseX;
    Int  m_iBaseChromaPhaseY;
};

class ResizeParameters {
public:
    ResizeParameters()
    { m_iExtendedSpatialScalability = ESS_NONE;
    m_iSpatialScalabilityType = SST_RATIO_1;
    m_bCrop = false;

    m_iChromaPhaseX = -1;  
    m_iChromaPhaseY = 0;
    m_iBaseChromaPhaseX = -1;  
    m_iBaseChromaPhaseY = 0;
    m_iIntraUpsamplingType = INTRA_UPSAMPLING_TYPE_DEFAULT;

    m_pParamFile = NULL;
#ifndef DOWN_CONVERT_STATIC
    init();
#endif
    };

    Void init ();

    Void  setCurrentPictureParametersWith ( Int index );
    Void setPictureParametersByOffset ( Int iIndex, Int iOl, Int iOr, Int iOt, Int iOb, Int iBcpx, Int iBcpy );
    Void setPictureParametersByValue ( Int index, Int px, Int py, Int ow, Int oh, Int bcpx, Int bcpy );

    const PictureParameters* getCurrentPictureParameters ( Int index ) 
                    { return &m_acCurrentGop[index%MAX_PICT_PARAM_NB];} 
    Int  getLeftOffset   ( Int index ) const 
                    { return (m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosX) /2; }
    Int  getRightOffset  ( Int index ) const 
                     { return (m_iGlobWidth - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosX - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iOutWidth) /2; }
    Int  getTopOffset    ( Int index ) const 
                     { return (m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosY) /2; }
    Int  getBottomOffset ( Int index ) const 
                     { return (m_iGlobHeight - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosY - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iOutHeight) /2; }
    Int  getBaseChromaPhaseX ( Int index ) const 
                     { return m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iBaseChromaPhaseX; }
    Int  getBaseChromaPhaseY ( Int index ) const 
                     { return m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iBaseChromaPhaseY; }

    Int  getPOC() const { return m_iPOC; }; 
    Void setPOC( Int poc ) { m_iPOC = poc; }; 
    ErrVal readPictureParameters ( Int index ); 
    Void initRefListPoc();

    Void print ();

public:
    Int m_iExtendedSpatialScalability;
    Int m_iSpatialScalabilityType;

    Bool m_bCrop;                // if crop is needed

    // ---- Global
    Int  m_iInWidth;       // Size of the baselayer
    Int  m_iInHeight;      //
    Int  m_iGlobWidth;     // Global size  Wenh      if (!bCrop) then it's equal to iOutWidth
    Int  m_iGlobHeight;    //              Henh      if (!bCrop) then it's equal to iOutHeight
    Int m_iChromaPhaseX;
    Int m_iChromaPhaseY;

    // ----- Last Value if by picture
    Int  m_iPosX;          // Position     Xorig
    Int  m_iPosY;          //              Yorig
    Int  m_iOutWidth;      // Size of the upsampled baselayer
    Int  m_iOutHeight;     //  
    Int  m_iBaseChromaPhaseX;
    Int  m_iBaseChromaPhaseY;

    // ----- Intra Upsampling method
    Int m_iIntraUpsamplingType;   // 1:lanczos, 2:?pel + bilin ?pel

    // ----- PICT LEVEL   
    Int   m_iPOC; 
    FILE  *m_pParamFile; 
    Int   m_aiRefListPoc[2][MAX_REFLIST_SIZE]; 

protected:
    PictureParameters m_acCurrentGop[MAX_PICT_PARAM_NB];

private:
    Void xCleanGopParameters     ( PictureParameters * pc );
    Void xCleanPictureParameters ( PictureParameters * pc );
    Void xInitPictureParameters  ( PictureParameters * pc );
} ;

#undef INTRA_UPSAMPLING_TYPE_DEFAULT
//H264AVC_NAMESPACE_END

#endif 
