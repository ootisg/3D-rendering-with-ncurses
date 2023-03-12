#ifndef COLOR_MAPPING_H
#define COLOR_MAPPING_H

#include "vector.h"

#include <stdint.h>

#define PALATTE_FLAG_HAS_COLOR_DATA 0x1
#define PALATTE_FLAG_IS_XTERM_256 0x2
#define PALATTE_FLAG_IS_XTERM_16 0x4
#define PALATTE_FLAG_IS_MONOCHROME 0x8
#define PALATTE_FLAG_HAS_LUT 0x10
#define PALATTE_FLAG_USES_SYSTEM_COLORS 0x20


struct palatte {
	uint32_t flags;
	char palatte_name[32];
	v3* palatte_colors;
	int num_colors;
	void* palatte_data;
	uint8_t (*get_color_id)(struct palatte* p, int color);
};

typedef struct palatte palatte;

void display_color_map (palatte* p);

void* print_xterm_color_demo ();
char* palatte_info_from_file (char palatte_name[32], v3** palatte, int* num_colors, char* filename);
void palatte_autopopulate (palatte* palatte, float (*compare_func)(v3*, v3*));
void palatte_use (palatte* p);
void init_color_pairs ();
uint8_t get_color_index (palatte* p, int color);

palatte* palatte_from_file (void* loc, char* filename);
palatte* palatte_default_xterm_256 (void* loc);

#endif
