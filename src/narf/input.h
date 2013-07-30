#ifndef NARFBLOCK_INPUT_H
#define NARFBLOCK_INPUT_H

#include "narf/vector.h"

namespace narf {

class Display;

class Input {
public:

	Input(float look_sensitivity_x, float look_sensitivity_y) :
		look_sensitivity_x_(look_sensitivity_x),
		look_sensitivity_y_(look_sensitivity_y),
		move_forward_(false),
		move_backward_(false),
		last_move_(last_move_neither),
		strafe_left_(false),
		strafe_right_(false),
		last_strafe_(last_strafe_neither),
		jump_(false),
		exit_(false),
		action_primary_(false),
		action_primary_begin_(false),
		action_primary_end_(false),
		action_secondary_(false),
		action_secondary_begin_(false),
		action_secondary_end_(false),
		toggle_wireframe_(false),
		toggle_backface_culling_(false),
		look_rel_(0.0f, 0.0f) { }

	void begin_sample();
	void process_event(const SDL_Event *event);
	void end_sample();

	bool strafe_left() const { return strafe_left_ && (last_strafe_ == last_strafe_left || !strafe_right_); }
	bool strafe_right() const { return strafe_right_ && (last_strafe_ == last_strafe_right || !strafe_left_); }

	bool move_forward() const { return move_forward_ && (last_move_ == last_move_forward || !move_backward_); }
	bool move_backward() const { return move_backward_ && (last_move_ == last_move_backward || !move_forward_); }

	bool jump() const { return jump_; }
	bool exit() const { return exit_; }

	bool action_primary() const { return action_primary_; }
	bool action_primary_begin() const { return action_primary_begin_; }
	bool action_primary_end() const { return action_primary_end_; }

	bool action_secondary() const { return action_secondary_; }
	bool action_secondary_begin() const { return action_secondary_begin_; }
	bool action_secondary_end() const { return action_secondary_end_; }

	bool toggle_wireframe() const { return toggle_wireframe_; }
	bool toggle_backface_culling() const { return toggle_backface_culling_; }

	const Vector2f look_rel() const { return look_rel_; }

private:
	float look_sensitivity_x_;
	float look_sensitivity_y_;

	bool move_forward_;
	bool move_backward_;
	enum { last_move_neither, last_move_forward, last_move_backward } last_move_;

	bool strafe_left_;
	bool strafe_right_;
	enum { last_strafe_neither, last_strafe_left, last_strafe_right } last_strafe_;

	bool jump_;

	bool exit_;

	bool action_primary_; // left click
	bool action_primary_begin_;
	bool action_primary_end_;

	bool action_secondary_; // right click
	bool action_secondary_begin_;
	bool action_secondary_end_;

	bool toggle_wireframe_;
	bool toggle_backface_culling_;

	Vector2f look_rel_;
};

} // namespace nar

#endif // NARFBLOCK_INPUT_H
