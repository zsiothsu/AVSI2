#!/usr/bin/make

ROOT			:= $(CURDIR)
CDIR			:= $(ROOT)/C
STDDIR			:= $(ROOT)/std

CDIR_OUT		:= $(CDIR)/obj
STDDIR_OUT		:= $(STDDIR)/build

LIBAVSI			:= libavsi.a
STDBC			:= std.bc


all: $(CDIR_OUT)/$(LIBAVSI) $(STDDIR_OUT)/$(STDBC)

$(CDIR_OUT)/$(LIBAVSI): $(CDIR)/avsi.c
	@echo "building libavsi"
	@cd $(CDIR) && make

$(STDDIR_OUT)/$(STDBC): $(STDDIR)/__init__.sl
	@echo "building .bc file"
	@$(ROOT)/../build/avsi $(STDDIR)/__init__.sl -m