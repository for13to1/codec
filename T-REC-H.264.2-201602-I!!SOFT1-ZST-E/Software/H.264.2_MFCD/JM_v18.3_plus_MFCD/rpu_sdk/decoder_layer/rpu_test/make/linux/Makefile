#
# Makefile for Decoder layer test app and Demux 3D test app
#

# Project names
DEC_LYR_TEST_APP=c3d_decoder_layer_test_app
DEMUX_3D_TEST_APP=c3d_demux_3D_test_app

# Build Decoder layer, Demux 3D and RPU kernel libraries
SUBDIRS:=../../../../rpu_kernel/make/linux ../../../make/linux

# Additional library directories
LIBDIR_RPU_KRNL=../../../../rpu_kernel/lib/linux
LIBDIR_DEC_LYR=../../../lib/linux
LIBDIR_EXPAT=../../lib/linux

# Directories
BINDIR=../../bin/linux
INCDIR1_DEC_LYR=../../inc
INCDIR2_DEC_LYR=../../../inc
INCDIR_RPU_KRNL=../../../../rpu_kernel/inc
SRCDIR=../../src
OBJDIR=obj

OBJSUF=.o

# Files
SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%$(OBJSUF))

# Application names including the paths
BIN_DEC_LYR_TEST_APP=$(BINDIR)/$(DEC_LYR_TEST_APP)
BIN_DEMUX_3D_TEST_APP=$(BINDIR)/$(DEMUX_3D_TEST_APP)

CFLAGS=-Wall -I$(INCDIR1_DEC_LYR) -I$(INCDIR2_DEC_LYR) -I$(INCDIR_RPU_KRNL) -O3

.PHONY: all build $(SUBDIRS) objdir_mk bindir_mk clean_all clean

all: $(DEC_LYR_TEST_APP) $(DEMUX_3D_TEST_APP) Makefile

$(DEC_LYR_TEST_APP): build
	@echo
	@echo 'Creating binary: $(BIN_DEC_LYR_TEST_APP)'
	@$(CC) $(CFLAGS) -o $(BIN_DEC_LYR_TEST_APP) $(OBJDIR)/c3d_xml_parser.o $(OBJDIR)/c3d_decoder_layer_test.o -L$(LIBDIR_EXPAT) -L$(LIBDIR_DEC_LYR) -L$(LIBDIR_RPU_KRNL) -lexpat -lc3d_decoder_layer -lc3d_rpu_kernel
	@echo '... done'

$(DEMUX_3D_TEST_APP): build
	@echo	
	@echo 'Creating binary: $(BIN_DEMUX_3D_TEST_APP)'
	@$(CC) $(CFLAGS) -o $(BIN_DEMUX_3D_TEST_APP) $(OBJDIR)/c3d_decoder_layer_demux3D_test.o -L$(LIBDIR_RPU_KRNL) -L$(LIBDIR_DEC_LYR) -lc3d_rpu_kernel -lc3d_demux_3D
	@echo '... done'

# Build an object file for each source file
$(OBJDIR)/%$(OBJSUF): $(SRCDIR)/%.c
	@echo 'Compiling object file: $@'
	@$(CC) -c -o $@ $(CFLAGS) $<

$(SUBDIRS):
	$(MAKE) -C $@

build: $(SUBDIRS) objdir_mk bindir_mk $(OBJ)

objdir_mk:
	@echo 'Creating directory: $(OBJDIR)'	
	@mkdir -p $(OBJDIR)

bindir_mk:
	@echo 'Creating directory: $(BINDIR)'
	@mkdir -p $(BINDIR)

clean:
	@echo 'Removing all temporary files'
	@rm -rf $(OBJDIR)	
	@for i in $(SUBDIRS); do make -C $$i $@; done

clean_all: clean
	@echo 'Removing all files'
	@for i in $(SUBDIRS); do make -C $$i $@; done
	@rm -rf $(BINDIR)

