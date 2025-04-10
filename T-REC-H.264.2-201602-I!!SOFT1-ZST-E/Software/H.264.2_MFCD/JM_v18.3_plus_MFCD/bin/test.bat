REM case1: full resolution progressive texture and depth
lencod_Release_x64.exe -d encoder_mfc_texture.cfg -depd encoder_mfc_depth.cfg
ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=0 
md5sum *.yuv

pause;


ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=1 
pause;

REM case2: full resolution progressive texture and half depth
lencod_Release_x64.exe -d encoder_mfc_texture.cfg -depd encoder_mfc_sub_depth.cfg
ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=0 
md5sum *.yuv

pause;

ldecod_Release_x64.exe -d decoder_mfc_depth.cfg -p ExportViews=1 

pause;

