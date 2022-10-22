#pragma once

struct vertex {
	float x;
	float y;
};

typedef struct vertex vertex;

struct tri {
	vertex a;
	vertex b;
	vertex c;	
};

typedef struct tri tri;
