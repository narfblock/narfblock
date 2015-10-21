#include <SDL.h>

#include <stdlib.h>

#include "narf/input.h"
#include "narf/client/console.h"

// HACK: this should be exposed in a better way
extern narf::ClientConsole* clientConsole;

void narf::Input::processEvent(const SDL_Event *event) {
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
		// repeating keys
		switch (event->key.keysym.sym) {
		case SDLK_KP_9:
			if (event->key.keysym.mod & KMOD_NUM) {
				break;
			}
			// fallthrough
		case SDLK_PAGEUP:
			clientConsole->pageUp();
			return;

		case SDLK_KP_3:
			if (event->key.keysym.mod & KMOD_NUM) {
				break;
			}
			// fallthrough
		case SDLK_PAGEDOWN:
			clientConsole->pageDown();
			return;
		case SDLK_HOME:
			if (event->key.keysym.mod & KMOD_CTRL) {
				clientConsole->scrollHome();
				return;
			}
			break;
		case SDLK_END:
			if (event->key.keysym.mod & KMOD_CTRL) {
				clientConsole->scrollEnd();
				return;
			}
			break;
		}

		if (event->key.repeat) {
			return;
		}

		switch (event->key.keysym.sym) {
		case SDLK_RETURN:
			if (event->key.keysym.mod & KMOD_ALT) {
				toggleFullscreen_ = true;
				break;
			}
			// fall through
		case SDLK_SLASH:
			state_ = InputStateText;
			break;
		case SDLK_ESCAPE:
			exit_ = true;
			break;
		case SDLK_w:
			moveForward_ = true;
			lastMove_ = LastMoveForward;
			break;
		case SDLK_s:
			moveBackward_ = true;
			lastMove_ = LastMoveBackward;
			break;
		case SDLK_a:
			strafeLeft_ = true;
			lastStrafe_ = LastStrafeLeft;
			break;
		case SDLK_d:
			strafeRight_ = true;
			lastStrafe_ = LastStrafeRight;
			break;
		case SDLK_q:
			actionTernaryBegin_ = !actionTernary_;
			actionTernary_ = true;
			break;
		case SDLK_SPACE:
			jump_ = true;
			break;
		case SDLK_LSHIFT:
			run_ = true;
			break;
		case SDLK_F1:
			toggleWireframe_ = true;
			break;
		case SDLK_F2:
			toggleBackfaceCulling_ = true;
			break;
		case SDLK_F3:
			screenshot_ = true;
			break;
		case SDLK_F4:
			toggleFog_ = true;
			break;
		case SDLK_PAUSE:
			togglePause_ = true;
			break;
		}
		break;

	case SDL_KEYUP:
		switch (event->key.keysym.sym) {
		case SDLK_w:
			moveForward_ = false;
			break;
		case SDLK_s:
			moveBackward_ = false;
			break;
		case SDLK_a:
			strafeLeft_ = false;
			break;
		case SDLK_d:
			strafeRight_ = false;
			break;
		case SDLK_q:
			actionTernary_ = false;
			actionTernaryEnd_ = true;
			break;
		case SDLK_LSHIFT:
			run_ = false;
			break;
		}
		break;

	case SDL_MOUSEMOTION:
		lookRel_ += Vector2f((float)event->motion.xrel * lookSensitivityX_,
		                     (float)event->motion.yrel * lookSensitivityY_);
		break;

	case SDL_MOUSEBUTTONDOWN:
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			actionPrimaryBegin_ = !actionPrimary_;
			actionPrimary_ = true;
			break;
		case SDL_BUTTON_RIGHT:
			actionSecondaryBegin_ = !actionSecondary_;
			actionSecondary_ = true;
			break;
		case SDL_BUTTON_MIDDLE:
			actionTernaryBegin_ = !actionTernary_;
			actionTernary_ = true;
			break;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			actionPrimary_ = false;
			actionPrimaryEnd_ = true;
			break;
		case SDL_BUTTON_RIGHT:
			actionSecondary_ = false;
			actionSecondaryEnd_ = true;
			break;
		case SDL_BUTTON_MIDDLE:
			actionTernary_ = false;
			actionTernaryEnd_ = true;
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
			textEditor.moveCursorLeft();
			return;
		case SDLK_RIGHT:
			textEditor.moveCursorRight();
			return;
		case SDLK_HOME:
			textEditor.homeCursor();
			return;
		case SDLK_END:
			textEditor.endCursor();
			return;
		case SDLK_v:
			if (event->key.keysym.mod & KMOD_CTRL) {
				paste();
			}
			return;
		case SDLK_INSERT:
			if (event->key.keysym.mod & KMOD_SHIFT) {
				paste();
			}
		default:
			// eat any other key - actual text entry happens in SDL_TEXTINPUT
			return;
		}
		break;

	case SDL_TEXTINPUT:
		textEditor.addString(event->text.text);
		return;

	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		// eat mouse input for now
		// TODO: mouse-based cursor movement and selection
		return;
	}

	// pass any unhandled input to normal handler
	processNormalEvent(event);
}


void narf::Input::paste() {
	// TODO: handle newlines?
	textEditor.addString(SDL_GetClipboardText());
}


void narf::Input::reset()
{
	// reset one-shot events and relative measurements
	jump_ = false;
	actionPrimaryBegin_ = actionPrimaryEnd_ = false;
	actionSecondaryBegin_ = actionSecondaryEnd_ = false;
	actionTernaryBegin_ = actionTernaryEnd_ = false;
	toggleWireframe_ = false;
	toggleBackfaceCulling_ = false;
	toggleFog_ = false;
	toggleFullscreen_ = false;
	togglePause_ = false;
	screenshot_ = false;
	lookRel_ = Vector2f(0.0f, 0.0f);
	text_.clear();
}
