#include <SDL.h>

#include <stdlib.h>

#include "narf/input.h"
#include "narf/client/console.h"

void narf::Input::process_event(const SDL_Event *event) {
	switch (state_) {
	case InputStateNormal:
		processNormalEvent(event);
		break;
	case InputStateText:
		processTextEvent(event);
		break;
	}
}


void narf::Input::processNormalEvent(const SDL_Event *event) {
	// TODO: describe input mappings with data so they can be configurable by the user

	switch (event->type) {
	case SDL_QUIT:
		exit_ = true;
		break;

	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_RETURN:
		case SDLK_SLASH:
			state_ = InputStateText;
			break;
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
		case SDLK_F3:
			screenshot_ = true;
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
		look_rel_ += narf::math::Vector2f((float)event->motion.xrel * look_sensitivity_x_,
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
		case SDL_BUTTON_MIDDLE:
			action_ternary_begin_ = !action_ternary_;
			action_ternary_ = true;
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
		case SDL_BUTTON_MIDDLE:
			action_ternary_ = false;
			action_ternary_end_ = true;
			break;
		}
		break;
	}
}


void narf::Input::processTextEvent(const SDL_Event *event) {
	switch (event->type) {
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_RETURN:
			// return the entered text and go back to normal mode
			text_ += textEditor.getString();
			// fall through
		case SDLK_ESCAPE: // go back to normal state but do not return any text
			state_ = InputStateNormal;
			textEditor.clear();
			return;
		case SDLK_BACKSPACE:
			textEditor.delAtCursor(-1);
			return;
		case SDLK_DELETE:
			textEditor.delAtCursor(1);
			return;
		case SDLK_LEFT:
			textEditor.moveCursor(-1);
			return;
		case SDLK_RIGHT:
			textEditor.moveCursor(1);
			return;
		case SDLK_HOME:
			textEditor.homeCursor();
			return;
		case SDLK_END:
			textEditor.endCursor();
			return;
		default:
			// eat any other key - actual text entry happens in SDL_TEXTINPUT
			return;
		}
		break;

	case SDL_TEXTINPUT:
		textEditor.addString(event->text.text);
		return;
	}

	// pass any unhandled input to normal handler
	processNormalEvent(event);
}


void narf::Input::begin_sample()
{
	// reset one-shot events and relative measurements
	jump_ = false;
	action_primary_begin_ = action_primary_end_ = false;
	action_secondary_begin_ = action_secondary_end_ = false;
	action_ternary_begin_ = action_ternary_end_ = false;
	toggle_wireframe_ = false;
	toggle_backface_culling_ = false;
	screenshot_ = false;
	look_rel_ = narf::math::Vector2f(0.0f, 0.0f);
	text_.clear();
}


void narf::Input::end_sample()
{
}
