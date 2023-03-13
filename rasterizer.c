#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

#include "rasterizer.h"
#include "matrix.h"
#include "tri.h"
#include "color_mapping.h"

palatte* terminal_palatte;

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


void draw_polygon (v3* verts, tri* t, float* winv_buffer, int num_vertices, output_char* buffer, uint32_t* depth_buffer, int width, int height) {

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
			//Compute color
			v3 bary_vec;
			float fx, fy;
			fx = (((float)wx + 0.5) / width) * 2 - 1;
			fy = (((float)i + 0.5) / height) * 2 - 1;
			tri rasterized;
			v3 ra, rb, rc, pt;
			initv3 (&ra, t->a.x, t->a.y, 0);
			initv3 (&rb, t->b.x, t->b.y, 0);
			initv3 (&rc, t->c.x, t->c.y, 0);
			inittri (&rasterized, &ra, &rb, &rc);
			initv3 (&pt, fx, fy, 0);
			initv3 (&(rasterized.n), 0, 0, -1);
			barycentric (&bary_vec, &pt, &rasterized); 
			bary_vec.x = bary_vec.x < 0 ? 0 : bary_vec.x;
			bary_vec.y = bary_vec.y < 0 ? 0 : bary_vec.y;
			bary_vec.z = bary_vec.z < 0 ? 0 : bary_vec.z;
			//Perspective-coorect mapping
			float depth = bary_vec.x * t->a.z + bary_vec.y * t->b.z + bary_vec.z * t->c.z; //Why doesn't this work when using perspective-correct interpolation?
			float pc_denominator = 1 / (bary_vec.x * winv_buffer[0] + bary_vec.y * winv_buffer[1] + bary_vec.z * winv_buffer[2]);
			bary_vec.x = bary_vec.x * winv_buffer[0] * pc_denominator;
			bary_vec.y = bary_vec.y * winv_buffer[1] * pc_denominator;
			bary_vec.z = bary_vec.z * winv_buffer[2] * pc_denominator;
			//Depth buffer compute/check
			uint32_t depth_int = (uint32_t)(depth * 16777215); //Between 0 and (2^24) - 1, inclusive
			uint32_t buff_depth = depth_buffer[i * width + wx];
			if ((buff_depth == 0 || depth_int < buff_depth) && depth >= 0) {
				//printw ("%f\n", depth);
				//TODO find out why w is negative
				depth_buffer[i * width + wx] = depth_int;
				v3* color_vec = &bary_vec;
				int r = (int)(color_vec->x * 255);
				int g = (int)(color_vec->y * 255);
				int b = (int)(color_vec->z * 255);
				buffer[(i) * width + wx].color = (r << 16) | (g << 8) | (b << 0);
				buffer[(i) * width + wx].style = 0;
			}
		}
	}
	free (raster_buffer);

}

void draw_tri (tri* t, output_char* buffer, uint32_t* depth_buffer, float* winv_buffer, int width, int height) {

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
		draw_polygon (&(t->a), t, winv_buffer, 3, buffer, depth_buffer, width, height);
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
			draw_polygon (verts, t, winv_buffer, vertex_count, buffer, depth_buffer, width, height);
		}

	}

}

int main () {
	
	//Init curses
	initscr ();
	raw ();
	noecho ();
	keypad (stdscr, 1);

	//Init colors
	int RGB_PAIR_START = 0;
	int RGB_COLOR_START = 0;
	
	if (has_colors ()) {
		start_color ();
		terminal_palatte = malloc (sizeof (palatte));
		palatte_hsv_60hue (terminal_palatte);
		//palatte_default_xterm_256 (terminal_palatte);
		palatte_use (terminal_palatte);
		init_color_pairs ();
	} else {
		endwin ();
		exit (1);
	}
	
	display_color_map (terminal_palatte);

	//Init cursor
	int cursor_x = 0;
	int cursor_y = 0;

	//Init triangle
	int num_tris = 2;
	tri tris[num_tris];
	
	tris[0].a.x = 0.0;
	tris[0].a.y = -0.5;
	tris[0].a.z = 0.0;
	tris[0].b.x = -0.5;
	tris[0].b.y = 0.5;
	tris[0].b.z = 0.0;
	tris[0].c.x = 0.5;
	tris[0].c.y = 0.5;
	tris[0].c.z = 0.0;
	
	tris[1].a.x = 0.0;
	tris[1].a.y = -0.5;
	tris[1].a.z = 0.0;
	tris[1].b.x = 0.0;
	tris[1].b.y = 0.5;
	tris[1].b.z = -0.5;
	tris[1].c.x = 0.0;
	tris[1].c.y = 0.5;
	tris[1].c.z = 0.5;
	
	int max_x, max_y;
	getmaxyx (stdscr, max_y, max_x);
	int ang;
	for (ang = 0; ang < 360; ang+=2) {
		int curr_tri;
		output_char* screen_buffer = calloc (sizeof (output_char), max_x * max_y);
		uint32_t* depth_buffer = calloc (sizeof (uint32_t), max_x * max_y);
		for (curr_tri = 0; curr_tri < num_tris; curr_tri++) {
			//Transform triangle
			tri t = tris[curr_tri];
			v3 origs[3];
			origs[0] = t.a;
			origs[1] = t.b;
			origs[2] = t.c;
			v3* vertices[3] = {&(t.a), &(t.b), &(t.c)};
			mat4 scl, rot, trans, lookat, perspective;
			matrix_trans4 (&trans, 0.0, 0.0, 3.0);
			matrix_scale4 (&scl, 10, 2.5, 10);
			matrix_lookat (&lookat, newv3 (3.0, -2.0, 0.0), newv3 (0.0, 0.0, 3.0), newv3 (0, 1, 0));
			double aspect = (double)max_x / max_y;
			matrix_perspective (&perspective, M_PI/4, aspect, 0.1, 50);
			float winv_buffer[3];
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
			draw_tri (&t, screen_buffer, depth_buffer, winv_buffer, max_x, max_y);
		}
		
		//Update the screen with the pixel buffer
		int wx, wy;
		int first = 1;
		//move (0, 0);
		//printw ("%d\n", ang);
		for (wy = 0; wy < max_y; wy++) {
			for (wx = 0; wx < max_x; wx++) {
				output_char curr = screen_buffer[wy * max_x + wx];
				move (wy, wx);
				if (curr.color) {
					int color_index = terminal_palatte->get_color_id (terminal_palatte, curr.color);
					attron (COLOR_PAIR (color_index));
					addch ('#');
					attroff (COLOR_PAIR (color_index));
				} else {
					addch (' ');
				}
			}
		}
		
		free (screen_buffer);
		free (depth_buffer);
		int c = 0;
		int arrow = 0;
		while (!arrow) {
			c = getch ();
			if (c == KEY_RIGHT) {
				arrow = 1;
			}
			if (c == 'q') {
				endwin ();
				exit (1);
			}
			//Cursor controls
			int old_cursor_x = cursor_x;
			int old_cursor_y = cursor_y;
			if (c == 'w') {
				cursor_y--;
			}
			if (c == 'a') {
				cursor_x--;
			}
			if (c == 's') {
				cursor_y++;
			}
			if (c == 'd') {
				cursor_x++;
			}
			if (cursor_x < 0) {
				cursor_x = 0;
			}
			if (cursor_x >= max_x) {
				cursor_x = max_x - 1;
			}
			if (cursor_y < 0) {
				cursor_y = 0;
			}
			if (cursor_y >= max_y) {
				cursor_y = max_y - 1;
			}
			if (old_cursor_x != cursor_x || old_cursor_y != cursor_y) {
				move (old_cursor_y, old_cursor_x);
				output_char curr = screen_buffer[old_cursor_y * max_x + old_cursor_x];
				if (curr.color) {
					int color_index = get_color_index (terminal_palatte, curr.color);
					attron (COLOR_PAIR (color_index));
					addch ('#');
					attroff (COLOR_PAIR (color_index));
				} else {
					addch (' ');
				}
				move (cursor_y, cursor_x);
				addch (' ');
				//Print info
				move (0, 0);
				clrtoeol ();

				uint32_t px_depth = depth_buffer[cursor_y * max_x + cursor_x];
				printw ("%d\n", px_depth);

			}
		}
	}

	//Wait and end
	getch ();
	endwin ();

}
