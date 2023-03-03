#include "color_mapping.h"

#include <stdlib.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>

float sqrdist (v3* a, v3* b) {
	return (a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y) + (a->z - b->z) * (a->z - b->z);
}

float taxicab (v3* a, v3* b) {
	return fabs (a->x - b->x) + fabs (a->y - b->y) + fabs (a->z - b->z);
}

void RGB_to_HSV (v3* hsv, v3* rgb) {
	
	//Find the min/max components
	double* rgbArr = (double*)rgb;
	double max_val = fmax (rgb->x, fmax (rgb->y, rgb->z));
	double min_val = fmin (rgb->x, fmin (rgb->y, rgb->z));

	//Compute hue, saturation, and value
	double hue, saturation, value;
	saturation = max_val == 0 ? 0 : (max_val - min_val) / max_val;
	value = max_val;
	if (saturation == 0) {
		hue = 0;
	} else {
		if (max_val == rgb->x) {
			hue = (rgb->y - rgb->z) / (max_val - min_val) / 6;
		} else if (max_val == rgb->y) {
			hue = 0.333333 + (rgb->z - rgb->x) / (max_val - min_val) / 6;
		} else if (max_val == rgb->z) {
			hue = 0.666666 + (rgb->x - rgb->y) / (max_val - min_val) / 6;
		}
	}
	if (hue < 0) {
		hue += 1;
	}

	//Populate the hsv vector
	hsv->x = hue;
	hsv->y = saturation;
	hsv->z = value;

}

void HSV_to_RGB (v3* rgb, v3* hsv) {

	int index = hsv->x == 1.0 ? 5 : (int)(hsv->x * 6);
	double max_val = 1.0;
	double min_val = 1.0 - hsv->y;
	double frac = hsv->x == 1.0 ? 1.0 : hsv->x * 6 - (int)(hsv->x * 6);
	double up_val = min_val + (max_val - min_val) * frac;
	double down_val = max_val - (max_val - min_val) * frac;
	switch (index) {
		case 0:
			rgb->x = max_val;
			rgb->y = up_val;
			rgb->z = min_val;
			break;
		case 1:
			rgb->x = down_val;
			rgb->y = max_val;
			rgb->z = min_val;
			break;
		case 2:
			rgb->x = min_val;
			rgb->y = max_val;
			rgb->z = up_val;
			break;
		case 3:
			rgb->x = min_val;
			rgb->y = down_val;
			rgb->z = max_val;
			break;
		case 4:
			rgb->x = up_val;
			rgb->y = min_val;
			rgb->z = max_val;
			break;
		case 5:
			rgb->x = max_val;
			rgb->y = min_val;
			rgb->z = down_val;
			break;
	}
	rgb->x *= hsv->z;
	rgb->y *= hsv->z;
	rgb->z *= hsv->z;

}

float hsvdist (v3* a, v3* b) {
	v3 aa, bb;
	RGB_to_HSV (&aa, a);
	RGB_to_HSV (&bb, b);
	return sqrdist (&aa, &bb);
}

void* print_xterm_color_demo () {

	//Draw the 16 system colors (0-15)
	int wx, wy;
	move (0, 0);
	printw ("System colors:");
	for (wx = 0; wx < 8; wx++) {
		for (wy = 0; wy < 2; wy++) {
			move (wy + 2, wx);
			attron (COLOR_PAIR (wy * 8 + wx));
			addch (' ');
			attroff (COLOR_PAIR (wy * 8 + wx));
		}	
	}

	//Draw the RGB colors (16-231)
	move (6, 0);
	printw ("6x6 color cube:");
	int cube;
	for (cube = 0; cube < 6; cube++) {
		for (wx = 0; wx < 6; wx++) {
			for (wy = 0; wy < 6; wy++) {
				move (8 + (cube / 3) * 7 + wy, (cube % 3) * 7 + wx);
				int color = 16 + cube * 36 + wy * 6 + wx;
				attron (COLOR_PAIR (color));
				addch (' ');
				attroff (COLOR_PAIR (color));
			}
		}
	}

	//Draw the grayscale colors (232-255)
	move (23, 0);
	printw ("Grayscale:");
	for (wx = 0; wx < 24; wx++) {
		move (25, wx);
		attron (COLOR_PAIR (wx + 232));
		addch (' ');
		attron (COLOR_PAIR (wx + 232));
	}

}

void* palatte_print_LUT_RGB (palatte* lut) {
	int wx, wy;
	for (int cube = 0; cube < 16; cube++) {
		for (wx = 0; wx < 16; wx++) {
			for (wy = 0; wy < 16; wy++) {
				int color = lut->color_LUT[cube * 256 + wy * 16 + wx];
				move ((cube / 8) * 17 + wy, (cube % 8) * 17 + wx);
				attron (COLOR_PAIR (color));
				addch (' ');
				attroff (COLOR_PAIR (color));
			}
		}
	}
}

char* palatte_info_from_file (char palatte_name[32], v3 palatte[256], char* filename) {

	//Open the file and read the palatte name
	FILE* f = fopen (filename, "r");
	if (f == NULL) {
		//Return NULL if file could not be opened
		return NULL;
	}
	char* status = fgets (palatte_name, 32, f);
	if (status == NULL) {
		//Return NULL if file is empty
		return NULL;
	}
	int pname_len = strlen (palatte_name);
	
	//Trim the newline off the end of the palatte name (with \r\n case for windows compatibility)
	if (palatte_name[pname_len - 2] == '\r' && palatte_name[pname_len - 1] == '\n') {
		palatte_name[pname_len - 2] = '\0';	
	} else if (palatte_name[pname_len - 1] == '\n') {
		palatte_name[pname_len - 1] = '\0';
	}

	//Read the list of hex colors
	char buf[32];
	char* endchar = (char*)buf + 6;
	char** endptr = &endchar;
	int i;
	for (i = 0; i < 256; i++) {
		char* status = fgets (buf, 32, f);
		if (status != NULL) {
			long int numval;
			numval = strtol (buf, endptr, 16);
			float r = ((numval & 0xFF0000) >> 16) / 255.0;
			float g = ((numval & 0x00FF00) >> 8) / 255.0;
			float b = ((numval & 0x0000FF) >> 0) / 255.0;
			initv3 (&(palatte[i]), r, g, b);
		} else {
			exit(0);
			//TODO end of stream error handling
		}
	}

	//Close the file and return the palatte name
	fclose (f);
	return palatte_name;

}

palatte* palatte_from_file (void* loc, char* filename) {
	
	//Load palatte from file
	palatte* pLUT = (palatte*)loc;
	palatte_info_from_file (pLUT->palatte_name, pLUT->palatte_colors, "xterm_palatte.txt");

	//Generate palatte table
	palatte_autopopulate (pLUT, 256, hsvdist);
	return pLUT;

}

void palatte_autopopulate (palatte* palatte, int num_colors, float (*compare_func)(v3*, v3*)) {
	
	//Ignore the first 16 colors if not using system colors
	int start_color;
	#ifndef USE_SYSTEM_COLORS
	start_color = 16;
	#else
	start_color = 0;
	#endif

	//Construct the table
	v3* color_palatte = palatte->palatte_colors;
	int i, j;
	for (i = 0; i < 4096; i++) {
		int table_r = ((i & 0xF00) >> 8) << 4;
		int table_g = ((i & 0xF0) >> 4) << 4;
		int table_b = ((i & 0xF) >> 0) << 4;
		v3 table_vec, palatte_vec;
		initv3 (&table_vec, table_r / 255.0, table_g / 255.0, table_b / 255.0);
		int smallest_idx = -1;
		float smallest_dist = INT_MAX;
		for (int j = start_color; j < num_colors; j++) {
			initv3 (&palatte_vec, color_palatte[j].x, color_palatte[j].y, color_palatte[j].z);
			float dist = compare_func (&table_vec, &palatte_vec);
			if (dist < smallest_dist) {
				smallest_dist = dist;
				smallest_idx = j;
			}
		}
		palatte->color_LUT[i] = (uint8_t)smallest_idx;
	}

}

uint8_t get_color_index (palatte* p, int color) {
	int r = (color & 0xFF0000) >> 20;
	int g = (color & 0x00FF00) >> 12;
	int b = (color & 0x0000FF) >> 4;
	return p->color_LUT[(r << 8) + (g << 4) + b];
}

void palatte_use (palatte* p) {
	//Load palatte colors
	int start_color;
	#ifndef USE_SYSTEM_COLORS
	start_color = 16;
	#else
	start_color = 0;
	#endif
	int i;
	for (i = start_color; i < 256; i++) {
		v3 c = p->palatte_colors[i];
		int r = (int)(c.x * 1000);
		int g = (int)(c.y * 1000);
		int b = (int)(c.z * 1000);
		init_color (i, r, g, b);
	}
}

void init_color_pairs () {
	int i;
	for (i = 0; i < 256; i++) {
		init_pair (i, i, 0);
	}
}

void display_color_map () {
	
	//THIS IS A DEBUG FUNCTION
	palatte pLUT; 
	palatte_from_file (&pLUT, "xterm_palatte.txt");
	if (has_colors ()) {
		start_color ();
		palatte_use (&pLUT);
		init_color_pairs ();
	} else {
		endwin ();
		exit (1);
	}
	
	palatte_print_LUT_RGB (&pLUT);
	//print_xterm_color_demo ();

	getch ();
	endwin ();
	exit (0);
	
}