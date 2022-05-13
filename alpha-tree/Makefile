all: build_sub_dirs build_project

build_sub_dirs:
	$(MAKE) -C util
	$(MAKE) -C source
	gcc -O2 -c main.c

build_project: util
	gcc util/PPMImageReadWrite.o util/EdgeDetection.o util/TreeFilter.o source/EdgeQueue.o source/SalienceTree.o main.o -o saliencetree

clean:
	rm -f *~
	rm -f *.o
	rm -f util/*.o
	rm -f source/*.o
	rm saliencetree