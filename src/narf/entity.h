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

#include "narf/bytestream.h"
#include "narf/time.h"
#include "narf/math/vector.h"

namespace narf {

class World;
class EntityManager;

class Entity {
public:

	typedef uint32_t ID;
	static const ID InvalidID = UINT32_MAX;

	Entity(World* world, EntityManager* entMgr, ID id) : id(id), bouncy(false), explodey(false), model(false), onGround(false), antigrav(false), world_(world), entMgr_(entMgr) { }

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
	bool update(timediff dt);

	void serialize(ByteStreamWriter& s) const;
	void deserialize(ByteStreamReader& s);

private:
	World* world_;
	EntityManager* entMgr_;
};


class EntityRef {
public:
	EntityRef(EntityManager& entMgr, Entity::ID id);
	~EntityRef();

	Entity* ent;
	Entity::ID id;
	EntityManager& entMgr;

	Entity* operator ->() { return ent; }

	// no copying
	EntityRef(const EntityRef&) = delete;
	EntityRef& operator=(const EntityRef&) = delete;
};


class EntityIDAllocator {
public:
	EntityIDAllocator();
	~EntityIDAllocator();

	Entity::ID get();
	void put(Entity::ID id);

private:
	Entity::ID firstFreeID_;
	std::vector<Entity::ID> freeIDPool_;
};


class EntityManager {
public:
	EntityManager(World* world);

	Entity* getEntityRef(Entity::ID id);
	void releaseEntityRef(Entity::ID id);

	Entity::ID newEntity();
	void deleteEntity(Entity::ID id);

	size_t getNumEntities() const { return entities_.size(); }

	// TODO: this shouldn't be public
	const std::vector<Entity>& getEntities() const { return entities_; }

	void update(timediff dt);
	void update(Entity::ID entID, double t, double dt);

	void deserializeEntity(ByteStreamReader& s);

private:
	World* world_;
	EntityIDAllocator idAllocator_;
	uint32_t entityRefs_;
	std::vector<narf::Entity> entities_;

	// internal client-only function - create entity with given ID
	Entity::ID newEntity(Entity::ID id);
};

} // namespace narf

#endif // NARF_ENTITY_H
