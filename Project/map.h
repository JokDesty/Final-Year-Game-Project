#pragma once

#include <unordered_map>

#include "general.h"
#include "mesh.h"
#include "entity.h"
#include "map_gen.h"

// the step of the world raytrace
#define MAP_RAYTRACE_ACCURACY 0.25f

extern dynarray* map_road_segments;
extern dynarray* map_segments;

extern u32 map_max_x;
extern u32 map_max_z;

struct cover {
	vec3 pos;

	u32 height;
};

void map_init();
void map_draw();

void map_resetup();

vec3 map_get_block_pos(vec3 pos);
bool map_pos_equal(vec3 pos1, vec3 pos2);
float map_distance_squared(vec3 pos1, vec3 pos2);

void map_update_entity_pos(entity* ent, vec3 new_pos);

void map_set_cover(vec3 block_pos, u8 height);
void map_clear_blocks();
bool map_is_cover(vec3 block_pos);
u8 map_get_cover_height(vec3 block_pos);
entity* map_get_entity_at_block(vec3 block_pos);

bool map_is_movable(vec3 pos);

bool map_is_adjacent_to_cover(vec3 pos);
vec3 map_get_adjacent_cover(vec3 start, vec3 closest_to);

// @Todo: these have bad names
bool map_has_los_internal(float start_x, float start_z, float end_x, float end_z);

bool map_has_los(vec3 pos1, vec3 pos2);
bool map_has_los(entity* ent1, entity* ent2);

// Returns a floating point value between 0.0f and 1.0f (0.0f = 100% covered, 0.5f = 50% covered, 1.0f = no blocking cover)
float map_get_los_angle(entity* inflict_ent, entity* target_ent);

float map_get_shot_chance(entity* inflict_ent, entity* target_ent);