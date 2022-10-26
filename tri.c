#include "tri.h"
#include "vector.h"

#include <stdlib.h>
#include <stdio.h>

tri* inittri (void* loc, v3* a, v3* b, v3* c) {
	tri* t = (tri*)loc;
	(t->a).x = a->x;
	(t->a).y = a->y;
	(t->a).z = a->z;
	(t->b).x = b->x;
	(t->b).y = b->y;
	(t->b).z = b->z;
	(t->c).x = c->x;
	(t->c).y = c->y;
	(t->c).z = c->z;
	normal (&(t->n), a, b, c);
	return t;
}

void update_normal (tri* t) {
	normal (&(t->n), &(t->a), &(t->b), &(t->c));
}

v3* normal (void* loc, v3* a, v3* b, v3* c) {
	v3 u, v;
	vector_diff3 (&u, a, b);
	vector_diff3 (&v, b, c);
	vector_cross3 (loc, &u, &v);
	vector_normalize3 (loc, (v3*)loc);
}

v3* barycentric (void* loc, v3* pt, tri* t) {
	
	//Declare the scratch vectors
	v3 wv1, wv2, wv3;
	
	//Calculate the area of the full triangle
	vector_diff3 (&wv1, &(t->c), &(t->a));
	vector_diff3 (&wv2, &(t->c), &(t->b));
	vector_cross3 (&wv3, &wv1, &wv2);
	double area_abc = vector_dot3 (&wv3, &(t->n)) / 2;
	
	//Calculate the area of triangle apb
	vector_diff3 (&wv1, &(t->a), pt);
	vector_diff3 (&wv2, &(t->b), pt);
	vector_cross3 (&wv3, &wv1, &wv2);
	double area_apb = vector_dot3 (&wv3, &(t->n)) / 2;
	//Calculate the area of triangle bpc
	vector_diff3 (&wv1, &(t->b), pt);
	vector_diff3 (&wv2, &(t->c), pt);
	vector_cross3 (&wv3, &wv1, &wv2);
	double area_bpc = vector_dot3 (&wv3, &(t->n)) / 2;
	//Calculate the area of triangle cpa
	vector_diff3 (&wv1, &(t->c), pt);
	vector_diff3 (&wv2, &(t->a), pt);
	vector_cross3 (&wv3, &wv1, &wv2);
	double area_cpa = vector_dot3 (&wv3, &(t->n)) / 2;
	
	//Cast the result vector
	v3* res = (v3*)loc;
	
	//Calculate the coords
	res->x = area_bpc / area_abc; //alpha
	res->y = area_cpa / area_abc; //beta
	res->z = area_apb / area_abc; //gamma
	return res;
	
}
