a.out:	src/tiffcomp.o src/progressbar.o
	gcc -o a.out \
	src/tiffcomp.o src/progressbar.o

tiffcomp.o:	src/tiffcomp.c
	gcc -o src/$@ -c src/tiffcomp.c

progressbar.o:	src/progressbar.c
	gcc -o src/$@ -c src/progressbar.c


clean:
	rm -f src/tiffcomp.o src/progressbar.o
