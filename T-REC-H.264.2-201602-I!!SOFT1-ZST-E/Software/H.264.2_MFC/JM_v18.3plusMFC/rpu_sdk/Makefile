all: buildall 

buildall:
	(cd rpu_kernel/make/linux/; $(MAKE))
	(cd utilities/make/linux/;$(MAKE))
	(cd encoder_layer/make/linux/;$(MAKE))
	(cd decoder_layer/make/linux/;$(MAKE))
clean:
	(cd rpu_kernel/make/linux/; $(MAKE) clean_all)
	(cd utilities/make/linux/;$(MAKE) clean_all)
	(cd encoder_layer/make/linux/;$(MAKE) clean_all)
	(cd decoder_layer/make/linux/;$(MAKE) clean_all)
