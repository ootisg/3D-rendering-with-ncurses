#ifndef COLOR_MAPPING_H
#define COLOR_MAPPING_H

#include "vector.h"

#include <stdint.h>

struct palatte {
	char palatte_name[32];
	v3 palatte_colors[256];
	uint8_t color_LUT[4096];
};

typedef struct palatte palatte;

void display_color_map ();

void* print_xterm_color_demo ();
void* palatte_print_LUT_RGB (palatte* lut);
char* palatte_info_from_file (char palatte_name[32], v3 palatte[256], char* filename);
void palatte_autopopulate (palatte* palatte, int num_colors, float (*compare_func)(v3*, v3*));
palatte* palatte_from_file (void* loc, char* filename);
void palatte_use (palatte* p);
void init_color_pairs ();
uint8_t get_color_index (palatte* p, int color);

#endif
