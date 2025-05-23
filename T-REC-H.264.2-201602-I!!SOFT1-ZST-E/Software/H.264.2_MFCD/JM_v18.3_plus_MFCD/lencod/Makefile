###
###     Makefile for H.264/AVC encoder
###
###             generated for UNIX/LINUX environments
###             by H. Schwarz, Limin Wang
###



NAME=   lencod

### include debug information: 1=yes, 0=no
DBG?= 0 
### Generate 32 bit executable : 1=yes, 0=no
M32?= 0
### include O level optimization : 0-3
OPT?= 3
### Static Compilation
STC?= 0
### OPENMP support : 1=yes, 0=no
OPENMP?= 1

DEPEND= dependencies

BINDIR= ../bin
INCDIR= inc
SRCDIR= src
OBJDIR= obj

ADDSRCDIR= ../lcommon/src
ADDINCDIR= ../lcommon/inc -I../rpu_sdk/encoder_layer/inc -I../rpu_sdk/rpu_kernel/inc -I../rpu_sdk/utilities/inc 

ifeq ($(STC),1)
ifeq ($(DBG),1)  ### Do not use static compilation for Debug mode
STC=0
STATIC=
else
STATIC= -static
endif
else
STATIC= 
endif

LIBS=   -lm  -L../rpu_sdk/encoder_layer/lib/linux/ -L../rpu_sdk/rpu_kernel/lib/linux -L../rpu_sdk/utilities/lib/linux  -lc3d_encoder_layer -lc3d_rpu_kernel  -lc3d_utilities $(STATIC)
CFLAGS= -D USEMMX -std=gnu99 -pedantic -ffloat-store -fno-strict-aliasing -fsigned-char $(STATIC)
FLAGS=  $(CFLAGS) -Wall -I$(INCDIR) -I$(ADDINCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64

ARCH         := $(shell arch)

ifeq ($(ARCH),i686)
CFLAGS      += -march=pentium4
endif

ifeq ($(ARCH),x86_64)
CFLAGS      += -march=athlon64 
endif

ifeq ($(M32),1)
FLAGS+=-m32
endif

ifeq ($(OPENMP),1)
FLAGS+=-fopenmp
endif


OPT_FLAG = -O$(OPT)
ifeq ($(DBG),1)
SUFFIX= .dbg
FLAGS+= -g
else
SUFFIX=
FLAGS+= $(OPT_FLAG)
endif

OBJSUF= .o$(SUFFIX)

SRC=    $(wildcard $(SRCDIR)/*.c) 
ADDSRC= $(wildcard $(ADDSRCDIR)/*.c)
OBJ=    $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o$(SUFFIX)) $(ADDSRC:$(ADDSRCDIR)/%.c=$(OBJDIR)/%.o$(SUFFIX)) 
BIN=    $(BINDIR)/$(NAME)$(SUFFIX).exe

.PHONY: default distclean clean tags depend

default: messages objdir_mk depend bin 

messages:
ifeq ($(M32),1)
	@echo 'Compiling with M32 support...'
endif
ifeq ($(DBG),1)
	@echo 'Compiling with Debug support...'
	@echo 'Note static compilation not supported in this mode.'
endif
ifeq ($(STC),1)
	@echo 'Compiling with -static support...'
endif
ifeq ($(OPENMP),1)
	@echo 'Compiling with -fopenmp support...'
endif

clean:
	@echo remove all objects
	@rm -rf $(OBJDIR)

distclean: clean
	@rm -f $(DEPEND) tags
	@rm -f $(BIN)

tags:
	@echo update tag table
	@ctags inc/*.h src/*.c

bin:    $(OBJ)
	@echo
	@echo 'creating binary "$(BIN)"'
	@$(CC) $(FLAGS) -o $(BIN) $(OBJ) $(LIBS)
	@echo '... done'
	@echo

depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) $(FLAGS) -MM $(CFLAGS) -I$(INCDIR) -I$(ADDINCDIR) $(SRC) $(ADDSRC)                  \
         | sed '\''s@\(.*\)\.o[ :]@$(OBJDIR)/\1.o$(SUFFIX):@g'\''               \
         >$(DEPEND)'
	@echo

$(OBJDIR)/%.o$(SUFFIX): $(SRCDIR)/%.c
	@echo 'compiling object file "$@" ...'
	@echo '@$(CC) -c -o $@ $(FLAGS) $<'
	@$(CC) -c -o $@ $(FLAGS) $<

$(OBJDIR)/%.o$(SUFFIX): $(ADDSRCDIR)/%.c
	@echo 'compiling object file "$@" ...'
	@$(CC) -c -o $@ $(FLAGS) $<

objdir_mk:
	@echo 'Creating $(OBJDIR) ...'
	@mkdir -p $(OBJDIR)

-include $(DEPEND)

