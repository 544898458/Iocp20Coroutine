#pragma once
#include <memory>
class Entity;
typedef std::shared_ptr<Entity> SpEntity;
typedef std::weak_ptr<Entity> WpEntity;

