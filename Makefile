# Compiler and flags
CC      = gcc
CFLAGS  = -g -Wall -std=c99 -O0

# Directories
OBJDIR  = build
SRCDIR  = src
BINDIR  = dist
HDRDIR  = src/include

# List of targets
UTILS   = fblock tftp_msgs inet_utils debug_utils tftp
TARGETS = tftp_client tftp_server

UTILS_OBJ = $(addsuffix .o, $(addprefix $(OBJDIR)/,$(UTILS)))

all: $(addprefix $(BINDIR)/,$(TARGETS))

.SECONDARY: $(UTILSOBJ)

# Build targets
$(BINDIR)/%: $(OBJDIR)/%.o $(UTILS_OBJ) $(HDRDIR)/*.h
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Build generic .o file from .c file
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDRDIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

# clean
.PHONY: clean
clean:
	rm -f $(OBJDIR)/* $(BINDIR)/*
