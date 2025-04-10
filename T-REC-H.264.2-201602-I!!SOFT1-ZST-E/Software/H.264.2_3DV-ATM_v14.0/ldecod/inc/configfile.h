/*
 * Disclaimer of Warranty
 *
 * Copyright 2001-2015, International Telecommunication Union, Geneva
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis. The ITU disclaims
 * any and all warranties, whether express, implied, or statutory,
 * including any implied warranties of merchantability or of fitness
 * for a particular purpose.  In no event shall the ITU be liable for
 * any incidental, punitive, or consequential damages of any kind
 * whatsoever arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and the user's customers, employees, agents, transferees,
 * successors, and assigns.
 *
 * The ITU does not represent or warrant that the programs furnished
 * hereunder are free of infringement of any third-party patents.
 * Commercial implementations of ITU-T Recommendations, including
 * shareware, may be subject to royalty fees to patent holders.
 * Information regarding the ITU-T patent policy is available from the
 * ITU web site at http://www.itu.int.
 *
 * THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.
 *
 */



/*!
 ***********************************************************************
 *  \file
 *     configfile.h
 *  \brief
 *     Prototypes for configfile.c and definitions of used structures.
 ***********************************************************************
 */

#ifndef _CONFIGFILE_H_
#define _CONFIGFILE_H_


#define DEFAULTCONFIGFILENAME "decoder.cfg"

#include "config_common.h"
//#define PROFILE_IDC     88
//#define LEVEL_IDC       21


InputParameters cfgparams;
#if EXT3D
InputParameters cfgparamsDepth;
#endif

#ifdef INCLUDED_BY_CONFIGFILE_C
// Mapping_Map Syntax:
// {NAMEinConfigFile,  &cfgparams.VariableName, Type, InitialValue, LimitType, MinLimit, MaxLimit, CharSize}
// Types : {0:int, 1:text, 2: double}
// LimitType: {0:none, 1:both, 2:minimum, 3: QP based}
// We could separate this based on types to make it more flexible and allow also defaults for text types.
Mapping Map[] = {
    {"InputFile",               &cfgparams.infile,                       1,   0.0,                       0,  0.0,              0.0,             FILE_NAME_SIZE, },
#if EXT3D
    {"OutputFileForTexture",     &(cfgparams.outfile[0]),                      1,   0.0,                       0,  0.0,              0.0,             FILE_NAME_SIZE, },
    {"OutputFileForDepth",       &(cfgparams.outfile[1]),                      1,   0.0,                       0,  0.0,              0.0,             FILE_NAME_SIZE, },
    {"WriteUVForTexture",        &(cfgparams.write_uv[0]),                0,   1.0,                       1,  0.0,              1.0,                             },
    {"WriteUVForDepth",          &(cfgparams.write_uv[1]),                0,   0.0,                       1,  0.0,              1.0,                             },
    {"OutputRecYUVFile",         &(cfgparams.OutputYUVRecFile),           0,   0.0,                       1,  0.0,              1.0,                             },
    {"NormalizeDepth",           &(cfgparams.NormalizeResolutionDepth),   0,   0.0,                       1,  0.0,              1.0,                             },

    {"PostDilation",             &(cfgparams.PostDilation),               0,   0.0,                       1,  0.0,              1.0,                             },

#else
    {"OutputFile",               &cfgparams.outfile,                      1,   0.0,                       0,  0.0,              0.0,             FILE_NAME_SIZE, },
    {"RefFile",                  &cfgparams.reffile,                      1,   0.0,                       0,  0.0,              0.0,             FILE_NAME_SIZE, },
    {"WriteUV",                  &cfgparams.write_uv,                     0,   0.0,                       1,  0.0,              1.0,                             },
#endif
    {"FileFormat",               &cfgparams.FileFormat,                   0,   0.0,                       1,  0.0,              1.0,                             },
    {"RefOffset",                &cfgparams.ref_offset,                   0,   0.0,                       1,  0.0,              256.0,                             },
    {"POCScale",                 &cfgparams.poc_scale,                    0,   2.0,                       1,  1.0,              10.0,                            },
#ifdef _LEAKYBUCKET_
    {"R_decoder",                &cfgparams.R_decoder,                    0,   0.0,                       2,  0.0,              0.0,                             },
    {"B_decoder",                &cfgparams.B_decoder,                    0,   0.0,                       2,  0.0,              0.0,                             },
    {"F_decoder",                &cfgparams.F_decoder,                    0,   0.0,                       2,  0.0,              0.0,                             },
    {"LeakyBucketParamFile",     &cfgparams.LeakyBucketParamFile,         1,   0.0,                       0,  0.0,              0.0,             FILE_NAME_SIZE, },
#endif
    {"DisplayDecParams",         &cfgparams.bDisplayDecParams,            0,   1.0,                       1,  0.0,              1.0,                             },
    {"ConcealMode",              &cfgparams.conceal_mode,                 0,   0.0,                       1,  0.0,              2.0,                             },
    {"RefPOCGap",                &cfgparams.ref_poc_gap,                  0,   2.0,                       1,  0.0,              4.0,                             },
    {"POCGap",                   &cfgparams.poc_gap,                      0,   2.0,                       1,  0.0,              4.0,                             },
    {"Silent",                   &cfgparams.silent,                       0,   0.0,                       1,  0.0,              1.0,                             },
    {"IntraProfileDeblocking",   &cfgparams.intra_profile_deblocking,     0,   1.0,                       1,  0.0,              1.0,                             },
    {"DecFrmNum",                &cfgparams.iDecFrmNum,                   0,   0.0,                       2,  0.0,              0.0,                             },
#if MVC_EXTENSION_ENABLE||EXT3D
    {"DecodeAllLayers",          &cfgparams.DecodeAllLayers,              0,   0.0,                       1,  0.0,              1.0,                             },
#endif

#if EXT3D
    {"CameraFile",                &cfgparams.CameraFile,                   1,   0.0,                       0,  0.0,               0.0,     FILE_NAME_SIZE,        },

    {"3DReferenceDisplaysOutputFile", &cfgparams.RefDispOutputFile, 1,   0.0,                       0,  0.0,               0.0,     FILE_NAME_SIZE,        },
#endif
    {NULL,                       NULL,                                   -1,   0.0,                       0,  0.0,              0.0,                             },
};
#endif

#ifndef INCLUDED_BY_CONFIGFILE_C
extern Mapping Map[];
#endif
extern void JMDecHelpExit ();
extern void ParseCommand(InputParameters *p_Inp, int ac, char *av[]);



#endif

