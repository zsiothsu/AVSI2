#!/usr/bin/make

PROJECT_NAME	:= Interpreter

export ECHO		:= echo
export RM		:= rm
export GREP		:= grep
export XARGS	:= xargs
export CD		:= cd
export LS		:= ls


# compiler
CXX			:= g++

# project basic path
DIR_ROOT	:= .
DIR_INC		:= ./inc
DIR_SRC		:= ./src
DIR_LIB		:= ./lib
DIR_OUTPUT	:= ./build

# compiler flags
CXXFLAGS	:= -Wall -g -O2 --std=c++17 -I$(DIR_INC) \
#-Wextra \
#-fno-omit-frame-pointer \
#-fsanitize=address \
#-fsanitize-recover=address

LDFLAGS		:= -lstdc++ -L/usr/lib\
#-lasan

# important file
TARGET := $(PROJECT_NAME)
SRCS := $(wildcard $(DIR_SRC)/*.cpp)
OBJS := $(patsubst $(DIR_SRC)/%.cpp, $(DIR_OUTPUT)/%.o, $(SRCS))
BIN := $(TARGET)

MAIN := $(DIR_ROOT)/main.cpp

all: $(DIR_OUTPUT)/$(BIN)

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

