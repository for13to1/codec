all:
	make -C lib/H264AVCVideoIoLib
	make -C lib/H264AVCCommonLib
	make -C lib/H264AVCDecoderLib
	make -C lib/H264AVCEncoderLib
	make -C test/H264AVCDecoderLibTest
	make -C test/H264AVCEncoderLibTest
	make -C test/MVCBitStreamAssembler
	make -C test/MVCBitStreamExtractor
	make -C tools/DownConvert
	make -C tools/PSNR


debug:
	make -C lib/H264AVCVideoIoLib debug
	make -C lib/H264AVCCommonLib debug
	make -C lib/H264AVCDecoderLib debug
	make -C lib/H264AVCEncoderLib debug
	make -C test/H264AVCDecoderLibTest debug
	make -C test/H264AVCEncoderLibTest debug
	make -C test/MVCBitStreamAssembler debug
	make -C test/MVCBitStreamExtractor debug
	make -C tools/DownConvert debug
	make -C tools/PSNR debug

release:
	make -C lib/H264AVCVideoIoLib release
	make -C lib/H264AVCCommonLib release
	make -C lib/H264AVCDecoderLib release
	make -C lib/H264AVCEncoderLib release
	make -C test/H264AVCDecoderLibTest release
	make -C test/H264AVCEncoderLibTest release
	make -C test/MVCBitStreamAssembler release
	make -C test/MVCBitStreamExtractor release
	make -C tools/DownConvert release
	make -C tools/PSNR release

clean:
	make -C lib/H264AVCVideoIoLib clean
	make -C lib/H264AVCCommonLib clean
	make -C lib/H264AVCDecoderLib clean
	make -C lib/H264AVCEncoderLib clean
	make -C test/H264AVCDecoderLibTest clean
	make -C test/H264AVCEncoderLibTest clean
	make -C test/MVCBitStreamAssembler clean
	make -C test/MVCBitStreamExtractor clean
	make -C tools/DownConvert clean
	make -C tools/PSNR clean
