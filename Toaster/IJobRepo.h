#pragma once
#include "JobRequest.h"
// dpp
#include <dpp/snowflake.h>
// microsoft
#include <guiddef.h>
// std library
#include <memory>
#include <vector>

class IJobRepo
{
public:
    virtual ~IJobRepo() = default;

    virtual void InsertJob(const dpp::snowflake& guildID, const std::shared_ptr<const JobRequest>& job) = 0;
    virtual void UpdateJob(const std::shared_ptr<const JobRequest>& job) = 0;
    virtual void DeleteJob(const GUID&) = 0;

    virtual std::vector<dpp::snowflake> GetGuildsWithJobs() = 0;
    virtual std::vector<std::shared_ptr<JobRequest>> LoadJobs(dpp::snowflake guild) = 0;
};

