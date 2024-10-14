#include "pch.h"
#include "CrowdToolState.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"
#include "Sample_TempObstacles.h"

void CrowdToolState::addAgent(const float* p)
{
	if (!m_sample) return;
	dtCrowd* crowd = m_sample->getCrowd();

	dtCrowdAgentParams ap;
	memset(&ap, 0, sizeof(ap));
	ap.radius = m_sample->getAgentRadius();
	ap.height = m_sample->getAgentHeight();
	ap.maxAcceleration = 8.0f;
	ap.maxSpeed = 3.5f;
	ap.collisionQueryRange = ap.radius * 12.0f;
	ap.pathOptimizationRange = ap.radius * 30.0f;
	ap.updateFlags = 0;
	//if (m_toolParams.m_anticipateTurns)
	ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	//if (m_toolParams.m_optimizeVis)
	ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	//if (m_toolParams.m_optimizeTopo)
	ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	//if (m_toolParams.m_obstacleAvoidance)
	ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	//if (m_toolParams.m_separation)
	ap.updateFlags |= DT_CROWD_SEPARATION;
	ap.obstacleAvoidanceType = 3;// (unsigned char)m_toolParams.m_obstacleAvoidanceType;
	ap.separationWeight = 2.00f;// m_toolParams.m_separationWeight;

	int idx = crowd->addAgent(p, &ap);
	if (idx != -1)
	{
		//if (m_targetRef)
		//	crowd->requestMoveTarget(idx, m_targetRef, m_targetPos);

		//// Init trail
		//AgentTrail* trail = &m_trails[idx];
		//for (int i = 0; i < AGENT_MAX_TRAIL; ++i)
		//	dtVcopy(&trail->trail[i * 3], p);
		//trail->htrail = 0;
	}
}