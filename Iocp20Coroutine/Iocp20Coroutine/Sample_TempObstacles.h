#pragma once

#include "../recastnavigation-main/RecastDemo/Include/Sample.h"
//class InputGeom;
//class dtNavMesh;
//class dtNavMeshQuery;
//class dtCrowd;
//class dtTileCache;
//
//enum SamplePolyFlags
//{
//	SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
//	SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
//	SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
//	SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
//	SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
//	SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
//};
//
//class Sample
//{
//public:
//	class dtNavMesh* getNavMesh() { return m_navMesh; }
//	class InputGeom* getInputGeom() { return m_geom; }
//	class dtNavMeshQuery* getNavMeshQuery() { return m_navQuery; }
//	class dtCrowd* getCrowd() { return m_crowd; }
//	float getAgentRadius() { return m_agentRadius; }
//	float getAgentHeight() { return m_agentHeight; }
//	float getAgentClimb() { return m_agentMaxClimb; }
//
//	class InputGeom* m_geom;
//	class dtNavMesh* m_navMesh;
//	class dtNavMeshQuery* m_navQuery;
//	class dtCrowd* m_crowd;
//
//	unsigned char m_navMeshDrawFlags;
//
//	float m_cellSize;
//	float m_cellHeight;
//	float m_agentHeight;
//	float m_agentRadius;
//	float m_agentMaxClimb;
//	float m_agentMaxSlope;
//	float m_regionMinSize;
//	float m_regionMergeSize;
//	float m_edgeMaxLen;
//	float m_edgeMaxError;
//	float m_vertsPerPoly;
//	float m_detailSampleDist;
//	float m_detailSampleMaxError;
//	int m_partitionType;
//
//	bool m_filterLowHangingObstacles;
//	bool m_filterLedgeSpans;
//	bool m_filterWalkableLowHeightSpans;
//
//	Sample();
//
//};

class Sample_TempObstacles :public Sample
{
public:
	Sample_TempObstacles();
	void handleUpdate(const float dt);
	bool handleBuild();
	struct LinearAllocator* m_talloc;
	struct FastLZCompressor* m_tcomp;
	struct MeshProcess* m_tmproc;

	class dtTileCache* m_tileCache;
	int m_maxTiles;
	int m_maxPolysPerTile;
	float m_tileSize;
	bool m_keepInterResults;

	float m_cacheBuildTimeMs;
	int m_cacheCompressedSize;
	int m_cacheRawSize;
	int m_cacheLayerCount;
	unsigned int m_cacheBuildMemUsage;

	enum DrawMode
	{
		DRAWMODE_NAVMESH,
		DRAWMODE_NAVMESH_TRANS,
		DRAWMODE_NAVMESH_BVTREE,
		DRAWMODE_NAVMESH_NODES,
		DRAWMODE_NAVMESH_PORTALS,
		DRAWMODE_NAVMESH_INVIS,
		DRAWMODE_MESH,
		DRAWMODE_CACHE_BOUNDS,
		MAX_DRAWMODE
	};

	DrawMode m_drawMode;

};

