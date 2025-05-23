/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2016, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TAppEncCfg.cpp
    \brief    Handle encoder configuration parameters
*/

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include <limits>
#include "TLibCommon/TComRom.h"
#include "TAppEncCfg.h"
#include "TAppCommon/program_options_lite.h"
#include "TLibEncoder/TEncRateCtrl.h"
#ifdef WIN32
#define strdup _strdup
#endif

#define MACRO_TO_STRING_HELPER(val) #val
#define MACRO_TO_STRING(val) MACRO_TO_STRING_HELPER(val)

using namespace std;
namespace po = df::program_options_lite;



enum ExtendedProfileName // this is used for determining profile strings, where multiple profiles map to a single profile idc with various constraint flag combinations
{
  NONE = 0,
  MAIN = 1,
  MAIN10 = 2,
  MAINSTILLPICTURE = 3,
  MAINREXT = 4,
  HIGHTHROUGHPUTREXT = 5, // Placeholder profile for development
  MAINSCC  = 9, // Placeholder profile for development
  // The following are RExt profiles, which would map to the MAINREXT profile idc.
  // The enumeration indicates the bit-depth constraint in the bottom 2 digits
  //                           the chroma format in the next digit
  //                           the intra constraint in the next digit
  //                           If it is a RExt still picture, there is a '1' for the top digit.
  MONOCHROME_8      = 1008,
  MONOCHROME_12     = 1012,
  MONOCHROME_16     = 1016,
  MAIN_12           = 1112,
  MAIN_422_10       = 1210,
  MAIN_422_12       = 1212,
  MAIN_444          = 1308,
  MAIN_444_10       = 1310,
  MAIN_444_12       = 1312,
  MAIN_444_16       = 1316, // non-standard profile definition, used for development purposes
  MAIN_INTRA        = 2108,
  MAIN_10_INTRA     = 2110,
  MAIN_12_INTRA     = 2112,
  MAIN_422_10_INTRA = 2210,
  MAIN_422_12_INTRA = 2212,
  MAIN_444_INTRA    = 2308,
  MAIN_444_10_INTRA = 2310,
  MAIN_444_12_INTRA = 2312,
  MAIN_444_16_INTRA = 2316,
  MAIN_444_STILL_PICTURE = 11308,
  MAIN_444_16_STILL_PICTURE = 12316,
  // The following are SCC profiles, which would map to the MAINSCC profile idc.
  // The enumeration indicates the bit-depth constraint in the bottom 2 digits
  //                           the chroma format in the next digit
  //                           the intra constraint in the next digit
  //                           If it is a SCC profile there is a '2' for the next digit.
  //                           If it is a highthroughput , there is a '2' for the top digit else '1' for the top digit
  SCC_MAIN                  = 121108,
  SCC_MAIN_10               = 121110,
  SCC_MAIN_444              = 121308,
  SCC_MAIN_444_10           = 121310,
  SCC_HIGHTHROUGHPUT_444    = 221308,
  SCC_HIGHTHROUGHPUT_444_10 = 221310,
  SCC_HIGHTHROUGHPUT_444_14 = 221314
};


//! \ingroup TAppEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncCfg::TAppEncCfg()
: m_inputColourSpaceConvert(IPCOLOURSPACE_UNCHANGED)
, m_snrInternalColourSpace(false)
, m_outputInternalColourSpace(false)
{
  m_aidQP = NULL;
  m_startOfCodedInterval = NULL;
  m_codedPivotValue = NULL;
  m_targetPivotValue = NULL;
}

TAppEncCfg::~TAppEncCfg()
{
  if ( m_aidQP )
  {
    delete[] m_aidQP;
  }
  if ( m_startOfCodedInterval )
  {
    delete[] m_startOfCodedInterval;
    m_startOfCodedInterval = NULL;
  }
   if ( m_codedPivotValue )
  {
    delete[] m_codedPivotValue;
    m_codedPivotValue = NULL;
  }
  if ( m_targetPivotValue )
  {
    delete[] m_targetPivotValue;
    m_targetPivotValue = NULL;
  }
}

Void TAppEncCfg::create()
{
}

Void TAppEncCfg::destroy()
{
}

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry)     //input
{
  in>>entry.m_sliceType;
  in>>entry.m_POC;
  in>>entry.m_QPOffset;
#if X0038_LAMBDA_FROM_QP_CAPABILITY
  in>>entry.m_QPOffsetModelOffset;
  in>>entry.m_QPOffsetModelScale;
#endif
#if W0038_CQP_ADJ
  in>>entry.m_CbQPoffset;
  in>>entry.m_CrQPoffset;
#endif
  in>>entry.m_QPFactor;
  in>>entry.m_tcOffsetDiv2;
  in>>entry.m_betaOffsetDiv2;
  in>>entry.m_temporalId;
  in>>entry.m_numRefPicsActive;
  in>>entry.m_numRefPics;
  for ( Int i = 0; i < entry.m_numRefPics; i++ )
  {
    in>>entry.m_referencePics[i];
  }
  in>>entry.m_interRPSPrediction;
  if (entry.m_interRPSPrediction==1)
  {
    in>>entry.m_deltaRPS;
    in>>entry.m_numRefIdc;
    for ( Int i = 0; i < entry.m_numRefIdc; i++ )
    {
      in>>entry.m_refIdc[i];
    }
  }
  else if (entry.m_interRPSPrediction==2)
  {
    in>>entry.m_deltaRPS;
  }
  return in;
}

Bool confirmPara(Bool bflag, const TChar* message);

static inline ChromaFormat numberToChromaFormat(const Int val)
{
  switch (val)
  {
    case 400: return CHROMA_400; break;
    case 420: return CHROMA_420; break;
    case 422: return CHROMA_422; break;
    case 444: return CHROMA_444; break;
    default:  return NUM_CHROMA_FORMAT;
  }
}

static const struct MapStrToProfile
{
  const TChar* str;
  Profile::Name value;
}
strToProfile[] =
{
  {"none",                 Profile::NONE               },
  {"main",                 Profile::MAIN               },
  {"main10",               Profile::MAIN10             },
  {"main-still-picture",   Profile::MAINSTILLPICTURE   },
  {"main-RExt",            Profile::MAINREXT           },
  {"high-throughput-RExt", Profile::HIGHTHROUGHPUTREXT },
  {"main-SCC",             Profile::MAINSCC            }
};

static const struct MapStrToExtendedProfile
{
  const TChar* str;
  ExtendedProfileName value;
}
strToExtendedProfile[] =
{
    {"none",                      NONE             },
    {"main",                      MAIN             },
    {"main10",                    MAIN10           },
    {"main_still_picture",        MAINSTILLPICTURE },
    {"main-still-picture",        MAINSTILLPICTURE },
    {"main_RExt",                 MAINREXT         },
    {"main-RExt",                 MAINREXT         },
    {"main_rext",                 MAINREXT         },
    {"main-rext",                 MAINREXT         },
    {"main-SCC",                  MAINSCC          },
    {"main_SCC",                  MAINSCC          },
    {"main-scc",                  MAINSCC          },
    {"main_scc",                  MAINSCC          },
    {"high_throughput_RExt",      HIGHTHROUGHPUTREXT },
    {"high-throughput-RExt",      HIGHTHROUGHPUTREXT },
    {"high_throughput_rext",      HIGHTHROUGHPUTREXT },
    {"high-throughput-rext",      HIGHTHROUGHPUTREXT },
    {"monochrome",                MONOCHROME_8     },
    {"monochrome12",              MONOCHROME_12    },
    {"monochrome16",              MONOCHROME_16    },
    {"main12",                    MAIN_12          },
    {"main_422_10",               MAIN_422_10      },
    {"main_422_12",               MAIN_422_12      },
    {"main_444",                  MAIN_444         },
    {"main_444_10",               MAIN_444_10      },
    {"main_444_12",               MAIN_444_12      },
    {"main_444_16",               MAIN_444_16      },
    {"main_intra",                MAIN_INTRA       },
    {"main_10_intra",             MAIN_10_INTRA    },
    {"main_12_intra",             MAIN_12_INTRA    },
    {"main_422_10_intra",         MAIN_422_10_INTRA},
    {"main_422_12_intra",         MAIN_422_12_INTRA},
    {"main_444_intra",            MAIN_444_INTRA   },
    {"main_444_still_picture",    MAIN_444_STILL_PICTURE },
    {"main_444_10_intra",         MAIN_444_10_INTRA},
    {"main_444_12_intra",         MAIN_444_12_INTRA},
    {"main_444_16_intra",         MAIN_444_16_INTRA},
    {"main_444_16_still_picture", MAIN_444_16_STILL_PICTURE },
    {"scc_main",                  SCC_MAIN                  },
    {"scc_main_10",               SCC_MAIN_10               },
    {"scc_main_444",              SCC_MAIN_444              },
    {"scc_main_444_10",           SCC_MAIN_444_10           },
    {"scc_high_throughput_444",   SCC_HIGHTHROUGHPUT_444    },
    {"scc_high_throughput_444_10",SCC_HIGHTHROUGHPUT_444_10 },
    {"scc_high_throughput_444_14",SCC_HIGHTHROUGHPUT_444_14 },
};

static const ExtendedProfileName validRExtProfileNames[2/* intraConstraintFlag*/][4/* bit depth constraint 8=0, 10=1, 12=2, 16=3*/][4/*chroma format*/]=
{
    {
        { MONOCHROME_8,  NONE,          NONE,              MAIN_444          }, // 8-bit  inter for 400, 420, 422 and 444
        { NONE,          NONE,          MAIN_422_10,       MAIN_444_10       }, // 10-bit inter for 400, 420, 422 and 444
        { MONOCHROME_12, MAIN_12,       MAIN_422_12,       MAIN_444_12       }, // 12-bit inter for 400, 420, 422 and 444
        { MONOCHROME_16, NONE,          NONE,              MAIN_444_16       }  // 16-bit inter for 400, 420, 422 and 444 (the latter is non standard used for development)
    },
    {
        { NONE,          MAIN_INTRA,    NONE,              MAIN_444_INTRA    }, // 8-bit  intra for 400, 420, 422 and 444
        { NONE,          MAIN_10_INTRA, MAIN_422_10_INTRA, MAIN_444_10_INTRA }, // 10-bit intra for 400, 420, 422 and 444
        { NONE,          MAIN_12_INTRA, MAIN_422_12_INTRA, MAIN_444_12_INTRA }, // 12-bit intra for 400, 420, 422 and 444
        { NONE,          NONE,          NONE,              MAIN_444_16_INTRA }  // 16-bit intra for 400, 420, 422 and 444
    }
};

static const ExtendedProfileName validSCCProfileNames[2/* high throughput*/][4/* bit depth constraint 8=0, 10=1, 12=2, 14=3*/][4/*chroma format*/]=
{
   {
        { NONE,         SCC_MAIN,      NONE,      SCC_MAIN_444                     }, // 8-bit  intra for 400, 420, 422 and 444
        { NONE,         SCC_MAIN_10,   NONE,      SCC_MAIN_444_10                  }, // 10-bit intra for 400, 420, 422 and 444
        { NONE,         NONE,          NONE,      NONE                             }, // 12-bit intra for 400, 420, 422 and 444
        { NONE,         NONE,          NONE,      NONE                             }  // 16-bit intra for 400, 420, 422 and 444
    },
    {
        { NONE,         NONE,          NONE,       SCC_HIGHTHROUGHPUT_444          }, // 8-bit  inter for 400, 420, 422 and 444
        { NONE,         NONE,          NONE,       SCC_HIGHTHROUGHPUT_444_10       }, // 10-bit inter for 400, 420, 422 and 444
        { NONE,         NONE,          NONE,       NONE                            }, // 12-bit inter for 400, 420, 422 and 444
        { NONE,         NONE,          NONE,       SCC_HIGHTHROUGHPUT_444_14       }  // 16-bit inter for 400, 420, 422 and 444 (the latter is non standard used for development)
    }
};

static const struct MapStrToTier
{
  const TChar* str;
  Level::Tier value;
}
strToTier[] =
{
  {"main", Level::MAIN},
  {"high", Level::HIGH},
};

static const struct MapStrToLevel
{
  const TChar* str;
  Level::Name value;
}
strToLevel[] =
{
  {"none",Level::NONE},
  {"1",   Level::LEVEL1},
  {"2",   Level::LEVEL2},
  {"2.1", Level::LEVEL2_1},
  {"3",   Level::LEVEL3},
  {"3.1", Level::LEVEL3_1},
  {"4",   Level::LEVEL4},
  {"4.1", Level::LEVEL4_1},
  {"5",   Level::LEVEL5},
  {"5.1", Level::LEVEL5_1},
  {"5.2", Level::LEVEL5_2},
  {"6",   Level::LEVEL6},
  {"6.1", Level::LEVEL6_1},
  {"6.2", Level::LEVEL6_2},
  {"8.5", Level::LEVEL8_5},
};

#if U0132_TARGET_BITS_SATURATION
UInt g_uiMaxCpbSize[2][21] =
{
  //         LEVEL1,        LEVEL2,LEVEL2_1,     LEVEL3, LEVEL3_1,      LEVEL4, LEVEL4_1,       LEVEL5,  LEVEL5_1,  LEVEL5_2,    LEVEL6,  LEVEL6_1,  LEVEL6_2 
  { 0, 0, 0, 350000, 0, 0, 1500000, 3000000, 0, 6000000, 10000000, 0, 12000000, 20000000, 0,  25000000,  40000000,  60000000,  60000000, 120000000, 240000000 },
  { 0, 0, 0,      0, 0, 0,       0,       0, 0,       0,        0, 0, 30000000, 50000000, 0, 100000000, 160000000, 240000000, 240000000, 480000000, 800000000 }
};
#endif

static const struct MapStrToCostMode
{
  const TChar* str;
  CostMode    value;
}
strToCostMode[] =
{
  {"lossy",                     COST_STANDARD_LOSSY},
  {"sequence_level_lossless",   COST_SEQUENCE_LEVEL_LOSSLESS},
  {"lossless",                  COST_LOSSLESS_CODING},
  {"mixed_lossless_lossy",      COST_MIXED_LOSSLESS_LOSSY_CODING}
};

static const struct MapStrToScalingListMode
{
  const TChar* str;
  ScalingListMode value;
}
strToScalingListMode[] =
{
  {"0",       SCALING_LIST_OFF},
  {"1",       SCALING_LIST_DEFAULT},
  {"2",       SCALING_LIST_FILE_READ},
  {"off",     SCALING_LIST_OFF},
  {"default", SCALING_LIST_DEFAULT},
  {"file",    SCALING_LIST_FILE_READ}
};

template<typename T, typename P>
static std::string enumToString(P map[], UInt mapLen, const T val)
{
  for (UInt i = 0; i < mapLen; i++)
  {
    if (val == map[i].value)
    {
      return map[i].str;
    }
  }
  return std::string();
}

template<typename T, typename P>
static istream& readStrToEnum(P map[], UInt mapLen, istream &in, T &val)
{
  string str;
  in >> str;

  for (UInt i = 0; i < mapLen; i++)
  {
    if (str == map[i].str)
    {
      val = map[i].value;
      goto found;
    }
  }
  /* not found */
  in.setstate(ios::failbit);
found:
  return in;
}

//inline to prevent compiler warnings for "unused static function"

static inline istream& operator >> (istream &in, ExtendedProfileName &profile)
{
  return readStrToEnum(strToExtendedProfile, sizeof(strToExtendedProfile)/sizeof(*strToExtendedProfile), in, profile);
}

namespace Level
{
  static inline istream& operator >> (istream &in, Tier &tier)
  {
    return readStrToEnum(strToTier, sizeof(strToTier)/sizeof(*strToTier), in, tier);
  }

  static inline istream& operator >> (istream &in, Name &level)
  {
    return readStrToEnum(strToLevel, sizeof(strToLevel)/sizeof(*strToLevel), in, level);
  }
}

static inline istream& operator >> (istream &in, CostMode &mode)
{
  return readStrToEnum(strToCostMode, sizeof(strToCostMode)/sizeof(*strToCostMode), in, mode);
}

static inline istream& operator >> (istream &in, ScalingListMode &mode)
{
  return readStrToEnum(strToScalingListMode, sizeof(strToScalingListMode)/sizeof(*strToScalingListMode), in, mode);
}

template <class T>
struct SMultiValueInput
{
  const T              minValIncl;
  const T              maxValIncl;
  const std::size_t    minNumValuesIncl;
  const std::size_t    maxNumValuesIncl; // Use 0 for unlimited
        std::vector<T> values;
  SMultiValueInput() : minValIncl(0), maxValIncl(0), minNumValuesIncl(0), maxNumValuesIncl(0), values() { }
  SMultiValueInput(std::vector<T> &defaults) : minValIncl(0), maxValIncl(0), minNumValuesIncl(0), maxNumValuesIncl(0), values(defaults) { }
  SMultiValueInput(const T &minValue, const T &maxValue, std::size_t minNumberValues=0, std::size_t maxNumberValues=0)
    : minValIncl(minValue), maxValIncl(maxValue), minNumValuesIncl(minNumberValues), maxNumValuesIncl(maxNumberValues), values()  { }
  SMultiValueInput(const T &minValue, const T &maxValue, std::size_t minNumberValues, std::size_t maxNumberValues, const T* defValues, const UInt numDefValues)
    : minValIncl(minValue), maxValIncl(maxValue), minNumValuesIncl(minNumberValues), maxNumValuesIncl(maxNumberValues), values(defValues, defValues+numDefValues)  { }
  SMultiValueInput<T> &operator=(const std::vector<T> &userValues) { values=userValues; return *this; }
  SMultiValueInput<T> &operator=(const SMultiValueInput<T> &userValues) { values=userValues.values; return *this; }

  T readValue(const TChar *&pStr, Bool &bSuccess);

  istream& readValues(std::istream &in);
};

template <class T>
static inline istream& operator >> (std::istream &in, SMultiValueInput<T> &values)
{
  return values.readValues(in);
}

template<>
UInt SMultiValueInput<UInt>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  UInt val=strtoul(pStr, &eptr, 0);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<minValIncl || val>maxValIncl);
  return val;
}

template<>
Int SMultiValueInput<Int>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  Int val=strtol(pStr, &eptr, 0);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<minValIncl || val>maxValIncl);
  return val;
}

template<>
Double SMultiValueInput<Double>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  Double val=strtod(pStr, &eptr);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<minValIncl || val>maxValIncl);
  return val;
}

template<>
Bool SMultiValueInput<Bool>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  Int val=strtol(pStr, &eptr, 0);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<Int(minValIncl) || val>Int(maxValIncl));
  return val!=0;
}

template <class T>
istream& SMultiValueInput<T>::readValues(std::istream &in)
{
  values.clear();
  string str;
  while (!in.eof())
  {
    string tmp; in >> tmp; str+=" " + tmp;
  }
  if (!str.empty())
  {
    const TChar *pStr=str.c_str();
    // soak up any whitespace
    for(;isspace(*pStr);pStr++);

    while (*pStr != 0)
    {
      Bool bSuccess=true;
      T val=readValue(pStr, bSuccess);
      if (!bSuccess)
      {
        in.setstate(ios::failbit);
        break;
      }

      if (maxNumValuesIncl != 0 && values.size() >= maxNumValuesIncl)
      {
        in.setstate(ios::failbit);
        break;
      }
      values.push_back(val);
      // soak up any whitespace and up to 1 comma.
      for(;isspace(*pStr);pStr++);
      if (*pStr == ',')
      {
        pStr++;
      }
      for(;isspace(*pStr);pStr++);
    }
  }
  if (values.size() < minNumValuesIncl)
  {
    in.setstate(ios::failbit);
  }
  return in;
}

static Void
automaticallySelectRExtProfile(const Bool bUsingGeneralRExtTools,
                               const Bool bUsingChromaQPAdjustment,
                               const Bool bUsingExtendedPrecision,
                               const Bool bIntraConstraintFlag,
                               UInt &bitDepthConstraint,
                               ChromaFormat &chromaFormatConstraint,
                               const Int  maxBitDepth,
                               const ChromaFormat chromaFormat)
{
  // Try to choose profile, according to table in Q1013.
  UInt trialBitDepthConstraint=maxBitDepth;
  if (trialBitDepthConstraint<8)
  {
    trialBitDepthConstraint=8;
  }
  else if (trialBitDepthConstraint==9 || trialBitDepthConstraint==11)
  {
    trialBitDepthConstraint++;
  }
  else if (trialBitDepthConstraint>12)
  {
    trialBitDepthConstraint=16;
  }

  // both format and bit depth constraints are unspecified
  if (bUsingExtendedPrecision || trialBitDepthConstraint==16)
  {
    bitDepthConstraint = 16;
    chromaFormatConstraint = (!bIntraConstraintFlag && chromaFormat==CHROMA_400) ? CHROMA_400 : CHROMA_444;
  }
  else if (bUsingGeneralRExtTools)
  {
    if (chromaFormat == CHROMA_400 && !bIntraConstraintFlag)
    {
      bitDepthConstraint = 16;
      chromaFormatConstraint = CHROMA_400;
    }
    else
    {
      bitDepthConstraint = trialBitDepthConstraint;
      chromaFormatConstraint = CHROMA_444;
    }
  }
  else if (chromaFormat == CHROMA_400)
  {
    if (bIntraConstraintFlag)
    {
      chromaFormatConstraint = CHROMA_420; // there is no intra 4:0:0 profile.
      bitDepthConstraint     = trialBitDepthConstraint;
    }
    else
    {
      chromaFormatConstraint = CHROMA_400;
      bitDepthConstraint     = trialBitDepthConstraint == 8 ? 8 : 12;
    }
  }
  else
  {
    bitDepthConstraint = trialBitDepthConstraint;
    chromaFormatConstraint = chromaFormat;
    if (bUsingChromaQPAdjustment && chromaFormat == CHROMA_420)
    {
      chromaFormatConstraint = CHROMA_422; // 4:2:0 cannot use the chroma qp tool.
    }
    if (chromaFormatConstraint == CHROMA_422 && bitDepthConstraint == 8)
    {
      bitDepthConstraint = 10; // there is no 8-bit 4:2:2 profile.
    }
    if (chromaFormatConstraint == CHROMA_420 && !bIntraConstraintFlag)
    {
      bitDepthConstraint = 12; // there is no 8 or 10-bit 4:2:0 inter RExt profile.
    }
  }
}
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, TChar* argv[] )
{
  Bool do_help = false;

  Int tmpChromaFormat;
  Int tmpInputChromaFormat;
  Int tmpConstraintChromaFormat;
  Int tmpWeightedPredictionMethod;
  Int tmpFastInterSearchMode;
  Int tmpMotionEstimationSearchMethod;
  Int tmpSliceMode;
  Int tmpSliceSegmentMode;
  Int tmpDecodedPictureHashSEIMappedType;
  string inputColourSpaceConvert;
  ExtendedProfileName extendedProfile;
  Int saoOffsetBitShift[MAX_NUM_CHANNEL_TYPE];

  // Multi-value input fields:                                // minval, maxval (incl), min_entries, max_entries (incl) [, default values, number of default values]
  SMultiValueInput<UInt> cfg_ColumnWidth                     (0, std::numeric_limits<UInt>::max(), 0, std::numeric_limits<UInt>::max());
  SMultiValueInput<UInt> cfg_RowHeight                       (0, std::numeric_limits<UInt>::max(), 0, std::numeric_limits<UInt>::max());
  SMultiValueInput<Int>  cfg_startOfCodedInterval            (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, 1<<16);
  SMultiValueInput<Int>  cfg_codedPivotValue                 (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, 1<<16);
  SMultiValueInput<Int>  cfg_targetPivotValue                (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, 1<<16);

  SMultiValueInput<Double> cfg_adIntraLambdaModifier         (0, std::numeric_limits<Double>::max(), 0, MAX_TLAYER); ///< Lambda modifier for Intra pictures, one for each temporal layer. If size>temporalLayer, then use [temporalLayer], else if size>0, use [size()-1], else use m_adLambdaModifier.

#if SHARP_LUMA_DELTA_QP
  const Int defaultLumaLevelTodQp_QpChangePoints[]   =  {-3,  -2,  -1,   0,   1,   2,   3,   4,   5,   6};
  const Int defaultLumaLevelTodQp_LumaChangePoints[] =  { 0, 301, 367, 434, 501, 567, 634, 701, 767, 834};
  SMultiValueInput<Int>  cfg_lumaLeveltoDQPMappingQP         (-MAX_QP, MAX_QP,                    0, LUMA_LEVEL_TO_DQP_LUT_MAXSIZE, defaultLumaLevelTodQp_QpChangePoints,   sizeof(defaultLumaLevelTodQp_QpChangePoints  )/sizeof(Int));
  SMultiValueInput<Int>  cfg_lumaLeveltoDQPMappingLuma       (0, std::numeric_limits<Int>::max(), 0, LUMA_LEVEL_TO_DQP_LUT_MAXSIZE, defaultLumaLevelTodQp_LumaChangePoints, sizeof(defaultLumaLevelTodQp_LumaChangePoints)/sizeof(Int));
  UInt lumaLevelToDeltaQPMode;
#endif

  const UInt defaultInputKneeCodes[3]  = { 600, 800, 900 };
  const UInt defaultOutputKneeCodes[3] = { 100, 250, 450 };
  SMultiValueInput<UInt> cfg_kneeSEIInputKneePointValue      (1,  999, 0, 999, defaultInputKneeCodes,  sizeof(defaultInputKneeCodes )/sizeof(UInt));
  SMultiValueInput<UInt> cfg_kneeSEIOutputKneePointValue     (0, 1000, 0, 999, defaultOutputKneeCodes, sizeof(defaultOutputKneeCodes)/sizeof(UInt));
  const Int defaultPrimaryCodes[6]     = { 0,50000, 0,0, 50000,0 };
  const Int defaultWhitePointCode[2]   = { 16667, 16667 };
  SMultiValueInput<Int>  cfg_DisplayPrimariesCode            (0, 50000, 6, 6, defaultPrimaryCodes,   sizeof(defaultPrimaryCodes  )/sizeof(Int));
  SMultiValueInput<Int>  cfg_DisplayWhitePointCode           (0, 50000, 2, 2, defaultWhitePointCode, sizeof(defaultWhitePointCode)/sizeof(Int));

  SMultiValueInput<Bool> cfg_timeCodeSeiTimeStampFlag        (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiNumUnitFieldBasedFlag(0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiCountingType         (0,  6, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiFullTimeStampFlag    (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiDiscontinuityFlag    (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiCntDroppedFlag       (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiNumberOfFrames       (0,511, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiSecondsValue         (0, 59, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiMinutesValue         (0, 59, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiHoursValue           (0, 23, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiSecondsFlag          (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiMinutesFlag          (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiHoursFlag            (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiTimeOffsetLength     (0, 31, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiTimeOffsetValue      (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, MAX_TIMECODE_SEI_SETS);
  Int warnUnknowParameter = 0;

  po::Options opts;
  opts.addOptions()
  ("help",                                            do_help,                                          false, "this help text")
  ("c",    po::parseConfigFile, "configuration file name")
  ("WarnUnknowParameter,w",                           warnUnknowParameter,                                  0, "warn for unknown configuration parameters instead of failing")

  // File, I/O and source parameters
  ("InputFile,i",                                     m_inputFileName,                             string(""), "Original YUV input file name")
  ("BitstreamFile,b",                                 m_bitstreamFileName,                         string(""), "Bitstream output file name")
  ("ReconFile,o",                                     m_reconFileName,                             string(""), "Reconstructed YUV output file name")
  ("SourceWidth,-wdt",                                m_iSourceWidth,                                       0, "Source picture width")
  ("SourceHeight,-hgt",                               m_iSourceHeight,                                      0, "Source picture height")
  ("InputBitDepth",                                   m_inputBitDepth[CHANNEL_TYPE_LUMA],                   8, "Bit-depth of input file")
  ("OutputBitDepth",                                  m_outputBitDepth[CHANNEL_TYPE_LUMA],                  0, "Bit-depth of output file (default:InternalBitDepth)")
  ("MSBExtendedBitDepth",                             m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA],             0, "bit depth of luma component after addition of MSBs of value 0 (used for synthesising High Dynamic Range source material). (default:InputBitDepth)")
  ("InternalBitDepth",                                m_internalBitDepth[CHANNEL_TYPE_LUMA],                0, "Bit-depth the codec operates at. (default:MSBExtendedBitDepth). If different to MSBExtendedBitDepth, source data will be converted")
  ("InputBitDepthC",                                  m_inputBitDepth[CHANNEL_TYPE_CHROMA],                 0, "As per InputBitDepth but for chroma component. (default:InputBitDepth)")
  ("OutputBitDepthC",                                 m_outputBitDepth[CHANNEL_TYPE_CHROMA],                0, "As per OutputBitDepth but for chroma component. (default:InternalBitDepthC)")
  ("MSBExtendedBitDepthC",                            m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA],           0, "As per MSBExtendedBitDepth but for chroma component. (default:MSBExtendedBitDepth)")
  ("InternalBitDepthC",                               m_internalBitDepth[CHANNEL_TYPE_CHROMA],              0, "As per InternalBitDepth but for chroma component. (default:InternalBitDepth)")
  ("ExtendedPrecision",                               m_extendedPrecisionProcessingFlag,                false, "Increased internal accuracies to support high bit depths (not valid in V1 profiles)")
  ("HighPrecisionPredictionWeighting",                m_highPrecisionOffsetsEnabledFlag,                false, "Use high precision option for weighted prediction (not valid in V1 profiles)")
  ("InputColourSpaceConvert",                         inputColourSpaceConvert,                     string(""), "Colour space conversion to apply to input video. Permitted values are (empty string=UNCHANGED) " + getListOfColourSpaceConverts(true))
  ("SNRInternalColourSpace",                          m_snrInternalColourSpace,                         false, "If true, then no colour space conversion is applied prior to SNR, otherwise inverse of input is applied.")
  ("OutputInternalColourSpace",                       m_outputInternalColourSpace,                      false, "If true, then no colour space conversion is applied for reconstructed video, otherwise inverse of input is applied.")
  ("InputChromaFormat",                               tmpInputChromaFormat,                               420, "InputChromaFormatIDC")
  ("MSEBasedSequencePSNR",                            m_printMSEBasedSequencePSNR,                      false, "0 (default) emit sequence PSNR only as a linear average of the frame PSNRs, 1 = also emit a sequence PSNR based on an average of the frame MSEs")
  ("PrintFrameMSE",                                   m_printFrameMSE,                                  false, "0 (default) emit only bit count and PSNRs for each frame, 1 = also emit MSE values")
  ("PrintSequenceMSE",                                m_printSequenceMSE,                               false, "0 (default) emit only bit rate and PSNRs for the whole sequence, 1 = also emit MSE values")
  ("PrintClippedPSNR",                                m_printClippedPSNR,                               false, "0: (default) print lossless PSNR values as 999.99 dB, 1: clip lossless PSNR according to resolution" )
  ("CabacZeroWordPaddingEnabled",                     m_cabacZeroWordPaddingEnabled,                     true, "0 do not add conforming cabac-zero-words to bit streams, 1 (default) = add cabac-zero-words as required")
  ("ChromaFormatIDC,-cf",                             tmpChromaFormat,                                      0, "ChromaFormatIDC (400|420|422|444 or set 0 (default) for same as InputChromaFormat)")
  ("ConformanceMode",                                 m_conformanceWindowMode,                              0, "Deprecated alias of ConformanceWindowMode")
  ("ConformanceWindowMode",                           m_conformanceWindowMode,                              0, "Window conformance mode (0: no window, 1:automatic padding, 2:padding, 3:conformance")
  ("HorizontalPadding,-pdx",                          m_aiPad[0],                                           0, "Horizontal source padding for conformance window mode 2")
  ("VerticalPadding,-pdy",                            m_aiPad[1],                                           0, "Vertical source padding for conformance window mode 2")
  ("ConfLeft",                                        m_confWinLeft,                                        0, "Deprecated alias of ConfWinLeft")
  ("ConfRight",                                       m_confWinRight,                                       0, "Deprecated alias of ConfWinRight")
  ("ConfTop",                                         m_confWinTop,                                         0, "Deprecated alias of ConfWinTop")
  ("ConfBottom",                                      m_confWinBottom,                                      0, "Deprecated alias of ConfWinBottom")
  ("ConfWinLeft",                                     m_confWinLeft,                                        0, "Left offset for window conformance mode 3")
  ("ConfWinRight",                                    m_confWinRight,                                       0, "Right offset for window conformance mode 3")
  ("ConfWinTop",                                      m_confWinTop,                                         0, "Top offset for window conformance mode 3")
  ("ConfWinBottom",                                   m_confWinBottom,                                      0, "Bottom offset for window conformance mode 3")
  ("AccessUnitDelimiter",                             m_AccessUnitDelimiter,                            false, "Enable Access Unit Delimiter NALUs")
  ("FrameRate,-fr",                                   m_iFrameRate,                                         0, "Frame rate")
  ("FrameSkip,-fs",                                   m_FrameSkip,                                         0u, "Number of frames to skip at start of input YUV")
  ("TemporalSubsampleRatio,-ts",                      m_temporalSubsampleRatio,                            1u, "Temporal sub-sample ratio when reading input YUV")
  ("FramesToBeEncoded,f",                             m_framesToBeEncoded,                                  0, "Number of frames to be encoded (default=all)")
  ("ClipInputVideoToRec709Range",                     m_bClipInputVideoToRec709Range,                   false, "If true then clip input video to the Rec. 709 Range on loading when InternalBitDepth is less than MSBExtendedBitDepth")
  ("ClipOutputVideoToRec709Range",                    m_bClipOutputVideoToRec709Range,                  false, "If true then clip output video to the Rec. 709 Range on saving when OutputBitDepth is less than InternalBitDepth")
  ("SummaryOutFilename",                              m_summaryOutFilename,                          string(), "Filename to use for producing summary output file. If empty, do not produce a file.")
  ("SummaryPicFilenameBase",                          m_summaryPicFilenameBase,                      string(), "Base filename to use for producing summary picture output files. The actual filenames used will have I.txt, P.txt and B.txt appended. If empty, do not produce a file.")
  ("SummaryVerboseness",                              m_summaryVerboseness,                                0u, "Specifies the level of the verboseness of the text output")

  //Field coding parameters
  ("FieldCoding",                                     m_isField,                                        false, "Signals if it's a field based coding")
  ("TopFieldFirst, Tff",                              m_isTopFieldFirst,                                false, "In case of field based coding, signals whether if it's a top field first or not")
  ("EfficientFieldIRAPEnabled",                       m_bEfficientFieldIRAPEnabled,                      true, "Enable to code fields in a specific, potentially more efficient, order.")
  ("HarmonizeGopFirstFieldCoupleEnabled",             m_bHarmonizeGopFirstFieldCoupleEnabled,            true, "Enables harmonization of Gop first field couple")

  // Profile and level
  ("Profile",                                         extendedProfile,                                   NONE, "Profile name to use for encoding. Use main (for main), main10 (for main10), main-still-picture, main-RExt (for Range Extensions profile), any of the RExt specific profile names, or none")
  ("Level",                                           m_level,                                    Level::NONE, "Level limit to be used, eg 5.1, or none")
  ("Tier",                                            m_levelTier,                                Level::MAIN, "Tier to use for interpretation of --Level (main or high only)")
  ("MaxBitDepthConstraint",                           m_bitDepthConstraint,                                0u, "Bit depth to use for profile-constraint for RExt profiles. 0=automatically choose based upon other parameters")
  ("MaxChromaFormatConstraint",                       tmpConstraintChromaFormat,                            0, "Chroma-format to use for the profile-constraint for RExt profiles. 0=automatically choose based upon other parameters")
  ("IntraConstraintFlag",                             m_intraConstraintFlag,                            false, "Value of general_intra_constraint_flag to use for RExt profiles (not used if an explicit RExt sub-profile is specified)")
  ("OnePictureOnlyConstraintFlag",                    m_onePictureOnlyConstraintFlag,                   false, "Value of general_one_picture_only_constraint_flag to use for RExt profiles (not used if an explicit RExt sub-profile is specified)")
  ("LowerBitRateConstraintFlag",                      m_lowerBitRateConstraintFlag,                      true, "Value of general_lower_bit_rate_constraint_flag to use for RExt profiles")
  ("SCCHighThroughputFlag",                           m_sccHighThroughputFlag,                             0u, "High throughput setting for SCC profile is enabled or not")
  ("ProgressiveSource",                               m_progressiveSourceFlag,                          false, "Indicate that source is progressive")
  ("InterlacedSource",                                m_interlacedSourceFlag,                           false, "Indicate that source is interlaced")
  ("NonPackedSource",                                 m_nonPackedConstraintFlag,                        false, "Indicate that source does not contain frame packing")
  ("FrameOnly",                                       m_frameOnlyConstraintFlag,                        false, "Indicate that the bitstream contains only frames")

  // Unit definition parameters
  ("MaxCUWidth",                                      m_uiMaxCUWidth,                                     64u)
  ("MaxCUHeight",                                     m_uiMaxCUHeight,                                    64u)
  // todo: remove defaults from MaxCUSize
  ("MaxCUSize,s",                                     m_uiMaxCUWidth,                                     64u, "Maximum CU size")
  ("MaxCUSize,s",                                     m_uiMaxCUHeight,                                    64u, "Maximum CU size")
  ("MaxPartitionDepth,h",                             m_uiMaxCUDepth,                                      4u, "CU depth")

  ("QuadtreeTULog2MaxSize",                           m_uiQuadtreeTULog2MaxSize,                           6u, "Maximum TU size in logarithm base 2")
  ("QuadtreeTULog2MinSize",                           m_uiQuadtreeTULog2MinSize,                           2u, "Minimum TU size in logarithm base 2")

  ("QuadtreeTUMaxDepthIntra",                         m_uiQuadtreeTUMaxDepthIntra,                         1u, "Depth of TU tree for intra CUs")
  ("QuadtreeTUMaxDepthInter",                         m_uiQuadtreeTUMaxDepthInter,                         2u, "Depth of TU tree for inter CUs")

  // Coding structure paramters
  ("IntraPeriod,-ip",                                 m_iIntraPeriod,                                      -1, "Intra period in frames, (-1: only first frame)")
  ("DecodingRefreshType,-dr",                         m_iDecodingRefreshType,                               0, "Intra refresh type (0:none 1:CRA 2:IDR 3:RecPointSEI)")
  ("GOPSize,g",                                       m_iGOPSize,                                           1, "GOP size of temporal structure")

  // motion search options
  ("DisableIntraInInter",                             m_bDisableIntraPUsInInterSlices,                  false, "Flag to disable intra PUs in inter slices")
  ("FastSearch",                                      tmpMotionEstimationSearchMethod,  Int(MESEARCH_DIAMOND), "0:Full search 1:Diamond 2:Selective 3:Enhanced Diamond")
  ("HashBasedIntraBlockCopySearchEnabled",            m_useHashBasedIntraBlockCopySearch,               false, "Enable the use of hash based search for intra block copying on 8x8 blocks")
  ("IntraBlockCopySearchWidthInCTUs",                 m_intraBlockCopySearchWidthInCTUs,                   -1, "Search range for IBC (-1: full frame search)")
  ("IntraBlockCopyNonHashSearchWidthInCTUs",          m_intraBlockCopyNonHashSearchWidthInCTUs,            1u, "Search range for IBC conventional search method (i.e., fast/full search)")
  ("HashBasedME",                                     m_useHashBasedME,                                 false, "Hash based inter search")
  ("SearchRange,-sr",                                 m_iSearchRange,                                      96, "Motion search range")
  ("BipredSearchRange",                               m_bipredSearchRange,                                  4, "Motion search range for bipred refinement")
  ("MinSearchWindow",                                 m_minSearchWindow,                                    8, "Minimum motion search window size for the adaptive window ME")
  ("RestrictMESampling",                              m_bRestrictMESampling,                            false, "Restrict ME Sampling for selective inter motion search")
  ("ClipForBiPredMEEnabled",                          m_bClipForBiPredMeEnabled,                        false, "Enables clipping in the Bi-Pred ME. It is disabled to reduce encoder run-time")
  ("FastMEAssumingSmootherMVEnabled",                 m_bFastMEAssumingSmootherMVEnabled,                true, "Enables fast ME assuming a smoother MV.")

  ("HadamardME",                                      m_bUseHADME,                                       true, "Hadamard ME for fractional-pel")
  ("ASR",                                             m_bUseASR,                                        false, "Adaptive motion search range");
  opts.addOptions()

  // Mode decision parameters
  ("LambdaModifier0,-LM0",                            m_adLambdaModifier[ 0 ],                  ( Double )1.0, "Lambda modifier for temporal layer 0. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifier1,-LM1",                            m_adLambdaModifier[ 1 ],                  ( Double )1.0, "Lambda modifier for temporal layer 1. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifier2,-LM2",                            m_adLambdaModifier[ 2 ],                  ( Double )1.0, "Lambda modifier for temporal layer 2. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifier3,-LM3",                            m_adLambdaModifier[ 3 ],                  ( Double )1.0, "Lambda modifier for temporal layer 3. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifier4,-LM4",                            m_adLambdaModifier[ 4 ],                  ( Double )1.0, "Lambda modifier for temporal layer 4. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifier5,-LM5",                            m_adLambdaModifier[ 5 ],                  ( Double )1.0, "Lambda modifier for temporal layer 5. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifier6,-LM6",                            m_adLambdaModifier[ 6 ],                  ( Double )1.0, "Lambda modifier for temporal layer 6. If LambdaModifierI is used, this will not affect intra pictures")
  ("LambdaModifierI,-LMI",                            cfg_adIntraLambdaModifier,    cfg_adIntraLambdaModifier, "Lambda modifiers for Intra pictures, comma separated, up to one the number of temporal layer. If entry for temporalLayer exists, then use it, else if some are specified, use the last, else use the standard LambdaModifiers.")
  ("IQPFactor,-IQF",                                  m_dIntraQpFactor,                                  -1.0, "Intra QP Factor for Lambda Computation. If negative, the default will scale lambda based on GOP size (unless LambdaFromQpEnable then IntraQPOffset is used instead)")

  /* Quantization parameters */
  ("QP,q",                                            m_fQP,                                             30.0, "Qp value, if value is float, QP is switched once during encoding")
#if X0038_LAMBDA_FROM_QP_CAPABILITY
  ("IntraQPOffset",                                   m_intraQPOffset,                                      0, "Qp offset value for intra slice, typically determined based on GOP size")
  ("LambdaFromQpEnable",                              m_lambdaFromQPEnable,                             false, "Enable flag for derivation of lambda from QP")
#endif
  ("DeltaQpRD,-dqr",                                  m_uiDeltaQpRD,                                       0u, "max dQp offset for slice")
  ("MaxDeltaQP,d",                                    m_iMaxDeltaQP,                                        0, "max dQp offset for block")
  ("MaxCuDQPDepth,-dqd",                              m_iMaxCuDQPDepth,                                     0, "max depth for a minimum CuDQP")
  ("MaxCUChromaQpAdjustmentDepth",                    m_diffCuChromaQpOffsetDepth,                         -1, "Maximum depth for CU chroma Qp adjustment - set less than 0 to disable")
  ("FastDeltaQP",                                     m_bFastDeltaQP,                                   false, "Fast Delta QP Algorithm")
#if SHARP_LUMA_DELTA_QP
  ("LumaLevelToDeltaQPMode",                          lumaLevelToDeltaQPMode,                              0u, "Luma based Delta QP 0(default): not used. 1: Based on CTU average, 2: Based on Max luma in CTU")
  ("LumaLevelToDeltaQPMaxValWeight",                  m_lumaLevelToDeltaQPMapping.maxMethodWeight,        1.0, "Weight of block max luma val when LumaLevelToDeltaQPMode = 2")
  ("LumaLevelToDeltaQPMappingLuma",                   cfg_lumaLeveltoDQPMappingLuma,  cfg_lumaLeveltoDQPMappingLuma, "Luma to Delta QP Mapping - luma thresholds")
  ("LumaLevelToDeltaQPMappingDQP",                    cfg_lumaLeveltoDQPMappingQP,  cfg_lumaLeveltoDQPMappingQP, "Luma to Delta QP Mapping - DQP values")
#endif

  ("CbQpOffset,-cbqpofs",                             m_cbQpOffset,                                         0, "Chroma Cb QP Offset")
  ("CrQpOffset,-crqpofs",                             m_crQpOffset,                                         0, "Chroma Cr QP Offset")
  ("ActQpYOffset",                                    m_actYQpOffset,                                      -5, "ACT Y QP Offset")
  ("ActQpCbOffset",                                   m_actCbQpOffset,                                     -5, "ACT Cb QP Offset")
  ("ActQpCrOffset",                                   m_actCrQpOffset,                                     -3, "ACT Cr QP Offset")
#if ER_CHROMA_QP_WCG_PPS
  ("WCGPPSEnable",                                    m_wcgChromaQpControl.enabled,                     false, "1: Enable the WCG PPS chroma modulation scheme. 0 (default) disabled")
  ("WCGPPSCbQpScale",                                 m_wcgChromaQpControl.chromaCbQpScale,               1.0, "WCG PPS Chroma Cb QP Scale")
  ("WCGPPSCrQpScale",                                 m_wcgChromaQpControl.chromaCrQpScale,               1.0, "WCG PPS Chroma Cr QP Scale")
  ("WCGPPSChromaQpScale",                             m_wcgChromaQpControl.chromaQpScale,                 0.0, "WCG PPS Chroma QP Scale")
  ("WCGPPSChromaQpOffset",                            m_wcgChromaQpControl.chromaQpOffset,                0.0, "WCG PPS Chroma QP Offset")
#endif
#if W0038_CQP_ADJ
  ("SliceChromaQPOffsetPeriodicity",                  m_sliceChromaQpOffsetPeriodicity,                    0u, "Used in conjunction with Slice Cb/Cr QpOffsetIntraOrPeriodic. Use 0 (default) to disable periodic nature.")
  ("SliceCbQpOffsetIntraOrPeriodic",                  m_sliceChromaQpOffsetIntraOrPeriodic[0],              0, "Chroma Cb QP Offset at slice level for I slice or for periodic inter slices as defined by SliceChromaQPOffsetPeriodicity. Replaces offset in the GOP table.")
  ("SliceCrQpOffsetIntraOrPeriodic",                  m_sliceChromaQpOffsetIntraOrPeriodic[1],              0, "Chroma Cr QP Offset at slice level for I slice or for periodic inter slices as defined by SliceChromaQPOffsetPeriodicity. Replaces offset in the GOP table.")
#endif
#if ADAPTIVE_QP_SELECTION
  ("AdaptiveQpSelection,-aqps",                       m_bUseAdaptQpSelect,                              false, "AdaptiveQpSelection")
#endif

  ("AdaptiveQP,-aq",                                  m_bUseAdaptiveQP,                                 false, "QP adaptation based on a psycho-visual model")
  ("MaxQPAdaptationRange,-aqr",                       m_iQPAdaptationRange,                                 6, "QP adaptation range")
  ("dQPFile,m",                                       m_dQPFileName,                               string(""), "dQP file name")
  ("RDOQ",                                            m_useRDOQ,                                         true)
  ("RDOQTS",                                          m_useRDOQTS,                                       true)
#if T0196_SELECTIVE_RDOQ
  ("SelectiveRDOQ",                                   m_useSelectiveRDOQ,                               false, "Enable selective RDOQ")
#endif
  ("RDpenalty",                                       m_rdPenalty,                                          0,  "RD-penalty for 32x32 TU for intra in non-intra slices. 0:disabled  1:RD-penalty  2:maximum RD-penalty")

  // Deblocking filter parameters
  ("LoopFilterDisable",                               m_bLoopFilterDisable,                             false)
  ("LoopFilterOffsetInPPS",                           m_loopFilterOffsetInPPS,                           true)
  ("LoopFilterBetaOffset_div2",                       m_loopFilterBetaOffsetDiv2,                           0)
  ("LoopFilterTcOffset_div2",                         m_loopFilterTcOffsetDiv2,                             0)
#if W0038_DB_OPT
  ("DeblockingFilterMetric",                          m_deblockingFilterMetric,                             0)
#else
  ("DeblockingFilterMetric",                          m_DeblockingFilterMetric,                         false)
#endif
  // Coding tools
  ("AMP",                                             m_enableAMP,                                       true, "Enable asymmetric motion partitions")
  ("CrossComponentPrediction",                        m_crossComponentPredictionEnabledFlag,            false, "Enable the use of cross-component prediction (not valid in V1 profiles)")
  ("IntraBlockCopyEnabled",                           m_useIntraBlockCopy,                              false, "Enable the use of intra block copying vectors (not valid in V1 profiles)")
  ("IntraBoundaryFilterDisabled",                     m_disableIntraBoundaryFilter,                     false, "Disable the use of intra boundary filters")
  ("IntraBlockCopyFastSearch",                        m_intraBlockCopyFastSearch,                        true, "Use a restricted search range for intra block-copy motion vectors to reduce the encoding time")
  ("ReconBasedCrossCPredictionEstimate",              m_reconBasedCrossCPredictionEstimate,             false, "When determining the alpha value for cross-component prediction, use the decoded residual rather than the pre-transform encoder-side residual")
  ("SaoLumaOffsetBitShift",                           saoOffsetBitShift[CHANNEL_TYPE_LUMA],                 0, "Specify the luma SAO bit-shift. If negative, automatically calculate a suitable value based upon bit depth and initial QP")
  ("SaoChromaOffsetBitShift",                         saoOffsetBitShift[CHANNEL_TYPE_CHROMA],               0, "Specify the chroma SAO bit-shift. If negative, automatically calculate a suitable value based upon bit depth and initial QP")
  ("TransformSkip",                                   m_useTransformSkip,                               false, "Intra transform skipping")
  ("TransformSkipFast",                               m_useTransformSkipFast,                           false, "Fast intra transform skipping")
  ("TransformSkipLog2MaxSize",                        m_log2MaxTransformSkipBlockSize,                     2U, "Specify transform-skip maximum size. Minimum 2. (not valid in V1 profiles)")
  ("ImplicitResidualDPCM",                            m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT],        false, "Enable implicitly signalled residual DPCM for intra (also known as sample-adaptive intra predict) (not valid in V1 profiles)")
  ("ExplicitResidualDPCM",                            m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT],        false, "Enable explicitly signalled residual DPCM for inter (not valid in V1 profiles)")
  ("ResidualRotation",                                m_transformSkipRotationEnabledFlag,               false, "Enable rotation of transform-skipped and transquant-bypassed TUs through 180 degrees prior to entropy coding (not valid in V1 profiles)")
  ("SingleSignificanceMapContext",                    m_transformSkipContextEnabledFlag,                false, "Enable, for transform-skipped and transquant-bypassed TUs, the selection of a single significance map context variable for all coefficients (not valid in V1 profiles)")
  ("GolombRiceParameterAdaptation",                   m_persistentRiceAdaptationEnabledFlag,            false, "Enable the adaptation of the Golomb-Rice parameter over the course of each slice")
  ("AlignCABACBeforeBypass",                          m_cabacBypassAlignmentEnabledFlag,                false, "Align the CABAC engine to a defined fraction of a bit prior to coding bypass data. Must be 1 in high bit rate profile, 0 otherwise" )
  ("SAO",                                             m_bUseSAO,                                         true, "Enable Sample Adaptive Offset")
  ("TestSAODisableAtPictureLevel",                    m_bTestSAODisableAtPictureLevel,                  false, "Enables the testing of disabling SAO at the picture level after having analysed all blocks")
  ("SaoEncodingRate",                                 m_saoEncodingRate,                                 0.75, "When >0 SAO early picture termination is enabled for luma and chroma")
  ("SaoEncodingRateChroma",                           m_saoEncodingRateChroma,                            0.5, "The SAO early picture termination rate to use for chroma (when m_SaoEncodingRate is >0). If <=0, use results for luma")
  ("MaxNumOffsetsPerPic",                             m_maxNumOffsetsPerPic,                             2048, "Max number of SAO offset per picture (Default: 2048)")
  ("SAOLcuBoundary",                                  m_saoCtuBoundary,                                 false, "0: right/bottom CTU boundary areas skipped from SAO parameter estimation, 1: non-deblocked pixels are used for those areas")
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
  ("SAOResetEncoderStateAfterIRAP",                   m_saoResetEncoderStateAfterIRAP,                  false, "When true, resets the encoder's SAO state after an IRAP (POC order). Disabled by default.")
#endif
  ("SliceMode",                                       tmpSliceMode,                            Int(NO_SLICES), "0: Disable all Recon slice limits, 1: Enforce max # of CTUs, 2: Enforce max # of bytes, 3:specify tiles per dependent slice")
  ("SliceArgument",                                   m_sliceArgument,                                      0, "Depending on SliceMode being:"
                                                                                                               "\t1: max number of CTUs per slice"
                                                                                                               "\t2: max number of bytes per slice"
                                                                                                               "\t3: max number of tiles per slice")
  ("SliceSegmentMode",                                tmpSliceSegmentMode,                     Int(NO_SLICES), "0: Disable all slice segment limits, 1: Enforce max # of CTUs, 2: Enforce max # of bytes, 3:specify tiles per dependent slice")
  ("SliceSegmentArgument",                            m_sliceSegmentArgument,                               0, "Depending on SliceSegmentMode being:"
                                                                                                               "\t1: max number of CTUs per slice segment"
                                                                                                               "\t2: max number of bytes per slice segment"
                                                                                                               "\t3: max number of tiles per slice segment")
  ("LFCrossSliceBoundaryFlag",                        m_bLFCrossSliceBoundaryFlag,                       true)

  ("ConstrainedIntraPred",                            m_bUseConstrainedIntraPred,                       false, "Constrained Intra Prediction")
  ("FastUDIUseMPMEnabled",                            m_bFastUDIUseMPMEnabled,                           true, "If enabled, adapt intra direction search, accounting for MPM")
  ("FastMEForGenBLowDelayEnabled",                    m_bFastMEForGenBLowDelayEnabled,                   true, "If enabled use a fast ME for generalised B Low Delay slices")
  ("UseBLambdaForNonKeyLowDelayPictures",             m_bUseBLambdaForNonKeyLowDelayPictures,            true, "Enables use of B-Lambda for non-key low-delay pictures")
  ("PCMEnabledFlag",                                  m_usePCM,                                         false)
  ("PCMLog2MaxSize",                                  m_pcmLog2MaxSize,                                    5u)
  ("PCMLog2MinSize",                                  m_uiPCMLog2MinSize,                                  3u)

  ("PCMInputBitDepthFlag",                            m_bPCMInputBitDepthFlag,                           true)
  ("PCMFilterDisableFlag",                            m_bPCMFilterDisableFlag,                          false)
  ("IntraReferenceSmoothing",                         m_enableIntraReferenceSmoothing,                   true, "0: Disable use of intra reference smoothing (not valid in V1 profiles). 1: Enable use of intra reference smoothing (same as V1)")
  ("WeightedPredP,-wpP",                              m_useWeightedPred,                                false, "Use weighted prediction in P slices")
  ("WeightedPredB,-wpB",                              m_useWeightedBiPred,                              false, "Use weighted (bidirectional) prediction in B slices")
  ("WeightedPredMethod,-wpM",                         tmpWeightedPredictionMethod, Int(WP_PER_PICTURE_WITH_SIMPLE_DC_COMBINED_COMPONENT), "Weighted prediction method")
  ("Log2ParallelMergeLevel",                          m_log2ParallelMergeLevel,                            2u, "Parallel merge estimation region")
    //deprecated copies of renamed tile parameters
  ("UniformSpacingIdc",                               m_tileUniformSpacingFlag,                         false,      "deprecated alias of TileUniformSpacing")
  ("ColumnWidthArray",                                cfg_ColumnWidth,                        cfg_ColumnWidth, "deprecated alias of TileColumnWidthArray")
  ("RowHeightArray",                                  cfg_RowHeight,                            cfg_RowHeight, "deprecated alias of TileRowHeightArray")

  ("TileUniformSpacing",                              m_tileUniformSpacingFlag,                         false,      "Indicates that tile columns and rows are distributed uniformly")
  ("NumTileColumnsMinus1",                            m_numTileColumnsMinus1,                               0,          "Number of tile columns in a picture minus 1")
  ("NumTileRowsMinus1",                               m_numTileRowsMinus1,                                  0,          "Number of rows in a picture minus 1")
  ("TileColumnWidthArray",                            cfg_ColumnWidth,                        cfg_ColumnWidth, "Array containing tile column width values in units of CTU")
  ("TileRowHeightArray",                              cfg_RowHeight,                            cfg_RowHeight, "Array containing tile row height values in units of CTU")
  ("LFCrossTileBoundaryFlag",                         m_bLFCrossTileBoundaryFlag,                        true, "1: cross-tile-boundary loop filtering. 0:non-cross-tile-boundary loop filtering")
  ("WaveFrontSynchro",                                m_entropyCodingSyncEnabledFlag,                   false, "0: entropy coding sync disabled; 1 entropy coding sync enabled")
  ("ScalingList",                                     m_useScalingListId,                    SCALING_LIST_OFF, "0/off: no scaling list, 1/default: default scaling lists, 2/file: scaling lists specified in ScalingListFile")
  ("ScalingListFile",                                 m_scalingListFileName,                       string(""), "Scaling list file name. Use an empty string to produce help.")
  ("SignHideFlag,-SBH",                               m_signDataHidingEnabledFlag,                                    true)
  ("MaxNumMergeCand",                                 m_maxNumMergeCand,                                   5u, "Maximum number of merge candidates")
  /* Misc. */
  ("SEIDecodedPictureHash",                           tmpDecodedPictureHashSEIMappedType,                   0, "Control generation of decode picture hash SEI messages\n"
                                                                                                               "\t3: checksum\n"
                                                                                                               "\t2: CRC\n"
                                                                                                               "\t1: use MD5\n"
                                                                                                               "\t0: disable")
  ("TMVPMode",                                        m_TMVPModeId,                                         1, "TMVP mode 0: TMVP disable for all slices. 1: TMVP enable for all slices (default) 2: TMVP enable for certain slices only")
  ("FEN",                                             tmpFastInterSearchMode,   Int(FASTINTERSEARCH_DISABLED), "fast encoder setting")
  ("ECU",                                             m_bUseEarlyCU,                                    false, "Early CU setting")
  ("FDM",                                             m_useFastDecisionForMerge,                         true, "Fast decision for Merge RD Cost")
  ("CFM",                                             m_bUseCbfFastMode,                                false, "Cbf fast mode setting")
  ("ESD",                                             m_useEarlySkipDetection,                          false, "Early SKIP detection setting")
  ( "RateControl",                                    m_RCEnableRateControl,                            false, "Rate control: enable rate control" )
  ( "TargetBitrate",                                  m_RCTargetBitrate,                                    0, "Rate control: target bit-rate" )
  ( "KeepHierarchicalBit",                            m_RCKeepHierarchicalBit,                              0, "Rate control: 0: equal bit allocation; 1: fixed ratio bit allocation; 2: adaptive ratio bit allocation" )
  ( "LCULevelRateControl",                            m_RCLCULevelRC,                                    true, "Rate control: true: CTU level RC; false: picture level RC" )
  ( "RCLCUSeparateModel",                             m_RCUseLCUSeparateModel,                           true, "Rate control: use CTU level separate R-lambda model" )
  ( "InitialQP",                                      m_RCInitialQP,                                        0, "Rate control: initial QP" )
  ( "RCForceIntraQP",                                 m_RCForceIntraQP,                                 false, "Rate control: force intra QP to be equal to initial QP" )
#if U0132_TARGET_BITS_SATURATION
  ( "RCCpbSaturation",                                m_RCCpbSaturationEnabled,                         false, "Rate control: enable target bits saturation to avoid CPB overflow and underflow" )
  ( "RCCpbSize",                                      m_RCCpbSize,                                         0u, "Rate control: CPB size" )
  ( "RCInitialCpbFullness",                           m_RCInitialCpbFullness,                             0.9, "Rate control: initial CPB fullness" )
#endif

  ("TransquantBypassEnable",                          m_TransquantBypassEnabledFlag,                    false, "transquant_bypass_enabled_flag indicator in PPS")
  ("TransquantBypassEnableFlag",                      m_TransquantBypassEnabledFlag,                    false, "deprecated alias for TransquantBypassEnable")
  ("CUTransquantBypassFlagForce",                     m_CUTransquantBypassFlagForce,                    false, "Force transquant bypass mode, when transquant_bypass_enabled_flag is enabled")
  ("TransquantBypassInferTUSplit",                    m_bTransquantBypassInferTUSplit,                  false, "Infer TU splitting for transquant bypass CUs, when transquant_bypass_enable_flag is enabled")
  ("CUNoSplitIntraACT",                               m_bNoTUSplitIntraACTEnabled,                       true, "Encoder no TU splitting for large CUs in intra mode, when adaptive color transform is enabled")
  ("CostMode",                                        m_costMode,                         COST_STANDARD_LOSSY, "Use alternative cost functions: choose between 'lossy', 'sequence_level_lossless', 'lossless' (which forces QP to " MACRO_TO_STRING(LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP) ") and 'mixed_lossless_lossy' (which used QP'=" MACRO_TO_STRING(LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP_PRIME) " for pre-estimates of transquant-bypass blocks).")
  ("RecalculateQPAccordingToLambda",                  m_recalculateQPAccordingToLambda,                 false, "Recalculate QP values according to lambda values. Do not suggest to be enabled in all intra case")
  ("StrongIntraSmoothing,-sis",                       m_useStrongIntraSmoothing,                         true, "Enable strong intra smoothing for 32x32 blocks")
  ("SEIActiveParameterSets",                          m_activeParameterSetsSEIEnabled,                      0, "Enable generation of active parameter sets SEI messages");
  opts.addOptions()
  ("VuiParametersPresent,-vui",                       m_vuiParametersPresentFlag,                       false, "Enable generation of vui_parameters()")
  ("AspectRatioInfoPresent",                          m_aspectRatioInfoPresentFlag,                     false, "Signals whether aspect_ratio_idc is present")
  ("AspectRatioIdc",                                  m_aspectRatioIdc,                                     0, "aspect_ratio_idc")
  ("SarWidth",                                        m_sarWidth,                                           0, "horizontal size of the sample aspect ratio")
  ("SarHeight",                                       m_sarHeight,                                          0, "vertical size of the sample aspect ratio")
  ("OverscanInfoPresent",                             m_overscanInfoPresentFlag,                        false, "Indicates whether conformant decoded pictures are suitable for display using overscan\n")
  ("OverscanAppropriate",                             m_overscanAppropriateFlag,                        false, "Indicates whether conformant decoded pictures are suitable for display using overscan\n")
  ("VideoSignalTypePresent",                          m_videoSignalTypePresentFlag,                     false, "Signals whether video_format, video_full_range_flag, and colour_description_present_flag are present")
  ("VideoFormat",                                     m_videoFormat,                                        5, "Indicates representation of pictures")
  ("VideoFullRange",                                  m_videoFullRangeFlag,                             false, "Indicates the black level and range of luma and chroma signals")
  ("ColourDescriptionPresent",                        m_colourDescriptionPresentFlag,                   false, "Signals whether colour_primaries, transfer_characteristics and matrix_coefficients are present")
  ("ColourPrimaries",                                 m_colourPrimaries,                                    2, "Indicates chromaticity coordinates of the source primaries")
  ("TransferCharacteristics",                         m_transferCharacteristics,                            2, "Indicates the opto-electronic transfer characteristics of the source")
  ("MatrixCoefficients",                              m_matrixCoefficients,                                 2, "Describes the matrix coefficients used in deriving luma and chroma from RGB primaries")
  ("ChromaLocInfoPresent",                            m_chromaLocInfoPresentFlag,                       false, "Signals whether chroma_sample_loc_type_top_field and chroma_sample_loc_type_bottom_field are present")
  ("ChromaSampleLocTypeTopField",                     m_chromaSampleLocTypeTopField,                        0, "Specifies the location of chroma samples for top field")
  ("ChromaSampleLocTypeBottomField",                  m_chromaSampleLocTypeBottomField,                     0, "Specifies the location of chroma samples for bottom field")
  ("NeutralChromaIndication",                         m_neutralChromaIndicationFlag,                    false, "Indicates that the value of all decoded chroma samples is equal to 1<<(BitDepthCr-1)")
  ("DefaultDisplayWindowFlag",                        m_defaultDisplayWindowFlag,                       false, "Indicates the presence of the Default Window parameters")
  ("DefDispWinLeftOffset",                            m_defDispWinLeftOffset,                               0, "Specifies the left offset of the default display window from the conformance window")
  ("DefDispWinRightOffset",                           m_defDispWinRightOffset,                              0, "Specifies the right offset of the default display window from the conformance window")
  ("DefDispWinTopOffset",                             m_defDispWinTopOffset,                                0, "Specifies the top offset of the default display window from the conformance window")
  ("DefDispWinBottomOffset",                          m_defDispWinBottomOffset,                             0, "Specifies the bottom offset of the default display window from the conformance window")
  ("FrameFieldInfoPresentFlag",                       m_frameFieldInfoPresentFlag,                      false, "Indicates that pic_struct and field coding related values are present in picture timing SEI messages")
  ("PocProportionalToTimingFlag",                     m_pocProportionalToTimingFlag,                    false, "Indicates that the POC value is proportional to the output time w.r.t. first picture in CVS")
  ("NumTicksPocDiffOneMinus1",                        m_numTicksPocDiffOneMinus1,                           0, "Number of ticks minus 1 that for a POC difference of one")
  ("BitstreamRestriction",                            m_bitstreamRestrictionFlag,                       false, "Signals whether bitstream restriction parameters are present")
  ("TilesFixedStructure",                             m_tilesFixedStructureFlag,                        false, "Indicates that each active picture parameter set has the same values of the syntax elements related to tiles")
  ("MotionVectorsOverPicBoundaries",                  m_motionVectorsOverPicBoundariesFlag,             false, "Indicates that no samples outside the picture boundaries are used for inter prediction")
  ("MaxBytesPerPicDenom",                             m_maxBytesPerPicDenom,                                2, "Indicates a number of bytes not exceeded by the sum of the sizes of the VCL NAL units associated with any coded picture")
  ("MaxBitsPerMinCuDenom",                            m_maxBitsPerMinCuDenom,                               1, "Indicates an upper bound for the number of bits of coding_unit() data")
  ("Log2MaxMvLengthHorizontal",                       m_log2MaxMvLengthHorizontal,                         15, "Indicate the maximum absolute value of a decoded horizontal MV component in quarter-pel luma units")
  ("Log2MaxMvLengthVertical",                         m_log2MaxMvLengthVertical,                           15, "Indicate the maximum absolute value of a decoded vertical MV component in quarter-pel luma units");
  opts.addOptions()
  ("SEIColourRemappingInfoFileRoot,-cri",             m_colourRemapSEIFileRoot,                    string(""), "Colour Remapping Information SEI parameters root file name (wo num ext)")
  ("SEIRecoveryPoint",                                m_recoveryPointSEIEnabled,                        false, "Control generation of recovery point SEI messages")
  ("SEIBufferingPeriod",                              m_bufferingPeriodSEIEnabled,                      false, "Control generation of buffering period SEI messages")
  ("SEIPictureTiming",                                m_pictureTimingSEIEnabled,                        false, "Control generation of picture timing SEI messages")
  ("SEIToneMappingInfo",                              m_toneMappingInfoSEIEnabled,                      false, "Control generation of Tone Mapping SEI messages")
  ("SEIToneMapId",                                    m_toneMapId,                                          0, "Specifies Id of Tone Mapping SEI message for a given session")
  ("SEIToneMapCancelFlag",                            m_toneMapCancelFlag,                              false, "Indicates that Tone Mapping SEI message cancels the persistence or follows")
  ("SEIToneMapPersistenceFlag",                       m_toneMapPersistenceFlag,                          true, "Specifies the persistence of the Tone Mapping SEI message")
  ("SEIToneMapCodedDataBitDepth",                     m_toneMapCodedDataBitDepth,                           8, "Specifies Coded Data BitDepth of Tone Mapping SEI messages")
  ("SEIToneMapTargetBitDepth",                        m_toneMapTargetBitDepth,                              8, "Specifies Output BitDepth of Tone mapping function")
  ("SEIToneMapModelId",                               m_toneMapModelId,                                     0, "Specifies Model utilized for mapping coded data into target_bit_depth range\n"
                                                                                                               "\t0:  linear mapping with clipping\n"
                                                                                                               "\t1:  sigmoidal mapping\n"
                                                                                                               "\t2:  user-defined table mapping\n"
                                                                                                               "\t3:  piece-wise linear mapping\n"
                                                                                                               "\t4:  luminance dynamic range information ")
  ("SEIToneMapMinValue",                              m_toneMapMinValue,                                    0, "Specifies the minimum value in mode 0")
  ("SEIToneMapMaxValue",                              m_toneMapMaxValue,                                 1023, "Specifies the maximum value in mode 0")
  ("SEIToneMapSigmoidMidpoint",                       m_sigmoidMidpoint,                                  512, "Specifies the centre point in mode 1")
  ("SEIToneMapSigmoidWidth",                          m_sigmoidWidth,                                     960, "Specifies the distance between 5% and 95% values of the target_bit_depth in mode 1")
  ("SEIToneMapStartOfCodedInterval",                  cfg_startOfCodedInterval,      cfg_startOfCodedInterval, "Array of user-defined mapping table")
  ("SEIToneMapNumPivots",                             m_numPivots,                                          0, "Specifies the number of pivot points in mode 3")
  ("SEIToneMapCodedPivotValue",                       cfg_codedPivotValue,                cfg_codedPivotValue, "Array of pivot point")
  ("SEIToneMapTargetPivotValue",                      cfg_targetPivotValue,              cfg_targetPivotValue, "Array of pivot point")
  ("SEIToneMapCameraIsoSpeedIdc",                     m_cameraIsoSpeedIdc,                                  0, "Indicates the camera ISO speed for daylight illumination")
  ("SEIToneMapCameraIsoSpeedValue",                   m_cameraIsoSpeedValue,                              400, "Specifies the camera ISO speed for daylight illumination of Extended_ISO")
  ("SEIToneMapExposureIndexIdc",                      m_exposureIndexIdc,                                   0, "Indicates the exposure index setting of the camera")
  ("SEIToneMapExposureIndexValue",                    m_exposureIndexValue,                               400, "Specifies the exposure index setting of the camera of Extended_ISO")
  ("SEIToneMapExposureCompensationValueSignFlag",     m_exposureCompensationValueSignFlag,               false, "Specifies the sign of ExposureCompensationValue")
  ("SEIToneMapExposureCompensationValueNumerator",    m_exposureCompensationValueNumerator,                 0, "Specifies the numerator of ExposureCompensationValue")
  ("SEIToneMapExposureCompensationValueDenomIdc",     m_exposureCompensationValueDenomIdc,                  2, "Specifies the denominator of ExposureCompensationValue")
  ("SEIToneMapRefScreenLuminanceWhite",               m_refScreenLuminanceWhite,                          350, "Specifies reference screen brightness setting in units of candela per square metre")
  ("SEIToneMapExtendedRangeWhiteLevel",               m_extendedRangeWhiteLevel,                          800, "Indicates the luminance dynamic range")
  ("SEIToneMapNominalBlackLevelLumaCodeValue",        m_nominalBlackLevelLumaCodeValue,                    16, "Specifies luma sample value of the nominal black level assigned decoded pictures")
  ("SEIToneMapNominalWhiteLevelLumaCodeValue",        m_nominalWhiteLevelLumaCodeValue,                   235, "Specifies luma sample value of the nominal white level assigned decoded pictures")
  ("SEIToneMapExtendedWhiteLevelLumaCodeValue",       m_extendedWhiteLevelLumaCodeValue,                  300, "Specifies luma sample value of the extended dynamic range assigned decoded pictures")
  ("SEIChromaResamplingFilterHint",                   m_chromaResamplingFilterSEIenabled,               false, "Control generation of the chroma sampling filter hint SEI message")
  ("SEIChromaResamplingHorizontalFilterType",         m_chromaResamplingHorFilterIdc,                       2, "Defines the Index of the chroma sampling horizontal filter\n"
                                                                                                               "\t0: unspecified  - Chroma filter is unknown or is determined by the application"
                                                                                                               "\t1: User-defined - Filter coefficients are specified in the chroma sampling filter hint SEI message"
                                                                                                               "\t2: Standards-defined - ITU-T Rec. T.800 | ISO/IEC15444-1, 5/3 filter")
  ("SEIChromaResamplingVerticalFilterType",           m_chromaResamplingVerFilterIdc,                         2, "Defines the Index of the chroma sampling vertical filter\n"
                                                                                                               "\t0: unspecified  - Chroma filter is unknown or is determined by the application"
                                                                                                               "\t1: User-defined - Filter coefficients are specified in the chroma sampling filter hint SEI message"
                                                                                                               "\t2: Standards-defined - ITU-T Rec. T.800 | ISO/IEC15444-1, 5/3 filter")
  ("SEIFramePacking",                                 m_framePackingSEIEnabled,                         false, "Control generation of frame packing SEI messages")
  ("SEIFramePackingType",                             m_framePackingSEIType,                                0, "Define frame packing arrangement\n"
                                                                                                               "\t3: side by side - frames are displayed horizontally\n"
                                                                                                               "\t4: top bottom - frames are displayed vertically\n"
                                                                                                               "\t5: frame alternation - one frame is alternated with the other")
  ("SEIFramePackingId",                               m_framePackingSEIId,                                  0, "Id of frame packing SEI message for a given session")
  ("SEIFramePackingQuincunx",                         m_framePackingSEIQuincunx,                            0, "Indicate the presence of a Quincunx type video frame")
  ("SEIFramePackingInterpretation",                   m_framePackingSEIInterpretation,                      0, "Indicate the interpretation of the frame pair\n"
                                                                                                               "\t0: unspecified\n"
                                                                                                               "\t1: stereo pair, frame0 represents left view\n"
                                                                                                               "\t2: stereo pair, frame0 represents right view")
  ("SEISegmentedRectFramePacking",                    m_segmentedRectFramePackingSEIEnabled,            false, "Controls generation of segmented rectangular frame packing SEI messages")
  ("SEISegmentedRectFramePackingCancel",              m_segmentedRectFramePackingSEICancel,             false, "If equal to 1, cancels the persistence of any previous SRFPA SEI message")
  ("SEISegmentedRectFramePackingType",                m_segmentedRectFramePackingSEIType,                   0, "Specifies the arrangement of the frames in the reconstructed picture")
  ("SEISegmentedRectFramePackingPersistence",         m_segmentedRectFramePackingSEIPersistence,        false, "If equal to 0, the SEI applies to the current frame only")
  ("SEIDisplayOrientation",                           m_displayOrientationSEIAngle,                         0, "Control generation of display orientation SEI messages\n"
                                                                                                               "\tN: 0 < N < (2^16 - 1) enable display orientation SEI message with anticlockwise_rotation = N and display_orientation_repetition_period = 1\n"
                                                                                                               "\t0: disable")
  ("SEITemporalLevel0Index",                          m_temporalLevel0IndexSEIEnabled,                  false, "Control generation of temporal level 0 index SEI messages")
  ("SEIGradualDecodingRefreshInfo",                   m_gradualDecodingRefreshInfoEnabled,              false, "Control generation of gradual decoding refresh information SEI message")
  ("SEINoDisplay",                                    m_noDisplaySEITLayer,                                 0, "Control generation of no display SEI message\n"
                                                                                                               "\tN: 0 < N enable no display SEI message for temporal layer N or higher\n"
                                                                                                               "\t0: disable")
  ("SEIDecodingUnitInfo",                             m_decodingUnitInfoSEIEnabled,                     false, "Control generation of decoding unit information SEI message.")
  ("SEISOPDescription",                               m_SOPDescriptionSEIEnabled,                       false, "Control generation of SOP description SEI messages")
  ("SEIScalableNesting",                              m_scalableNestingSEIEnabled,                      false, "Control generation of scalable nesting SEI messages")
  ("SEITempMotionConstrainedTileSets",                m_tmctsSEIEnabled,                                false, "Control generation of temporal motion constrained tile sets SEI message")
  ("SEITimeCodeEnabled",                              m_timeCodeSEIEnabled,                             false, "Control generation of time code information SEI message")
  ("SEITimeCodeNumClockTs",                           m_timeCodeSEINumTs,                                   0, "Number of clock time sets [0..3]")
  ("SEITimeCodeTimeStampFlag",                        cfg_timeCodeSeiTimeStampFlag,          cfg_timeCodeSeiTimeStampFlag,         "Time stamp flag associated to each time set")
  ("SEITimeCodeFieldBasedFlag",                       cfg_timeCodeSeiNumUnitFieldBasedFlag,  cfg_timeCodeSeiNumUnitFieldBasedFlag, "Field based flag associated to each time set")
  ("SEITimeCodeCountingType",                         cfg_timeCodeSeiCountingType,           cfg_timeCodeSeiCountingType,          "Counting type associated to each time set")
  ("SEITimeCodeFullTsFlag",                           cfg_timeCodeSeiFullTimeStampFlag,      cfg_timeCodeSeiFullTimeStampFlag,     "Full time stamp flag associated to each time set")
  ("SEITimeCodeDiscontinuityFlag",                    cfg_timeCodeSeiDiscontinuityFlag,      cfg_timeCodeSeiDiscontinuityFlag,     "Discontinuity flag associated to each time set")
  ("SEITimeCodeCntDroppedFlag",                       cfg_timeCodeSeiCntDroppedFlag,         cfg_timeCodeSeiCntDroppedFlag,        "Counter dropped flag associated to each time set")
  ("SEITimeCodeNumFrames",                            cfg_timeCodeSeiNumberOfFrames,         cfg_timeCodeSeiNumberOfFrames,        "Number of frames associated to each time set")
  ("SEITimeCodeSecondsValue",                         cfg_timeCodeSeiSecondsValue,           cfg_timeCodeSeiSecondsValue,          "Seconds value for each time set")
  ("SEITimeCodeMinutesValue",                         cfg_timeCodeSeiMinutesValue,           cfg_timeCodeSeiMinutesValue,          "Minutes value for each time set")
  ("SEITimeCodeHoursValue",                           cfg_timeCodeSeiHoursValue,             cfg_timeCodeSeiHoursValue,            "Hours value for each time set")
  ("SEITimeCodeSecondsFlag",                          cfg_timeCodeSeiSecondsFlag,            cfg_timeCodeSeiSecondsFlag,           "Flag to signal seconds value presence in each time set")
  ("SEITimeCodeMinutesFlag",                          cfg_timeCodeSeiMinutesFlag,            cfg_timeCodeSeiMinutesFlag,           "Flag to signal minutes value presence in each time set")
  ("SEITimeCodeHoursFlag",                            cfg_timeCodeSeiHoursFlag,              cfg_timeCodeSeiHoursFlag,             "Flag to signal hours value presence in each time set")
  ("SEITimeCodeOffsetLength",                         cfg_timeCodeSeiTimeOffsetLength,       cfg_timeCodeSeiTimeOffsetLength,      "Time offset length associated to each time set")
  ("SEITimeCodeTimeOffset",                           cfg_timeCodeSeiTimeOffsetValue,        cfg_timeCodeSeiTimeOffsetValue,       "Time offset associated to each time set")
  ("SEIKneeFunctionInfo",                             m_kneeSEIEnabled,                                 false, "Control generation of Knee function SEI messages")
  ("SEIKneeFunctionId",                               m_kneeSEIId,                                          0, "Specifies Id of Knee function SEI message for a given session")
  ("SEIKneeFunctionCancelFlag",                       m_kneeSEICancelFlag,                              false, "Indicates that Knee function SEI message cancels the persistence or follows")
  ("SEIKneeFunctionPersistenceFlag",                  m_kneeSEIPersistenceFlag,                          true, "Specifies the persistence of the Knee function SEI message")
  ("SEIKneeFunctionInputDrange",                      m_kneeSEIInputDrange,                              1000, "Specifies the peak luminance level for the input picture of Knee function SEI messages")
  ("SEIKneeFunctionInputDispLuminance",               m_kneeSEIInputDispLuminance,                        100, "Specifies the expected display brightness for the input picture of Knee function SEI messages")
  ("SEIKneeFunctionOutputDrange",                     m_kneeSEIOutputDrange,                             4000, "Specifies the peak luminance level for the output picture of Knee function SEI messages")
  ("SEIKneeFunctionOutputDispLuminance",              m_kneeSEIOutputDispLuminance,                       800, "Specifies the expected display brightness for the output picture of Knee function SEI messages")
  ("SEIKneeFunctionNumKneePointsMinus1",              m_kneeSEINumKneePointsMinus1,                         2, "Specifies the number of knee points - 1")
  ("SEIKneeFunctionInputKneePointValue",              cfg_kneeSEIInputKneePointValue,   cfg_kneeSEIInputKneePointValue, "Array of input knee point")
  ("SEIKneeFunctionOutputKneePointValue",             cfg_kneeSEIOutputKneePointValue, cfg_kneeSEIOutputKneePointValue, "Array of output knee point")
  ("SEIMasteringDisplayColourVolume",                 m_masteringDisplay.colourVolumeSEIEnabled,         false, "Control generation of mastering display colour volume SEI messages")
  ("SEIMasteringDisplayMaxLuminance",                 m_masteringDisplay.maxLuminance,                  10000u, "Specifies the mastering display maximum luminance value in units of 1/10000 candela per square metre (32-bit code value)")
  ("SEIMasteringDisplayMinLuminance",                 m_masteringDisplay.minLuminance,                      0u, "Specifies the mastering display minimum luminance value in units of 1/10000 candela per square metre (32-bit code value)")
  ("SEIMasteringDisplayPrimaries",                    cfg_DisplayPrimariesCode,       cfg_DisplayPrimariesCode, "Mastering display primaries for all three colour planes in CIE xy coordinates in increments of 1/50000 (results in the ranges 0 to 50000 inclusive)")
  ("SEIMasteringDisplayWhitePoint",                   cfg_DisplayWhitePointCode,     cfg_DisplayWhitePointCode, "Mastering display white point CIE xy coordinates in normalised increments of 1/50000 (e.g. 0.333 = 16667)")
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  ("SEIPreferredTransferCharacterisics",              m_preferredTransferCharacteristics,                   -1, "Value for the preferred_transfer_characteristics field of the Alternative transfer characteristics SEI which will override the corresponding entry in the VUI. If negative, do not produce the respective SEI message")
#endif
  ("SEIGreenMetadataType",                            m_greenMetadataType,                   0u, "Value for the green_metadata_type specifies the type of metadata that is present in the SEI message. If green_metadata_type is 1, then metadata enabling quality recovery after low-power encoding is present")
  ("SEIXSDMetricType",                                m_xsdMetricType,                      0u, "Value for the xsd_metric_type indicates the type of the objective quality metric. PSNR is the only type currently supported")
  ("ColourTransform",                                 m_useColourTrans,                                   false, "Enable the colour transform (not valid in V1 profiles")
  ("PaletteMode",                                     m_usePaletteMode,                                  false, "Enable the palette mode (not valid in V1 profiles")
  ("PaletteMaxSize",                                  m_uiPaletteMaxSize,                                  63u,  "Maximum palette size")
  ("PaletteMaxPredSize",                              m_uiPaletteMaxPredSize,                             128u,  "Maximum palette predictor size")
  ("MotionVectorResolutionControlIdc",                m_motionVectorResolutionControlIdc,                    0, "0 (default): use 1/4-pel mv; 1: use integer-pel mv; 2: adaptive mv resolution (not valid in V1 profiles)")
  ("PalettePredInSPSEnabled",                         m_palettePredInSPSEnabled,                          false, "Transmit palette predictor in SPS")
  ("PalettePredInPPSEnabled",                         m_palettePredInPPSEnabled,                          false, "Transmit palette predictor in PPS")
  ;

  for(Int i=1; i<MAX_GOP+1; i++)
  {
    std::ostringstream cOSS;
    cOSS<<"Frame"<<i;
    opts.addOptions()(cOSS.str(), m_GOPList[i-1], GOPEntry());
  }
  po::setDefaults(opts);
  po::ErrorReporter err;
  const list<const TChar*>& argv_unhandled = po::scanArgv(opts, argc, (const TChar**) argv, err);

  for (list<const TChar*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++)
  {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }

  if (argc == 1 || do_help)
  {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    return false;
  }

  if (err.is_errored)
  {
    if (!warnUnknowParameter)
    {
      /* error report has already been printed on stderr */
      return false;
    }
  }

  /*
   * Set any derived parameters
   */

  m_framesToBeEncoded = ( m_framesToBeEncoded + m_temporalSubsampleRatio - 1 ) / m_temporalSubsampleRatio;
  m_adIntraLambdaModifier = cfg_adIntraLambdaModifier.values;
  if(m_isField)
  {
    //Frame height
    m_iSourceHeightOrg = m_iSourceHeight;
    //Field height
    m_iSourceHeight = m_iSourceHeight >> 1;
    //number of fields to encode
    m_framesToBeEncoded *= 2;
  }

  if( !m_tileUniformSpacingFlag && m_numTileColumnsMinus1 > 0 )
  {
    if (cfg_ColumnWidth.values.size() > m_numTileColumnsMinus1)
    {
      printf( "The number of columns whose width are defined is larger than the allowed number of columns.\n" );
      exit( EXIT_FAILURE );
    }
    else if (cfg_ColumnWidth.values.size() < m_numTileColumnsMinus1)
    {
      printf( "The width of some columns is not defined.\n" );
      exit( EXIT_FAILURE );
    }
    else
    {
      m_tileColumnWidth.resize(m_numTileColumnsMinus1);
      for(UInt i=0; i<cfg_ColumnWidth.values.size(); i++)
      {
        m_tileColumnWidth[i]=cfg_ColumnWidth.values[i];
      }
    }
  }
  else
  {
    m_tileColumnWidth.clear();
  }

  if( !m_tileUniformSpacingFlag && m_numTileRowsMinus1 > 0 )
  {
    if (cfg_RowHeight.values.size() > m_numTileRowsMinus1)
    {
      printf( "The number of rows whose height are defined is larger than the allowed number of rows.\n" );
      exit( EXIT_FAILURE );
    }
    else if (cfg_RowHeight.values.size() < m_numTileRowsMinus1)
    {
      printf( "The height of some rows is not defined.\n" );
      exit( EXIT_FAILURE );
    }
    else
    {
      m_tileRowHeight.resize(m_numTileRowsMinus1);
      for(UInt i=0; i<cfg_RowHeight.values.size(); i++)
      {
        m_tileRowHeight[i]=cfg_RowHeight.values[i];
      }
    }
  }
  else
  {
    m_tileRowHeight.clear();
  }

  /* rules for input, output and internal bitdepths as per help text */
  if (m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] = m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
  }
  if (m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] == 0)
  {
    m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] = m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
  }
  if (m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] = m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
  }
  if (m_internalBitDepth   [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_internalBitDepth   [CHANNEL_TYPE_CHROMA] = m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_inputBitDepth      [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_inputBitDepth      [CHANNEL_TYPE_CHROMA] = m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
  }
  if (m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] = m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_outputBitDepth     [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_outputBitDepth     [CHANNEL_TYPE_CHROMA] = m_internalBitDepth   [CHANNEL_TYPE_CHROMA];
  }

  m_InputChromaFormatIDC = numberToChromaFormat(tmpInputChromaFormat);
  m_chromaFormatIDC      = ((tmpChromaFormat == 0) ? (m_InputChromaFormatIDC) : (numberToChromaFormat(tmpChromaFormat)));

  assert(tmpWeightedPredictionMethod>=0 && tmpWeightedPredictionMethod<=WP_PER_PICTURE_WITH_HISTOGRAM_AND_PER_COMPONENT_AND_CLIPPING_AND_EXTENSION);
  if (!(tmpWeightedPredictionMethod>=0 && tmpWeightedPredictionMethod<=WP_PER_PICTURE_WITH_HISTOGRAM_AND_PER_COMPONENT_AND_CLIPPING_AND_EXTENSION))
  {
    exit(EXIT_FAILURE);
  }
  m_weightedPredictionMethod = WeightedPredictionMethod(tmpWeightedPredictionMethod);

  assert(tmpFastInterSearchMode>=0 && tmpFastInterSearchMode<=FASTINTERSEARCH_MODE3);
  if (tmpFastInterSearchMode<0 || tmpFastInterSearchMode>FASTINTERSEARCH_MODE3)
  {
    exit(EXIT_FAILURE);
  }
  m_fastInterSearchMode = FastInterSearchMode(tmpFastInterSearchMode);

  assert(tmpMotionEstimationSearchMethod>=0 && tmpMotionEstimationSearchMethod<MESEARCH_NUMBER_OF_METHODS);
  if (tmpMotionEstimationSearchMethod<0 || tmpMotionEstimationSearchMethod>=MESEARCH_NUMBER_OF_METHODS)
  {
    exit(EXIT_FAILURE);
  }
  m_motionEstimationSearchMethod=MESearchMethod(tmpMotionEstimationSearchMethod);

  if (extendedProfile >= 1000 && extendedProfile <= 12316)
  {
    m_profile = Profile::MAINREXT;
    if (m_bitDepthConstraint != 0 || tmpConstraintChromaFormat != 0)
    {
      fprintf(stderr, "Error: The bit depth and chroma format constraints are not used when an explicit RExt profile is specified\n");
      exit(EXIT_FAILURE);
    }
    m_bitDepthConstraint           = (extendedProfile%100);
    m_intraConstraintFlag          = ((extendedProfile%10000)>=2000);
    m_onePictureOnlyConstraintFlag = (extendedProfile >= 10000);
    switch ((extendedProfile/100)%10)
    {
      case 0:  tmpConstraintChromaFormat=400; break;
      case 1:  tmpConstraintChromaFormat=420; break;
      case 2:  tmpConstraintChromaFormat=422; break;
      default: tmpConstraintChromaFormat=444; break;
    }
  }
  else if (extendedProfile >= 121108 && extendedProfile <= 221314)
  {
    m_profile = Profile::MAINSCC;
    if (m_bitDepthConstraint != 0 || tmpConstraintChromaFormat != 0)
    {
      fprintf(stderr, "Error: The bit depth and chroma format constraints are not used when an explicit SCC profile is specified\n");
      exit(EXIT_FAILURE);
    }
    m_bitDepthConstraint           = (extendedProfile%100);
    m_intraConstraintFlag          = ((extendedProfile%10000)>=2000);
    m_sccHighThroughputFlag        = (extendedProfile > 121308);
    assert(m_intraConstraintFlag==0);
    m_onePictureOnlyConstraintFlag = 0;

    switch ((extendedProfile/100)%10)
    {
    case 0:  tmpConstraintChromaFormat=400; break;
    case 1:  tmpConstraintChromaFormat=420; break;
    case 2:  tmpConstraintChromaFormat=422; break;
    default: tmpConstraintChromaFormat=444; break;
    }
  }
  else
  {
    m_profile = Profile::Name(extendedProfile);
  }

  if (m_profile == Profile::HIGHTHROUGHPUTREXT )
  {
    if (m_bitDepthConstraint == 0)
    {
      m_bitDepthConstraint = 16;
    }
    m_chromaFormatConstraint = (tmpConstraintChromaFormat == 0) ? CHROMA_444 : numberToChromaFormat(tmpConstraintChromaFormat);
  }
  else if (m_profile == Profile::MAINREXT || m_profile == Profile::MAINSCC) // KR: needs checking
  {
    if (m_bitDepthConstraint == 0 && tmpConstraintChromaFormat == 0)
    {
      // produce a valid combination, if possible.
      const Bool bUsingGeneralRExtTools  = m_transformSkipRotationEnabledFlag        ||
                                           m_transformSkipContextEnabledFlag         ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ||
                                           !m_enableIntraReferenceSmoothing          ||
                                           m_persistentRiceAdaptationEnabledFlag     ||
                                           m_log2MaxTransformSkipBlockSize!=2;
      const Bool bUsingChromaQPAdjustment= m_diffCuChromaQpOffsetDepth >= 0;
      const Bool bUsingExtendedPrecision = m_extendedPrecisionProcessingFlag;
      if (m_onePictureOnlyConstraintFlag)
      {
        m_chromaFormatConstraint = CHROMA_444;
        if (m_intraConstraintFlag != true)
        {
          fprintf(stderr, "Error: Intra constraint flag must be true when one_picture_only_constraint_flag is true\n");
          exit(EXIT_FAILURE);
        }
        const Int maxBitDepth = m_chromaFormatIDC==CHROMA_400 ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
        m_bitDepthConstraint = maxBitDepth>8 ? 16:8;
      }
      else
      {
        m_chromaFormatConstraint = NUM_CHROMA_FORMAT;
        automaticallySelectRExtProfile(bUsingGeneralRExtTools,
                                       bUsingChromaQPAdjustment,
                                       bUsingExtendedPrecision,
                                       m_intraConstraintFlag,
                                       m_bitDepthConstraint,
                                       m_chromaFormatConstraint,
                                       m_chromaFormatIDC==CHROMA_400 ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]),
                                       m_chromaFormatIDC);
      }
    }
    else if (m_bitDepthConstraint == 0 || tmpConstraintChromaFormat == 0)
    {
      fprintf(stderr, "Error: The bit depth and chroma format constraints must either both be specified or both be configured automatically\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      m_chromaFormatConstraint = numberToChromaFormat(tmpConstraintChromaFormat);
    }
  }
  else
  {
    m_chromaFormatConstraint = (tmpConstraintChromaFormat == 0) ? m_chromaFormatIDC : numberToChromaFormat(tmpConstraintChromaFormat);
    m_bitDepthConstraint = (m_internalBitDepth[CHANNEL_TYPE_LUMA] > m_internalBitDepth[CHANNEL_TYPE_CHROMA])? m_internalBitDepth[CHANNEL_TYPE_LUMA]: m_internalBitDepth[CHANNEL_TYPE_CHROMA];
  }


  m_inputColourSpaceConvert = stringToInputColourSpaceConvert(inputColourSpaceConvert, true);
  m_bRGBformat    = (m_inputColourSpaceConvert == IPCOLOURSPACE_RGBtoGBR && m_chromaFormatIDC == CHROMA_444)? true: false;
  m_useLL         = m_costMode == COST_LOSSLESS_CODING ? true: false;

  switch (m_conformanceWindowMode)
  {
  case 0:
    {
      // no conformance or padding
      m_confWinLeft = m_confWinRight = m_confWinTop = m_confWinBottom = 0;
      m_aiPad[1] = m_aiPad[0] = 0;
      break;
    }
  case 1:
    {
      // automatic padding to minimum CU size
      Int minCuSize = m_uiMaxCUHeight >> (m_uiMaxCUDepth - 1);
      if (m_iSourceWidth % minCuSize)
      {
        m_aiPad[0] = m_confWinRight  = ((m_iSourceWidth / minCuSize) + 1) * minCuSize - m_iSourceWidth;
        m_iSourceWidth  += m_confWinRight;
      }
      if (m_iSourceHeight % minCuSize)
      {
        m_aiPad[1] = m_confWinBottom = ((m_iSourceHeight / minCuSize) + 1) * minCuSize - m_iSourceHeight;
        m_iSourceHeight += m_confWinBottom;
        if ( m_isField )
        {
          m_iSourceHeightOrg += m_confWinBottom << 1;
          m_aiPad[1] = m_confWinBottom << 1;
        }
      }
      if (m_aiPad[0] % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0)
      {
        fprintf(stderr, "Error: picture width is not an integer multiple of the specified chroma subsampling\n");
        exit(EXIT_FAILURE);
      }
      if (m_aiPad[1] % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0)
      {
        fprintf(stderr, "Error: picture height is not an integer multiple of the specified chroma subsampling\n");
        exit(EXIT_FAILURE);
      }
      break;
    }
  case 2:
    {
      //padding
      m_iSourceWidth  += m_aiPad[0];
      m_iSourceHeight += m_aiPad[1];
      m_confWinRight  = m_aiPad[0];
      m_confWinBottom = m_aiPad[1];
      break;
    }
  case 3:
    {
      // conformance
      if ((m_confWinLeft == 0) && (m_confWinRight == 0) && (m_confWinTop == 0) && (m_confWinBottom == 0))
      {
        fprintf(stderr, "Warning: Conformance window enabled, but all conformance window parameters set to zero\n");
      }
      if ((m_aiPad[1] != 0) || (m_aiPad[0]!=0))
      {
        fprintf(stderr, "Warning: Conformance window enabled, padding parameters will be ignored\n");
      }
      m_aiPad[1] = m_aiPad[0] = 0;
      break;
    }
  }

  if (tmpSliceMode<0 || tmpSliceMode>=Int(NUMBER_OF_SLICE_CONSTRAINT_MODES))
  {
    fprintf(stderr, "Error: bad slice mode\n");
    exit(EXIT_FAILURE);
  }
  m_sliceMode = SliceConstraint(tmpSliceMode);
  if (tmpSliceSegmentMode<0 || tmpSliceSegmentMode>=Int(NUMBER_OF_SLICE_CONSTRAINT_MODES))
  {
    fprintf(stderr, "Error: bad slice segment mode\n");
    exit(EXIT_FAILURE);
  }
  m_sliceSegmentMode = SliceConstraint(tmpSliceSegmentMode);

  if (tmpDecodedPictureHashSEIMappedType<0 || tmpDecodedPictureHashSEIMappedType>=Int(NUMBER_OF_HASHTYPES))
  {
    fprintf(stderr, "Error: bad checksum mode\n");
    exit(EXIT_FAILURE);
  }
  // Need to map values to match those of the SEI message:
  if (tmpDecodedPictureHashSEIMappedType==0)
  {
    m_decodedPictureHashSEIType=HASHTYPE_NONE;
  }
  else
  {
    m_decodedPictureHashSEIType=HashType(tmpDecodedPictureHashSEIMappedType-1);
  }

  // allocate slice-based dQP values
  m_aidQP = new Int[ m_framesToBeEncoded + m_iGOPSize + 1 ];
  ::memset( m_aidQP, 0, sizeof(Int)*( m_framesToBeEncoded + m_iGOPSize + 1 ) );

  // handling of floating-point QP values
  // if QP is not integer, sequence is split into two sections having QP and QP+1
  m_iQP = (Int)( m_fQP );
  if ( m_iQP < m_fQP )
  {
    Int iSwitchPOC = (Int)( m_framesToBeEncoded - (m_fQP - m_iQP)*m_framesToBeEncoded + 0.5 );

    iSwitchPOC = (Int)( (Double)iSwitchPOC / m_iGOPSize + 0.5 )*m_iGOPSize;
    for ( Int i=iSwitchPOC; i<m_framesToBeEncoded + m_iGOPSize + 1; i++ )
    {
      m_aidQP[i] = 1;
    }
  }

  for(UInt ch=0; ch<MAX_NUM_CHANNEL_TYPE; ch++)
  {
    if (saoOffsetBitShift[ch]<0)
    {
      if (m_internalBitDepth[ch]>10)
      {
        m_log2SaoOffsetScale[ch]=UInt(Clip3<Int>(0, m_internalBitDepth[ch]-10, Int(m_internalBitDepth[ch]-10 + 0.165*m_iQP - 3.22 + 0.5) ) );
      }
      else
      {
        m_log2SaoOffsetScale[ch]=0;
      }
    }
    else
    {
      m_log2SaoOffsetScale[ch]=UInt(saoOffsetBitShift[ch]);
    }
  }

#if SHARP_LUMA_DELTA_QP
  assert(lumaLevelToDeltaQPMode<LUMALVL_TO_DQP_NUM_MODES);
  if (lumaLevelToDeltaQPMode>=LUMALVL_TO_DQP_NUM_MODES)
  {
    exit(EXIT_FAILURE);
  }
  m_lumaLevelToDeltaQPMapping.mode=LumaLevelToDQPMode(lumaLevelToDeltaQPMode);

  if (m_lumaLevelToDeltaQPMapping.mode)
  {
    assert(  cfg_lumaLeveltoDQPMappingLuma.values.size() == cfg_lumaLeveltoDQPMappingQP.values.size() );
    m_lumaLevelToDeltaQPMapping.mapping.resize(cfg_lumaLeveltoDQPMappingLuma.values.size());
    for(UInt i=0; i<cfg_lumaLeveltoDQPMappingLuma.values.size(); i++)
    {
      m_lumaLevelToDeltaQPMapping.mapping[i]=std::pair<Int,Int>(cfg_lumaLeveltoDQPMappingLuma.values[i], cfg_lumaLeveltoDQPMappingQP.values[i]);
    }
  }
#endif

  // reading external dQP description from file
  if ( !m_dQPFileName.empty() )
  {
    FILE* fpt=fopen( m_dQPFileName.c_str(), "r" );
    if ( fpt )
    {
      Int iValue;
      Int iPOC = 0;
      while ( iPOC < m_framesToBeEncoded )
      {
        if ( fscanf(fpt, "%d", &iValue ) == EOF )
        {
          break;
        }
        m_aidQP[ iPOC ] = iValue;
        iPOC++;
      }
      fclose(fpt);
    }
  }

  if( m_masteringDisplay.colourVolumeSEIEnabled )
  {
    for(UInt idx=0; idx<6; idx++)
    {
      m_masteringDisplay.primaries[idx/2][idx%2] = UShort((cfg_DisplayPrimariesCode.values.size() > idx) ? cfg_DisplayPrimariesCode.values[idx] : 0);
    }
    for(UInt idx=0; idx<2; idx++)
    {
      m_masteringDisplay.whitePoint[idx] = UShort((cfg_DisplayWhitePointCode.values.size() > idx) ? cfg_DisplayWhitePointCode.values[idx] : 0);
    }
  }
    
  if( m_toneMappingInfoSEIEnabled && !m_toneMapCancelFlag )
  {
    if( m_toneMapModelId == 2 && !cfg_startOfCodedInterval.values.empty() )
    {
      const UInt num = 1u<< m_toneMapTargetBitDepth;
      m_startOfCodedInterval = new Int[num];
      for(UInt i=0; i<num; i++)
      {
        m_startOfCodedInterval[i] = cfg_startOfCodedInterval.values.size() > i ? cfg_startOfCodedInterval.values[i] : 0;
      }
    }
    else
    {
      m_startOfCodedInterval = NULL;
    }
    if( ( m_toneMapModelId == 3 ) && ( m_numPivots > 0 ) )
    {
      if( !cfg_codedPivotValue.values.empty() && !cfg_targetPivotValue.values.empty() )
      {
        m_codedPivotValue  = new Int[m_numPivots];
        m_targetPivotValue = new Int[m_numPivots];
        for(UInt i=0; i<m_numPivots; i++)
        {
          m_codedPivotValue[i]  = cfg_codedPivotValue.values.size()  > i ? cfg_codedPivotValue.values [i] : 0;
          m_targetPivotValue[i] = cfg_targetPivotValue.values.size() > i ? cfg_targetPivotValue.values[i] : 0;
        }
      }
    }
    else
    {
      m_codedPivotValue = NULL;
      m_targetPivotValue = NULL;
    }
  }

  if( m_kneeSEIEnabled && !m_kneeSEICancelFlag )
  {
    assert ( m_kneeSEINumKneePointsMinus1 >= 0 && m_kneeSEINumKneePointsMinus1 < 999 );
    m_kneeSEIInputKneePoint  = new Int[m_kneeSEINumKneePointsMinus1+1];
    m_kneeSEIOutputKneePoint = new Int[m_kneeSEINumKneePointsMinus1+1];
    for(Int i=0; i<(m_kneeSEINumKneePointsMinus1+1); i++)
    {
      m_kneeSEIInputKneePoint[i]  = cfg_kneeSEIInputKneePointValue.values.size()  > i ? cfg_kneeSEIInputKneePointValue.values[i]  : 1;
      m_kneeSEIOutputKneePoint[i] = cfg_kneeSEIOutputKneePointValue.values.size() > i ? cfg_kneeSEIOutputKneePointValue.values[i] : 0;
    }
  }

  if(m_timeCodeSEIEnabled)
  {
    for(Int i = 0; i < m_timeCodeSEINumTs && i < MAX_TIMECODE_SEI_SETS; i++)
    {
      m_timeSetArray[i].clockTimeStampFlag    = cfg_timeCodeSeiTimeStampFlag        .values.size()>i ? cfg_timeCodeSeiTimeStampFlag        .values [i] : false;
      m_timeSetArray[i].numUnitFieldBasedFlag = cfg_timeCodeSeiNumUnitFieldBasedFlag.values.size()>i ? cfg_timeCodeSeiNumUnitFieldBasedFlag.values [i] : 0;
      m_timeSetArray[i].countingType          = cfg_timeCodeSeiCountingType         .values.size()>i ? cfg_timeCodeSeiCountingType         .values [i] : 0;
      m_timeSetArray[i].fullTimeStampFlag     = cfg_timeCodeSeiFullTimeStampFlag    .values.size()>i ? cfg_timeCodeSeiFullTimeStampFlag    .values [i] : 0;
      m_timeSetArray[i].discontinuityFlag     = cfg_timeCodeSeiDiscontinuityFlag    .values.size()>i ? cfg_timeCodeSeiDiscontinuityFlag    .values [i] : 0;
      m_timeSetArray[i].cntDroppedFlag        = cfg_timeCodeSeiCntDroppedFlag       .values.size()>i ? cfg_timeCodeSeiCntDroppedFlag       .values [i] : 0;
      m_timeSetArray[i].numberOfFrames        = cfg_timeCodeSeiNumberOfFrames       .values.size()>i ? cfg_timeCodeSeiNumberOfFrames       .values [i] : 0;
      m_timeSetArray[i].secondsValue          = cfg_timeCodeSeiSecondsValue         .values.size()>i ? cfg_timeCodeSeiSecondsValue         .values [i] : 0;
      m_timeSetArray[i].minutesValue          = cfg_timeCodeSeiMinutesValue         .values.size()>i ? cfg_timeCodeSeiMinutesValue         .values [i] : 0;
      m_timeSetArray[i].hoursValue            = cfg_timeCodeSeiHoursValue           .values.size()>i ? cfg_timeCodeSeiHoursValue           .values [i] : 0;
      m_timeSetArray[i].secondsFlag           = cfg_timeCodeSeiSecondsFlag          .values.size()>i ? cfg_timeCodeSeiSecondsFlag          .values [i] : 0;
      m_timeSetArray[i].minutesFlag           = cfg_timeCodeSeiMinutesFlag          .values.size()>i ? cfg_timeCodeSeiMinutesFlag          .values [i] : 0;
      m_timeSetArray[i].hoursFlag             = cfg_timeCodeSeiHoursFlag            .values.size()>i ? cfg_timeCodeSeiHoursFlag            .values [i] : 0;
      m_timeSetArray[i].timeOffsetLength      = cfg_timeCodeSeiTimeOffsetLength     .values.size()>i ? cfg_timeCodeSeiTimeOffsetLength     .values [i] : 0;
      m_timeSetArray[i].timeOffsetValue       = cfg_timeCodeSeiTimeOffsetValue      .values.size()>i ? cfg_timeCodeSeiTimeOffsetValue      .values [i] : 0;
    }
  }

  // check validity of input parameters
  xCheckParameter();

  // compute actual CU depth with respect to config depth and max transform size
  UInt uiAddCUDepth  = 0;
  while( (m_uiMaxCUWidth>>m_uiMaxCUDepth) > ( 1 << ( m_uiQuadtreeTULog2MinSize + uiAddCUDepth )  ) )
  {
    uiAddCUDepth++;
  }

  m_uiMaxTotalCUDepth = m_uiMaxCUDepth + uiAddCUDepth + getMaxCUDepthOffset(m_chromaFormatIDC, m_uiQuadtreeTULog2MinSize); // if minimum TU larger than 4x4, allow for additional part indices for 4:2:2 SubTUs.
  m_uiLog2DiffMaxMinCodingBlockSize = m_uiMaxCUDepth - 1;

  // print-out parameters
  xPrintParameter();

  return true;
}


// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Void TAppEncCfg::xCheckParameter()
{
  if (m_decodedPictureHashSEIType==HASHTYPE_NONE)
  {
    fprintf(stderr, "******************************************************************\n");
    fprintf(stderr, "** WARNING: --SEIDecodedPictureHash is now disabled by default. **\n");
    fprintf(stderr, "**          Automatic verification of decoded pictures by a     **\n");
    fprintf(stderr, "**          decoder requires this option to be enabled.         **\n");
    fprintf(stderr, "******************************************************************\n");
  }
  if( m_profile==Profile::NONE )
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: For conforming bitstreams a valid Profile value must be set! **\n");
    fprintf(stderr, "***************************************************************************\n");
  }
  if( m_level==Level::NONE )
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: For conforming bitstreams a valid Level value must be set!   **\n");
    fprintf(stderr, "***************************************************************************\n");
  }

  Bool check_failed = false; /* abort if there is a fatal configuration problem */
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)

  xConfirmPara(m_bitstreamFileName.empty(), "A bitstream file name must be specified (BitstreamFile)");
  const UInt maxBitDepth=(m_chromaFormatIDC==CHROMA_400) ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
  xConfirmPara(m_bitDepthConstraint<maxBitDepth, "The internalBitDepth must not be greater than the bitDepthConstraint value");
  xConfirmPara(m_chromaFormatConstraint<m_chromaFormatIDC, "The chroma format used must not be greater than the chromaFormatConstraint value");

  if (m_profile==Profile::MAINREXT || m_profile==Profile::HIGHTHROUGHPUTREXT || m_profile==Profile::MAINSCC)
  {
    xConfirmPara(m_lowerBitRateConstraintFlag==false && m_intraConstraintFlag==false, "The lowerBitRateConstraint flag cannot be false when intraConstraintFlag is false");
    xConfirmPara(m_cabacBypassAlignmentEnabledFlag && m_profile!=Profile::HIGHTHROUGHPUTREXT, "AlignCABACBeforeBypass must not be enabled unless the high throughput profile is being used.");
    xConfirmPara(m_useIntraBlockCopy      && m_profile!=Profile::MAINSCC,  "UseIntraBlockCopy must not be enabled unless the main-SCC profile is being used.");
    if (m_profile == Profile::MAINREXT)
    {
      const UInt intraIdx = m_intraConstraintFlag ? 1:0;
      const UInt bitDepthIdx = (m_bitDepthConstraint == 8 ? 0 : (m_bitDepthConstraint ==10 ? 1 : (m_bitDepthConstraint == 12 ? 2 : (m_bitDepthConstraint == 16 ? 3 : 4 ))));
      const UInt chromaFormatIdx = UInt(m_chromaFormatConstraint);
      const Bool bValidProfile = (bitDepthIdx > 3 || chromaFormatIdx>3) ? false : (validRExtProfileNames[intraIdx][bitDepthIdx][chromaFormatIdx] != NONE);
      xConfirmPara(!bValidProfile, "Invalid intra constraint flag, bit depth constraint flag and chroma format constraint flag combination for a RExt profile");
      const Bool bUsingGeneralRExtTools  = m_transformSkipRotationEnabledFlag        ||
                                           m_transformSkipContextEnabledFlag         ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ||
                                           !m_enableIntraReferenceSmoothing          ||
                                           m_persistentRiceAdaptationEnabledFlag     ||
                                           m_log2MaxTransformSkipBlockSize!=2;
      const Bool bUsingChromaQPTool      = m_diffCuChromaQpOffsetDepth >= 0;
      const Bool bUsingExtendedPrecision = m_extendedPrecisionProcessingFlag;

      xConfirmPara((m_chromaFormatConstraint==CHROMA_420 || m_chromaFormatConstraint==CHROMA_400) && bUsingChromaQPTool, "CU Chroma QP adjustment cannot be used for 4:0:0 or 4:2:0 RExt profiles");
      xConfirmPara(m_bitDepthConstraint != 16 && bUsingExtendedPrecision, "Extended precision can only be used in 16-bit RExt profiles");
      if (!(m_chromaFormatConstraint == CHROMA_400 && m_bitDepthConstraint == 16) && m_chromaFormatConstraint!=CHROMA_444)
      {
        xConfirmPara(bUsingGeneralRExtTools, "Combination of tools and profiles are not possible in the specified RExt profile.");
      }
      xConfirmPara( m_onePictureOnlyConstraintFlag && m_chromaFormatConstraint!=CHROMA_444, "chroma format constraint must be 4:4:4 when one-picture-only constraint flag is 1");
      xConfirmPara( m_onePictureOnlyConstraintFlag && m_bitDepthConstraint != 8 && m_bitDepthConstraint != 16, "bit depth constraint must be 8 or 16 when one-picture-only constraint flag is 1");
      xConfirmPara( m_onePictureOnlyConstraintFlag && m_framesToBeEncoded > 1, "Number of frames to be encoded must be 1 when one-picture-only constraint flag is 1.");

      if (!m_intraConstraintFlag && m_bitDepthConstraint==16 && m_chromaFormatConstraint==CHROMA_444)
      {
        fprintf(stderr, "********************************************************************************************************\n");
        fprintf(stderr, "** WARNING: The RExt constraint flags describe a non standard combination (used for development only) **\n");
        fprintf(stderr, "********************************************************************************************************\n");
      }
    }
    else if (m_profile == Profile::HIGHTHROUGHPUTREXT)
    {
      xConfirmPara( m_chromaFormatConstraint != CHROMA_444, "chroma format constraint must be 4:4:4 in the High Throughput 4:4:4 16-bit Intra profile.");
      xConfirmPara( m_bitDepthConstraint     != 16,         "bit depth constraint must be 4:4:4 in the High Throughput 4:4:4 16-bit Intra profile.");
      xConfirmPara( m_intraConstraintFlag    != 1,          "intra constraint flag must be 1 in the High Throughput 4:4:4 16-bit Intra profile.");
    }
    else if (m_profile == Profile::MAINSCC)
    {
       xConfirmPara( m_intraConstraintFlag , "intra constraint flag must be 0 for SCC profiles");
       xConfirmPara( m_onePictureOnlyConstraintFlag , "one-picture-only constraint flag shall be 0 for SCC profiles");       

      const UInt sccHighThroughputFlag = m_sccHighThroughputFlag ? 1:0;
      const UInt bitDepthIdx = (m_bitDepthConstraint == 8 ? 0 : (m_bitDepthConstraint ==10 ? 1 : (m_bitDepthConstraint == 12 ? 2 : (m_bitDepthConstraint == 16 ? 3 : 4 ))));
      const UInt chromaFormatIdx = UInt(m_chromaFormatConstraint);
      const Bool bValidProfile = (bitDepthIdx > 2 || chromaFormatIdx>3) ? false : (validSCCProfileNames[sccHighThroughputFlag][bitDepthIdx][chromaFormatIdx] != NONE);
      xConfirmPara(!bValidProfile, "Invalid intra constraint flag, bit depth constraint flag and chroma format constraint flag combination for a RExt profile");

      const Bool bUsingChromaQPTool      = m_diffCuChromaQpOffsetDepth >= 0;
      const Bool bUsingExtendedPrecision = m_extendedPrecisionProcessingFlag;

      xConfirmPara((m_chromaFormatConstraint==CHROMA_420 || m_chromaFormatConstraint==CHROMA_400) && bUsingChromaQPTool, "CU Chroma QP adjustment cannot be used for 4:0:0 or 4:2:0 RExt profiles");
      xConfirmPara( bUsingExtendedPrecision, "Extended precision cannot be used for SCC profile");
    }
  }
  else
  {
    xConfirmPara(m_bitDepthConstraint!=((m_profile==Profile::MAIN10)?10:8), "BitDepthConstraint must be 8 for MAIN profile and 10 for MAIN10 profile.");
    xConfirmPara(m_chromaFormatConstraint!=CHROMA_420, "ChromaFormatConstraint must be 420 for non main-RExt profiles.");
    xConfirmPara(m_intraConstraintFlag==true, "IntraConstraintFlag must be false for non main_RExt profiles.");
    xConfirmPara(m_lowerBitRateConstraintFlag==false, "LowerBitrateConstraintFlag must be true for non main-RExt profiles.");
    xConfirmPara(m_profile == Profile::MAINSTILLPICTURE && m_framesToBeEncoded > 1, "Number of frames to be encoded must be 1 when main still picture profile is used.");

    xConfirmPara(m_crossComponentPredictionEnabledFlag==true, "CrossComponentPrediction must not be used for non main-RExt profiles.");
    xConfirmPara(m_log2MaxTransformSkipBlockSize!=2, "Transform Skip Log2 Max Size must be 2 for V1 profiles.");
    xConfirmPara(m_transformSkipRotationEnabledFlag==true, "UseResidualRotation must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_transformSkipContextEnabledFlag==true, "UseSingleSignificanceMapContext must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT]==true, "ImplicitResidualDPCM must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT]==true, "ExplicitResidualDPCM must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_persistentRiceAdaptationEnabledFlag==true, "GolombRiceParameterAdaption must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_extendedPrecisionProcessingFlag==true, "UseExtendedPrecision must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_highPrecisionOffsetsEnabledFlag==true, "UseHighPrecisionPredictionWeighting must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_useIntraBlockCopy==true, "UseIntraBlockCopy must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_enableIntraReferenceSmoothing==false, "EnableIntraReferenceSmoothing must be enabled for non main-RExt profiles.");
    xConfirmPara(m_cabacBypassAlignmentEnabledFlag, "AlignCABACBeforeBypass cannot be enabled for non main-RExt profiles.");
  }

  // check range of parameters
  xConfirmPara( m_inputBitDepth[CHANNEL_TYPE_LUMA  ] < 8,                                   "InputBitDepth must be at least 8" );
  xConfirmPara( m_inputBitDepth[CHANNEL_TYPE_CHROMA] < 8,                                   "InputBitDepthC must be at least 8" );

#if !RExt__HIGH_BIT_DEPTH_SUPPORT
  if (m_extendedPrecisionProcessingFlag)
  {
    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      xConfirmPara((m_internalBitDepth[channelType] > 8) , "Model is not configured to support high enough internal accuracies - enable RExt__HIGH_BIT_DEPTH_SUPPORT to use increased precision internal data types etc...");
    }
  }
  else
  {
    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      xConfirmPara((m_internalBitDepth[channelType] > 12) , "Model is not configured to support high enough internal accuracies - enable RExt__HIGH_BIT_DEPTH_SUPPORT to use increased precision internal data types etc...");
    }
  }
#endif

  xConfirmPara( (m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] < m_inputBitDepth[CHANNEL_TYPE_LUMA  ]), "MSB-extended bit depth for luma channel (--MSBExtendedBitDepth) must be greater than or equal to input bit depth for luma channel (--InputBitDepth)" );
  xConfirmPara( (m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] < m_inputBitDepth[CHANNEL_TYPE_CHROMA]), "MSB-extended bit depth for chroma channel (--MSBExtendedBitDepthC) must be greater than or equal to input bit depth for chroma channel (--InputBitDepthC)" );

  xConfirmPara( m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA]   > (m_internalBitDepth[CHANNEL_TYPE_LUMA  ]<10?0:(m_internalBitDepth[CHANNEL_TYPE_LUMA  ]-10)), "SaoLumaOffsetBitShift must be in the range of 0 to InternalBitDepth-10, inclusive");
  xConfirmPara( m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA] > (m_internalBitDepth[CHANNEL_TYPE_CHROMA]<10?0:(m_internalBitDepth[CHANNEL_TYPE_CHROMA]-10)), "SaoChromaOffsetBitShift must be in the range of 0 to InternalBitDepthC-10, inclusive");

  xConfirmPara( m_chromaFormatIDC >= NUM_CHROMA_FORMAT,                                     "ChromaFormatIDC must be either 400, 420, 422 or 444" );
  std::string sTempIPCSC="InputColourSpaceConvert must be empty, "+getListOfColourSpaceConverts(true);
  xConfirmPara( m_inputColourSpaceConvert >= NUMBER_INPUT_COLOUR_SPACE_CONVERSIONS,         sTempIPCSC.c_str() );
  xConfirmPara( m_InputChromaFormatIDC >= NUM_CHROMA_FORMAT,                                "InputChromaFormatIDC must be either 400, 420, 422 or 444" );
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_temporalSubsampleRatio < 1,                                               "Temporal subsample rate must be no less than 1" );
  xConfirmPara( m_framesToBeEncoded <= 0,                                                   "Total Number Of Frames encoded must be more than 0" );
  xConfirmPara( m_iGOPSize < 1 ,                                                            "GOP Size must be greater or equal to 1" );
  xConfirmPara( m_iGOPSize > 1 &&  m_iGOPSize % 2,                                          "GOP Size must be a multiple of 2, if GOP Size is greater than 1" );
  xConfirmPara( (m_iIntraPeriod > 0 && m_iIntraPeriod < m_iGOPSize) || m_iIntraPeriod == 0, "Intra period must be more than GOP size, or -1 , not 0" );
  xConfirmPara( m_iDecodingRefreshType < 0 || m_iDecodingRefreshType > 3,                   "Decoding Refresh Type must be comprised between 0 and 3 included" );
  if(m_iDecodingRefreshType == 3)
  {
    xConfirmPara( !m_recoveryPointSEIEnabled,                                               "When using RecoveryPointSEI messages as RA points, recoveryPointSEI must be enabled" );
  }

  if (m_isField)
  {
    if (!m_pictureTimingSEIEnabled)
    {
      fprintf(stderr, "****************************************************************************\n");
      fprintf(stderr, "** WARNING: Picture Timing SEI should be enabled for field coding!        **\n");
      fprintf(stderr, "****************************************************************************\n");
    }
  }

  if(m_crossComponentPredictionEnabledFlag && (m_chromaFormatIDC != CHROMA_444))
  {
    fprintf(stderr, "****************************************************************************\n");
    fprintf(stderr, "** WARNING: Cross-component prediction is specified for 4:4:4 format only **\n");
    fprintf(stderr, "****************************************************************************\n");

    m_crossComponentPredictionEnabledFlag = false;
  }

  if(m_useColourTrans && (m_chromaFormatIDC != CHROMA_444))
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: Adaptive Colour transform is specified for 4:4:4 format only **\n");
    fprintf(stderr, "***************************************************************************\n");

    m_useColourTrans = false;
  }
  if ( m_useColourTrans && m_profile != Profile::MAINSCC )
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: Adaptive Colour transform can be used in SCC profile only    **\n");
    fprintf(stderr, "***************************************************************************\n");

    m_useColourTrans = false;
  }
  if(m_useColourTrans && m_TransquantBypassEnabledFlag && m_CUTransquantBypassFlagForce && m_internalBitDepth[CHANNEL_TYPE_LUMA] != m_internalBitDepth[CHANNEL_TYPE_CHROMA])
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: Adaptive Colour transform is not used for lossless coding    **\n");
    fprintf(stderr, "**          with different luma and chroma bit depth                     **\n");
    fprintf(stderr, "***************************************************************************\n");

    m_useColourTrans = false;
  }

  if (m_usePaletteMode && !(m_chromaFormatIDC == CHROMA_444 || m_chromaFormatIDC == CHROMA_422 || m_chromaFormatIDC == CHROMA_420))
  {
    fprintf(stderr, "*******************************************************************************\n");
    fprintf(stderr, "** WARNING: Palette mode is supported for 4:4:4, 4:2:2 and 4:2:0 format only **\n");
    fprintf(stderr, "*******************************************************************************\n");

    m_usePaletteMode = false;
  }
  if (m_usePaletteMode && m_profile != Profile::MAINSCC)
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: Palette mode can be used in SCC profile only    **\n");
    fprintf(stderr, "***************************************************************************\n");

    m_usePaletteMode = false;
  }


  if ( m_CUTransquantBypassFlagForce && m_bUseHADME )
  {
    fprintf(stderr, "****************************************************************************\n");
    fprintf(stderr, "** WARNING: --HadamardME has been disabled due to the enabling of         **\n");
    fprintf(stderr, "**          --CUTransquantBypassFlagForce                                 **\n");
    fprintf(stderr, "****************************************************************************\n");

    m_bUseHADME = false; // this has been disabled so that the lambda is calculated slightly differently for lossless modes (as a result of JCTVC-R0104).
  }

  xConfirmPara (m_log2MaxTransformSkipBlockSize < 2, "Transform Skip Log2 Max Size must be at least 2 (4x4)");

  if ( !m_TransquantBypassEnabledFlag && m_bTransquantBypassInferTUSplit )
  {
    fprintf(stderr, "****************************************************************************\n");
    fprintf(stderr, "** WARNING: --TransquantbypassInferTUSplit has been disabled              **\n");
    fprintf(stderr, "**  due to the disabling of --TransquantBypassEnableFlag                  **\n");
    fprintf(stderr, "****************************************************************************\n");

    m_bTransquantBypassInferTUSplit = false;
  }

  if ( !m_useColourTrans && m_bNoTUSplitIntraACTEnabled )
  {
    fprintf(stderr, "****************************************************************************\n");
    fprintf(stderr, "** WARNING: --CUNoSplitIntraACT has been disabled                         **\n");
    fprintf(stderr, "**  due to the disabling of --ColourTransform                             **\n");
    fprintf(stderr, "****************************************************************************\n");

    m_bNoTUSplitIntraACTEnabled = false;
  }

  if (m_log2MaxTransformSkipBlockSize!=2 && m_useTransformSkipFast)
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: Transform skip fast is enabled (which only tests NxN splits),**\n");
    fprintf(stderr, "**          but transform skip log2 max size is not 2 (4x4)              **\n");
    fprintf(stderr, "**          It may be better to disable transform skip fast mode         **\n");
    fprintf(stderr, "***************************************************************************\n");
  }

  xConfirmPara( m_iQP <  -6 * (m_internalBitDepth[CHANNEL_TYPE_LUMA] - 8) || m_iQP > 51,    "QP exceeds supported range (-QpBDOffsety to 51)" );
#if W0038_DB_OPT
  xConfirmPara( m_deblockingFilterMetric!=0 && (m_bLoopFilterDisable || m_loopFilterOffsetInPPS), "If DeblockingFilterMetric is non-zero then both LoopFilterDisable and LoopFilterOffsetInPPS must be 0");
#else
  xConfirmPara( m_DeblockingFilterMetric && (m_bLoopFilterDisable || m_loopFilterOffsetInPPS), "If DeblockingFilterMetric is true then both LoopFilterDisable and LoopFilterOffsetInPPS must be 0");
#endif
  xConfirmPara( m_loopFilterBetaOffsetDiv2 < -6 || m_loopFilterBetaOffsetDiv2 > 6,        "Loop Filter Beta Offset div. 2 exceeds supported range (-6 to 6)");
  xConfirmPara( m_loopFilterTcOffsetDiv2 < -6 || m_loopFilterTcOffsetDiv2 > 6,            "Loop Filter Tc Offset div. 2 exceeds supported range (-6 to 6)");

  xConfirmPara( m_iSearchRange < 0 ,                                                        "Search Range must be more than 0" );
  xConfirmPara( m_bipredSearchRange < 0 ,                                                   "Bi-prediction refinement search range must be more than 0" );
  xConfirmPara( m_minSearchWindow < 0,                                                      "Minimum motion search window size for the adaptive window ME must be greater than or equal to 0" );
  xConfirmPara( m_iMaxDeltaQP > 7,                                                          "Absolute Delta QP exceeds supported range (0 to 7)" );
#if SHARP_LUMA_DELTA_QP
  xConfirmPara(m_lumaLevelToDeltaQPMapping.mode &&  m_uiDeltaQpRD > 0, "Luma-level-based Delta QP cannot be used together with slice level multiple-QP optimization\n" );
#endif
  xConfirmPara( m_iMaxCuDQPDepth > m_uiMaxCUDepth - 1,                                          "Absolute depth for a minimum CuDQP exceeds maximum coding unit depth" );

  xConfirmPara( m_cbQpOffset < -12,   "Min. Chroma Cb QP Offset is -12" );
  xConfirmPara( m_cbQpOffset >  12,   "Max. Chroma Cb QP Offset is  12" );
  xConfirmPara( m_crQpOffset < -12,   "Min. Chroma Cr QP Offset is -12" );
  xConfirmPara( m_crQpOffset >  12,   "Max. Chroma Cr QP Offset is  12" );

  xConfirmPara( m_iQPAdaptationRange <= 0,                                                  "QP Adaptation Range must be more than 0" );
  if (m_iDecodingRefreshType == 2)
  {
    xConfirmPara( m_iIntraPeriod > 0 && m_iIntraPeriod <= m_iGOPSize ,                      "Intra period must be larger than GOP size for periodic IDR pictures");
  }
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
  if (m_saoResetEncoderStateAfterIRAP)
  {
    xConfirmPara( m_iIntraPeriod > 0 && m_iIntraPeriod <= m_iGOPSize ,                      "Intra period must be larger than GOP size when SAOResetEncoderStateAfterIRAP is enabled");
  }
#endif
  xConfirmPara( m_uiMaxCUDepth < 1,                                                         "MaxPartitionDepth must be greater than zero");
  xConfirmPara( (m_uiMaxCUWidth  >> m_uiMaxCUDepth) < 4,                                    "Minimum partition width size should be larger than or equal to 8");
  xConfirmPara( (m_uiMaxCUHeight >> m_uiMaxCUDepth) < 4,                                    "Minimum partition height size should be larger than or equal to 8");
  xConfirmPara( m_uiMaxCUWidth < 16,                                                        "Maximum partition width size should be larger than or equal to 16");
  xConfirmPara( m_uiMaxCUHeight < 16,                                                       "Maximum partition height size should be larger than or equal to 16");
  xConfirmPara( (m_iSourceWidth  % (m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)))!=0,             "Resulting coded frame width must be a multiple of the minimum CU size");
  xConfirmPara( (m_iSourceHeight % (m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)))!=0,             "Resulting coded frame height must be a multiple of the minimum CU size");

  xConfirmPara( m_uiQuadtreeTULog2MinSize < 2,                                        "QuadtreeTULog2MinSize must be 2 or greater.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize > 5,                                        "QuadtreeTULog2MaxSize must be 5 or smaller.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize < m_uiQuadtreeTULog2MinSize,                "QuadtreeTULog2MaxSize must be greater than or equal to m_uiQuadtreeTULog2MinSize.");
  xConfirmPara( (1<<m_uiQuadtreeTULog2MaxSize) > m_uiMaxCUWidth,                      "QuadtreeTULog2MaxSize must be log2(maxCUSize) or smaller.");
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) >= ( m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than or equal to minimum CU size" );
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) >= ( m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than or equal to minimum CU size" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthInter < 1,                                                         "QuadtreeTUMaxDepthInter must be greater than or equal to 1" );
  xConfirmPara( m_uiMaxCUWidth < ( 1 << (m_uiQuadtreeTULog2MinSize + m_uiQuadtreeTUMaxDepthInter - 1) ), "QuadtreeTUMaxDepthInter must be less than or equal to the difference between log2(maxCUSize) and QuadtreeTULog2MinSize plus 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthIntra < 1,                                                         "QuadtreeTUMaxDepthIntra must be greater than or equal to 1" );
  xConfirmPara( m_uiMaxCUWidth < ( 1 << (m_uiQuadtreeTULog2MinSize + m_uiQuadtreeTUMaxDepthIntra - 1) ), "QuadtreeTUMaxDepthInter must be less than or equal to the difference between log2(maxCUSize) and QuadtreeTULog2MinSize plus 1" );

  xConfirmPara(  m_maxNumMergeCand < 1,  "MaxNumMergeCand must be 1 or greater.");
  xConfirmPara(  m_maxNumMergeCand > 5,  "MaxNumMergeCand must be 5 or smaller.");

#if ADAPTIVE_QP_SELECTION
  xConfirmPara( m_bUseAdaptQpSelect == true && m_iQP < 0,                                              "AdaptiveQpSelection must be disabled when QP < 0.");
  xConfirmPara( m_bUseAdaptQpSelect == true && (m_cbQpOffset !=0 || m_crQpOffset != 0 ),               "AdaptiveQpSelection must be disabled when ChromaQpOffset is not equal to 0.");
#endif

  if( m_usePCM)
  {
    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      xConfirmPara(((m_MSBExtendedBitDepth[channelType] > m_internalBitDepth[channelType]) && m_bPCMInputBitDepthFlag), "PCM bit depth cannot be greater than internal bit depth (PCMInputBitDepthFlag cannot be used when InputBitDepth or MSBExtendedBitDepth > InternalBitDepth)");
    }
    xConfirmPara(  m_uiPCMLog2MinSize < 3,                                      "PCMLog2MinSize must be 3 or greater.");
    xConfirmPara(  m_uiPCMLog2MinSize > 5,                                      "PCMLog2MinSize must be 5 or smaller.");
    xConfirmPara(  m_pcmLog2MaxSize > 5,                                        "PCMLog2MaxSize must be 5 or smaller.");
    xConfirmPara(  m_pcmLog2MaxSize < m_uiPCMLog2MinSize,                       "PCMLog2MaxSize must be equal to or greater than m_uiPCMLog2MinSize.");
  }

  if (m_sliceMode!=NO_SLICES)
  {
    xConfirmPara( m_sliceArgument < 1 ,         "SliceArgument should be larger than or equal to 1" );
  }
  if (m_sliceSegmentMode!=NO_SLICES)
  {
    xConfirmPara( m_sliceSegmentArgument < 1 ,         "SliceSegmentArgument should be larger than or equal to 1" );
  }

  Bool tileFlag = (m_numTileColumnsMinus1 > 0 || m_numTileRowsMinus1 > 0 );
  if (m_profile!=Profile::HIGHTHROUGHPUTREXT)
  {
    xConfirmPara( tileFlag && m_entropyCodingSyncEnabledFlag, "Tiles and entropy-coding-sync (Wavefronts) can not be applied together, except in the High Throughput Intra 4:4:4 16 profile");
  }

  xConfirmPara( m_iSourceWidth  % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Picture width must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_iSourceHeight % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Picture height must be an integer multiple of the specified chroma subsampling");

  xConfirmPara( m_aiPad[0] % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Horizontal padding must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_aiPad[1] % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Vertical padding must be an integer multiple of the specified chroma subsampling");

  xConfirmPara( m_confWinLeft   % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Left conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinRight  % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Right conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinTop    % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Top conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinBottom % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Bottom conformance window offset must be an integer multiple of the specified chroma subsampling");

  xConfirmPara( m_defaultDisplayWindowFlag && !m_vuiParametersPresentFlag, "VUI needs to be enabled for default display window");

  if (m_defaultDisplayWindowFlag)
  {
    xConfirmPara( m_defDispWinLeftOffset   % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Left default display window offset must be an integer multiple of the specified chroma subsampling");
    xConfirmPara( m_defDispWinRightOffset  % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Right default display window offset must be an integer multiple of the specified chroma subsampling");
    xConfirmPara( m_defDispWinTopOffset    % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Top default display window offset must be an integer multiple of the specified chroma subsampling");
    xConfirmPara( m_defDispWinBottomOffset % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Bottom default display window offset must be an integer multiple of the specified chroma subsampling");
  }

  // max CU width and height should be power of 2
  UInt ui = m_uiMaxCUWidth;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
    {
      xConfirmPara( ui != 1 , "Width should be 2^n");
    }
  }
  ui = m_uiMaxCUHeight;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
    {
      xConfirmPara( ui != 1 , "Height should be 2^n");
    }
  }

  /* if this is an intra-only sequence, ie IntraPeriod=1, don't verify the GOP structure
   * This permits the ability to omit a GOP structure specification */
  if (m_iIntraPeriod == 1 && m_GOPList[0].m_POC == -1)
  {
    m_GOPList[0] = GOPEntry();
    m_GOPList[0].m_QPFactor = 1;
    m_GOPList[0].m_betaOffsetDiv2 = 0;
    m_GOPList[0].m_tcOffsetDiv2 = 0;
    m_GOPList[0].m_POC = 1;
    m_GOPList[0].m_numRefPicsActive = 4;
  }
  else
  {
    xConfirmPara( m_intraConstraintFlag, "IntraConstraintFlag cannot be 1 for inter sequences");
  }

  Bool verifiedGOP=false;
  Bool errorGOP=false;
  Int checkGOP=1;
  Int numRefs = m_isField ? 2 : 1;
  Int refList[MAX_NUM_REF_PICS+1];
  refList[0]=0;
  if(m_isField)
  {
    refList[1] = 1;
  }
  Bool isOK[MAX_GOP];
  for(Int i=0; i<MAX_GOP; i++)
  {
    isOK[i]=false;
  }
  Int numOK=0;
  xConfirmPara( m_iIntraPeriod >=0&&(m_iIntraPeriod%m_iGOPSize!=0), "Intra period must be a multiple of GOPSize, or -1" );

  for(Int i=0; i<m_iGOPSize; i++)
  {
    if(m_GOPList[i].m_POC==m_iGOPSize)
    {
      xConfirmPara( m_GOPList[i].m_temporalId!=0 , "The last frame in each GOP must have temporal ID = 0 " );
    }
  }

  if ( (m_iIntraPeriod != 1) && !m_loopFilterOffsetInPPS && (!m_bLoopFilterDisable) )
  {
    for(Int i=0; i<m_iGOPSize; i++)
    {
      xConfirmPara( (m_GOPList[i].m_betaOffsetDiv2 + m_loopFilterBetaOffsetDiv2) < -6 || (m_GOPList[i].m_betaOffsetDiv2 + m_loopFilterBetaOffsetDiv2) > 6, "Loop Filter Beta Offset div. 2 for one of the GOP entries exceeds supported range (-6 to 6)" );
      xConfirmPara( (m_GOPList[i].m_tcOffsetDiv2 + m_loopFilterTcOffsetDiv2) < -6 || (m_GOPList[i].m_tcOffsetDiv2 + m_loopFilterTcOffsetDiv2) > 6, "Loop Filter Tc Offset div. 2 for one of the GOP entries exceeds supported range (-6 to 6)" );
    }
  }

#if W0038_CQP_ADJ
  for(Int i=0; i<m_iGOPSize; i++)
  {
    xConfirmPara( abs(m_GOPList[i].m_CbQPoffset               ) > 12, "Cb QP Offset for one of the GOP entries exceeds supported range (-12 to 12)" );
    xConfirmPara( abs(m_GOPList[i].m_CbQPoffset + m_cbQpOffset) > 12, "Cb QP Offset for one of the GOP entries, when combined with the PPS Cb offset, exceeds supported range (-12 to 12)" );
    xConfirmPara( abs(m_GOPList[i].m_CrQPoffset               ) > 12, "Cr QP Offset for one of the GOP entries exceeds supported range (-12 to 12)" );
    xConfirmPara( abs(m_GOPList[i].m_CrQPoffset + m_crQpOffset) > 12, "Cr QP Offset for one of the GOP entries, when combined with the PPS Cr offset, exceeds supported range (-12 to 12)" );
  }
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[0]                 ) > 12, "Intra/periodic Cb QP Offset exceeds supported range (-12 to 12)" );
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[0]  + m_cbQpOffset ) > 12, "Intra/periodic Cb QP Offset, when combined with the PPS Cb offset, exceeds supported range (-12 to 12)" );
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[1]                 ) > 12, "Intra/periodic Cr QP Offset exceeds supported range (-12 to 12)" );
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[1]  + m_crQpOffset ) > 12, "Intra/periodic Cr QP Offset, when combined with the PPS Cr offset, exceeds supported range (-12 to 12)" );
#endif

  m_extraRPSs=0;
  //start looping through frames in coding order until we can verify that the GOP structure is correct.
  while(!verifiedGOP&&!errorGOP)
  {
    Int curGOP = (checkGOP-1)%m_iGOPSize;
    Int curPOC = ((checkGOP-1)/m_iGOPSize)*m_iGOPSize + m_GOPList[curGOP].m_POC;
    if(m_GOPList[curGOP].m_POC<0)
    {
      printf("\nError: found fewer Reference Picture Sets than GOPSize\n");
      errorGOP=true;
    }
    else
    {
      //check that all reference pictures are available, or have a POC < 0 meaning they might be available in the next GOP.
      Bool beforeI = false;
      for(Int i = 0; i< m_GOPList[curGOP].m_numRefPics; i++)
      {
        Int absPOC = curPOC+m_GOPList[curGOP].m_referencePics[i];
        if(absPOC < 0)
        {
          beforeI=true;
        }
        else
        {
          Bool found=false;
          for(Int j=0; j<numRefs; j++)
          {
            if(refList[j]==absPOC)
            {
              found=true;
              for(Int k=0; k<m_iGOPSize; k++)
              {
                if(absPOC%m_iGOPSize == m_GOPList[k].m_POC%m_iGOPSize)
                {
                  if(m_GOPList[k].m_temporalId<=m_GOPList[curGOP].m_temporalId)
                  {
                    m_GOPList[k].m_refPic = true;
                  }
                  m_GOPList[curGOP].m_usedByCurrPic[i]=m_GOPList[k].m_temporalId<=m_GOPList[curGOP].m_temporalId;
                }
              }
            }
          }
          if(!found)
          {
            printf("\nError: ref pic %d is not available for GOP frame %d\n",m_GOPList[curGOP].m_referencePics[i],curGOP+1);
            errorGOP=true;
          }
        }
      }
      if(!beforeI&&!errorGOP)
      {
        //all ref frames were present
        if(!isOK[curGOP])
        {
          numOK++;
          isOK[curGOP]=true;
          if(numOK==m_iGOPSize)
          {
            verifiedGOP=true;
          }
        }
      }
      else
      {
        //create a new GOPEntry for this frame containing all the reference pictures that were available (POC > 0)
        m_GOPList[m_iGOPSize+m_extraRPSs]=m_GOPList[curGOP];
        Int newRefs=0;
        for(Int i = 0; i< m_GOPList[curGOP].m_numRefPics; i++)
        {
          Int absPOC = curPOC+m_GOPList[curGOP].m_referencePics[i];
          if(absPOC>=0)
          {
            m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[newRefs]=m_GOPList[curGOP].m_referencePics[i];
            m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[newRefs]=m_GOPList[curGOP].m_usedByCurrPic[i];
            newRefs++;
          }
        }
        Int numPrefRefs = m_GOPList[curGOP].m_numRefPicsActive;

        for(Int offset = -1; offset>-checkGOP; offset--)
        {
          //step backwards in coding order and include any extra available pictures we might find useful to replace the ones with POC < 0.
          Int offGOP = (checkGOP-1+offset)%m_iGOPSize;
          Int offPOC = ((checkGOP-1+offset)/m_iGOPSize)*m_iGOPSize + m_GOPList[offGOP].m_POC;
          if(offPOC>=0&&m_GOPList[offGOP].m_temporalId<=m_GOPList[curGOP].m_temporalId)
          {
            Bool newRef=false;
            for(Int i=0; i<numRefs; i++)
            {
              if(refList[i]==offPOC)
              {
                newRef=true;
              }
            }
            for(Int i=0; i<newRefs; i++)
            {
              if(m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[i]==offPOC-curPOC)
              {
                newRef=false;
              }
            }
            if(newRef)
            {
              Int insertPoint=newRefs;
              //this picture can be added, find appropriate place in list and insert it.
              if(m_GOPList[offGOP].m_temporalId<=m_GOPList[curGOP].m_temporalId)
              {
                m_GOPList[offGOP].m_refPic = true;
              }
              for(Int j=0; j<newRefs; j++)
              {
                if(m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j]<offPOC-curPOC||m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j]>0)
                {
                  insertPoint = j;
                  break;
                }
              }
              Int prev = offPOC-curPOC;
              Int prevUsed = m_GOPList[offGOP].m_temporalId<=m_GOPList[curGOP].m_temporalId;
              for(Int j=insertPoint; j<newRefs+1; j++)
              {
                Int newPrev = m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j];
                Int newUsed = m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[j];
                m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j]=prev;
                m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[j]=prevUsed;
                prevUsed=newUsed;
                prev=newPrev;
              }
              newRefs++;
            }
          }
          if(newRefs>=numPrefRefs)
          {
            break;
          }
        }
        m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefPics=newRefs;
        m_GOPList[m_iGOPSize+m_extraRPSs].m_POC = curPOC;
        if (m_extraRPSs == 0)
        {
          m_GOPList[m_iGOPSize+m_extraRPSs].m_interRPSPrediction = 0;
          m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefIdc = 0;
        }
        else
        {
          Int rIdx =  m_iGOPSize + m_extraRPSs - 1;
          Int refPOC = m_GOPList[rIdx].m_POC;
          Int refPics = m_GOPList[rIdx].m_numRefPics;
          Int newIdc=0;
          for(Int i = 0; i<= refPics; i++)
          {
            Int deltaPOC = ((i != refPics)? m_GOPList[rIdx].m_referencePics[i] : 0);  // check if the reference abs POC is >= 0
            Int absPOCref = refPOC+deltaPOC;
            Int refIdc = 0;
            for (Int j = 0; j < m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefPics; j++)
            {
              if ( (absPOCref - curPOC) == m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j])
              {
                if (m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[j])
                {
                  refIdc = 1;
                }
                else
                {
                  refIdc = 2;
                }
              }
            }
            m_GOPList[m_iGOPSize+m_extraRPSs].m_refIdc[newIdc]=refIdc;
            newIdc++;
          }
          m_GOPList[m_iGOPSize+m_extraRPSs].m_interRPSPrediction = 1;
          m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefIdc = newIdc;
          m_GOPList[m_iGOPSize+m_extraRPSs].m_deltaRPS = refPOC - m_GOPList[m_iGOPSize+m_extraRPSs].m_POC;
        }
        curGOP=m_iGOPSize+m_extraRPSs;
        m_extraRPSs++;
      }
      numRefs=0;
      for(Int i = 0; i< m_GOPList[curGOP].m_numRefPics; i++)
      {
        Int absPOC = curPOC+m_GOPList[curGOP].m_referencePics[i];
        if(absPOC >= 0)
        {
          refList[numRefs]=absPOC;
          numRefs++;
        }
      }
      refList[numRefs]=curPOC;
      numRefs++;
    }
    checkGOP++;
  }
  xConfirmPara(errorGOP,"Invalid GOP structure given");
  m_maxTempLayer = 1;
  for(Int i=0; i<m_iGOPSize; i++)
  {
    if(m_GOPList[i].m_temporalId >= m_maxTempLayer)
    {
      m_maxTempLayer = m_GOPList[i].m_temporalId+1;
    }
    xConfirmPara(m_GOPList[i].m_sliceType!='B' && m_GOPList[i].m_sliceType!='P' && m_GOPList[i].m_sliceType!='I', "Slice type must be equal to B or P or I");
  }
  for(Int i=0; i<MAX_TLAYER; i++)
  {
    m_numReorderPics[i] = 0;
    m_maxDecPicBuffering[i] = 1 + m_useIntraBlockCopy;
  }
  for(Int i=0; i<m_iGOPSize; i++)
  {
    if(m_GOPList[i].m_numRefPics+1 > m_maxDecPicBuffering[m_GOPList[i].m_temporalId])
    {
      m_maxDecPicBuffering[m_GOPList[i].m_temporalId] = m_GOPList[i].m_numRefPics + 1 + m_useIntraBlockCopy;
    }

    Int highestDecodingNumberWithLowerPOC = 0;
    for(Int j=0; j<m_iGOPSize; j++)
    {
      if(m_GOPList[j].m_POC <= m_GOPList[i].m_POC)
      {
        highestDecodingNumberWithLowerPOC = j;
      }
    }
    Int numReorder = 0;
    for(Int j=0; j<highestDecodingNumberWithLowerPOC; j++)
    {
      if(m_GOPList[j].m_temporalId <= m_GOPList[i].m_temporalId &&
        m_GOPList[j].m_POC > m_GOPList[i].m_POC)
      {
        numReorder++;
      }
    }
    if(numReorder > m_numReorderPics[m_GOPList[i].m_temporalId])
    {
      m_numReorderPics[m_GOPList[i].m_temporalId] = numReorder;
    }
  }
  for(Int i=0; i<MAX_TLAYER-1; i++)
  {
    // a lower layer can not have higher value of m_numReorderPics than a higher layer
    if(m_numReorderPics[i+1] < m_numReorderPics[i])
    {
      m_numReorderPics[i+1] = m_numReorderPics[i];
    }
    // the value of num_reorder_pics[ i ] shall be in the range of 0 to max_dec_pic_buffering[ i ] - 1, inclusive
    if(m_numReorderPics[i] > m_maxDecPicBuffering[i] - 1)
    {
      m_maxDecPicBuffering[i] = m_numReorderPics[i] + 1;
    }
    // a lower layer can not have higher value of m_uiMaxDecPicBuffering than a higher layer
    if(m_maxDecPicBuffering[i+1] < m_maxDecPicBuffering[i])
    {
      m_maxDecPicBuffering[i+1] = m_maxDecPicBuffering[i];
    }
  }

  // the value of num_reorder_pics[ i ] shall be in the range of 0 to max_dec_pic_buffering[ i ] -  1, inclusive
  if(m_numReorderPics[MAX_TLAYER-1] > m_maxDecPicBuffering[MAX_TLAYER-1] - 1)
  {
    m_maxDecPicBuffering[MAX_TLAYER-1] = m_numReorderPics[MAX_TLAYER-1] + 1;
  }
  if(m_vuiParametersPresentFlag && m_bitstreamRestrictionFlag)
  {
    Int PicSizeInSamplesY =  m_iSourceWidth * m_iSourceHeight;
    if(tileFlag)
    {
      Int maxTileWidth = 0;
      Int maxTileHeight = 0;
      Int widthInCU = (m_iSourceWidth % m_uiMaxCUWidth) ? m_iSourceWidth/m_uiMaxCUWidth + 1: m_iSourceWidth/m_uiMaxCUWidth;
      Int heightInCU = (m_iSourceHeight % m_uiMaxCUHeight) ? m_iSourceHeight/m_uiMaxCUHeight + 1: m_iSourceHeight/m_uiMaxCUHeight;
      if(m_tileUniformSpacingFlag)
      {
        maxTileWidth = m_uiMaxCUWidth*((widthInCU+m_numTileColumnsMinus1)/(m_numTileColumnsMinus1+1));
        maxTileHeight = m_uiMaxCUHeight*((heightInCU+m_numTileRowsMinus1)/(m_numTileRowsMinus1+1));
        // if only the last tile-row is one treeblock higher than the others
        // the maxTileHeight becomes smaller if the last row of treeblocks has lower height than the others
        if(!((heightInCU-1)%(m_numTileRowsMinus1+1)))
        {
          maxTileHeight = maxTileHeight - m_uiMaxCUHeight + (m_iSourceHeight % m_uiMaxCUHeight);
        }
        // if only the last tile-column is one treeblock wider than the others
        // the maxTileWidth becomes smaller if the last column of treeblocks has lower width than the others
        if(!((widthInCU-1)%(m_numTileColumnsMinus1+1)))
        {
          maxTileWidth = maxTileWidth - m_uiMaxCUWidth + (m_iSourceWidth % m_uiMaxCUWidth);
        }
      }
      else // not uniform spacing
      {
        if(m_numTileColumnsMinus1<1)
        {
          maxTileWidth = m_iSourceWidth;
        }
        else
        {
          Int accColumnWidth = 0;
          for(Int col=0; col<(m_numTileColumnsMinus1); col++)
          {
            maxTileWidth = m_tileColumnWidth[col]>maxTileWidth ? m_tileColumnWidth[col]:maxTileWidth;
            accColumnWidth += m_tileColumnWidth[col];
          }
          maxTileWidth = (widthInCU-accColumnWidth)>maxTileWidth ? m_uiMaxCUWidth*(widthInCU-accColumnWidth):m_uiMaxCUWidth*maxTileWidth;
        }
        if(m_numTileRowsMinus1<1)
        {
          maxTileHeight = m_iSourceHeight;
        }
        else
        {
          Int accRowHeight = 0;
          for(Int row=0; row<(m_numTileRowsMinus1); row++)
          {
            maxTileHeight = m_tileRowHeight[row]>maxTileHeight ? m_tileRowHeight[row]:maxTileHeight;
            accRowHeight += m_tileRowHeight[row];
          }
          maxTileHeight = (heightInCU-accRowHeight)>maxTileHeight ? m_uiMaxCUHeight*(heightInCU-accRowHeight):m_uiMaxCUHeight*maxTileHeight;
        }
      }
      Int maxSizeInSamplesY = maxTileWidth*maxTileHeight;
      m_minSpatialSegmentationIdc = 4*PicSizeInSamplesY/maxSizeInSamplesY-4;
    }
    else if(m_entropyCodingSyncEnabledFlag)
    {
      m_minSpatialSegmentationIdc = 4*PicSizeInSamplesY/((2*m_iSourceHeight+m_iSourceWidth)*m_uiMaxCUHeight)-4;
    }
    else if(m_sliceMode == FIXED_NUMBER_OF_CTU)
    {
      m_minSpatialSegmentationIdc = 4*PicSizeInSamplesY/(m_sliceArgument*m_uiMaxCUWidth*m_uiMaxCUHeight)-4;
    }
    else
    {
      m_minSpatialSegmentationIdc = 0;
    }
  }

  if (m_toneMappingInfoSEIEnabled)
  {
    xConfirmPara( m_toneMapCodedDataBitDepth < 8 || m_toneMapCodedDataBitDepth > 14 , "SEIToneMapCodedDataBitDepth must be in rage 8 to 14");
    xConfirmPara( m_toneMapTargetBitDepth < 1 || (m_toneMapTargetBitDepth > 16 && m_toneMapTargetBitDepth < 255) , "SEIToneMapTargetBitDepth must be in rage 1 to 16 or equal to 255");
    xConfirmPara( m_toneMapModelId < 0 || m_toneMapModelId > 4 , "SEIToneMapModelId must be in rage 0 to 4");
    xConfirmPara( m_cameraIsoSpeedValue == 0, "SEIToneMapCameraIsoSpeedValue shall not be equal to 0");
    xConfirmPara( m_exposureIndexValue  == 0, "SEIToneMapExposureIndexValue shall not be equal to 0");
    xConfirmPara( m_extendedRangeWhiteLevel < 100, "SEIToneMapExtendedRangeWhiteLevel should be greater than or equal to 100");
    xConfirmPara( m_nominalBlackLevelLumaCodeValue >= m_nominalWhiteLevelLumaCodeValue, "SEIToneMapNominalWhiteLevelLumaCodeValue shall be greater than SEIToneMapNominalBlackLevelLumaCodeValue");
    xConfirmPara( m_extendedWhiteLevelLumaCodeValue < m_nominalWhiteLevelLumaCodeValue, "SEIToneMapExtendedWhiteLevelLumaCodeValue shall be greater than or equal to SEIToneMapNominalWhiteLevelLumaCodeValue");
  }

  if (m_kneeSEIEnabled && !m_kneeSEICancelFlag)
  {
    xConfirmPara( m_kneeSEINumKneePointsMinus1 < 0 || m_kneeSEINumKneePointsMinus1 > 998, "SEIKneeFunctionNumKneePointsMinus1 must be in the range of 0 to 998");
    for ( UInt i=0; i<=m_kneeSEINumKneePointsMinus1; i++ )
    {
      xConfirmPara( m_kneeSEIInputKneePoint[i] < 1 || m_kneeSEIInputKneePoint[i] > 999, "SEIKneeFunctionInputKneePointValue must be in the range of 1 to 999");
      xConfirmPara( m_kneeSEIOutputKneePoint[i] < 0 || m_kneeSEIOutputKneePoint[i] > 1000, "SEIKneeFunctionInputKneePointValue must be in the range of 0 to 1000");
      if ( i > 0 )
      {
        xConfirmPara( m_kneeSEIInputKneePoint[i-1] >= m_kneeSEIInputKneePoint[i],  "The i-th SEIKneeFunctionInputKneePointValue must be greater than the (i-1)-th value");
        xConfirmPara( m_kneeSEIOutputKneePoint[i-1] > m_kneeSEIOutputKneePoint[i],  "The i-th SEIKneeFunctionOutputKneePointValue must be greater than or equal to the (i-1)-th value");
      }
    }
  }

  if (m_chromaResamplingFilterSEIenabled)
  {
    xConfirmPara( (m_chromaFormatIDC == CHROMA_400 ), "chromaResamplingFilterSEI is not allowed to be present when ChromaFormatIDC is equal to zero (4:0:0)" );
    xConfirmPara(m_vuiParametersPresentFlag && m_chromaLocInfoPresentFlag && (m_chromaSampleLocTypeTopField != m_chromaSampleLocTypeBottomField ), "When chromaResamplingFilterSEI is enabled, ChromaSampleLocTypeTopField has to be equal to ChromaSampleLocTypeBottomField" );
  }

  if ( m_RCEnableRateControl )
  {
    if ( m_RCForceIntraQP )
    {
      if ( m_RCInitialQP == 0 )
      {
        printf( "\nInitial QP for rate control is not specified. Reset not to use force intra QP!" );
        m_RCForceIntraQP = false;
      }
    }
    xConfirmPara( m_uiDeltaQpRD > 0, "Rate control cannot be used together with slice level multiple-QP optimization!\n" );
#if U0132_TARGET_BITS_SATURATION
    if ((m_RCCpbSaturationEnabled) && (m_level!=Level::NONE) && (m_profile!=Profile::NONE))
    {
      UInt uiLevelIdx = (m_level / 10) + (UInt)((m_level % 10) / 3);    // (m_level / 30)*3 + ((m_level % 10) / 3);
      xConfirmPara(m_RCCpbSize > g_uiMaxCpbSize[m_levelTier][uiLevelIdx], "RCCpbSize should be smaller than or equal to Max CPB size according to tier and level");
      xConfirmPara(m_RCInitialCpbFullness > 1, "RCInitialCpbFullness should be smaller than or equal to 1");
    }
#endif
  }
#if U0132_TARGET_BITS_SATURATION
  else
  {
    xConfirmPara( m_RCCpbSaturationEnabled != 0, "Target bits saturation cannot be processed without Rate control" );
  }
  if (m_vuiParametersPresentFlag)
  {
    xConfirmPara(m_RCTargetBitrate == 0, "A target bit rate is required to be set for VUI/HRD parameters.");
    if (m_RCCpbSize == 0)
    {
      printf ("Warning: CPB size is set equal to zero. Adjusting value to be equal to TargetBitrate!\n");
      m_RCCpbSize = m_RCTargetBitrate;
    }
  }
#endif

  xConfirmPara(!m_TransquantBypassEnabledFlag && m_CUTransquantBypassFlagForce, "CUTransquantBypassFlagForce cannot be 1 when TransquantBypassEnableFlag is 0");

  xConfirmPara(m_log2ParallelMergeLevel < 2, "Log2ParallelMergeLevel should be larger than or equal to 2");

  if (m_framePackingSEIEnabled)
  {
    xConfirmPara(m_framePackingSEIType < 3 || m_framePackingSEIType > 5 , "SEIFramePackingType must be in rage 3 to 5");
  }

  if (m_segmentedRectFramePackingSEIEnabled)
  {
    xConfirmPara(m_framePackingSEIEnabled , "SEISegmentedRectFramePacking must be 0 when SEIFramePacking is 1");
  }

  if((m_numTileColumnsMinus1 <= 0) && (m_numTileRowsMinus1 <= 0) && m_tmctsSEIEnabled)
  {
    printf("Warning: SEITempMotionConstrainedTileSets is set to false to disable temporal motion-constrained tile sets SEI message because there are no tiles enabled.\n");
    m_tmctsSEIEnabled = false;
  }

  if(m_timeCodeSEIEnabled)
  {
    xConfirmPara(m_timeCodeSEINumTs > MAX_TIMECODE_SEI_SETS, "Number of time sets cannot exceed 3");
  }

#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  xConfirmPara(m_preferredTransferCharacteristics > 255, "transfer_characteristics_idc should not be greater than 255.");
#endif

  if( m_useIntraBlockCopy )
  {
    if( m_useHashBasedIntraBlockCopySearch )
    {
      xConfirmPara(m_intraBlockCopySearchWidthInCTUs < -1, "IntraBlockCopySearchWidth should be greater than or equal to -1\n");
      if( m_intraBlockCopySearchWidthInCTUs >= 0 )
      {
        xConfirmPara((Int)m_intraBlockCopyNonHashSearchWidthInCTUs > m_intraBlockCopySearchWidthInCTUs, "IntraBlockCopyNonHashSearchWidth should be less than or equal to IntraBlockCopySearchWidth\n");
        if( m_intraBlockCopySearchWidthInCTUs == (Int)m_intraBlockCopyNonHashSearchWidthInCTUs )
        {
          m_useHashBasedIntraBlockCopySearch = false;
        }
      }
    }
    else
    {
      xConfirmPara(m_intraBlockCopySearchWidthInCTUs < 0, "HashBasedIntraBlockCopySearch must be set to 1 to enable IntraBlockCopy full frame search\n");
      m_intraBlockCopySearchWidthInCTUs = (Int)m_intraBlockCopyNonHashSearchWidthInCTUs;
    }
    if( !(m_intraBlockCopySearchWidthInCTUs == -1 && m_intraBlockCopyNonHashSearchWidthInCTUs == 1)
      && !(m_intraBlockCopySearchWidthInCTUs == 3 && m_intraBlockCopyNonHashSearchWidthInCTUs == 1) )
    {
      fprintf(stderr, "***************************************************************************\n");
      fprintf(stderr, "** WARNING: IntraBC search ranges are not part of CTC/CE test conditions **\n");
      fprintf(stderr, "***************************************************************************\n");
    }
  }


#undef xConfirmPara
  if (check_failed)
  {
    exit(EXIT_FAILURE);
  }
}

const TChar *profileToString(const Profile::Name profile)
{
  static const UInt numberOfProfiles = sizeof(strToProfile)/sizeof(*strToProfile);

  for (UInt profileIndex = 0; profileIndex < numberOfProfiles; profileIndex++)
  {
    if (strToProfile[profileIndex].value == profile)
    {
      return strToProfile[profileIndex].str;
    }
  }

  //if we get here, we didn't find this profile in the list - so there is an error
  std::cerr << "ERROR: Unknown profile \"" << profile << "\" in profileToString" << std::endl;
  assert(false);
  exit(1);
  return "";
}

Void TAppEncCfg::xPrintParameter()
{
  printf("\n");
  printf("Input          File                    : %s\n", m_inputFileName.c_str()          );
  printf("Bitstream      File                    : %s\n", m_bitstreamFileName.c_str()      );
  printf("Reconstruction File                    : %s\n", m_reconFileName.c_str()          );
  printf("Real     Format                        : %dx%d %gHz\n", m_iSourceWidth - m_confWinLeft - m_confWinRight, m_iSourceHeight - m_confWinTop - m_confWinBottom, (Double)m_iFrameRate/m_temporalSubsampleRatio );
  printf("Internal Format                        : %dx%d %gHz\n", m_iSourceWidth, m_iSourceHeight, (Double)m_iFrameRate/m_temporalSubsampleRatio );
  printf("Sequence PSNR output                   : %s\n", (m_printMSEBasedSequencePSNR ? "Linear average, MSE-based" : "Linear average only") );
  printf("Sequence MSE output                    : %s\n", (m_printSequenceMSE ? "Enabled" : "Disabled") );
  printf("Frame MSE output                       : %s\n", (m_printFrameMSE    ? "Enabled" : "Disabled") );
  printf("Print Clipped PSNR                     : %s\n", (m_printClippedPSNR ? "Enabled" : "Disabled") );
  printf("Cabac-zero-word-padding                : %s\n", (m_cabacZeroWordPaddingEnabled? "Enabled" : "Disabled") );
  if (m_isField)
  {
    printf("Frame/Field                            : Field based coding\n");
    printf("Field index                            : %u - %d (%d fields)\n", m_FrameSkip, m_FrameSkip+m_framesToBeEncoded-1, m_framesToBeEncoded );
    printf("Field Order                            : %s field first\n", m_isTopFieldFirst?"Top":"Bottom");

  }
  else
  {
    printf("Frame/Field                            : Frame based coding\n");
    printf("Frame index                            : %u - %d (%d frames)\n", m_FrameSkip, m_FrameSkip+m_framesToBeEncoded-1, m_framesToBeEncoded );
  }
  if (m_profile == Profile::MAINREXT)
  {
    ExtendedProfileName validProfileName;
    if (m_onePictureOnlyConstraintFlag)
    {
      validProfileName = m_bitDepthConstraint == 8 ? MAIN_444_STILL_PICTURE : (m_bitDepthConstraint == 16 ? MAIN_444_16_STILL_PICTURE : NONE);
    }
    else
    {
      const UInt intraIdx = m_intraConstraintFlag ? 1:0;
      const UInt bitDepthIdx = (m_bitDepthConstraint == 8 ? 0 : (m_bitDepthConstraint ==10 ? 1 : (m_bitDepthConstraint == 12 ? 2 : (m_bitDepthConstraint == 16 ? 3 : 4 ))));
      const UInt chromaFormatIdx = UInt(m_chromaFormatConstraint);
      validProfileName = (bitDepthIdx > 3 || chromaFormatIdx>3) ? NONE : validRExtProfileNames[intraIdx][bitDepthIdx][chromaFormatIdx];
    }
    std::string rextSubProfile;
    if (validProfileName!=NONE)
    {
      rextSubProfile=enumToString(strToExtendedProfile, sizeof(strToExtendedProfile)/sizeof(*strToExtendedProfile), validProfileName);
    }
    if (rextSubProfile == "main_444_16")
    {
      rextSubProfile="main_444_16 [NON STANDARD]";
    }
    printf("Profile                                : %s (%s)\n", profileToString(m_profile), (rextSubProfile.empty())?"INVALID REXT PROFILE":rextSubProfile.c_str() );
  }
  else
  {
    printf("Profile                                : %s\n", profileToString(m_profile) );
  }
  printf("CU size / depth / total-depth          : %d / %d / %d\n", m_uiMaxCUWidth, m_uiMaxCUDepth, m_uiMaxTotalCUDepth );
  printf("RQT trans. size (min / max)            : %d / %d\n", 1 << m_uiQuadtreeTULog2MinSize, 1 << m_uiQuadtreeTULog2MaxSize );
  printf("Max RQT depth inter                    : %d\n", m_uiQuadtreeTUMaxDepthInter);
  printf("Max RQT depth intra                    : %d\n", m_uiQuadtreeTUMaxDepthIntra);
  printf("Min PCM size                           : %d\n", 1 << m_uiPCMLog2MinSize);
  printf("Motion search range                    : %d\n", m_iSearchRange );
  printf("Intra period                           : %d\n", m_iIntraPeriod );
  printf("Decoding refresh type                  : %d\n", m_iDecodingRefreshType );
  printf("QP                                     : %5.2f\n", m_fQP );
  printf("Max dQP signaling depth                : %d\n", m_iMaxCuDQPDepth);

  printf("Cb QP Offset                           : %d\n", m_cbQpOffset   );
  printf("Cr QP Offset                           : %d\n", m_crQpOffset);
  printf("QP adaptation                          : %d (range=%d)\n", m_bUseAdaptiveQP, (m_bUseAdaptiveQP ? m_iQPAdaptationRange : 0) );
  printf("GOP size                               : %d\n", m_iGOPSize );
  printf("Input bit depth                        : (Y:%d, C:%d)\n", m_inputBitDepth[CHANNEL_TYPE_LUMA], m_inputBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("MSB-extended bit depth                 : (Y:%d, C:%d)\n", m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA], m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("Internal bit depth                     : (Y:%d, C:%d)\n", m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("PCM sample bit depth                   : (Y:%d, C:%d)\n", m_bPCMInputBitDepthFlag ? m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA] : m_internalBitDepth[CHANNEL_TYPE_LUMA],
                                                                    m_bPCMInputBitDepthFlag ? m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] : m_internalBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("Intra reference smoothing              : %s\n", (m_enableIntraReferenceSmoothing           ? "Enabled" : "Disabled") );
  printf("diff_cu_chroma_qp_offset_depth         : %d\n", m_diffCuChromaQpOffsetDepth);
  printf("extended_precision_processing_flag     : %s\n", (m_extendedPrecisionProcessingFlag         ? "Enabled" : "Disabled") );
  printf("Intra block copying                    : %s\n", (m_useIntraBlockCopy                       ? (m_intraBlockCopyFastSearch ? "Enabled (fast search)" : "Enabled (full search)") : "Disabled") );
  printf("implicit_rdpcm_enabled_flag            : %s\n", (m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ? "Enabled" : "Disabled") );
  printf("explicit_rdpcm_enabled_flag            : %s\n", (m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ? "Enabled" : "Disabled") );
  printf("transform_skip_rotation_enabled_flag   : %s\n", (m_transformSkipRotationEnabledFlag        ? "Enabled" : "Disabled") );
  printf("transform_skip_context_enabled_flag    : %s\n", (m_transformSkipContextEnabledFlag         ? "Enabled" : "Disabled") );
  printf("cross_component_prediction_enabled_flag: %s\n", (m_crossComponentPredictionEnabledFlag     ? (m_reconBasedCrossCPredictionEstimate ? "Enabled (reconstructed-residual-based estimate)" : "Enabled (encoder-side-residual-based estimate)") : "Disabled") );
  printf("high_precision_offsets_enabled_flag    : %s\n", (m_highPrecisionOffsetsEnabledFlag         ? "Enabled" : "Disabled") );
  printf("persistent_rice_adaptation_enabled_flag: %s\n", (m_persistentRiceAdaptationEnabledFlag     ? "Enabled" : "Disabled") );
  printf("cabac_bypass_alignment_enabled_flag    : %s\n", (m_cabacBypassAlignmentEnabledFlag         ? "Enabled" : "Disabled") );
  printf("Adaptive colour transform              : %s\n", (m_useColourTrans                          ? "Enabled" : "Disabled") );
  printf("Palette mode                           : %s\n", (m_usePaletteMode                          ? "Enabled" : "Disabled") );

  if (m_bUseSAO)
  {
    printf("log2_sao_offset_scale_luma             : %d\n", m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA]);
    printf("log2_sao_offset_scale_chroma           : %d\n", m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA]);
  }

  switch (m_costMode)
  {
    case COST_STANDARD_LOSSY:               printf("Cost function:                         : Lossy coding (default)\n"); break;
    case COST_SEQUENCE_LEVEL_LOSSLESS:      printf("Cost function:                         : Sequence_level_lossless coding\n"); break;
    case COST_LOSSLESS_CODING:              printf("Cost function:                         : Lossless coding with fixed QP of %d\n", LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP); break;
    case COST_MIXED_LOSSLESS_LOSSY_CODING:  printf("Cost function:                         : Mixed_lossless_lossy coding with QP'=%d for lossless evaluation\n", LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP_PRIME); break;
    default:                                printf("Cost function:                         : Unknown\n"); break;
  }

  switch ( m_motionEstimationSearchMethod )
  {
    case MESEARCH_FULL:              printf( "Motion Estimation                      : Full search\n" ); break;
    case MESEARCH_DIAMOND:           printf( "Motion Estimation                      : Diamond search\n" ); break;
    case MESEARCH_SELECTIVE:         printf( "Motion Estimation                      : Selective search\n" ); break;
    case MESEARCH_DIAMOND_ENHANCED:  printf( "Motion Estimation                      : Diamond enhanced search\n" ); break;
    default:                         printf( "Motion Estimation                      : Unknown\n" ); break;
  }

  if( m_useIntraBlockCopy )
  {
    printf("Hash based IntraBC search              : %s\n", (m_useHashBasedIntraBlockCopySearch ? "Enabled" : "Disabled") );
    if( m_intraBlockCopySearchWidthInCTUs == -1 )
    {
      printf("IntraBC search range                   : full frame\n");
    }
    else
    {
      printf("IntraBC search range                   : 1x%d CTU%s\n", m_intraBlockCopySearchWidthInCTUs+1, m_intraBlockCopySearchWidthInCTUs ? "s" : "" );
    }
    printf("IntraBC non-hash search range          : 1x%d CTU%s\n", m_intraBlockCopyNonHashSearchWidthInCTUs+1, m_intraBlockCopyNonHashSearchWidthInCTUs ? "s" : "" );
  }

  printf("HashME                                 : %d\n", m_useHashBasedME ? 1 : 0 );

  printf("RateControl                            : %d\n", m_RCEnableRateControl );
  printf("WPMethod                               : %d\n", Int(m_weightedPredictionMethod));

  if(m_RCEnableRateControl)
  {
    printf("TargetBitrate                          : %d\n", m_RCTargetBitrate );
    printf("KeepHierarchicalBit                    : %d\n", m_RCKeepHierarchicalBit );
    printf("LCULevelRC                             : %d\n", m_RCLCULevelRC );
    printf("UseLCUSeparateModel                    : %d\n", m_RCUseLCUSeparateModel );
    printf("InitialQP                              : %d\n", m_RCInitialQP );
    printf("ForceIntraQP                           : %d\n", m_RCForceIntraQP );
#if U0132_TARGET_BITS_SATURATION
    printf("CpbSaturation                          : %d\n", m_RCCpbSaturationEnabled );
    if (m_RCCpbSaturationEnabled)
    {
      printf("CpbSize                                : %d\n", m_RCCpbSize);
      printf("InitalCpbFullness                      : %.2f\n", m_RCInitialCpbFullness);
    }
#endif
  }

  printf("Max Num Merge Candidates               : %d\n", m_maxNumMergeCand);
  printf("\n");

  printf("TOOL CFG: ");
  printf("IBD:%d ", ((m_internalBitDepth[CHANNEL_TYPE_LUMA] > m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA]) || (m_internalBitDepth[CHANNEL_TYPE_CHROMA] > m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA])));
  printf("HAD:%d ", m_bUseHADME                          );
  printf("RDQ:%d ", m_useRDOQ                            );
  printf("RDQTS:%d ", m_useRDOQTS                        );
  printf("RDpenalty:%d ", m_rdPenalty                    );
#if SHARP_LUMA_DELTA_QP
  printf("LQP:%d ", m_lumaLevelToDeltaQPMapping.mode     );
#endif
  printf("SQP:%d ", m_uiDeltaQpRD                        );
  printf("ASR:%d ", m_bUseASR                            );
  printf("MinSearchWindow:%d ", m_minSearchWindow        );
  printf("RestrictMESampling:%d ", m_bRestrictMESampling );
  printf("FEN:%d ", Int(m_fastInterSearchMode)           );
  printf("ECU:%d ", m_bUseEarlyCU                        );
  printf("FDM:%d ", m_useFastDecisionForMerge            );
  printf("CFM:%d ", m_bUseCbfFastMode                    );
  printf("ESD:%d ", m_useEarlySkipDetection              );
  printf("RQT:%d ", 1                                    );
  printf("TransformSkip:%d ",     m_useTransformSkip     );
  printf("TransformSkipFast:%d ", m_useTransformSkipFast );
  printf("TransformSkipLog2MaxSize:%d ", m_log2MaxTransformSkipBlockSize);
  printf("Slice: M=%d ", Int(m_sliceMode));
  if (m_sliceMode!=NO_SLICES)
  {
    printf("A=%d ", m_sliceArgument);
  }
  printf("SliceSegment: M=%d ",m_sliceSegmentMode);
  if (m_sliceSegmentMode!=NO_SLICES)
  {
    printf("A=%d ", m_sliceSegmentArgument);
  }
  printf("CIP:%d ", m_bUseConstrainedIntraPred);
  printf("SAO:%d ", (m_bUseSAO)?(1):(0));
  printf("PCM:%d ", (m_usePCM && (1<<m_uiPCMLog2MinSize) <= m_uiMaxCUWidth)? 1 : 0);

  if (m_TransquantBypassEnabledFlag && m_CUTransquantBypassFlagForce)
  {
    printf("TransQuantBypassEnabled: =1");
  }
  else
  {
    printf("TransQuantBypassEnabled:%d ", (m_TransquantBypassEnabledFlag)? 1:0 );
  }

  printf("TransQuantBypassInferTUSplit:%d ", m_bTransquantBypassInferTUSplit);
  printf("CUNoSplitIntraACT:%d ", m_bNoTUSplitIntraACTEnabled);

  printf("WPP:%d ", (Int)m_useWeightedPred);
  printf("WPB:%d ", (Int)m_useWeightedBiPred);
  printf("PME:%d ", m_log2ParallelMergeLevel);
  const Int iWaveFrontSubstreams = m_entropyCodingSyncEnabledFlag ? (m_iSourceHeight + m_uiMaxCUHeight - 1) / m_uiMaxCUHeight : 1;
  printf(" WaveFrontSynchro:%d WaveFrontSubstreams:%d", m_entropyCodingSyncEnabledFlag?1:0, iWaveFrontSubstreams);
  printf(" ScalingList:%d ", m_useScalingListId );
  printf("TMVPMode:%d ", m_TMVPModeId     );
#if ADAPTIVE_QP_SELECTION
  printf("AQpS:%d", m_bUseAdaptQpSelect   );
#endif

  printf(" SignBitHidingFlag:%d ", m_signDataHidingEnabledFlag);
  printf("RecalQP:%d", m_recalculateQPAccordingToLambda ? 1 : 0 );
  if (m_usePaletteMode)
  {
    printf(" MaxPaletteSize:%d", m_uiPaletteMaxSize);
    printf(" MaxPalettePredictorSize:%d", m_uiPaletteMaxPredSize);
  }
  printf( " MvResControl:%d", m_motionVectorResolutionControlIdc );

  printf("\n\n");

  fflush(stdout);
}

Bool confirmPara(Bool bflag, const TChar* message)
{
  if (!bflag)
  {
    return false;
  }

  printf("Error: %s\n",message);
  return true;
}

//! \}
