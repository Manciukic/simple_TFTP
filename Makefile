# Compiler and flags
CC      = gcc
CFLAGS  = -g -Wall -std=c99 -O0

# Directories
OBJDIR  = build
SRCDIR  = src
BINDIR  = dist
HDRDIR  = src/include
DOCDIR  = doc
DOCTMPDIR = build/doc

# List of targets
UTILS   = fblock tftp_msgs inet_utils debug_utils tftp
TARGETS = tftp_client tftp_server

DOCPDFNAME = TFTP_documentation.pdf
DOXYGENCFG = doxygen.cfg

UTILS_OBJ = $(addsuffix .o, $(addprefix $(OBJDIR)/,$(UTILS)))

.SECONDARY: $(UTILSOBJ)

# Builds only the executables: default rule
exe: $(addprefix $(BINDIR)/,$(TARGETS))

# Build targets
$(BINDIR)/%: $(OBJDIR)/%.o $(UTILS_OBJ) $(HDRDIR)/*.h
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Build generic .o file from .c file
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDRDIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build documentation pdf
$(DOCDIR)/$(DOCPDFNAME): $(SRCDIR)/*.c $(HDRDIR)/*.h $(DOXYGENCFG)
	doxygen $(DOXYGENCFG)
	( cd $(DOCTMPDIR)/latex ; make )
	cp $(DOCTMPDIR)/latex/refman.pdf $(DOCDIR)/$(DOCPDFNAME)

# Builds everything
all: exe $(DOCDIR)/$(DOCPDFNAME)

# clean
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/* $(DOCTMPDIR)/* $(DOCDIR)/$(DOCPDFNAME)

# clean everything and then rebuild
rebuild: clean all

# just opens output documentation
doc_open: $(DOCDIR)/$(DOCPDFNAME)
	xdg-open $(DOCDIR)/$(DOCPDFNAME)

# generates documentation and opens it in default pdf viewer
doc: $(DOCDIR)/$(DOCPDFNAME) doc_open

# these targets aren't name of files
.PHONY: all exe clean rebuild doc_open doc