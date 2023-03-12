rasterizer: rasterizer.c vector matrix tri color_mapping
	gcc -g rasterizer.c vector.o matrix.o tri.o color_mapping.o -lncurses -lm
vector: vector.c vector.h
	gcc -c -g vector.c
matrix: matrix.c matrix.h
	gcc -c -g matrix.c
tri: tri.c tri.h
	gcc -c -g tri.c
color_mapping: color_mapping.c color_mapping.h
	gcc -c -g color_mapping.c
