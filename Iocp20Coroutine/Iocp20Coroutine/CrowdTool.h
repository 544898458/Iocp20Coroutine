#pragma once

class Sample_TempObstacles;

class CrowdToolState// : public SampleToolState
{
public:
	void addAgent(const float* pos);
	void updateTick(const float dt);
	void handleUpdate(const float dt);
	void setMoveTarget(const float* p, bool adjust);

	float m_targetPos[3];
	dtPolyRef m_targetRef;

	Sample_TempObstacles* m_sample;
	dtCrowdAgentDebugInfo m_agentDebug;
	dtObstacleAvoidanceDebugData* m_vod;
	bool m_run = true;

};

class CrowdTool
{
public:
	
	
	


};

