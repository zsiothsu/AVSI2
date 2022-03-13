#!/usr/bin/make

# Package Bash shell command
export ECHO        := echo
export EECHO       := $(ECHO) -e
export RM          := rm -f
export UNAME       := uname -o -m
export MKDIR       := mkdir

# Project
PROJECT_NAME   := libavsi

# Definition of toolchain
AR                 := ar
CC                 := clang

# Path
ROOT           := $(CURDIR)
SRCDIR         := $(ROOT)
OBJDIR         := $(ROOT)/obj

# Important file
TARGET         := $(PROJECT_NAME).a
SRCS           := $(wildcard $(SRCDIR)/*.c)
OBJS           := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGET)
	@$(EECHO) =============================
	@$(EECHO) target: $(TARGET) generated
	@$(EECHO) done!
	@$(EECHO) =============================

startup: 
	@$(EECHO) =============================
	@$(EECHO) AVSI2 basic library
	@$(EECHO) target: $(TARGET)
	@$(EECHO) at `$(UNAME)`
	@$(EECHO) =============================
	@$(EECHO) start building ...

$(SRCS): startup

$(TARGET): $(OBJS)
	@@$(AR) -c -r $@ $^
	@$(EECHO) AR $(@F)

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@if [ ! -d $(OBJDIR) ]; then $(MKDIR) $(OBJDIR); fi
	@$(CC) -c $< -o $@
	@$(EECHO) CC $(<F)

.PHONY: clean
clean:
	@$(RM) -r $(OBJDIR)
	@$(RM) $(TARGET)