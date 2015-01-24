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
		moveForward_(false),
		moveBackward_(false),
		lastMove_(LastMoveNeither),
		strafeLeft_(false),
		strafeRight_(false),
		lastStrafe_(LastStrafeNeither),
		jump_(false),
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
		lookRel_(0.0f, 0.0f) { }

	void beginSample();
	void processEvent(const SDL_Event *event);
	void endSample();

	bool strafeLeft() const { return strafeLeft_ && (lastStrafe_ == LastStrafeLeft || !strafeRight_); }
	bool strafeRight() const { return strafeRight_ && (lastStrafe_ == LastStrafeRight || !strafeLeft_); }

	bool moveForward() const { return moveForward_ && (lastMove_ == LastMoveForward || !moveBackward_); }
	bool moveBackward() const { return moveBackward_ && (lastMove_ == LastMoveBackward || !moveForward_); }

	bool jump() const { return jump_; }
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
	bool screenshot() const { return screenshot_; }

	const Vector2f lookRel() const { return lookRel_; }

	const std::string &text() const { return text_; }

	State state() const { return state_; }

private:

	void processNormalEvent(const SDL_Event *event);
	void processTextEvent(const SDL_Event *event);

	TextEditor &textEditor;

	State state_;

	float lookSensitivityX_;
	float lookSensitivityY_;

	bool moveForward_;
	bool moveBackward_;
	enum { LastMoveNeither, LastMoveForward, LastMoveBackward } lastMove_;

	bool strafeLeft_;
	bool strafeRight_;
	enum { LastStrafeNeither, LastStrafeLeft, LastStrafeRight } lastStrafe_;

	bool jump_;

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
	bool screenshot_;

	Vector2f lookRel_;

	std::string text_;
};

} // namespace narf

#endif // NARFBLOCK_INPUT_H
