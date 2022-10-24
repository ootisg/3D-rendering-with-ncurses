rasterizer: rasterizer.c vector matrix
	gcc rasterizer.c vector.o matrix.o -lncurses -lm
vector: vector.c vector.h
	gcc -c vector.c
matrix: matrix.c matrix.h
	gcc -c matrix.c