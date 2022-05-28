#!/usr/bin/make

ROOT			:= $(CURDIR)
CDIR			:= $(ROOT)/C
STDDIR			:= $(ROOT)/std

CDIR_OUT		:= $(CDIR)/obj
STDDIR_OUT		:= $(STDDIR)/build

LIBAVSI			:= libavsi.a
STDBC			:= std/std.bc


all:  $(STDDIR_OUT)/$(STDBC) $(CDIR)/$(LIBAVSI)

$(CDIR)/$(LIBAVSI): $(CDIR)/avsi.c $(STDDIR_OUT)/$(STDBC)
	@echo "building libavsi"
	@make -C $(CDIR)

$(STDDIR_OUT)/$(STDBC): $(STDDIR)/__init__.sl
	@echo "building .bc file"
	@$(ROOT)/../build/avsi $(STDDIR)/__init__.sl -m -o $(STDDIR_OUT)


.PHONY: clean
clean:
	@rm -r $(CDIR_OUT)
	@rm $(CDIR)/$(LIBAVSI)
	@rm -r $(STDDIR_OUT)

.PHONY: install
install:
	@mkdir /usr/include/avsi
	@cp -r $(STDDIR_OUT)/std /usr/include/avsi/std
	@cp $(CDIR)/$(LIBAVSI) /usr/lib/$(LIBAVSI)

.PHONY: uninstall
uninstall:
	@rm -r /usr/include/avsi
	@cp $(CDIR)/$(LIBAVSI) /usr/lib/$(LIBAVSI)
	@rm /usr/lib/$(LIBAVSI)