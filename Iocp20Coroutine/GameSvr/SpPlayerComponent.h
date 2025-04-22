#pragma once
#include <memory>
class PlayerComponent;
typedef std::unique_ptr<PlayerComponent> UpPlayerComponent;
typedef std::weak_ptr<PlayerComponent> WpPlayerComponent;