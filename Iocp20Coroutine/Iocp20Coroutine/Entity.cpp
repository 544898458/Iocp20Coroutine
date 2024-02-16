#include "Entity.h"
#include "MySession.h"
Entity::Entity(float x, Space& space,std::function< CoTask<int> (Entity*,float&,bool&)> fun):space ( space)
{
	//����һ��Э�̣������߶�
	this->x = x;
	co = fun(this,this->x, coStop);
}

void Entity::ReplaceCo(std::function< CoTask<int>(Entity*, float&, bool&)> fun)
{
	coStop = true;
	co.Run();
	assert(co.Finished());//20240205
	coStop = false;
	co = fun(this, this->x, coStop);
}