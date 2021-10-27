#pragma once
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <map>
#include <set>

constexpr bool DEBUG =
#ifdef NDEBUG
false;
#else
true;
#endif