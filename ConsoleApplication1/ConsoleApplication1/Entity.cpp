#include "Entity.h"
#include "MySession.h"
Entity::Entity(float x, Space& space,std::function< CoTask<int> (float&)> fun):space ( space)
{
	//����һ��Э�̣������߶�
	this->x = x;
	co = fun(this->x);
}