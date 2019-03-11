#pragma once
#include "platform.h"
#include "renderer.h"

struct GameState {
	RendererState* rs;
};

void UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input);