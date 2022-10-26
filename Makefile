rasterizer: rasterizer.c vector matrix tri
	gcc rasterizer.c vector.o matrix.o tri.o -lncurses -lm
vector: vector.c vector.h
	gcc -c vector.c
matrix: matrix.c matrix.h
	gcc -c matrix.c
tri: tri.c tri.h
	gcc -c tri.c
