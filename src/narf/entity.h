/*
 * NarfBlock entity class
 *
 * Copyright (c) 2013 Daniel Verkamp
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

#ifndef NARF_ENTITY_H
#define NARF_ENTITY_H

#include "narf/time.h"
#include "narf/math/vector.h"

namespace narf {

class World;

class Entity {
public:

	typedef uint32_t ID;
	static const ID InvalidID = UINT32_MAX;

	Entity(World *world, ID id) : id(id), bouncy(false), explodey(false), model(false), onGround(false), antigrav(false), world_(world) { }

	ID id;

	Point3f position;
	Vector3f velocity;

	Point3f prevPosition; // TODO: put all state into prev and cur structs

	// temp hack stuff
	bool bouncy;
	bool explodey;
	bool model; // TODO: replace with 3d model object; for now, indicates whether to draw a cube
	bool onGround; // on solid ground, i.e. can jump
	bool antigrav; // magic!

	// return true if object is still alive or false if it should be deleted
	bool update(narf::timediff dt);

private:
	World *world_;

};


class EntityRef {
public:
	EntityRef(World* world, Entity::ID id);
	~EntityRef();

	Entity* ent;
	Entity::ID id;
	World* world;

	Entity* operator ->() { return ent; }

	// no copying
	EntityRef(const EntityRef&) = delete;
	EntityRef& operator=(const EntityRef&) = delete;
};

} // namespace narf

#endif // NARF_ENTITY_H
