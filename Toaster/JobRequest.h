#pragma once
#include "Resource.h"
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
// microsoft
#include <guiddef.h>
// tinyxml
#include "tinyxml2.h"
// std library
#include <chrono>
#include <string>
#include <cstdlib>

class JobRequest
{
public:
    // Enum for Job status
    enum status
    {
        open = 1,
        stalled,
        assigned,
        active,
        hold,
        complete
    };

    // Enum for Job priority
    enum priority
    {
        low = 1,
        medium,
        high,
        critical
    };

    JobRequest();
    ~JobRequest() = default;

    const std::size_t GetCreatedTime() const { return m_timeCreated; }
    void SetCreatedTime(const std::size_t time) { m_timeCreated = time; }

    const std::size_t GetLastEditTime() const { return m_timeLastEdit; }
    void SetLastEditTime(const std::size_t time) { m_timeLastEdit = time; }

    const dpp::snowflake& GetCustomerID() const { return m_idCustomer; }
    void SetCustomerID(const dpp::snowflake& id) { m_idCustomer = id; }

    const dpp::snowflake& GetWorkerID() const { return m_idWorker; }
    void SetWorkerID(const dpp::snowflake& id) { m_idWorker = id; }

    const std::string& GetSCHandle() const { return m_strSCHandle; }
    void SetSCHandle(const std::string& handle) { m_strSCHandle = handle; }

    const priority GetPriority() const { return m_eJobPriority; }
    void SetPriority(const priority p) { m_eJobPriority = p; }
    static std::string PriorityToString(priority p);
    static priority StringToPriority(const std::string& str);
    static const char* PriorityToEmoji(priority p);

    const status GetStatus() const { return m_eJobStatus; }
    void SetStatus(const status s) { m_eJobStatus = s; }
    static std::string StatusToString(status s);
    static status StringToStatus(const std::string& str);
    static const char* StatusToEmoji(status s);

    const GUID& GetID() const { return m_id; }

    virtual std::size_t JobType() const { return JOB_TYPE_GENERAL; }
    virtual std::string JobTypeToString() const { return "General"; }
    virtual bool SupportsType(const std::size_t type) const { return type == JobType(); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent);
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent);
    virtual std::string PrintJobDetails(dpp::cluster& cluster) const;

    const std::string GetCustomerName(dpp::cluster& cluster) const;
    const std::string GetWorkerName(dpp::cluster& cluster) const;

private:
    std::size_t m_timeCreated;
    std::size_t m_timeLastEdit;
    dpp::snowflake m_idCustomer;            // Customer id of who submitted the job
    dpp::snowflake m_idWorker;              // Id of the worker assigned to the job
    std::string m_strSCHandle;              // SC Handle for identification (could be username or custom ID)
    priority m_eJobPriority = priority::low;// Priority of the job
    status m_eJobStatus = status::open;     // Current status of the job
    GUID m_id;                              // Unique identifier for the job
    // todo std::vector<std::string> m_vNotes
};