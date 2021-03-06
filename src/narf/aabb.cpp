/*
 * Axis-aligned bounding box intersection
 *
 * Copyright (c) 2015 Daniel Verkamp
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

#include "narf/aabb.h"

#include <algorithm>

bool narf::AABB::intersect(const AABB& o) const {
	auto d = (this->center - o.center).abs();
	auto s = this->halfSize + o.halfSize;
	return d <= s;
}


bool narf::AABB::intersect(const Ray<float>& ray, Point3f& p) const {
	// http://tavianator.com/2011/05/fast-branchless-raybounding-box-intersections/
	auto minP = center - halfSize;
	auto maxP = center + halfSize;
	auto rP = ray.initialPoint();
	auto rInvN = ray.inverseDirection();

	Vector3f t1((minP - rP) * rInvN);
	Vector3f t2((maxP - rP) * rInvN);

	float tMin = min(t1, t2).maxComponent();
	float tMax = max(t1, t2).minComponent();

	if (tMax >= 0.0f && tMax >= tMin) {
		p = rP + ray.direction() * tMax;
		return true;
	}
	return false;
}
