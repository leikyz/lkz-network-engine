#pragma once

#include <vector>

struct Vector3;
struct dtNavMesh;
class dtNavMeshQuery;
class dtQueryFilter;
class dtCrowd;

class World
{
public:
	World() = default;
	~World() = default;

	void initialize();
	void update(double deltaTime);
	void shutdown();

	Vector3 FindNearestPoint(dtNavMeshQuery* navQuery, const Vector3& point);

	std::vector<Vector3> CalculatePath(dtNavMeshQuery* navQuery, const Vector3& start, const Vector3& end);

	Vector3 getRandomNavMeshPoint(dtNavMeshQuery* navQuery);

	dtNavMesh* getNavMesh() const { return navMesh; }
	dtCrowd* getCrowd() const { return crowd; }

	uint16_t getAnimatedEntitiesCount() const { return animatedEntitiesCount; }
	uint16_t getPrimitiveEntitiesCount() const { return primitiveEntitiesCount; }

	void UpdateCrowd(double deltaTime);
private:
	dtNavMesh* navMesh = nullptr;
	dtCrowd* crowd;
	uint16_t animatedEntitiesCount; // Represent unity's "Humanoid" rig, which has 16 bones that can be animated (including root). This is used to track how many entities in the world are using the humanoid rig and need their animations updated each frame.
	uint16_t primitiveEntitiesCount; // Represent unity's "Primitive" rig, which has a simpler structure. This is used to track how many entities in the world are using the primitive rig and need their animations updated each frame.
	dtQueryFilter* m_filter = nullptr;
};
