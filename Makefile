#!/usr/bin/make

PROJECT_NAME	:= avsi

export ECHO		:= echo
export RM		:= rm
export GREP		:= grep
export XARGS	:= xargs
export CD		:= cd
export LS		:= ls
export CP		:= cp


# compiler
CXX			:= clang

# project basic path
DIR_ROOT	:= .
DIR_INC		:= ./inc
DIR_SRC		:= ./src
DIR_LIB		:= ./lib
DIR_OUTPUT	:= ./build
DIR_SYSEXEC := /usr/bin

# compiler flags
CXXFLAGS 	:= -Wall -g --std=c++17 -I$(DIR_INC)
LDFLAGS		:= -lstdc++ -lLLVM -lm -L/usr/lib\
#-lasan

# important file
TARGET := $(PROJECT_NAME)
SRCS := $(wildcard $(DIR_SRC)/*.cpp)
OBJS := $(patsubst $(DIR_SRC)/%.cpp, $(DIR_OUTPUT)/%.o, $(SRCS))
BIN := $(TARGET)

MAIN := $(DIR_ROOT)/main.cpp

all: CXXFLAGS += -O2
all: $(DIR_OUTPUT)/$(BIN)

release: CXXFLAGS += -O2
release: $(DIR_OUTPUT)/$(BIN)

debug: CXXFLAGS += -O0
debug: $(DIR_OUTPUT)/$(BIN)

$(DIR_OUTPUT)/$(BIN): $(DIR_OUTPUT)/main.o $(OBJS) $(DIR_LIB)/libgflags_nothreads.a
	@$(ECHO) "CXX    $<"
	@$(CXX) $(LDFLAGS) $^ -o $@

$(DIR_OUTPUT)/main.o: $(MAIN)
	@$(ECHO) "CXX    $<"
	@$(CXX) -c $(CXXFLAGS) $< -o $@

$(DIR_OUTPUT)/%.o : $(DIR_SRC)/%.cpp
	@$(ECHO) "CXX    $<"
	@$(CXX) -c $(CXXFLAGS) -I$(DIR_INC) -o $@ $<

$(DIR_OUTPUT)/%.d: $(DIR_SRC)/%.cpp
	@set -e; rm -f $@; $(CXX) -MM $< $(CXXFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(DIR_OUTPUT)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DIR_OUTPUT)/%.d: $(DIR_ROOT)/%.cpp
	@set -e; rm -f $@; $(CXX) -MM $< $(CXXFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(DIR_OUTPUT)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DIR_OUTPUT)/main.d
-include $(OBJS:.o=.d)

.PHONY: clean
clean:
	$(CD) $(DIR_OUTPUT) && $(LS) | $(GREP) -v ".gitkeep" | $(XARGS) $(RM)

.PHONY: install
install:
	@$(CP) $(DIR_OUTPUT)/$(TARGET) $(DIR_SYSEXEC)/

.PHONY: uninstall
uninstall:
	@$(RM) $(DIR_SYSEXEC)/$(TARGET)
