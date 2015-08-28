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
#include <cmath>

#include "narf/console.h"
#include "narf/gameloop.h"


narf::GameLoop::GameLoop(timediff maxFrameTime, double tickRate) :
	forceStatusUpdate(false),
	quit(false),
	callDraw(true),
	maxFrameTime_(maxFrameTime),
	tickTimeHistogram(nullptr) {
	setTickRate(tickRate);
}


narf::GameLoop::~GameLoop() {
	delete[] tickTimeHistogram;
}


void narf::GameLoop::setTickRate(double tickRateHz) {
	tickRate_ = tickRateHz;
	tickStep_ = timediff(1.0 / tickRateHz);
	narf::console->println("setTickRate(" + std::to_string(tickRateHz) + "): tickStep:" + std::to_string(tickStep_.us_));
	if (tickTimeHistogram) {
		delete[] tickTimeHistogram;
	}
	tickTimeHistogramBuckets = (size_t)log2(tickStep_.milliseconds() + 1) + 2;
	tickTimeHistogram = new uint32_t[tickTimeHistogramBuckets];
	for (size_t bucket = 0; bucket < tickTimeHistogramBuckets; bucket++) {
		tickTimeHistogram[bucket] = 0;
	}
}


void narf::GameLoop::recordTickTime(narf::timediff dt) {
	size_t bucket = 0;
	int dtMs = (int)dt.milliseconds();
	int bucketMax = 1;
	while (bucket < tickTimeHistogramBuckets - 1) {
		if (dtMs < bucketMax) {
			break;
		}
		bucketMax *= 2;
		bucket++;
	}
	tickTimeHistogram[bucket]++;
}


void narf::GameLoop::dumpTickTimeHistogram() {
	narf::console->println("Tick time stats:");
	int bucketMin = 0, bucketMax = 1;
	for (size_t bucket = 0; bucket < tickTimeHistogramBuckets; bucket++) {
		char buf[100];
		auto count = tickTimeHistogram[bucket];
		if (bucket == tickTimeHistogramBuckets - 1) {
			snprintf(buf, sizeof(buf), ">=%d ms: %u", bucketMin, count);
		} else {
			snprintf(buf, sizeof(buf), "< %d ms: %u", bucketMax, count);
		}
		bucketMin = bucketMax;
		bucketMax *= 2;
		narf::console->println(buf);
	}
}


void narf::GameLoop::run() {
	time t1 = narf::time::now();
	timediff tAccum = 0;

	time fpsT1 = narf::time::now();
	unsigned ticks = 0;
	unsigned draws = 0;

	timediff tickTime = 0, drawTime = 0;

	while (1) {
		time t2 = narf::time::now();
		timediff frameTime = t2 - t1;

		if (frameTime > maxFrameTime_) {
			frameTime = maxFrameTime_;
		}

		t1 = t2;

		tAccum += frameTime;

		auto physicsStart = narf::time::now();
		while (tAccum >= tickStep_) {
			tick(tickStep_);
			if (quit) {
				return;
			}

			ticks++;

			tAccum -= tickStep_;
		}
		auto thisTickTime = narf::time::now() - physicsStart;
		recordTickTime(thisTickTime);
		tickTime += thisTickTime;

		// how far are we between the previous tick's state and the current state?
		float stateBlend = (float)tAccum / (float)tickStep_;

		timediff fpsDt = narf::time::now() - fpsT1;
		if (fpsDt >= 1.0 || forceStatusUpdate) {
			forceStatusUpdate = false;
			// update fps counter
			char fpsStr[1000];
			const char* unit;
			double tickMS;
			if (draws) {
				tickMS = (tickTime / draws) * 1000.0;
				unit = "draw";
			} else if (ticks) {
				tickMS = (tickTime / ticks) * 1000.0;
				unit = "tick";
			} else {
				tickMS = 0;
				unit = "??";
			}
			sprintf(fpsStr, "%.0f ticks/s (%.2f ms/%s)",
				(double)ticks / fpsDt, tickMS, unit);
			if (callDraw) {
				auto drawMS = draws ? (drawTime / draws) * 1000.0 : 0.0;
				auto len = strlen(fpsStr);
				sprintf(fpsStr + len,
					" %.0f draws/s (%.2f ms/draw)",
					(double)draws / fpsDt, drawMS);
			}
			updateStatus(fpsStr);

			fpsT1 = narf::time::now();
			draws = ticks = 0;
			tickTime = drawTime = 0;
		}

		if (callDraw) {
			auto drawStart = narf::time::now();
			draw(stateBlend);
			draws++;
			drawTime += narf::time::now() - drawStart;
		} else {
			// sleep until next tick
			auto sleepTime = tickStep_ - tAccum;
			double minSleep = 0.001;
			// TODO: do short sleeps in a loop to avoid oversleeping the next tick?
			if (sleepTime > minSleep) {
				narf::sleep(sleepTime);
			}
		}
	}
}
