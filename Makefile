#!/bin/bash

MAJOR := 0
MINOR := 9
PATCH := 0
STATE := a
VERSION := $(MAJOR).$(MINOR)$(STATE)$(PATCH)
# Utilitys
RM := rm -f
CP := cp
MKDIR := mkdir -p
# Directories
DESTDIR ?=
PREFIX ?= /usr
INSTALL_LOCATION=$(DESTDIR)$(PREFIX)
# Compiler
CC ?= gcc
CFLAGS := -DARPSC_VERSION=\"$(VERSION)\"
CLIBS := -lpthread
# Sources
SRC = $(wildcard *.c)
OBJS = $(notdir $(subst .c,.o,$(SRC)))
TARGET ?= arpsc

all : $(TARGET)
	@echo -n "Finished $(TARGET)! \n"

$(TARGET) : CFLAGS += -O2 
$(TARGET) : $(OBJS)
	$(CC) $(CLFAGS) $^ -o $@ $(CLIBS)

debug : CFLAGS += -g3 -O0
debug : $(OBJS)
	$(CC) $(CLFAGS) $^ -o $(TARGET) $(CLIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

install : $(TARGET)
	@echo "Installing arpsc.\n"
	$(MKDIR) $(INSTALL_LOCATION)/bin
	$(CP) $(TARGET) $(INSTALL_LOCATION)/bin
	$(CP) arpsc.bc $(INSTALL_LOCATION)/share/bash-completion/completions/arpsc

distribution : $(TARGET)
	$(RM) -r $(TARGET)-$(VERSION)
	$(MKDIR) $(TARGET)-$(VERSION)
	$(CP) *.c *.h Makefile LICENSE $(TARGET)-$(VERSION)
	tar cf - $(TARGET)-$(VERSION) | gzip -c > $(TARGET)-$(VERSION).tar.gz
	$(RM) -r $(TARGET)-$(VERSION)

clean :
	$(RM) *.o

.PHONY : all install distribution clean debug

