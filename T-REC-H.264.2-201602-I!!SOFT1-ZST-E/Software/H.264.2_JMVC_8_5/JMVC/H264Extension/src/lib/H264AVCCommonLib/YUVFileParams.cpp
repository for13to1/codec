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


#include "YUVFileParams.h"
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/CommonDefs.h"
#include <iostream>


YUVFileParams::YUVFileParams(const std::string& fileName, const int view_id,
			     const int height, const int vertPadding, 
				 const int width, const int horPadding)
	:_fileName(fileName),_view_id(view_id), _height(height), _width(width), 
     _lumaSize( (_height + vertPadding + 2*YUV_Y_MARGIN)*(_width + horPadding + 2*YUV_X_MARGIN) ),
     _bufSize( 3 * _lumaSize / 2 ),
     _lumaOffset( (_width + horPadding + 2*YUV_X_MARGIN) * YUV_Y_MARGIN + YUV_X_MARGIN ),
     _cbOffset( (((_width+horPadding)/2) + YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 
		+ _lumaSize ),
     _crOffset( (((_width+horPadding)/2) + YUV_X_MARGIN) * YUV_Y_MARGIN/2 
		+ YUV_X_MARGIN/2 + 5*_lumaSize/4 ),
     _stride( (_width+horPadding) + 2*YUV_X_MARGIN ) {
  if (0 == height || 0 == width) {
    std::cerr 
      << "Attempted to construct a YUVFileParams object with " << std::endl
      << "either height or width of 0.  Did you make sure to " << std::endl
      << "put the AddViewRef keywords in the config file     " << std::endl
      << "AFTER the SourceWidth and SourceHeight keywords.   " << std::endl;
    abort();
  }
 
}

YUVFileParams::YUVFileParams(const YUVFileParams& other) {
  _fileName = other._fileName;
  _view_id = other._view_id;
  _height = other._height;       
  _width = other._width;
  _lumaSize = other._lumaSize;
  _bufSize = other._bufSize;
  _lumaOffset = other._lumaOffset;
  _cbOffset = other._cbOffset;
  _crOffset = other._crOffset;
  _stride = other._stride;
}
