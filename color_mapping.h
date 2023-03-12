#ifndef COLOR_MAPPING_H
#define COLOR_MAPPING_H

#include "vector.h"

#include <stdint.h>

struct palatte {
	char palatte_name[32];
	v3* palatte_colors;
	int num_colors;
	void* palatte_data;
	int (*get_color_id)(struct palatte* p, v3* color);
};

typedef struct palatte palatte;

void display_color_map ();

void* print_xterm_color_demo ();
void* palatte_print_LUT_RGB (palatte* lut);
char* palatte_info_from_file (char palatte_name[32], v3** palatte, int* num_colors, char* filename);
void palatte_autopopulate (palatte* palatte, float (*compare_func)(v3*, v3*));
palatte* palatte_from_file (void* loc, char* filename);
void palatte_use (palatte* p);
void init_color_pairs ();
uint8_t get_color_index (palatte* p, int color);

#endif
