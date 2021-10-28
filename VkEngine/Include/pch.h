#pragma once
#include "VkRenderer/pch.h"
#include <fstream>
#include <array>
#include "Cecsar.h"

constexpr uint32_t ENTITY_MAX_COUNT = 100;
typedef ce::Cecsar<ENTITY_MAX_COUNT> Cecsar;