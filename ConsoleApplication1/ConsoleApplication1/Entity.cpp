#include "Entity.h"
#include "MySession.h"
Entity::Entity(float x, Space& space,std::function< CoTask<int> (float&)> fun):space ( space)
{
	//创建一个协程，来回走动
	this->x = x;
	co = fun(this->x);
}