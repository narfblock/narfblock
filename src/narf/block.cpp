#include "narf/block.h"

static void calcTexCoord(narf::BlockTexCoord *tc, unsigned texId) {
	unsigned texX = texId % 16;
	unsigned texY = texId / 16;

	float texCoordTileSize = 1.0f / 16.0f; // TODO: calculate from actual texture size

	tc->u1 = (float)texX * texCoordTileSize;
	tc->v1 = (float)texY * texCoordTileSize;

	tc->u2 = tc->u1 + texCoordTileSize;
	tc->v2 = tc->v1 + texCoordTileSize;
}


narf::BlockType::BlockType(unsigned texXPos, unsigned texXNeg, unsigned texYPos, unsigned texYNeg, unsigned texZPos, unsigned texZNeg) {
	solid = true;
	indestructible = false;

	// calc tex coords from id
	calcTexCoord(&texCoords[narf::XPos], texXPos);
	calcTexCoord(&texCoords[narf::XNeg], texXNeg);
	calcTexCoord(&texCoords[narf::YPos], texYPos);
	calcTexCoord(&texCoords[narf::YNeg], texYNeg);
	calcTexCoord(&texCoords[narf::ZPos], texZPos);
	calcTexCoord(&texCoords[narf::ZNeg], texZNeg);

	// for now, all blocks have exactly 1x1x1 AABB
	aabbCenterOffset = math::Vector3f(0.0f, 0.0f, 0.0f);
	aabbHalfSize = math::Vector3f(0.5f, 0.5f, 0.5f);
}


narf::AABB narf::BlockType::getAABB(const narf::BlockCoord& bc) const {
	math::Vector3f blockCenter((float)bc.x + 0.5f, (float)bc.y + 0.5f, (float)bc.z + 0.5f);
	return AABB(blockCenter + aabbCenterOffset, aabbHalfSize);
}
