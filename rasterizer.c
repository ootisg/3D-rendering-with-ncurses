#include <ncurses.h>
#include <math.h>

void draw_line (int x1, int y1, int x2, int y2) {

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

		//Put current char
		move (curr_y, curr_x);
		addch ('#');

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

int main () {

	//Init curses
	initscr ();
	raw ();
	noecho ();

	//Uhh
	float theta;
	for (theta = 0; theta < M_PI * 2; theta += M_PI / 360) {
		clear ();
		float center_x = 115;
		float center_y = 30;
		float offs_x = cos (theta) * 20;
		float offs_y = sin (theta) * 20;
		draw_line (center_x, center_y, center_x + offs_x, center_y + offs_y);
		getch ();
		refresh ();
	}

	//Wait and end
	getch ();
	endwin ();

}
