#!/usr/bin/make

PROJECT_NAME	:= Interpreter

# compiler
CXX			:= g++

# project basic path
DIR_ROOT	:= .
DIR_INC		:= ./inc
DIR_SRC		:= ./src
DIR_OUTPUT	:= ./build

# compiler flags
CXXFLAGS	:= -Wall -g -O2 --std=c++17 -I$(DIR_INC) \
#-Wextra \
#-fno-omit-frame-pointer \
#-fsanitize=address \
#-fsanitize-recover=address

LDFLAGS		:= -lstdc++ -L/usr/lib \
#-lasan

# important file
TARGET := $(PROJECT_NAME)
SRCS := $(wildcard $(DIR_SRC)/*.cpp)
OBJS := $(patsubst $(DIR_SRC)/%.cpp, $(DIR_OUTPUT)/%.o, $(SRCS))
BIN := $(TARGET)

MAIN := $(DIR_ROOT)/main.cpp

all: $(DIR_OUTPUT)/$(BIN)

$(DIR_OUTPUT)/$(BIN): $(DIR_OUTPUT)/main.o $(OBJS)
	@$(ECHO) "CXX    $<"
	@$(CXX) $(LDFLAGS) $^ -o $@

$(DIR_OUTPUT)/main.o: $(MAIN)
	@$(ECHO) "CXX    $<"
	@$(CXX) -c $(CXXFLAGS) $< -o $@

$(DIR_OUTPUT)/%.o : $(DIR_SRC)/%.cpp
	@$(ECHO) "CXX    $<"
	@$(CXX) -c $(CXXFLAGS) -I$(DIR_INC) -o $@ $<

#$(TARGET): $(SRC_FILES) $(MAIN_FILE)
#	@$(ECHO) "CXX    $<"
#	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(DIR_INC) -o $@ $<

.PHONY: clean

clean:
	cd $(DIR_OUTPUT) && ls | grep -v ".gitkeep" | xargs rm

