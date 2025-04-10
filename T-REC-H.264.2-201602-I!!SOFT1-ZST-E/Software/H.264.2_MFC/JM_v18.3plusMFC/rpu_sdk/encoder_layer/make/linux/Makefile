#
#  Makefile for building Decoder layer and Demux 3D libraries
#

# Project names
ENC_LYR=c3d_encoder_layer

AR=ar

# c = Suppresses the message ar normally prints when it creates a new archive file.
# r = Replaces or adds file to archive.
# s = Regenerates the symbol table regardless of whether the command modifies the archive.
# v = Gives verbose output.
ARFLAGS=crvs

# Directories
LIBDIR=../../lib/linux
INCDIR_ENC_LYR=../../inc
INCDIR_RPU_KRNL=../../../rpu_kernel/inc
INCDIR_UTILITIES=../../../utilities/inc
SRCDIR=../../src
OBJDIR=obj

OBJSUF=.o
PREFIX=lib

# Source and header files
SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%$(OBJSUF))

# Library names, including the path
LIB_ENC_LYR=$(LIBDIR)/$(PREFIX)$(ENC_LYR).a

CFLAGS=-Wall -I$(INCDIR_ENC_LYR) -I$(INCDIR_RPU_KRNL) -I$(INCDIR_UTILITIES) -O3

ifeq ($(M32),1)
CFLAGS+=-m32
endif


.PHONY: all build objdir_mk libdir_mk clean_all clean

all: $(ENC_LYR) Makefile

$(ENC_LYR): build
	@echo
	@echo 'Creating library: $(LIB_ENC_LYR)'
	@$(AR) $(ARFLAGS) $(LIB_ENC_LYR) $(OBJDIR)/*.o
	@echo '... done'

# Build an object file for each source file
$(OBJDIR)/%$(OBJSUF): $(SRCDIR)/%.c
	@echo 'Compiling object file: $@'
	@$(CC) -c -o $@ $(CFLAGS) $<

build: objdir_mk libdir_mk $(OBJ)

objdir_mk:
	@echo 'Creating directory: $(OBJDIR)'
	@mkdir -p $(OBJDIR)

libdir_mk:
	@echo 'Creating directory: $(LIBDIR)'
	@mkdir -p $(LIBDIR)

clean_all: clean
	@echo 'Removing library files'
	@rm -f $(LIB_ENC_LYR)

clean:
	@echo 'Removing object files'
	@rm -rf $(OBJDIR)

