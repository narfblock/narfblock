#ifndef NARFBLOCK_INPUT_H
#define NARFBLOCK_INPUT_H

#include "narf/math/vector.h"
#include "narf/texteditor.h"

namespace narf {

class Display;

class Input {
public:

	enum State {
		InputStateNormal,
		InputStateText,
	};

	Input(TextEditor &textEditor, float lookSensitivityX, float lookSensitivityY) :
		textEditor(textEditor),
		state_(InputStateNormal),
		lookSensitivityX_(lookSensitivityX),
		lookSensitivityY_(lookSensitivityY),
		moveForward_(0),
		moveBackward_(0),
		lastMove_(LastMoveNeither),
		strafeLeft_(0),
		strafeRight_(0),
		lastStrafe_(LastStrafeNeither),
		jump_(false),
		run_(false),
		exit_(false),
		actionPrimary_(false),
		actionPrimaryBegin_(false),
		actionPrimaryEnd_(false),
		actionSecondary_(false),
		actionSecondaryBegin_(false),
		actionSecondaryEnd_(false),
		actionTernary_(false),
		actionTernaryBegin_(false),
		actionTernaryEnd_(false),
		toggleWireframe_(false),
		toggleBackfaceCulling_(false),
		toggleFog_(false),
		toggleFullscreen_(false),
		togglePause_(false),
		screenshot_(false),
		lookRel_(0.0f, 0.0f) { }

	void reset();
	void processEvent(const SDL_Event *event);

	bool strafeLeft() const { return strafeLeft_ && (lastStrafe_ == LastStrafeLeft || !strafeRight_); }
	bool strafeRight() const { return strafeRight_ && (lastStrafe_ == LastStrafeRight || !strafeLeft_); }

	bool moveForward() const { return moveForward_ && (lastMove_ == LastMoveForward || !moveBackward_); }
	bool moveBackward() const { return moveBackward_ && (lastMove_ == LastMoveBackward || !moveForward_); }

	bool jump() const { return jump_; }
	bool run() const { return run_; }
	bool exit() const { return exit_; }

	bool actionPrimary() const { return actionPrimary_; }
	bool actionPrimaryBegin() const { return actionPrimaryBegin_; }
	bool actionPrimaryEnd() const { return actionPrimaryEnd_; }

	bool actionSecondary() const { return actionSecondary_; }
	bool actionSecondaryBegin() const { return actionSecondaryBegin_; }
	bool actionSecondaryEnd() const { return actionSecondaryEnd_; }

	bool actionTernary() const { return actionTernary_; }
	bool actionTernaryBegin() const { return actionTernaryBegin_; }
	bool actionTernaryEnd() const { return actionTernaryEnd_; }

	bool toggleWireframe() const { return toggleWireframe_; }
	bool toggleBackfaceCulling() const { return toggleBackfaceCulling_; }
	bool toggleFog() const { return toggleFog_; }
	bool toggleFullscreen() const { return toggleFullscreen_; }
	bool togglePause() const { return togglePause_; }
	bool screenshot() const { return screenshot_; }

	const Vector2f lookRel() const { return lookRel_; }
	void resetLookRel() { lookRel_ = { 0,0 }; }

	const std::string &text() const { return text_; }

	State state() const { return state_; }

private:

	void processNormalEvent(const SDL_Event *event);
	void processTextEvent(const SDL_Event *event);
	void paste();

	TextEditor &textEditor;

	State state_;

	float lookSensitivityX_;
	float lookSensitivityY_;

	int moveForward_;
	int moveBackward_;
	enum { LastMoveNeither, LastMoveForward, LastMoveBackward } lastMove_;

	int strafeLeft_;
	int strafeRight_;
	enum { LastStrafeNeither, LastStrafeLeft, LastStrafeRight } lastStrafe_;

	bool jump_;
	bool run_;
	bool exit_;

	bool actionPrimary_; // left click
	bool actionPrimaryBegin_;
	bool actionPrimaryEnd_;

	bool actionSecondary_; // right click
	bool actionSecondaryBegin_;
	bool actionSecondaryEnd_;

	bool actionTernary_; // middle click
	bool actionTernaryBegin_;
	bool actionTernaryEnd_;

	bool toggleWireframe_;
	bool toggleBackfaceCulling_;
	bool toggleFog_;
	bool toggleFullscreen_;
	bool togglePause_;
	bool screenshot_;

	Vector2f lookRel_;

	std::string text_;
};

} // namespace narf

#endif // NARFBLOCK_INPUT_H
