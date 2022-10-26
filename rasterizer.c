#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

#include "rasterizer.h"
#include "matrix.h"
	
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
			buffer[(height - i) * width + wx] = '#';
		}
	}
	free (raster_buffer);

}

int main () {

	//Init curses
	initscr ();
	raw ();
	noecho ();

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
	v3 origs[3];
	origs[0] = t.a;
	origs[1] = t.b;
	origs[2] = t.c;
	v3* vertices[3] = {&(t.a), &(t.b), &(t.c)};
	mat4 scl, rot, trans, lookat, perspective;
	matrix_trans4 (&trans, 0.0, 0.0, 3.0);
	matrix_scale4 (&scl, 2.5, 2.5, 2.5);
	matrix_lookat (&lookat, newv3 (3.0, -2.0, 0.0), newv3 (0.0, 0.0, 3.0), newv3 (0, 1, 0));
	matrix_perspective (&perspective, M_PI/4, 1.0, 0.1, 50);
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
		int max_x, max_y;
		getmaxyx (stdscr, max_y, max_x);
		char* screen_buffer = calloc (1, max_x * max_y);
		draw_tri (&t, screen_buffer, max_x, max_y);
		int wx, wy;
		for (wy = 0; wy < max_y; wy++) {
			for (wx = 0; wx < max_x; wx++) {
				char curr = screen_buffer[wy * max_x + wx];
				move (wy, wx);
				if (curr) {
					addch (curr);
				} else {
					addch (' ');
				}
			}
		}
		free (screen_buffer);
		getch ();

	}

	//Wait and end
	getch ();
	endwin ();

}
