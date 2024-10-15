#include "pch.h"
#include "Sample_TempObstacles.h"
#include "../recastnavigation-main/DetourTileCache/Include/DetourTileCache.h"
#include "../recastnavigation-main/DebugUtils/Include/DetourDebugDraw.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"

Sample::Sample() :
	m_geom(0),
	m_navMesh(0),
	m_navQuery(0),
	m_crowd(0),
	m_navMeshDrawFlags(DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_CLOSEDLIST),
	m_filterLowHangingObstacles(true),
	m_filterLedgeSpans(true),
	m_filterWalkableLowHeightSpans(true)
	//m_tool(0),
	//m_ctx(0)
{
	//resetCommonSettings();
	m_navQuery = dtAllocNavMeshQuery();
	m_crowd = dtAllocCrowd();

	//for (int i = 0; i < MAX_TOOLS; i++)
		//m_toolStates[i] = 0;
}

void Sample_TempObstacles::handleUpdate(const float dt)
{
	if (!m_navMesh)
		return;
	if (!m_tileCache)
		return;

	m_tileCache->update(dt, m_navMesh);
}
