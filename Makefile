#!/usr/bin/make

PROJECT_NAME	:= avsi

export ECHO		:= echo
export RM		:= rm
export GREP		:= grep
export XARGS	:= xargs
export CD		:= cd
export LS		:= ls
export CP		:= cp
export INSTALL	:= install

# compiler
CXX			:= clang

# project basic path
DIR_ROOT	:= .
DIR_INC		:= $(DIR_ROOT)/inc
DIR_SRC		:= $(DIR_ROOT)/src
DIR_LIB		:= $(DIR_ROOT)/lib
DIR_OUTPUT	:= $(DIR_ROOT)/build
DIR_LIBAVSI	:= $(DIR_ROOT)/libavsi
DIR_CART	:= $(DIR_ROOT)/cart
DIR_SYSEXEC := /usr/bin

# compiler flags
CXXFLAGS 	:= -Wall --std=c++17 -I$(DIR_INC) \
-ffunction-sections \
-fdata-sections \
-I /usr/lib/llvm14/include

LDFLAGS		:= -lstdc++ -lm -L/usr/lib -L /usr/lib/llvm14/lib -lLLVM-14

# important file
TARGET		:= $(PROJECT_NAME)
SRCS		:= $(wildcard $(DIR_SRC)/*.cpp)
OBJS		:= $(patsubst $(DIR_SRC)/%.cpp, $(DIR_OUTPUT)/%.o, $(SRCS))
BIN			:= $(TARGET)
LIBAVSI		:= $(DIR_LIBAVSI)/C/libavsi.a $(DIR_LIBAVSI)/std/build/std/std.bc
CART		:= $(DIR_CART)/build/cart

MAIN := $(DIR_ROOT)/main.cpp

all: CXXFLAGS += -Os
all: $(DIR_OUTPUT)/$(BIN) $(LIBAVSI) $(CART)

release: CXXFLAGS += -Os
release: $(DIR_OUTPUT)/$(BIN)

debug: CXXFLAGS += -O0 -g
debug: $(DIR_OUTPUT)/$(BIN)

library: $(LIBAVSI)

cart: $(CART)

$(CART): $(DIR_CART)/main.cpp
	@$(CD) $(DIR_CART) && make all

$(LIBAVSI): $(DIR_LIBAVSI)/C/Makefile $(DIR_LIBAVSI)/std/__init__.sl $(DIR_OUTPUT)/$(BIN)
	@$(CD) $(DIR_LIBAVSI) && make all

$(DIR_OUTPUT)/$(BIN): $(DIR_OUTPUT)/main.o $(OBJS)
	@$(ECHO) "CXX    $<"
	@$(CXX) $(LDFLAGS) $^ -o $@
	@chmod +x $(DIR_OUTPUT)/$(BIN)

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
	@$(CD) $(DIR_LIBAVSI) && make clean
	@$(CD) $(DIR_CART) && make clean

.PHONY: libclean
libclean:
	@$(CD) $(DIR_LIBAVSI) && make clean

.PHONY: cartclean
cartclean:
	@$(CD) $(DIR_CART) && make clean

.PHONY: install
install:
	@$(INSTALL) $(DIR_OUTPUT)/$(TARGET) $(DIR_SYSEXEC)/
	@$(CD) $(DIR_LIBAVSI) && make install
	@$(CD) $(DIR_CART) && make install

.PHONY: uninstall
uninstall:
	@$(RM) $(DIR_SYSEXEC)/$(TARGET)
	@$(CD) $(DIR_LIBAVSI) && make uninstall
	@$(CD) $(DIR_CART) && make uninstall
