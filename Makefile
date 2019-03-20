# Makefile for compilation of tiffcomp

# tiffcomp:	progressbar.o
# 	gcc tiffcomp.c -o $@ progressbar.o

# progressbar.o:	progressbar.c
# 	gcc progressbar.c -c

testTiff:	test/testTiff.cc src/Tiff.cc
	g++-8 -o $@ test/testTiff.cc src/Tiff.cc \
	-lgtest -lgtest_main -lpthread \
	&& ./$@

# Tiff.o:	src/Tiff.cc
# 	g++-8 src/Tiff.cc -o $@

clean:
	rm -f testTiff Tiff.o
