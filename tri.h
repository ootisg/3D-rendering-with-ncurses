#pragma once

#include "vector.h"

struct tri {
	v3 a;
	v3 b;
	v3 c;
	v3 n;
};

typedef struct tri tri;

tri* inittri (void* loc, v3* a, v3* b, v3* c);
void update_normal (tri* t);
v3* normal (void* loc, v3* a, v3* b, v3* c);
v3* barycentric (void* loc, v3* pt, tri* t);
