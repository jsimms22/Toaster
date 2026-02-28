//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
// std library
#include <cstdlib>
#include <memory>

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class JobRequestFactory
{
public:
    static std::shared_ptr<JobRequest> Create(const std::size_t type);
};

