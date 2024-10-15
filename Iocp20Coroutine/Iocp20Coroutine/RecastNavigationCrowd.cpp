#include "pch.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"
#include "../recastnavigation-main/Detour/Include/DetourCommon.h"
#include "../recastnavigation-main/RecastDemo/Include/PerfTimer.h"
#include "CrowdTool.h"
#include "../recastnavigation-main/RecastDemo/Include/Sample_TempObstacles.h"

CrowdToolState& GetCrowdTool()
{
	static CrowdToolState* p;
	if (nullptr == p)
	{
		p = new CrowdToolState();
		p->init(new Sample_TempObstacles());
	}
	return *p;
}
void CrowToolAddAgent(float arrF[])
{
	GetCrowdTool().addAgent(arrF);
}

void CrowToolUpdate()
{
	const float SIM_RATE = 20;
	const float DELTA_TIME = 1.0f / SIM_RATE;
	GetCrowdTool().handleUpdate(DELTA_TIME);
}

void CrowdToolSetMoveTarget(const float* p, bool adjust)
{
	GetCrowdTool().setMoveTarget(p, adjust);
}