/*
 * NarfBlock timestamp manipulation
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

#include "narf/time.h"

#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

narf::time narf::time::now() {

	static bool initialized = false;
	static std::chrono::time_point<std::chrono::steady_clock> startupTime;

	auto now = std::chrono::steady_clock::now();

	// TODO: get rid of this
	if (!initialized) {
		startupTime = now;
		initialized = true;
	}

	// TODO: implement this in a way that doesn't require std::chrono
	auto t = std::chrono::duration_cast<std::chrono::microseconds>(now - startupTime).count();
	return narf::time(narf::timediff((int64_t)t));
}


void narf::sleep(const narf::timediff& td) {
#ifdef _WIN32
	// MinGW-w64 doesn't have std::this_thread?
	Sleep(static_cast<DWORD>(td.us_ / 1000));
#else
	auto sleepDuration = std::chrono::microseconds(td.us_);
	std::this_thread::sleep_for(sleepDuration);
#endif
}
