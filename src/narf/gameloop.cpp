/*
 * NarfBlock bytestream writer and reader
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "narf/gameloop.h"


narf::GameLoop::GameLoop(double maxFrameTime, double tickRate) : maxFrameTime_(maxFrameTime), forceStatusUpdate(false), quit(false) {
	setTickRate(tickRate);
}

void narf::GameLoop::setTickRate(double tickRateHz) {
	tickRate_ = tickRateHz;
	tickStep_ = (1.0 / tickRateHz);
}


void narf::GameLoop::run() {
	double t = 0.0;
	double t1 = getTime();
	double tAccum = 0.0;

	double fpsT1 = getTime();
	unsigned ticks = 0;
	unsigned draws = 0;

	double tickTime = 0.0, drawTime = 0.0;

	while (1) {
		double t2 = getTime();
		double frameTime = t2 - t1;

		if (frameTime > maxFrameTime_) {
			frameTime = maxFrameTime_;
		}

		t1 = t2;

		tAccum += frameTime;

		auto physicsStart = getTime();
		while (tAccum >= tickStep_) {
			tick(t, tickStep_);
			if (quit) {
				return;
			}

			ticks++;

			tAccum -= tickStep_;
			t += tickStep_;
		}
		tickTime += getTime() - physicsStart;

		double fps_dt = getTime() - fpsT1;
		if (fps_dt >= 1.0 || forceStatusUpdate) {
			forceStatusUpdate = false;
			// update fps counter
			char fpsStr[1000];
			auto physMS = draws ? (tickTime / draws) * 1000.0 : 0.0;
			auto drawMS = draws ? (drawTime / draws) * 1000.0 : 0.0;
			sprintf(fpsStr, "%.0f ticks/s (%.2f ms/draw) %.0f draws/s (%.2f ms/draw)",
				(double)ticks / fps_dt, physMS,
				(double)draws / fps_dt, drawMS);
			updateStatus(fpsStr);

			fpsT1 = getTime();
			draws = ticks = 0;
			tickTime = drawTime = 0.0;
		}

		auto drawStart = getTime();
		draw();
		draws++;
		drawTime += getTime() - drawStart;
	}
}
