#include "ai.h"

#include "entity.h"
#include "action.h"
#include "board_eval.h"
#include "turn.h"
#include "minimax.h"

void ai_perform_entity(entity* ent, u32 depth)
{
	team team = ent->team;

	// start with nothing action, anything better than doing nothing we do
	action best_action;
	action_evaluation best_eval = { 0 };
	best_eval.chance = 1.0;

	board_eval_build_cache();

	best_eval = minimax_search(ent, best_eval, depth, depth, team, team, -FLT_MAX, +FLT_MAX);
	
	best_action = best_eval.action;

	if (!best_eval.valid) best_action = action_nothing;

	board_eval_destroy_cache();

	//printf("Entity %i on team %s performing action %s on target (%f, %f) with chance %f\n", ent->id, team_get_name(ent->team), best_action.name, best_eval.target.x, best_eval.target.z, best_eval.chance);
	board_evaluation_print(best_eval.eval);

	best_action.perform(ent, best_eval.target, false);
}

void ai_perform_team(team team, u32 depth)
{
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = *((entity**) dynarray_get(entities, i));

		if (ent->team == team && !ent->dead)
		{
			ai_perform_entity(ent, depth);
			ai_perform_entity(ent, depth);
		}
	}
}