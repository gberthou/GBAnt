BINDIR=bin
OBJDIR=obj
SRCDIR=src

BINNAME=gbaemu
BIN=$(BINDIR)/$(BINNAME)
CLEAN=clean

DEBUGDIR=Debug
RELEASEDIR=Release

DEBUG=debug
RELEASE=release

GPP=g++
CFLAGS=-Wall -Wextra -Werror

INCLUDE=
LIB=

CFILES=$(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/core/*.cpp)

OBJ=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CFILES))

OBJDEBUG=$(addprefix $(DEBUGDIR)/,$(OBJ))
OBJRELEASE=$(addprefix $(RELEASEDIR)/,$(OBJ))

BINDEBUG=$(addprefix $(DEBUGDIR)/,$(BIN))
BINRELEASE=$(addprefix $(RELEASEDIR)/,$(BIN))

FLAGSDEBUG=-g -pg
FLAGSRELEASE=-O2
LIBS=

.PHONY: build
build:
	mkdir -p $(DEBUGDIR) $(DEBUGDIR)/$(BINDIR) $(DEBUGDIR)/$(OBJDIR)
	mkdir -p $(RELEASEDIR) $(RELEASEDIR)/$(BINDIR) $(RELEASEDIR)/$(OBJDIR)

$(DEBUG): $(OBJDEBUG)
	$(GPP) -o $(BINDEBUG) $(OBJDEBUG) $(LIB) $(LIBS)

$(RELEASE): $(OBJRELEASE)
	$(GPP) -o $(BINRELEASE) $(OBJRELEASE) $(LIB) $(LIBS)

$(CLEAN):
	rm -f $(BINDEBUG) $(BINRELEASE) $(OBJDEBUG) $(OBJRELEASE)

$(DEBUGDIR)/$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(GPP) $(CFLAGS) $(FLAGSDEBUG) $(INCLUDE) -c -o $@ $<

$(RELEASEDIR)/$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(GPP) $(CFLAGS) $(FLAGSRELEASE) $(INCLUDE) -c -o $@ $<

