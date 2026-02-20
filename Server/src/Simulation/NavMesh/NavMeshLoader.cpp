#include "LKZ/Simulation/Navmesh/NavMeshLoader.h"
#include "LKZ/Utility/Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <format>
#include <vector>
#include <string> 
#include <algorithm>

#include "Recast.h"
#include "RecastAlloc.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include <LKZ/Utility/Constants.h>

#define SAMPLE_POLYAREA_GROUND 1
#define SAMPLE_POLYFLAGS_WALK 0x01

static std::string& rtrim(std::string& s)
{
    if (!s.empty() && s.back() == '\r')
    {
        s.pop_back();
    }
    return s;
}
void NavMeshLoader::savePolyMeshToOBJ(const rcPolyMesh* pmesh, const std::string& filename)
{
    if (!pmesh) return;

    std::ofstream file(filename);
    if (!file.is_open())
    {
        Logger::Log("Failed to open file for OBJ export: " + filename, LogType::Error);
        return;
    }

    for (int i = 0; i < pmesh->nverts; ++i)
    {
        const unsigned short* v = &pmesh->verts[i * 3];
        const float x = pmesh->bmin[0] + v[0] * pmesh->cs;
        const float y = pmesh->bmin[1] + v[1] * pmesh->ch;
        const float z = pmesh->bmin[2] + v[2] * pmesh->cs;

        file << "v " << x << " " << y << " " << z << "\n";
    }

    for (int i = 0; i < pmesh->npolys; ++i)
    {
        const unsigned short* p = &pmesh->polys[i * 2 * pmesh->nvp];

        file << "f";
        for (int j = 0; j < pmesh->nvp; ++j)
        {
            if (p[j] == RC_MESH_NULL_IDX) break;
            file << " " << (p[j] + 1);
        }
        file << "\n";
    }

    file.close();
    Logger::Log("DEBUG: NavMesh exported to " + filename, LogType::Info);
}

static void savePolyMeshToOBJ(const rcPolyMesh* pmesh, const std::string& filename)
{
    if (!pmesh) return;

    std::ofstream file(filename);
    if (!file.is_open())
    {
        Logger::Log("Failed to open file for OBJ export: " + filename, LogType::Error);
        return;
    }

    for (int i = 0; i < pmesh->nverts; ++i)
    {
        const unsigned short* v = &pmesh->verts[i * 3];
        const float x = pmesh->bmin[0] + v[0] * pmesh->cs;
        const float y = pmesh->bmin[1] + v[1] * pmesh->ch;
        const float z = pmesh->bmin[2] + v[2] * pmesh->cs;
        file << "v " << x << " " << y << " " << z << "\n";
    }

    for (int i = 0; i < pmesh->npolys; ++i)
    {
        const unsigned short* p = &pmesh->polys[i * 2 * pmesh->nvp];
        file << "f";
        for (int j = 0; j < pmesh->nvp; ++j)
        {
            if (p[j] == RC_MESH_NULL_IDX) break;
            file << " " << (p[j] + 1);
        }
        file << "\n";
    }

    file.close();
    Logger::Log("DEBUG: NavMesh exported to " + filename, LogType::Info);
}

dtNavMesh* NavMeshLoader::BuildNavMesh()
{
    if (m_vertices.empty() || m_indices.empty())
    {
        Logger::Log("BuildNavMesh Error: No vertex or index data loaded.", LogType::Error);
        return nullptr;
    }
    float yOffset = -0.5f;
    std::vector<float> verts(m_vertices.size() * 3);
    for (size_t i = 0; i < m_vertices.size(); ++i)
    {
        verts[i * 3 + 0] = m_vertices[i].x;
        verts[i * 3 + 1] = m_vertices[i].y /*+ yOffset*/;
        verts[i * 3 + 2] = m_vertices[i].z;
    }

    int nverts = static_cast<int>(m_vertices.size());
    int ntris = static_cast<int>(m_indices.size() / 3);

    rcConfig cfg;
    memset(&cfg, 0, sizeof(cfg));

    rcCalcBounds(verts.data(), nverts, cfg.bmin, cfg.bmax);

    cfg.cs = 0.10f;
    cfg.ch = 0.05f;

	float agentHeight = Constants::AGENT_HEIGHT;
	float agentRadius = Constants::AGENT_RADIUS;

	float agentMaxClimb = Constants::AGENT_MAX_CLIMB;
	float agentMaxSlope = Constants::AGENT_MAX_SLOPE;   

    cfg.walkableSlopeAngle = agentMaxSlope;
    cfg.walkableHeight = (int)ceilf(agentHeight / cfg.ch);
    cfg.walkableClimb = (int)floorf(agentMaxClimb / cfg.ch);
 /*   cfg.walkableRadius = (int)ceilf(agentRadius / cfg.cs); */
    cfg.walkableRadius = 0;
    cfg.maxEdgeLen = (int)(12.0f / cfg.cs);
    cfg.maxSimplificationError = 1.3f;
 /*   cfg.minRegionArea = (int)(8.0f / (cfg.cs * cfg.cs));*/
    cfg.minRegionArea = 0;
    cfg.mergeRegionArea = (int)(20.0f / (cfg.cs * cfg.cs));
    cfg.maxVertsPerPoly = 6;
    cfg.detailSampleDist = 6.0f;
    cfg.detailSampleMaxError = 1.0f;
    cfg.tileSize = 0; 

    rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);
    Logger::Log(std::format("[NavMesh] Grid Size: {} x {}", cfg.width, cfg.height), LogType::Info);

    rcContext ctx;
    rcHeightfield* solid = rcAllocHeightfield();
    if (!solid) { Logger::Log("BuildNavMesh Error: Out of memory 'solid'.", LogType::Error); return nullptr; }

    if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
    {
        Logger::Log("BuildNavMesh Error: Could not create solid heightfield.", LogType::Error);
        rcFreeHeightField(solid); return nullptr;
    }

    if (!m_triAreas.data()) { Logger::Log("BuildNavMesh Error: m_triAreas null.", LogType::Error); rcFreeHeightField(solid); return nullptr; }

    rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, verts.data(), nverts, m_indices.data(), ntris, m_triAreas.data());

    if (!rcRasterizeTriangles(&ctx, verts.data(), nverts, m_indices.data(), m_triAreas.data(), ntris, *solid, cfg.walkableClimb))
    {
        Logger::Log("BuildNavMesh Error: Could not rasterize triangles.", LogType::Error);
        rcFreeHeightField(solid); return nullptr;
    }

    rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
    rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
    rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);

    rcCompactHeightfield* chf = rcAllocCompactHeightfield();
    if (!chf) { Logger::Log("BuildNavMesh Error: Out of memory 'chf'.", LogType::Error); rcFreeHeightField(solid); return nullptr; }

    if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf))
    {
        Logger::Log("BuildNavMesh Error: Could not build compact data.", LogType::Error);
        rcFreeHeightField(solid); rcFreeCompactHeightfield(chf); return nullptr;
    }
    rcFreeHeightField(solid);

    //if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf))
    //{
    //    Logger::Log("BuildNavMesh Error: Could not erode.", LogType::Error); rcFreeCompactHeightfield(chf); return nullptr;
    //}
    if (!rcBuildDistanceField(&ctx, *chf))
    {
        Logger::Log("BuildNavMesh Error: Could not build distance field.", LogType::Error); rcFreeCompactHeightfield(chf); return nullptr;
    }
    if (!rcBuildRegions(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
    {
        Logger::Log("BuildNavMesh Error: Could not build regions.", LogType::Error); rcFreeCompactHeightfield(chf); return nullptr;
    }

    rcContourSet* cset = rcAllocContourSet();
    if (!cset) { Logger::Log("BuildNavMesh Error: Out of memory 'cset'.", LogType::Error); rcFreeCompactHeightfield(chf); return nullptr; }

    if (!rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset))
    {
        Logger::Log("BuildNavMesh Error: Could not create contours.", LogType::Error);
        rcFreeCompactHeightfield(chf); rcFreeContourSet(cset); return nullptr;
    }

    rcPolyMesh* pmesh = rcAllocPolyMesh();
    if (!pmesh) { Logger::Log("BuildNavMesh Error: Out of memory 'pmesh'.", LogType::Error); rcFreeCompactHeightfield(chf); rcFreeContourSet(cset); return nullptr; }

    if (!rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh))
    {
        Logger::Log("BuildNavMesh Error: Could not build polymesh.", LogType::Error);
        rcFreeCompactHeightfield(chf); rcFreeContourSet(cset); rcFreePolyMesh(pmesh); return nullptr;
    }

    rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
    if (!dmesh) { Logger::Log("BuildNavMesh Error: Out of memory 'dmesh'.", LogType::Error); rcFreeCompactHeightfield(chf); rcFreeContourSet(cset); rcFreePolyMesh(pmesh); return nullptr; }

    if (!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh))
    {
        Logger::Log("BuildNavMesh Error: Could not build polymesh detail.", LogType::Error);
        rcFreeCompactHeightfield(chf); rcFreeContourSet(cset); rcFreePolyMesh(pmesh); rcFreePolyMeshDetail(dmesh); return nullptr;
    }

    rcFreeCompactHeightfield(chf);
    rcFreeContourSet(cset);

    for (int i = 0; i < pmesh->npolys; ++i)
    {
        if (pmesh->areas[i] == SAMPLE_POLYAREA_GROUND)
            pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
        else
            pmesh->flags[i] = 0;
    }

    if (pmesh->npolys == 0)
    {
        Logger::Log("CRITICAL ERROR: NavMesh generated but EMPTY...", LogType::Error);
    }
    else
    {
        Logger::Log(std::format("[NavMesh Success] Generated {} polygons...", pmesh->npolys), LogType::Info);

        savePolyMeshToOBJ(pmesh, "server_navmesh_debug.obj");
    }

    unsigned char* navData = 0;
    int navDataSize = 0;
    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));

    params.verts = pmesh->verts;
    params.vertCount = pmesh->nverts;
    params.polys = pmesh->polys;
    params.polyAreas = pmesh->areas;
    params.polyFlags = pmesh->flags;
    params.polyCount = pmesh->npolys;
    params.nvp = pmesh->nvp;
    params.detailMeshes = dmesh->meshes;
    params.detailVerts = dmesh->verts;
    params.detailVertsCount = dmesh->nverts;
    params.detailTris = dmesh->tris;
    params.detailTriCount = dmesh->ntris;

    params.walkableHeight = (float)cfg.walkableHeight * cfg.ch;
    params.walkableRadius = (float)cfg.walkableRadius * cfg.cs;
    params.walkableClimb = (float)cfg.walkableClimb * cfg.ch;

    rcVcopy(params.bmin, pmesh->bmin);
    rcVcopy(params.bmax, pmesh->bmax);
    params.cs = cfg.cs;
    params.ch = cfg.ch;
    params.buildBvTree = true;

    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
        Logger::Log("BuildNavMesh Error: dtCreateNavMeshData failed.", LogType::Error);
        rcFreePolyMesh(pmesh); rcFreePolyMeshDetail(dmesh); return nullptr;
    }

    dtNavMesh* navMesh = dtAllocNavMesh();
    if (!navMesh)
    {
        Logger::Log("BuildNavMesh Error: Alloc failed.", LogType::Error);
        dtFree(navData); rcFreePolyMesh(pmesh); rcFreePolyMeshDetail(dmesh); return nullptr;
    }

    dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
    if (dtStatusFailed(status))
    {
        Logger::Log("BuildNavMesh Error: navMesh->init failed.", LogType::Error);
        dtFree(navData); dtFreeNavMesh(navMesh);
        rcFreePolyMesh(pmesh); rcFreePolyMeshDetail(dmesh); return nullptr;
    }

    rcFreePolyMesh(pmesh);
    rcFreePolyMeshDetail(dmesh);

    return navMesh;
}

bool NavMeshLoader::LoadFromFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        Logger::Log(std::format("Error: Can't open the file: {}", path), LogType::Error);
        return false;
    }

    char bom[3] = { 0 };
    file.read(bom, 3);
    if (static_cast<unsigned char>(bom[0]) != 0xEF ||
        static_cast<unsigned char>(bom[1]) != 0xBB ||
        static_cast<unsigned char>(bom[2]) != 0xBF)
    {
        file.seekg(0, std::ios::beg);
    }

    m_vertices.clear();
    m_indices.clear();
    m_triAreas.clear();

    std::string line;
    std::string token;
    int vertexCount = 0;

    if (std::getline(file, line))
    {
        try { vertexCount = std::stoi(rtrim(line)); }
        catch (const std::exception& e)
        {
            Logger::Log(std::format("Error: Failed to read vertex count from first line. Content: '{}', Error: {}", line, e.what()), LogType::Error);
            file.close(); return false;
        }
    }
    else
    {
        Logger::Log("Error: File is empty or first line (vertex count) is missing.", LogType::Error);
        file.close(); return false;
    }

    if (vertexCount <= 0)
    {
        Logger::Log(std::format("Error: Invalid vertex count: {}. Must be > 0.", vertexCount), LogType::Error);
        file.close(); return false;
    }

    m_vertices.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i)
    {
        if (!std::getline(file, line))
        {
            Logger::Log(std::format("Error: Unexpected end of file. Expected {} vertices, but only found {}.", vertexCount, i), LogType::Error);
            file.close(); return false;
        }

        std::stringstream ss(rtrim(line));
        Vertex v;
        try
        {
            std::getline(ss, token, ','); v.x = std::stof(token);
            std::getline(ss, token, ','); v.y = std::stof(token);
            std::getline(ss, token, ','); v.z = std::stof(token);
            m_vertices.push_back(v);
        }
        catch (const std::exception& e)
        {
            Logger::Log(std::format("Error parsing vertex line [{}]: '{}'. Error: {}", i, line, e.what()), LogType::Error);
            file.close(); return false;
        }
    }

    int triangleCount = 0;
    if (std::getline(file, line))
    {
        try { triangleCount = std::stoi(rtrim(line)); }
        catch (const std::exception& e)
        {
            Logger::Log(std::format("Error: Failed to read triangle count. Content: '{}', Error: {}", line, e.what()), LogType::Error);
            file.close(); return false;
        }
    }
    else
    {
        Logger::Log("Error: Unexpected end of file. Expected triangle count after vertex data.", LogType::Error);
        file.close(); return false;
    }

    if (triangleCount <= 0)
    {
        Logger::Log(std::format("Error: Invalid triangle count: {}.", triangleCount), LogType::Error);
        file.close(); return false;
    }

    m_indices.reserve(triangleCount * 3);
    m_triAreas.reserve(triangleCount);

    for (int i = 0; i < triangleCount; ++i)
    {
        if (!std::getline(file, line))
        {
            Logger::Log(std::format("Error: Unexpected end of file. Expected {} triangles, but only found {}.", triangleCount, i), LogType::Error);
            file.close(); return false;
        }
        rtrim(line);
        if (line.empty()) { i--; continue; }

        std::stringstream ss(line);
        int i0, i1, i2;
        try
        {
            std::getline(ss, token, ','); i0 = std::stoi(token);
            std::getline(ss, token, ','); i1 = std::stoi(token);
            std::getline(ss, token, ','); i2 = std::stoi(token);
            m_indices.push_back(i0);
            m_indices.push_back(i1);
            m_indices.push_back(i2);
            m_triAreas.push_back(SAMPLE_POLYAREA_GROUND);
        }
        catch (const std::exception& e)
        {
            Logger::Log(std::format("Error parsing index line [{}]: '{}'. Error: gb{}", i, line, e.what()), LogType::Error);
            file.close(); return false;
        }
    }

    file.close();

    if (m_vertices.empty() || m_indices.empty())
    {
        Logger::Log("Error: No vertices or triangles loaded from the file.", LogType::Error);
        return false;
    }
    std::cout << std::format("[World] Successfully loaded {} vertices and {} triangles ({} indices).", m_vertices.size(), m_indices.size() / 3, m_indices.size()) << std::endl;

    return true;
}