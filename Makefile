# Makefile for compilation of tiffcomp

# tiffcomp:	progressbar.o
# 	gcc tiffcomp.c -o $@ progressbar.o

# progressbar.o:	progressbar.c
# 	gcc progressbar.c -c

test_tiff:	test/test_tiff.cc src/tiff.cc
	g++-8 -o $@ test/test_tiff.cc src/tiff.cc \
	-Llib \
	-lgtest -lgtest_main -lpthread -lprogress \
	&& ./$@

# Tiff.o:	src/Tiff.cc
# 	g++-8 src/Tiff.cc -o $@

clean:
	rm -f test_tiff tiff.o .copyWriteFrom.tif
