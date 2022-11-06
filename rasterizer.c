#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

#include "rasterizer.h"
#include "matrix.h"
#include "tri.h"

void draw_line (int x1, int y1, int x2, int y2, int* line_raster_buffer, int width, int height) {

	//Breensham's line drawing algorithm
	int curr_x = x1 * 2 + 1;
	int curr_y = y1 * 2 + 1;
	int step_x;
	int step_y;
	int* indep_axis;
	int* dep_axis;
	int* step_indep;
	int* step_dep;
	int num_steps;
	int numerator;
	int denominator;

	if (x1 < x2) {
		step_x = 2;
	} else {
		step_x = -2;
	}
	if (y1 < y2) {
		step_y = 2;
	} else {
		step_y = -2;
	}

	//Compute slope and set up axis increments
	float m;
	if ((x1 - x2) == 0 || abs (y1 - y2) > abs (x1 - x2)) {
		numerator = abs (x1 - x2);
		denominator = abs (y1 - y2);
	       	indep_axis = &curr_y;
		dep_axis = &curr_x;
		step_indep = &step_y;
		step_dep = &step_x;
		num_steps = abs (y1 - y2) + 1;
	} else {
		numerator = fabs (y1 - y2);
		denominator = fabs (x1 - x2);
		indep_axis = &curr_x;
		dep_axis = &curr_y;
		step_indep = &step_x;
		step_dep = &step_y;
		num_steps = abs (x1 - x2) + 1;
	}
	
	//Line drawing
	int i;
	float err = 0;
	for (i = 0; i < num_steps; i++) {
		int raster_x = (curr_x/2);
		int raster_y = (curr_y/2);
		raster_x = raster_x < 0 ? 0 : (raster_x >= width ? width - 1 : raster_x);
		raster_y = raster_y < 0 ? 0 : (raster_y >= height ? height - 1 : raster_y);
		//Add to raster buffer
		if (line_raster_buffer[raster_y * 2] == -1) {
			line_raster_buffer[raster_y * 2] = raster_x;
			line_raster_buffer[raster_y * 2 + 1] = raster_x; //First and last are identical for the first point
       		} else {
			if (raster_x < line_raster_buffer[raster_y * 2]){
				line_raster_buffer[raster_y * 2] = raster_x; //First element is the minimum
			} else if (raster_x > line_raster_buffer[raster_y * 2 + 1]) {
				line_raster_buffer[raster_y * 2 + 1] = raster_x; //Second element is the maximum
			}
		}

		//Move one step
		(*indep_axis) += *step_indep;
		err += numerator;
		if (err >= denominator) {
			err -= denominator;
			(*dep_axis) += *step_dep;
		}

	}
	

}

void sort_by_angle (v3* src, int num_vertices) {

	//Compute the angles about the center point
	float* angles = malloc (sizeof (float) * num_vertices);
	float center_x = (src[0].x + src[1].x) / 2;
	float center_y = (src[0].y + src[1].y) / 2;
	float dx, dy;
	int i, j;
	for (i = 0; i < num_vertices; i++) {
		dx = src[i].x - center_x;
		dy = src[i].y - center_y;
		angles[i] = atan2f (dy, dx); 
	}

	//Sort the points
	for (i = 0; i < num_vertices; i++) {
		float min_ang = angles[i];
		int min_index = i;
		for (j = i; j < num_vertices; j++) {
			if (angles[j] < min_ang) {
				min_ang = angles[j];
				min_index = j;
			}
		}
		if (min_index != i) {
			//Swap angles
			float tempf = angles[i];
			angles[i] = angles[min_index];
			angles[min_index] = tempf;
			//Swap vertices
			v3 tempv = src[i];
			src[i] = src[min_index];
			src[min_index] = tempv;
		}
	}

}


void draw_polygon (v3* verts, int num_vertices, char* buffer, int width, int height) {

	//Compute the top and bottom y
	int i;
	float top_y = verts[0].y;
	float bottom_y = verts[0].y;
	for (i = 0; i < num_vertices; i++) {
		float curr = verts[i].y;
		if (curr > top_y) {
			top_y = curr;
		}
		if (curr < bottom_y) {
			bottom_y = curr;
		}
	}
	int top_px = ((top_y + 1) / 2) * height;
	int bottom_px = ((bottom_y + 1) / 2) * height;
	
	//Draw the polygon
	int* raster_buffer = malloc (sizeof (int) * height * 2);
	for (i = 0; i < height * 2; i++) {
		raster_buffer[i] = -1;
	}
	for (i = 0; i < num_vertices; i++) {
		v3 v_from = verts[i];
		v3 v_to = verts[(i + 1) % num_vertices];
		draw_line (((v_from.x + 1) / 2) * width, ((v_from.y + 1) / 2) * height, ((v_to.x + 1) / 2) * width, ((v_to.y + 1) / 2) * height, raster_buffer, width, height);
	}
	for (i = bottom_px; i <= top_px; i++) {
		int a = raster_buffer[i * 2];
		int b = raster_buffer[i * 2 + 1];
		int wx;
		for (wx = a; wx <= b; wx++) {
			buffer[(i) * width + wx] = '#';
		}
	}
	free (raster_buffer);

}

void draw_tri (tri* t, char* buffer, int width, int height) {

	//Initial case
	int i;
	int is_normal = 1;
	for (i = 0; i < 3; i++) {
		v3* curr = (v3*)(&(t->a)) + i;
		if (!(curr->x > -1 && curr->x < 1 && curr->y > -1 && curr->y < 1)) {
			is_normal = 0;
			break;
		}
	}
	if (is_normal) {
		draw_polygon (&(t->a), 3, buffer, width, height);
		return;
	} else {

		//Allocate space for the vertices
		v3 verts[8];
		int vertex_count = 0;

		//Include all triangle vertices that are inside the NDC area
		for (i = 0; i < 3; i++) {
			v3* curr = (v3*)(&(t->a)) + i;
			if (curr->x > -1 && curr->x < 1 && curr->y > -1 && curr->y < 1) {
				verts[vertex_count] = *curr;
				vertex_count++;
			}
		}

		//Include all triangle edge intersections with the screen boundaries
		for (i = 0; i < 3; i++) {
			//Get the edge vector
			v3* from = (i == 0 ? &(t->a) : (i == 1 ? &(t->b) : &(t->c)));
			v3* to = (i == 0 ? &(t->b) : (i == 1 ? &(t->c) : &(t->a)));
			v3 edge;
			vector_diff3 (&edge, from, to);
			//Find the slope/intecept form
			float m, b;
			if (to->x == from->x) {
				//TODO do I need to include vertical edges?
				//Horizontal edges
				int j;
				for (j = -1; j <= 1; j += 2) {
					if (j > fminf (from->y, to->y) && j < fmaxf (from->y, to->y)) {
						initv3 (verts + vertex_count, from->x, j, 0);
						vertex_count++;
					}
				}
			} else {
				//TODO fix weird bug with horizontal lines
				m = (to->y - from->y) / (to->x - from->x); //m = dx/dy
				b = from->y - m * from->x; //b = y - mx
				//y = mx + b, (y - b) / m = x;
				float isect;
				//x=1, x=-1
				int j;
				for (j = -1; j <= 1; j += 2) {
					isect = m * j + b;
					if (isect >= -1 && isect <= 1 && isect > fminf (from->y, to->y) && isect < fmaxf (from->y, to->y)) {
						initv3 (verts + vertex_count, j, isect, 0);
						vertex_count++;
					}
				}
				//y=1, y=-1
				for (j = -1; j <= 1; j += 2) {
					if (m != 0) {
						isect = (j - b) / m;
						if (isect >= -1 && isect <= 1 && isect > fminf (from->x, to->x) && isect < fmaxf (from->x, to->x)) {
							initv3 (verts + vertex_count, isect, j, 0);
							vertex_count++;
						}
					}
				}
			}
		}

		//Include all of the screen corners within the triangle
		v3 corners[4];
		initv3 (corners, -1.0, -1.0, 0);
		initv3 (corners + 1, 1.0, -1.0, 0);
		initv3 (corners + 2, -1.0, 1.0, 0);
		initv3 (corners + 3, 1.0, 1.0, 0);
		for (i = 0; i < 4; i++) {
			v3 bary;
			barycentric (&bary, corners + i, t);
			if (bary.x >= 0 && bary.y >= 0 && bary.z >= 0) {
				verts[vertex_count] = corners[i];
				vertex_count++;
			}
		}
		
		if (vertex_count > 0) {
			sort_by_angle (verts, vertex_count);
			draw_polygon (verts, vertex_count, buffer, width, height);
		}

	}

}

int main () {
	
	//Init curses
	initscr ();
	raw ();
	noecho ();

	//Init colors
	int RGB_PAIR_START = 0;
	int RGB_COLOR_START = 0;
	
	if (has_colors ()) {
		start_color ();
		int i;
		int eighttable[8] = {0, 125, 250, 375, 500, 625, 750, 1000};
		int fourtable[4] = {0, 333, 666, 1000};
		for (i = 0; i < 256; i++) {
			#ifdef COLOR_MODE_256
			int r = eighttable[(i & 0xE0) >> 5];
			int g = eighttable[(i & 0x1C) >> 2];
		    	int b = fourtable[(i & 0x03) >> 0];
			int index = i;
			#else
			int r = fourtable[(i & 0x60) >> 5];
			int g = fourtable[(i & 0x1C) >> 2];
			int b = fourtable[(i & 0x03) >> 0];
			int index = i % 128 + 128;
			#endif
			init_color (index, r, g, b);
			init_pair (index, i, 0);
		}
	} else {
		endwin ();
		exit (1);
	}

	//Init hexagon
	v3 hexagon[6];
	initv3 (&(hexagon[0]), 0, 0.5, 0);
	initv3 (&(hexagon[1]), -0.3, 0.3, 0);
	initv3 (&(hexagon[2]), 0.3, 0.3, 0);
	initv3 (&(hexagon[3]), -0.3, -0.3, 0);
	initv3 (&(hexagon[4]), 0.3, -0.3, 0);
	initv3 (&(hexagon[5]), 0, -0.5, 0);
	sort_by_angle (hexagon, 6);

	//Init triangle
	tri t;
	t.a.x = 0;
	t.a.y = -0.5;
	t.a.z = 0.0;
	t.b.x = -0.5;
	t.b.y = 0.5;
	t.b.z = 0.0;
	t.c.x = 0.5;
	t.c.y = 0.5;
	t.c.z = 0.0;
	
	//Transform triangle
	int max_x, max_y;
	getmaxyx (stdscr, max_y, max_x);
	v3 origs[3];
	origs[0] = t.a;
	origs[1] = t.b;
	origs[2] = t.c;
	v3* vertices[3] = {&(t.a), &(t.b), &(t.c)};
	mat4 scl, rot, trans, lookat, perspective;
	matrix_trans4 (&trans, 0.0, 0.0, 3.0);
	matrix_scale4 (&scl, 10, 2.5, 1);
	matrix_lookat (&lookat, newv3 (3.0, -2.0, 0.0), newv3 (0.0, 0.0, 3.0), newv3 (0, 1, 0));
	double aspect = (double)max_x / max_y;
	matrix_perspective (&perspective, M_PI/4, aspect, 0.1, 50);
	int ang;
	float winv_buffer[3];
	for (ang = 0; ang < 360; ang++) {
		matrix_roty4 (&rot, (M_PI/180) * ang);
		int i;
		for (i = 0; i < 3; i++) {
			v4 tmp1, tmp2, res;
			initv4 (&tmp1, origs[i].x, origs[i].y, origs[i].z, 1.0);
			matrix_mul4v (&tmp2, &scl, &tmp1);
			matrix_mul4v (&tmp1, &rot, &tmp2);
			matrix_mul4v (&tmp2, &trans, &tmp1);
			matrix_mul4v (&tmp1, &lookat, &tmp2);
			matrix_mul4v (&res, &perspective, &tmp1);
			vertices[i]->x = res.x / res.w;
			vertices[i]->y = res.y / res.w;
			vertices[i]->z = res.z / res.w;
			winv_buffer[i] = 1 / res.w;
		}

		//Draw triangle
		update_normal (&t);
		char* screen_buffer = calloc (1, max_x * max_y);
		draw_tri (&t, screen_buffer, max_x, max_y);
		int wx, wy;
		int first = 1;
		for (wy = 0; wy < max_y; wy++) {
			for (wx = 0; wx < max_x; wx++) {
				char curr = screen_buffer[wy * max_x + wx];
				move (max_y - 1 - wy, wx);
				if (curr) {
					//Compute color
					v3 bary_vec;
					float fx, fy;
					fx = (((float)wx + 0.5) / max_x) * 2 - 1;
					fy = (((float)wy + 0.5) / max_y) * 2 - 1;
					tri rasterized;
					v3 ra, rb, rc, pt;
					initv3 (&ra, t.a.x, t.a.y, 0);
					initv3 (&rb, t.b.x, t.b.y, 0);
					initv3 (&rc, t.c.x, t.c.y, 0);
					inittri (&rasterized, &ra, &rb, &rc);
					initv3 (&pt, fx, fy, 0);
					initv3 (&(rasterized.n), 0, 0, -1);
					barycentric (&bary_vec, &pt, &rasterized); 
					bary_vec.x = bary_vec.x < 0 ? 0 : bary_vec.x;
					bary_vec.y = bary_vec.y < 0 ? 0 : bary_vec.y;
					bary_vec.z = bary_vec.z < 0 ? 0 : bary_vec.z;
					//Perspective-coorect mapping
					float pc_denominator = 1 / (bary_vec.x * winv_buffer[0] + bary_vec.y * winv_buffer[1] + bary_vec.z * winv_buffer[2]);
					bary_vec.x = bary_vec.x * winv_buffer[0] * pc_denominator;
					bary_vec.y = bary_vec.y * winv_buffer[1] * pc_denominator;
					bary_vec.z = bary_vec.z * winv_buffer[2] * pc_denominator;
					v3* color_vec = &bary_vec;
					#ifdef COLOR_MODE_256
					int r = (int)(color_vec->x * 8);
					int g = (int)(color_vec->y * 8);
					int b = (int)(color_vec->z * 4);
					int color_index = (r << 5) + (g << 2) + b;
					#else
					int r = (int)(color_vec->x * 4);
					int g = (int)(color_vec->y * 8);
					int b = (int)(color_vec->z * 4);
					int color_index = (r << 5) + (g << 2) + b + 128;
					#endif
					attron (COLOR_PAIR (color_index));
					addch (screen_buffer[wy * max_x + wx]);
					attroff (COLOR_PAIR (color_index));
				} else {
					addch (' ');
				}
			}
		}
		free (screen_buffer);
		int c = getch ();

	}


	//Restore default pallate
	#ifdef COLOR_MODE_256
	init_color (COLOR_BLACK, 0, 0, 0);
	init_color (COLOR_RED, 1000, 0, 0);
	init_color (COLOR_GREEN, 0, 1000, 0);
	init_color (COLOR_YELLOW, 1000, 1000, 0);
	init_color (COLOR_BLUE, 0, 0, 1000);
	init_color (COLOR_MAGENTA, 1000, 0, 1000);
	init_color (COLOR_CYAN, 1000, 1000, 0);
	init_color (COLOR_WHITE, 1000, 1000, 1000);
	#endif

	//Wait and end
	getch ();
	endwin ();

}
