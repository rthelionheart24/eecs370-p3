# Makefile
# Build rules for EECS 370 P1/P3

# Compiler
CXX = gcc

# Compiler flags (including debug info)
CXXFLAGS = -std=c99 -Wall -Werror -g3
LINKFLAGS = -lm
# -std=c99 restricts us to using C and not C++
# -lm links with libm, which includes math.h (maybe used in P4)
# -Wall and -Werror catch extra warnings as errors to decrease the chance of undefined behaviors on CAEN
# -g3 or -g includes debug info for gdb

# Compile Assembler
assembler: assembler.c
	$(CXX) $(CXXFLAGS) $< -o assembler

# Compile Assembler
simulator: simulator.c
	$(CXX) $(CXXFLAGS) $< -o simulator

# Compile any C program
%.exe: %.c
	$(CXX) $(CXXFLAGS) $< -o $@

# Assemble an LC2K file into Machine Code
%.mc: %.as assembler
	./assembler $< $@

# Simulate a machine code program to a file
%.out: %.mc simulator
	./simulator $< > $@

# Compare output to a *.mc.correct or *.out.correct file
%.diff: % %.correct
	diff $^ > $@

# Compare output to a *.mc.correct or *.out.correct file with full output
%.sdiff: % %.correct
	sdiff $^ > $@

# Remove anything created by a makefile
clean:
	rm -f *.obj *.mc *.out *.exe *.diff *.sdiff assembler simulator