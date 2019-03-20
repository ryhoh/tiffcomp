# Makefile for compilation of tiffcomp

# tiffcomp:	progressbar.o
# 	gcc tiffcomp.c -o $@ progressbar.o

# progressbar.o:	progressbar.c
# 	gcc progressbar.c -c

test_tiff:	test/test_tiff.cc src/tiff.cc
	g++-8 -o $@ test/test_tiff.cc src/tiff.cc \
	-lgtest -lgtest_main -lpthread \
	&& ./$@

test_tiffcomp:	test/test_tiffcomp.cc src/tiffcomp.cc
	g++-8 -o $@ test/test_tiffcomp.cc src/tiffcomp.cc \
	-Llib \
	-lprogress \
	&& ./$@

# --

clean:
	rm -f \
	test_tiff tiff.o \
	test_tiffcomp tiffcomp.o \
	.copyWriteFrom.tif .runTest.tif
