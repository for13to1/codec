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
#include "H264AVCCommonLib/CabacContextModel.h"


H264AVC_NAMESPACE_BEGIN


const Double CabacContextModel::m_afProbability[128] =
{
  0.981250,  0.980247,  0.979190,  0.978077,  0.976904,  0.975668,  0.974367,  0.972995,
  0.971550,  0.970028,  0.968425,  0.966736,  0.964956,  0.963081,  0.961106,  0.959025,
  0.956833,  0.954524,  0.952091,  0.949527,  0.946827,  0.943982,  0.940986,  0.937828,
  0.934502,  0.930998,  0.927306,  0.923417,  0.919320,  0.915004,  0.910457,  0.905666,
  0.900619,  0.895302,  0.889701,  0.883800,  0.877583,  0.871034,  0.864134,  0.856866,
  0.849208,  0.841141,  0.832642,  0.823688,  0.814256,  0.804318,  0.793849,  0.782820,
  0.771201,  0.758961,  0.746065,  0.732480,  0.718167,  0.703089,  0.687205,  0.670470,
  0.652841,  0.634268,  0.614701,  0.594088,  0.572371,  0.549493,  0.525391,  0.500000,
  0.500000,  0.474609,  0.450507,  0.427629,  0.405912,  0.385299,  0.365732,  0.347159,
  0.329530,  0.312795,  0.296911,  0.281833,  0.267520,  0.253935,  0.241039,  0.228799,
  0.217180,  0.206151,  0.195682,  0.185744,  0.176312,  0.167358,  0.158859,  0.150792,
  0.143134,  0.135866,  0.128966,  0.122417,  0.116200,  0.110299,  0.104698,  0.099381,
  0.094334,  0.089543,  0.084996,  0.080680,  0.076583,  0.072694,  0.069002,  0.065498,
  0.062172,  0.059014,  0.056018,  0.053173,  0.050473,  0.047909,  0.045476,  0.043167,
  0.040975,  0.038894,  0.036919,  0.035044,  0.033264,  0.031575,  0.029972,  0.028450,
  0.027005,  0.025633,  0.024332,  0.023096,  0.021923,  0.020810,  0.019753,  0.018750
};

const Double CabacContextModel::m_afEntropy[128] =
{
  0.027307,  0.028783,  0.030339,  0.031980,  0.033711,  0.035537,  0.037464,  0.039496,
  0.041639,  0.043901,  0.046288,  0.048807,  0.051465,  0.054271,  0.057233,  0.060359,
  0.063661,  0.067147,  0.070829,  0.074718,  0.078827,  0.083168,  0.087756,  0.092604,
  0.097730,  0.103150,  0.108882,  0.114945,  0.121361,  0.128150,  0.135338,  0.142949,
  0.151011,  0.159553,  0.168608,  0.178208,  0.188392,  0.199199,  0.210672,  0.222859,
  0.235810,  0.249581,  0.264232,  0.279830,  0.296447,  0.314162,  0.333063,  0.353247,
  0.374821,  0.397903,  0.422627,  0.449140,  0.477608,  0.508220,  0.541188,  0.576755,
  0.615197,  0.656836,  0.702043,  0.751252,  0.804976,  0.863826,  0.928535,  1.000000,
  1.000000,  1.075190,  1.150380,  1.225570,  1.300760,  1.375950,  1.451140,  1.526330,
  1.601519,  1.676709,  1.751899,  1.827089,  1.902279,  1.977469,  2.052659,  2.127849,
  2.203039,  2.278229,  2.353419,  2.428609,  2.503799,  2.578989,  2.654178,  2.729368,
  2.804558,  2.879748,  2.954938,  3.030128,  3.105318,  3.180508,  3.255698,  3.330888,
  3.406078,  3.481268,  3.556458,  3.631648,  3.706837,  3.782027,  3.857217,  3.932407,
  4.007597,  4.082787,  4.157977,  4.233167,  4.308357,  4.383547,  4.458737,  4.533927,
  4.609117,  4.684307,  4.759497,  4.834686,  4.909876,  4.985066,  5.060256,  5.135446,
  5.210636,  5.285826,  5.361016,  5.436206,  5.511396,  5.586586,  5.661776,  5.736966,
};



CabacContextModel::CabacContextModel() :
  m_ucState( 0 )
, m_uiCount( 0 )
{
}

CabacContextModel::~CabacContextModel()
{
}


H264AVC_NAMESPACE_END
