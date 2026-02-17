#pragma once

#include "tinyxml2.h"
#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>

#include <objbase.h>

constexpr std::size_t JOB_TYPE_GENERAL{ 10 };
constexpr std::size_t JOB_TYPE_CRAFTING{ 20 };
constexpr std::size_t JOB_TYPE_BUILDING{ 30 };
constexpr std::size_t JOB_TYPE_RESOURCE{ 40 };
constexpr std::size_t JOB_TYPE_REFINERY{ 50 };

constexpr std::string_view LOW_PRIORITY_ID{ "low" };
constexpr std::string_view MED_PRIORITY_ID{ "med" };
constexpr std::string_view HIGH_PRIORITY_ID{ "high" };
constexpr std::string_view CRITICAL_PRIORITY_ID{ "critical" };