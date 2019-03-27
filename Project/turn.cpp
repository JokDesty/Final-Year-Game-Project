#include "turn.h"

#include "entity.h"
#include "actionbar.h"
#include "ai.h"

u32 turn_number = 1;
team turn_team;

void turn_start(team team)
{
	turn_team = team;

	// reset ap
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if (entity->team == team) entity->ap = entity->max_ap;
	}

	actionbar_combatlog_add("%s team's turn started", team_get_name(team));

	if (team == TEAM_ENEMY)
	{
		ai_perform_team(team);
		turn_end();
	}
}

void turn_end()
{
	u32 enemies_alive = 0;
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if (entity->team == turn_team) entity->ap = 0;
		else enemies_alive++;
	}

	turn_number++;

	actionbar_combatlog_add("%s team's turn ended", team_get_name(turn_team));

	team new_team = team_get_opposite(turn_team);

	if(enemies_alive > 0)
	{
		turn_start(new_team);
	}
	else
	{
		printf("Game over! %s team won!\n", team_get_name(turn_team));
	}
}

bool turn_is_complete(team team)
{
	u32 team_entities = 0;
	u32 team_entities_ended = 0;

	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if (entity->team == team)
		{
			team_entities++;

			if (entity->ap <= 0) team_entities_ended++;
		}
	}

	return team_entities == team_entities_ended;
}