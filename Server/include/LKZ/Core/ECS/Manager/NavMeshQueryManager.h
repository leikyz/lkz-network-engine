#pragma once
#include "DetourNavMeshQuery.h"
#include "DetourNavMesh.h"
#include "DetourAlloc.h"
#include <LKZ/Utility/Logger.h> 
#include <LKZ/Simulation/Math/Vector.h>

// Manager for thread-local dtNavMeshQuery instances and navmesh operations
class NavMeshQueryManager
{
public:
	// Retrieves or creates a thread-local dtNavMeshQuery for the given navMesh
    static dtNavMeshQuery* GetThreadLocalQuery(dtNavMesh* navMesh);

	// Cleans up the thread-local dtNavMeshQuery instance
    static void CleanupThreadQuery();

	// Snaps a position to the nearest point on the navmesh within specified search radius
    static Vector3 SnapToNavMesh(dtNavMeshQuery* query, Vector3 approximatePos, float searchRadiusH, float searchRadiusV);
};
