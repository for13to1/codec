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

#if !defined(AFX_QUANTIZER_H__84CC26BC_52EB_45C4_A92E_C5A97D96BF64__INCLUDED_)
#define AFX_QUANTIZER_H__84CC26BC_52EB_45C4_A92E_C5A97D96BF64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN


class QpParameter
{
public :
  const QpParameter& operator= (const QpParameter& rcQp )
  {
    m_iPer  = rcQp.m_iPer;
    m_iRem  = rcQp.m_iRem;
    m_iBits = rcQp.m_iBits;
    m_iAdd  = rcQp.m_iAdd;
    return *this;
  }

  Void setQp( Int iQp, Bool bIntra )
  {
    m_iPer  = (iQp)/6;
    m_iRem  = (iQp)%6;
    m_iBits = QP_BITS + m_iPer;
    m_iAdd  = ( 1 << m_iBits) / (( bIntra ) ?  3 : 6);
  }

  const Int per()   const { return m_iPer; }
  const Int rem()   const { return m_iRem; }
  const Int bits()  const { return m_iBits; }
  const Int add()   const { return m_iAdd; }
  const Int mode()  const { return m_iMode; }
  const Int value() const { return 6*m_iPer+m_iRem; }

private:
  Int m_iPer;
  Int m_iRem;
  Int m_iBits;
  Int m_iAdd;
  Int m_iMode;
};



class H264AVCCOMMONLIB_API Quantizer
{
public:
  Quantizer();
  virtual ~Quantizer();


  Void setQp( const MbDataAccess& rcMbDataAccess, Bool bIntraMb )
  {
    Int   iLumaQp   = rcMbDataAccess.getMbData().getQp();
    Int   iCbQp     = rcMbDataAccess.getSH().getCbQp( iLumaQp );
    Int   iCrQp     = rcMbDataAccess.getSH().getCrQp( iLumaQp );
    Bool  bRefPic   = rcMbDataAccess.getSH().getNalRefIdc () != NAL_REF_IDC_PRIORITY_LOWEST;
    Bool  bKeyPic   = rcMbDataAccess.getSH().getTemporalId() == 0;
    Bool  bOneThird = ( bKeyPic || ( bRefPic && bIntraMb ) );

    m_cLumaQp.setQp( iLumaQp, bOneThird );
    m_cCbQp  .setQp( iCbQp,   bOneThird );
    m_cCrQp  .setQp( iCrQp,   bOneThird );
  }

  Void setDecompositionStages( Int iDStages )
  {
    m_iDStages = iDStages;
  }


  const QpParameter&  getCbQp     ()              const { return m_cCbQp; }
  const QpParameter&  getCrQp     ()              const { return m_cCrQp; }
  const QpParameter&  getChromaQp ( UInt uiIdx )  const { AOT( uiIdx > 1 ); return ( uiIdx ? getCrQp() : getCbQp() ); }
  const QpParameter&  getLumaQp   ()  const { return m_cLumaQp;   }

protected:
  QpParameter m_cLumaQp;
  QpParameter m_cCbQp;
  QpParameter m_cCrQp;
  Int         m_iDStages;
};


H264AVC_NAMESPACE_END

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

#endif // !defined(AFX_QUANTIZER_H__84CC26BC_52EB_45C4_A92E_C5A97D96BF64__INCLUDED_)
