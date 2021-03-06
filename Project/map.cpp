#include "map.h"

#include <stdlib.h>
#include <emmintrin.h>
#include <libmorton/morton.h>

#include "mesh.h"
#include "entity.h"
#include "asset_manager.h"
#include "debug.h"
#include "map_gen.h"

u32 map_max_x = 64;
u32 map_max_z = 64;

typedef enum block_type
{
	BLOCK_TYPE_NOTHING,
	BLOCK_TYPE_COVER,
	BLOCK_TYPE_ENTITY
} block_type_t;

union block_data
{
	entity* entity;
	u8 cover_height;
};

struct block
{
	block_type type;

	block_data data;
};

block* blocks;

dynarray* map_segments = dynarray_create(20, sizeof(map_segment));
dynarray* map_road_segments = dynarray_create(20, sizeof(map_road_segment));

void map_add_entity(entity* ent);
void map_setup();


void map_init()
{
	u32 next_power_of_2 = math_u32_next_power_of_2(max(map_max_x, map_max_z));
	blocks = (block*) debug_calloc(next_power_of_2 * next_power_of_2, sizeof(block));

	map_setup();
}

void map_setup()
{
	map_gen();

	map_road_segment* first_road_segment = ((map_road_segment*) dynarray_get(map_road_segments, 0));
	map_road_segment* last_road_segment = ((map_road_segment*) dynarray_get(map_road_segments, map_road_segments->len - 1));

	float road_end_x = last_road_segment->pos.x;

	// spawn friendlies
	entity* friendly;

	friendly = entity_create(vec3(road_end_x + 1, 0, 1), TEAM_FRIENDLY);
	friendly->rotation = 0.0f;
	map_add_entity(friendly);

	friendly = entity_create(vec3(road_end_x + 1, 0, 3), TEAM_FRIENDLY);
	friendly->rotation = 0.0f;
	map_add_entity(friendly);

	friendly = entity_create(vec3(road_end_x + 3, 0, 1), TEAM_FRIENDLY);
	friendly->rotation = 0.0f;
	map_add_entity(friendly);

	friendly = entity_create(vec3(road_end_x + 3, 0, 3), TEAM_FRIENDLY);
	friendly->rotation = 0.0f;
	map_add_entity(friendly);

	float enemy_facing_direction = 90.0f;

	if (first_road_segment->scale.y >= first_road_segment->scale.x) enemy_facing_direction = 180.0f;

	// spawn enemies
	entity* enemy;

	enemy = entity_create(vec3(1, 0, 53), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);

	enemy = entity_create(vec3(1, 0, 54), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);

	enemy = entity_create(vec3(1, 0, 55), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);

	enemy = entity_create(vec3(3, 0, 53), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);

	enemy = entity_create(vec3(3, 0, 54), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);

	enemy = entity_create(vec3(3, 0, 55), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);

	enemy = entity_create(vec3(5, 0, 54), TEAM_ENEMY);
	enemy->rotation = enemy_facing_direction;
	map_add_entity(enemy);
}

void map_resetup()
{
	map_clear_blocks();

	// clear entities
	for(u32 i = 0; i < entities->len; i++)
	{
		free(*(entity**) dynarray_get(entities, i));
	}

	dynarray_clear(entities);

	map_setup();
}

void map_draw()
{
	// draw cover
	for(u32 x = 0; x < map_max_x; x++)
	{
		for(u32 z = 0; z < map_max_z; z++)
		{
			u8 cover_height = map_get_cover_height(vec3(x, 0.0f, z));
			if(cover_height)
			{
				graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(vec3(x, 0.0f, z), 0.0f, vec3(1.0f), vec3(1.0f, cover_height, 1.0f)), vec4(0.6f, 0.6f, 0.6f, 1.0f));
			}
		}
	}

	// draw road
	for(u32 i = 0; i < map_road_segments->len; i++)
	{
		map_road_segment seg = *(map_road_segment*) dynarray_get(map_road_segments, i);
		graphics_draw_mesh(asset_manager_get_mesh("cube"),
			graphics_create_model_matrix(vec3(seg.pos.x, 0.0f, seg.pos.y), 0.0f, vec3(1.0f), vec3(seg.scale.x, 0.05f, seg.scale.y)), vec4(0.3f, 0.3f, 0.3f, 1.0f));
	}

	u32 color_progress = 0;

	// temp draw segments
	/*for(u32 i = 0; i < map_segments->len; i++)
	{
		map_segment seg = *(map_segment*) dynarray_get(map_segments, i);

		u32 current_color = color_progress += (u32) ((255 * 255 * 255) / (map_segments->len / 0.8f));
		u8 r = *((u8*) &current_color);
		u8 g = *((u8*) &current_color + 1);
		u8 b = *((u8*) &current_color + 2);

		vec4 seg_color = vec4((float) r / 255.0f, (float) g / 255.0f, (float) b / 255.0f, 1.0f);

		graphics_draw_mesh(asset_manager_get_mesh("cube"), 
			graphics_create_model_matrix(vec3(seg.pos.x, 0.0f, seg.pos.y), 0.0f, vec3(1.0f), vec3(seg.scale.x, 0.2f, seg.scale.y)), seg_color);
	}*/

	// draw entites and healthbars
	for(u32 i = 0; i < entities->len; i++)
	{
		entity* ent = *((entity**) dynarray_get(entities, i));

		if(!ent->dead)
		{
			vec3 healthbox_aspect = vec3(1.0f, 1.0f / 3.0f, 1.0f);

			// big hack
			ent->pos.y = 0.05f;

			if(ent->health > 0)
			{
				image* img;
				if(ent->team == TEAM_ENEMY) img = asset_manager_get_image("enemy_healthbar");
				else img = asset_manager_get_image("friendly_healthbar");

				graphics_draw_image(img, graphics_create_model_matrix(vec3(ent->pos.x + 0.033333f, ent->pos.y + 2.538f, ent->pos.z + 0.5f), ent->rotation, vec3(0.0f, 1.0f, 0.0f), vec3(ent->pos.x + 0.5f, 0.0f, ent->pos.z + 0.5f),
					vec3((0.5f - 0.1f / 3.0f) * (ent->health / (float) ent->max_health), 0.1285f, 1.0f)));
			}

			graphics_draw_image(asset_manager_get_image("healthbox"), graphics_create_model_matrix(vec3(ent->pos.x, ent->pos.y + 2.5f, ent->pos.z + 0.5f), ent->rotation, vec3(0.0f, 1.0f, 0.0f), 
				vec3(ent->pos.x + 0.5f, 0.0f, ent->pos.z + 0.5f), healthbox_aspect * 0.5f));

			vec4 entity_color = vec4(0.961f, 0.965f, 0.98f, 1.0f);

			// selected entity color change
			if (selected_entity == ent) entity_color = vec4(0.501f, 0.980f, 0.607f, 1.0f);

			// targeted entity color change
			if(targeted_entity == ent) entity_color = vec4(0.988f, 0.533f, 0.537f, 1.0f);

			graphics_draw_mesh(ent->mesh, graphics_create_model_matrix(ent->pos, ent->rotation, vec3(0.0f, 1.0f, 0.0f), vec3(ent->pos.x + 0.5f, 0.0f, ent->pos.z + 0.5f), vec3(1.0f)), entity_color);

			ent->pos.y = 0.0f;
		}
	}

	// draw terrain
	graphics_draw_mesh(asset_manager_get_mesh("terrain"), graphics_create_model_matrix(vec3(0.0f), 0.0f, vec3(1.0f), vec3(1.0f)), vec4(0.498f, 0.561f, 0.651f, 1.0f));
}

void map_clear_blocks()
{
	u32 next_power_of_2 = math_u32_next_power_of_2(max(map_max_x, map_max_z));
	memset(blocks, 0, next_power_of_2 * next_power_of_2 * sizeof(block));
}

void map_update_entity_pos(entity* ent, vec3 new_pos)
{
	block* old_block = blocks + libmorton::morton2D_32_encode((u32) ent->pos.x, (u32) ent->pos.z);
	old_block->data.entity = 0;
	old_block->type = BLOCK_TYPE_NOTHING;

	block* new_block = blocks + libmorton::morton2D_32_encode((u32) new_pos.x, (u32) new_pos.z);
	new_block->type = BLOCK_TYPE_ENTITY;
	new_block->data.entity = ent;
}

void map_add_entity(entity* ent)
{
	block* new_block = blocks + libmorton::morton2D_32_encode((u32) ent->pos.x, (u32) ent->pos.z);
	new_block->type = BLOCK_TYPE_ENTITY;
	new_block->data.entity = ent;
}

void map_set_cover(vec3 block_pos, u8 height)
{
	block b = {};

	if(height > 0)
	{
		b.type = BLOCK_TYPE_COVER;
		b.data.cover_height = height;
	}
	else
	{
		b.type = BLOCK_TYPE_NOTHING;
		b.data.cover_height = 0;
	}

	blocks[libmorton::morton2D_32_encode((u32) block_pos.x, (u32) block_pos.z)] = b;
}

u8 map_get_cover_height(u32 x, u32 z)
{
	block b = blocks[libmorton::morton2D_32_encode(x, z)];

	if (b.type == BLOCK_TYPE_COVER) 
		return b.data.cover_height;

	return 0;
}

u8 map_get_cover_height(vec3 block_pos)
{
	if (block_pos.x < 0 || block_pos.z < 0  || block_pos.x >= map_max_x || block_pos.z >= map_max_z) return false;
	return map_get_cover_height((u32) block_pos.x, (u32) block_pos.z);
}

bool map_is_cover(vec3 block_pos)
{
	return map_get_cover_height(block_pos);
}

bool map_is_adjacent_to_cover(vec3 pos)
{
	for (s8 x = -1; x <= 1; x++)
	{
		for (s8 z = -1; z <= 1; z++)
		{
			vec3 block_pos = pos;
			block_pos.x += x;
			block_pos.z += z;

			if (map_is_cover(block_pos))
			{
				return true;
			}
		}
	}

	return false;
}

vec3 map_get_adjacent_cover(vec3 start, vec3 closest_to)
{
	float smallest_distance = +FLT_MAX;
	vec3 closest_cover = vec3(-1.0f);

	for(s8 x = -1; x <= 1; x++)
	{
		for(s8 z = -1; z <= 1; z++)
		{
			vec3 block_pos = start;
			block_pos.x += x;
			block_pos.z += z;

			float x_delta = abs(block_pos.x - closest_to.x);
			float z_delta = abs(block_pos.z - closest_to.z);

			float distance = (x_delta * x_delta) + (z_delta * z_delta);
			if(smallest_distance > distance && map_is_cover(block_pos))
			{
				closest_cover = block_pos;
				smallest_distance = distance;
			}
		}
	}

	return closest_cover;
}

bool map_is_movable(vec3 pos)
{
	return blocks[libmorton::morton2D_32_encode((u32) pos.x, (u32) pos.z)].type == BLOCK_TYPE_NOTHING;
}

bool map_has_los_internal(float start_x, float start_z, float end_x, float end_z)
{
	vec2 direction = glm::normalize(vec2(end_x - start_x, end_z - start_z));

	start_x += 0.5f;
	start_z += 0.5f;

	float step_x = direction.x * MAP_RAYTRACE_ACCURACY;
	float step_z = direction.y * MAP_RAYTRACE_ACCURACY;

	long last_block_x = -1;
	long last_block_z = -1;
	long next_block_x;
	long next_block_z;

	while(true)
	{
		start_x += step_x;
		start_z += step_z;

		next_block_x = (long) start_x;
		next_block_z = (long) start_z;

		// only eval if this is a new block than last
		if(!(next_block_x == last_block_x && next_block_z == last_block_z))
		{
			// if this block is off the map, they have no los
			if(next_block_x < 0 || next_block_z < 0) return false;

			// if this block is cover, they have no los
			int_fast16_t index = libmorton::morton2D_32_encode(next_block_x, next_block_z);
			block b = blocks[index];
			if(b.type == BLOCK_TYPE_COVER && b.data.cover_height > 0) return false;

			// if the next block is diagonal to the current, check that we didn't move through a diagonal wall
			if(last_block_x != -1 && last_block_z != -1 && next_block_x != last_block_x && next_block_z != last_block_z)
			{
				int_fast16_t index2 = libmorton::morton2D_32_encode(next_block_x, last_block_z);
				block b2 = blocks[index2];
				if (b2.type == BLOCK_TYPE_COVER && b2.data.cover_height > 0)
				{
					int_fast16_t index3 = libmorton::morton2D_32_encode(last_block_x, next_block_z);
					block b3 = blocks[index3];
					if(b3.type == BLOCK_TYPE_COVER && b3.data.cover_height > 0) return false;
				}
			}

			// if we reached the target, we have full los
			if (next_block_x == end_x && next_block_z == end_z) return true;

			last_block_x = next_block_x;
			last_block_z = next_block_z;
		}
	}

	return false;
}

bool map_has_los(vec3 pos1, vec3 pos2)
{
	// how leinient is the los algorithm, in blocks
	float lean_distance = 0.5f;

	// @Todo: this is a bit much, maybe we only need to check along the opposite axis of the direction
	return map_has_los_internal(pos1.x, pos1.z, pos2.x, pos2.z) ||
		map_has_los_internal(pos1.x + lean_distance, pos1.z, pos2.x, pos2.z) ||
		map_has_los_internal(pos1.x - lean_distance, pos1.z, pos2.x, pos2.z) ||
		map_has_los_internal(pos1.x, pos1.z + lean_distance, pos2.x, pos2.z) ||
		map_has_los_internal(pos1.x, pos1.z - lean_distance, pos2.x, pos2.z) ||
		map_has_los_internal(pos2.x + lean_distance, pos2.z, pos1.x, pos1.z) ||
		map_has_los_internal(pos2.x - lean_distance, pos2.z, pos1.x, pos1.z) ||
		map_has_los_internal(pos2.x, pos2.z + lean_distance, pos1.x, pos1.z) ||
		map_has_los_internal(pos2.x, pos2.z - lean_distance, pos1.x, pos1.z);
}

bool map_has_los(entity* ent1, entity* ent2)
{
	return map_has_los(ent1->pos, ent2->pos);
}

float map_get_los_angle(entity* inflict_ent, entity* target_ent)
{
	float los_amount = 1.0f;

	// check if they have direct los
	if(!map_has_los_internal(target_ent->pos.x, target_ent->pos.z, inflict_ent->pos.x, inflict_ent->pos.z))
	{
		// @Todo: this is kinda bad but im tired (The way we use -1.0f as "no adjacent cover")
		vec3 closest_cover = map_get_adjacent_cover(target_ent->pos, inflict_ent->pos);

		if (!map_pos_equal(closest_cover, vec3(-1.0f)))
		{
			vec3 cover_to_covered_vector = glm::normalize(closest_cover - target_ent->pos);
			vec3 cover_to_shooter = glm::normalize(inflict_ent->pos - closest_cover);

			float angle = glm::dot(cover_to_covered_vector, cover_to_shooter);

			// if the angle is negative, that means that the cover is behind them (they're not covered!)
			if (angle >= 0.0f)
			{
				los_amount = (0.8f - angle);
			}
		}
	}

	return min(los_amount * 2.0f, 1.0f);
}

vec3 map_get_block_pos(vec3 pos)
{
	// @Volatile: there's no y axis support here D:
	return vec3(floor(pos.x + 0.5f), 0.0f, floor(pos.z + 0.5f));
}

bool map_pos_equal(vec3 pos1, vec3 pos2)
{
	// @Volatile: there's no y axis support here
	return pos1.x == pos2.x && pos1.z == pos2.z;
}

float map_distance_squared(vec3 pos1, vec3 pos2)
{
	// @Volatile: no y support
	return pow(abs(pos1.x - pos2.x), 2) + pow(abs(pos1.z - pos2.z), 2);
}

entity* map_get_entity_at_block(vec3 block_pos)
{
	// @Todo: it's very slow to loop through for lookup :(
	for(u32 i = 0; i < entities->len; i++)
	{
		entity* ent = *((entity**) dynarray_get(entities, i));

		if(map_pos_equal(ent->pos, block_pos))
		{
			return ent;
		}
	}

	return NULL;
}

float map_get_shot_chance(entity* inflict_ent, entity* target_ent)
{
	return map_get_los_angle(inflict_ent, target_ent) * min(0.82f, (float) ((sqrt(pow(map_max_x, 2.0f) + pow(map_max_z, 2.0f)) / 7.5f)
		/ sqrt(map_distance_squared(inflict_ent->pos, target_ent->pos))));
}