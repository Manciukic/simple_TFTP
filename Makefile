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
UTILS   = fblock tftp_msgs inet_utils debug_utils tftp netascii
TARGETS = tftp_client tftp_server

# Documentation output
DOCPDFNAME = TFTP_documentation.pdf

# Documentation config file
DOXYGENCFG = doxygen.cfg

# Test files
TESTS = 0.txt 4.txt 512.txt 513.txt 62836.txt 131073.txt

# Object files for utilities (aka libraries)
UTILS_OBJ = $(addsuffix .o, $(addprefix $(OBJDIR)/,$(UTILS)))

# Builds only the executables: default rule
exe: $(addprefix $(BINDIR)/,$(TARGETS))

# Utilities are secondary targets
.SECONDARY: $(UTILSOBJ)

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

# Builds everything (ecutables and documentation)
all: exe $(DOCDIR)/$(DOCPDFNAME)

# clean everything
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/* $(DOCTMPDIR)/* $(DOCDIR)/$(DOCPDFNAME)

# clean everything and then rebuild
rebuild: clean all

# just opens output documentation
doc_open: $(DOCDIR)/$(DOCPDFNAME)
	xdg-open $(DOCDIR)/$(DOCPDFNAME)

# generates documentation and opens it in default pdf viewer
doc: $(DOCDIR)/$(DOCPDFNAME) doc_open

# runs tests
# 1) removes old test files
# 2) runs server in background
# 3) downloads files in binary mode
# 4) downloads files in text mode
# 5) kills the server since it isn't needed anymore
# 6) compares bytes in binary outputs with original files
# 7) compares text in text outputs with original files 
# if there are no errors, there should be no output after every "Comparing..." line
test: exe
	rm test/test_*
	dist/tftp_server 9999 test &
	for test in $(TESTS); do echo "--- $$test ---"; printf "!get $$test test/test_bin_$$test\n!quit\n" | dist/tftp_client 127.0.0.1 9999; done
	for test in $(TESTS); do echo "--- $$test ---"; printf "!mode txt\n!get $$test test/test_txt_$$test\n!quit\n" | dist/tftp_client 127.0.0.1 9999; done
	pkill tftp_server
	@for test in $(TESTS); do echo "Comparing $$test ($$(stat --printf="%s" test/$$test)) test_bin_$$test ($$(stat --printf="%s" test/test_bin_$$test))"; cmp test/$$test test/test_bin_$$test; done
	@for test in $(TESTS); do echo "Comparing $$test ($$(stat --printf="%s" test/$$test)) test_txt_$$test ($$(stat --printf="%s" test/test_txt_$$test))"; diff test/$$test test/test_txt_$$test; done

help:
	@echo "all:         builds everything (both binaries and documentation)"
	@echo "clean:       deletes any intermediate or output file in build/, dist/ and doc/"
	@echo "doc:         builds documentation only and opens pdf file"
	@echo "doc_open:    opens documentation pdf"
	@echo "exe:         builds only binaries"
	@echo "help:        shows this message"
	@echo "rebuild:     same as calling clean and then all"
	@echo "test:        runs tests"

# these targets aren't name of files
.PHONY: all exe clean rebuild doc_open doc test help