#pragma once

class Sample_TempObstacles;

class CrowdTool
{
public:
	void addAgent(const float* pos);
	void handleUpdate(const float dt);
	void updateTick(const float dt);
	void setMoveTarget(const float* p, bool adjust);

	Sample_TempObstacles* m_sample;
	bool m_run = true;

	dtCrowdAgentDebugInfo m_agentDebug;
	dtObstacleAvoidanceDebugData* m_vod;

	float m_targetPos[3];
	dtPolyRef m_targetRef;


};

