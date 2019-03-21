# Makefile for compilation of tiffcomp

test_tiff:	test/test_tiff.cc src/tiff.o
	g++-8 -o $@ test/test_tiff.cc src/tiff.o \
	-lgtest -lgtest_main -lpthread \
	&& ./$@

test_tiffcomp:	test/test_tiffcomp.cc src/tiffcomp.o src/tiff.o
	g++-8 -o $@ \
	test/test_tiffcomp.cc src/tiffcomp.o src/tiff.o \
	-Llib \
	-lgtest -lgtest_main -lpthread -lprogress\
	&& ./$@

# --

tiffcomp.o:	src/tiffcomp.cc
	g++-8 -o src/$@ -c src/tiffcomp.cc

tiff.o:	src/tiff.cc
	g++-8 -o src/$@ -c src/tiff.cc

# --

clean:
	rm -f \
	test_tiff test_tiffcomp \
	src/tiff.o src/tiffcomp.o \
	.copyWriteFrom.tif .setUpTest.tif .runTest.tif
