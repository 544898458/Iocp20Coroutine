#include "pch.h"
#include "Sample_TempObstacles.h"
#include "../recastnavigation-main/DetourTileCache/Include/DetourTileCache.h"

void Sample_TempObstacles::handleUpdate(const float dt)
{
	if (!m_navMesh)
		return;
	if (!m_tileCache)
		return;

	m_tileCache->update(dt, m_navMesh);
}
