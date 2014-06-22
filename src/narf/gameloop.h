/*
 * NarfBlock game loop
 *
 * Copyright (c) 2014 Daniel Verkamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NARF_GAMELOOP_H
#define NARF_GAMELOOP_H

#include <string>

namespace narf {

class GameLoop {
public:
	GameLoop(double maxFrameTime, double tickRate);
	virtual ~GameLoop();

	void run();

	virtual double getTime() = 0;
	virtual void tick(double t, double dt) = 0;
	virtual void updateStatus(const std::string& status) = 0;
	virtual void draw() = 0;

	bool forceStatusUpdate;
	bool quit;
	bool callDraw;

	double tickRate() const { return tickRate_; }
	void setTickRate(double tickRateHz);

	void setMaxFrameTime(double maxFrameTime) { maxFrameTime_ = maxFrameTime; }
	double maxFrameTime() const { return maxFrameTime_; }

private:
	double tickRate_;
	double tickStep_;
	double maxFrameTime_;
};

} // namespace narf

#endif // NARF_GAMELOOP_H
