#pragma once
#include "vulkan/vulkan.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <map>
#include <set>
#include <array>

constexpr bool DEBUG =
#ifdef NDEBUG
false;
#else
true;
#endif