#!/usr/bin/make

ROOT			:= $(CURDIR)
CDIR			:= $(ROOT)/C
STDDIR			:= $(ROOT)/std

CDIR_OUT		:= $(CDIR)/obj
STDDIR_OUT		:= $(STDDIR)/build

LIBAVSI			:= libavsi.a
STDBC			:= std/std.bc

CC				:= $(ROOT)/../build/avsi

SYS_INCLUDE		:= /usr/include
SYS_LIB			:= /usr/lib

all:  $(STDDIR_OUT)/$(STDBC) $(CDIR)/$(LIBAVSI)

$(CDIR)/$(LIBAVSI): $(CDIR)/avsi.c $(STDDIR_OUT)/$(STDBC)
	@echo "building libavsi"
	@make all -C $(CDIR)

$(STDDIR_OUT)/$(STDBC): $(STDDIR)/__init__.sl
	@echo "building .bc file"
	@$(CC) $(STDDIR)/__init__.sl -m -o $(STDDIR_OUT)


.PHONY: clean
clean:
	@rm -f -r $(CDIR_OUT)
	@rm -f $(CDIR)/$(LIBAVSI)
	@rm -f -r $(STDDIR_OUT)

.PHONY: install
install:
	@if [ ! -d $(SYS_INCLUDE)/avsi ]; then mkdir $(SYS_INCLUDE)/avsi; fi
	@cp -r $(STDDIR_OUT)/std $(SYS_INCLUDE)/avsi/std
	@cp $(CDIR)/$(LIBAVSI) $(SYS_LIB)/$(LIBAVSI)

.PHONY: uninstall
uninstall:
	@rm -f -r /usr/include/avsi
	@cp $(CDIR)/$(LIBAVSI) $(SYS_LIB)/$(LIBAVSI)
	@rm -f $(SYS_LIB)/$(LIBAVSI)