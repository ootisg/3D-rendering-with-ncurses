#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

#include "rasterizer.h"
#include "matrix.h"
#include "tri.h"

void draw_line (int x1, int y1, int x2, int y2, int* line_raster_buffer) {

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
		//Add to raster buffer
		if (line_raster_buffer[raster_y * 2] == 0) {
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

void draw_tri (tri* triangle, char* buffer, int width, int height) {

	//Compute the top and bottom y
	float vals[3];
	vals[0] = triangle->a.y;
	vals[1] = triangle->b.y;
	vals[2] = triangle->c.y;
	//Inline bubble sort
	float temp;
	if (vals[0] > vals[1]) {
		temp = vals[0];
		vals[0] = vals[1];
		vals[1] = temp;
	}
	if (vals[1] > vals[2]) {
		temp = vals[1];
		vals[1] = vals[2];
		vals[2] = temp;
	}
	if (vals[0] > vals[1]) {
		temp = vals[0];
		vals[0] = vals[1];
		vals[1] = temp;
	}
	int tri_min_y = ((vals[0] + 1) / 2) * height;
	int tri_max_y = ((vals[2] + 1) / 2) * height;

	int i;
	int* raster_buffer = calloc (sizeof (int), height * 2);
	for (i = 0; i < 3; i++) {
		v3 v_from = (i == 0 ? triangle->a : (i == 1 ? triangle->b : triangle->c));
		v3 v_to = (i == 0 ? triangle->b : (i == 1 ? triangle->c : triangle->a));
		draw_line (((v_from.x + 1) / 2) * width, ((v_from.y + 1) / 2) * height, ((v_to.x + 1) / 2) * width, ((v_to.y + 1) / 2) * height, raster_buffer);
	}
	for (i = tri_min_y; i <= tri_max_y; i++) {
		int a = raster_buffer[i * 2];
		int b = raster_buffer[i * 2 + 1];
		int wx;
		for (wx = a; wx <= b; wx++) {
			buffer[(i) * width + wx] = '#';
		}
	}
	free (raster_buffer);

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
	matrix_scale4 (&scl, 1.25, 1.25, 1.25);
	matrix_lookat (&lookat, newv3 (3.0, -2.0, 0.0), newv3 (0.0, 0.0, 3.0), newv3 (0, 1, 0));
	double aspect = (double)max_x / max_y;
	matrix_perspective (&perspective, M_PI/4, aspect, 0.1, 50);
	int ang;
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
				move (max_y - wy, wx);
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
		getch ();

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
