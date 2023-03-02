rasterizer: rasterizer.c vector matrix tri color_mapping
	gcc rasterizer.c vector.o matrix.o tri.o color_mapping.o -lncurses -lm
vector: vector.c vector.h
	gcc -c vector.c
matrix: matrix.c matrix.h
	gcc -c matrix.c
tri: tri.c tri.h
	gcc -c tri.c
color_mapping: color_mapping.c color_mapping.h
	gcc -c color_mapping.c