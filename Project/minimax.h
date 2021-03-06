#pragma once

#include "general.h"
#include "action.h"

action_evaluation minimax_search(entity* ent, action_evaluation parent, u32 start_depth, u32 depth, team maximizing_team, team current_team, float alpha, float beta);