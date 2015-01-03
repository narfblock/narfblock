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

#ifndef NARF_TIME_H
#define NARF_TIME_H

#include <stdint.h>

namespace narf {

// Difference between two times in microseconds (may be negative).
class timediff {
public:
	int64_t us_;

	static constexpr double scaleSeconds = 1000000.0;
	static constexpr double scaleMilliseconds = 1000.0;

	timediff() : us_(0) {}

	timediff(const timediff& td) : us_(td.us_) { }
	timediff(int us) : us_(us) {}
	timediff(int64_t us) : us_(us) {}
	timediff(double seconds) : us_(static_cast<uint64_t>(seconds * scaleSeconds)) { }

	timediff& operator =(const timediff& td) {
		us_ = td.us_;
		return *this;
	}

	operator double() const {
		// convert to seconds
		return static_cast<double>(us_) / scaleSeconds;
	}

	double milliseconds() const {
		return static_cast<double>(us_) / scaleMilliseconds;
	}

	timediff operator +(const timediff& td) {
		return timediff(us_ + td.us_);
	}

	timediff& operator +=(const timediff& td) {
		us_ += td.us_;
		return *this;
	}

	timediff operator -(const timediff& td) {
		return timediff(us_ - td.us_);
	}

	timediff& operator -=(const timediff& td) {
		us_ -= td.us_;
		return *this;
	}
};

// Absolute timestamp representing microseconds since some arbitrary origin
class time {
public:
	timediff diff;

	time() : diff(0) { }
	time(timediff td) : diff(td) { }

	static time now();

	timediff operator -(const time& t2) const {
		return timediff(diff - t2.diff);
	}

	time operator +(const timediff& td) const {
		return time(diff + td);
	}

	bool operator >=(const time& t2) const {
		return diff >= t2.diff;
	}
};

void sleep(const narf::timediff& td);

} // namespace narf

#endif // NARF_TIME_H
