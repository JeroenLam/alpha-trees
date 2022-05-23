all: ppm edge filter distance_measures util

ppm: PPMImageReadWrite.c PPMImageReadWrite.h
	gcc -O2 -c PPMImageReadWrite.c

edge: EdgeDetection.c EdgeDetection.h
	gcc -O2 -c EdgeDetection.c

filter: TreeFilter.c TreeFilter.h
	gcc -O2 -c TreeFilter.c

distance_measures: DistanceMeasures.c DistanceMeasures.h
	gcc -O2 -c DistanceMeasures.c

util: util.c util.h
	gcc -O2 -c util.c

clean:
	rm -f *~
	rm -f *.o