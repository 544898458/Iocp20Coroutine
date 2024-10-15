#pragma once
class InputGeom;
class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
class dtTileCache;

class Sample
{
public:
	class dtCrowd* getCrowd() { return m_crowd; }
	float getAgentRadius() { return m_agentRadius; }
	float getAgentHeight() { return m_agentHeight; }
	float getAgentClimb() { return m_agentMaxClimb; }

	class InputGeom* m_geom;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	class dtCrowd* m_crowd;

	unsigned char m_navMeshDrawFlags;

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	int m_partitionType;

	bool m_filterLowHangingObstacles;
	bool m_filterLedgeSpans;
	bool m_filterWalkableLowHeightSpans;

};

class Sample_TempObstacles :public Sample
{
public:
	void handleUpdate(const float dt);
	class InputGeom* getInputGeom() { return m_geom; }
	class dtNavMesh* getNavMesh() { return m_navMesh; }
	class dtNavMeshQuery* getNavMeshQuery() { return m_navQuery; }
	class dtCrowd* getCrowd() { return m_crowd; }
	float getAgentRadius() { return m_agentRadius; }
	float getAgentHeight() { return m_agentHeight; }
	float getAgentClimb() { return m_agentMaxClimb; }

	class dtTileCache* m_tileCache;
};

