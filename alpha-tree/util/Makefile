util: ppm edge filter

ppm: PPMImageReadWrite.c PPMImageReadWrite.h
	gcc -O2 -c PPMImageReadWrite.c

edge: EdgeDetection.c EdgeDetection.h
	gcc -O2 -c EdgeDetection.c

filter: TreeFilter.c TreeFilter.h
	gcc -O2 -c TreeFilter.c

clean:
	rm -f *~
	rm -f *.o