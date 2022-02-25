#pragma once

#include <stddef.h>
#include <utility>
#include <intrin.h>
#include <string>
#include <sstream>
#include <span>
#include <variant>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <stdint.h>
#include "Types.h"

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))