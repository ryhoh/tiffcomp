# Makefile for compilation of compositeProcess

test_tiff:	test/test_tiff.cc src/tiff.o
	g++-8 -o $@ test/test_tiff.cc src/tiff.o \
	-lgtest -lgtest_main -lpthread \
	&& ./$@

test_compositeProcess:	test/test_compositeProcess.cc src/compositeProcess.o src/tiff.o
	g++-8 -o $@ \
	test/test_compositeProcess.cc src/compositeProcess.o src/tiff.o \
	-Llib \
	-lgtest -lgtest_main -lpthread -lprogress\
	&& ./$@

# --

compositeProcess.o:	src/compositeProcess.cc
	g++-8 -o src/$@ -c src/compositeProcess.cc

tiff.o:	src/tiff.cc
	g++-8 -o src/$@ -c src/tiff.cc

# --

clean:
	rm -f \
	test_tiff test_compositeProcess \
	src/tiff.o src/compositeProcess.o \
	.copyWriteFrom.tif .setUpTest.tif .runTest.tif
