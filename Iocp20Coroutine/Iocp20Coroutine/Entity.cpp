#include "Entity.h"
#include "MySession.h"
Entity::Entity(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun) :m_space(m_space)
{
	//创建一个协程，来回走动
	this->x = x;
	m_coWalk = fun(this, this->x, this->z, m_coStop);
}

void Entity::ReplaceCo(std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun)
{
	m_coStop = true;
	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	m_coStop = false;
	m_coWalk = fun(this, this->x, this->z, m_coStop);
}