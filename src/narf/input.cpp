#include <SDL/SDL.h>

#include <stdlib.h>

#include "narf/input.h"

void narf::Input::process_event(const SDL_Event *event)
{
	// TODO: describe input mappings with data so they can be configurable by the user

	switch (event->type) {
	case SDL_QUIT:
		exit_ = true;
		break;

	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
			exit_ = true;
			break;
		case SDLK_w:
			move_forward_ = true;
			last_move_ = last_move_forward;
			break;
		case SDLK_s:
			move_backward_ = true;
			last_move_ = last_move_backward;
			break;
		case SDLK_a:
			strafe_left_ = true;
			last_strafe_ = last_strafe_left;
			break;
		case SDLK_d:
			strafe_right_ = true;
			last_strafe_ = last_strafe_right;
			break;
		case SDLK_SPACE:
			jump_ = true;
			break;
		case SDLK_F1:
			toggle_wireframe_ = true;
			break;
		case SDLK_F2:
			toggle_backface_culling_ = true;
			break;
		}
		break;

	case SDL_KEYUP:
		switch (event->key.keysym.sym) {
		case SDLK_w:
			move_forward_ = false;
			break;
		case SDLK_s:
			move_backward_ = false;
			break;
		case SDLK_a:
			strafe_left_ = false;
			break;
		case SDLK_d:
			strafe_right_ = false;
			break;
		}
		break;

	case SDL_MOUSEMOTION:
		look_rel_ = Vector2f((float)event->motion.xrel * look_sensitivity_x_,
		                     (float)event->motion.yrel * look_sensitivity_y_);
		break;

	case SDL_MOUSEBUTTONDOWN:
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			action_primary_begin_ = !action_primary_;
			action_primary_ = true;
			break;
		case SDL_BUTTON_RIGHT:
			action_secondary_begin_ = !action_secondary_;
			action_secondary_ = true;
			break;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			action_primary_ = false;
			action_primary_end_ = true;
			break;
		case SDL_BUTTON_RIGHT:
			action_secondary_ = false;
			action_secondary_end_ = true;
			break;
		}
		break;

	}
}


void narf::Input::begin_sample()
{
	// reset one-shot events and relative measurements
	jump_ = false;
	action_primary_begin_ = action_primary_end_ = false;
	action_secondary_begin_ = action_secondary_end_ = false;
	toggle_wireframe_ = false;
	toggle_backface_culling_ = false;
	look_rel_ = Vector2f(0.0f, 0.0f);
}


void narf::Input::end_sample()
{
}
