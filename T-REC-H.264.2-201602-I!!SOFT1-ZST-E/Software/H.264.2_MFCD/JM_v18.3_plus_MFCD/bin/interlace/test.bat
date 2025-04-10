REM case 1: texture interlace and depth interlace
..\lencod_Release_x64.exe -d encoder_mfc_interlace_texture.cfg -depd encoder_mfc_interlace_depth.cfg
..\ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=0 
md5sum *.yuv

pause;

..\ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=1

pause 


REM case 2: texture interlace and depth progressive 
..\lencod_Release_x64.exe -d encoder_mfc_interlace_texture.cfg -depd encoder_mfc_progressive_depth.cfg
..\ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=0 
md5sum *.yuv

pause;

..\ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=1

pause 