#pragma once
#include <string>
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <vector>
#include <iostream>
#include <functional>
#include <map>

constexpr bool DEBUG =
#ifdef NDEBUG
false;
#else
true;
#endif