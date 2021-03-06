#pragma once

#include "general.h"
#include "mesh.h"
#include "team.h"
#include "dynarray.h"

#include <vector>

struct entity {
	u32 id;
	vec3 pos;
	float rotation;

	mesh* mesh;

	s32 health;
	s32 max_health;

	s32 ap;
	s32 max_ap;

	team team;
	bool dead;
};

extern dynarray* entities;

// @Cleanup: honestly idk where else to put this
extern entity* selected_entity;
extern entity* targeted_entity;

void entity_update();

entity* entity_create(vec3 pos, team t);

void entity_move(entity* ent, vec3 pos);

void entity_health_change(entity* ent, entity* inflict_ent, s32 amount);
void entity_health_change(entity* ent, entity* inflict_ent, s32 amount, bool temp);
bool entity_is_same_team(entity* ent1, entity* ent2);