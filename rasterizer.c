#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

#include "rasterizer.h"

void draw_line (int x1, int y1, int x2, int y2, int* line_raster_buffer) {

	//Flip start and end if in q3,q4
	if (y1 > y2 || (y1 == y2 && x2 < x1 /*For the record, I'm very mad that I had to special-case this*/)) {
		int temp;
		temp = x2;
		x2 = x1;
		x1 = temp;
		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	//Breensham's line drawing algorithm (unoptimized)
	int curr_x = x1;
	int curr_y = y1;
	float curr_err = 0;
	float m = (float)(y2 - y1) / (x2 - x1); //TODO this might trap

	//Slope manip
	int* indep_axis = &curr_x;
	int* dep_axis = &curr_y;
	int indep_max = x2;
	int q3 = m < -1 ? 1 : 0; //Special case for octants 3 and 7
	if (m > 1 || m < -1 || x1 - x2 == 0) { //Swap x and y for octants 2, 3, 6, and 7, or if the slope is vertical.
		m = 1 / m; //Correct the slope by inverting
		indep_axis = &curr_y;
		dep_axis = &curr_x; //Swap the independent and dependent axes
		indep_max = y2; //Change the indep limit to y rather than x. This always works because y1 is guaranteed to be less than y2.
	}

	do {

		//Add to raster buffer
		if (line_raster_buffer[curr_y * 2] == 0) {
			line_raster_buffer[curr_y * 2] = curr_x;
			line_raster_buffer[curr_y * 2] = curr_x; //First and last are identical for the first point
       		} else {
			if (curr_x < line_raster_buffer[curr_y * 2]) {
				line_raster_buffer[curr_y * 2] = curr_x; //First element is the minimum
			} else if (curr_x > line_raster_buffer[curr_y * 2 + 1]) {
				line_raster_buffer[curr_y * 2 + 1] = curr_x; //Second element is the maximum
			}
		}
		//move (curr_y, curr_x);
		//addch ('#');

		//Calculate next pos
		if (m < 0 || q3) {
			//Octants 3, 4, 7, 8
			curr_err -= m;
			if (curr_err > 1) {
				curr_err--;
				(*dep_axis) += q3 ? -1 : 1; //Octants 3 and 7 need a special case here
			}
		} else {
			//Octants 1, 2, 5, 6
			curr_err += m;
			if (curr_err > 1) {
				curr_err--;
				(*dep_axis)++;
			}
		}

		//Increment indep_axis
		if (m >= 0 || q3) {
			(*indep_axis)++;
		} else {
			(*indep_axis)--;
		}

	} while (m >= 0 || q3 ? *indep_axis <= indep_max : *indep_axis >= indep_max);

}

void draw_tri (tri* triangle) {

	//Calculate screen coords
	int max_x, max_y;
	getmaxyx (stdscr, max_y, max_x);

	//Compute the top and bottom y
	float vals[3];
	vals[0] = triangle->a.y;
	vals[1] = triangle->b.y;
	vals[2] = triangle->c.y;
	//Inline bubble sort
	int temp;
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
	int tri_min_y = ((vals[0] + 1) / 2) * max_y;
	int tri_max_y = ((vals[2] + 1) / 2) * max_y;

	int i;
	int* raster_buffer = malloc (sizeof (int) * max_y * 2);
	for (i = 0; i < 3; i++) {
		vertex v_from = (i == 0 ? triangle->a : (i == 1 ? triangle->b : triangle->c));
		vertex v_to = (i == 0 ? triangle->b : (i == 1 ? triangle->c : triangle->a));
		draw_line (((v_from.x + 1) / 2) * max_x, ((v_from.y + 1) / 2) * max_y, ((v_to.x + 1) / 2) * max_x, ((v_to.y + 1) / 2) * max_y, raster_buffer);
	}
	for (i = tri_min_y; i <= tri_max_y; i++) {
		int a = raster_buffer[i * 2];
		int b = raster_buffer[i * 2 + 1];
		int wx;
		for (wx = a; wx <= b; wx++) {
			move (max_y - i, wx);
			addch ('#');
		}
	}
	free (raster_buffer);

}

int main () {

	//Init curses
	initscr ();
	raw ();
	noecho ();

	//Uhh
	tri t;
	t.a.x = 0;
	t.a.y = -0.5;
	t.b.x = -0.5;
	t.b.y = 0.5;
	t.c.x = 0.5;
	t.c.y = 0.5;
	draw_tri (&t);

	//Wait and end
	getch ();
	endwin ();

}
