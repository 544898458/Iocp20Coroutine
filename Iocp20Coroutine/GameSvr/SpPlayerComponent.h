#pragma once
#include <memory>
class PlayerComponent;
typedef std::shared_ptr<PlayerComponent> SpPlayerComponent;
typedef std::weak_ptr<PlayerComponent> WpPlayerComponent;