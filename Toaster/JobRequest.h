#pragma once
#include "Resource.h"
#include "tinyxml2.h"

#include <chrono>
#include <string>
#include <cstdlib>
// Microsoft
#include <guiddef.h>

class JobRequest
{
public:
    // Enum for Job status
    enum status
    {
        open = 1,
        stalled,
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

    const std::string& GetAuthor() const { return m_userAuthor; }
    void SetAuthor(const std::string& author) { m_userAuthor = author; }

    const std::string& GetWorker() const { return m_userWorker; }
    void SetWorker(const std::string& worker) { m_userWorker = worker; }

    const std::string& GetSCHandle() const { return m_strSCHandle; }
    void SetSCHandle(const std::string& handle) { m_strSCHandle = handle; }

    priority GetPriority() const { return m_eJobPriority; }
    void SetPriority(const priority p) { m_eJobPriority = p; }

    status GetStatus() const { return m_eJobStatus; }
    void SetStatus(const status s) { m_eJobStatus = s; }

    // Getter for the GUID
    const GUID& GetID() const { return m_id; }

    static std::string StatusToString(status s);
    static std::string PriorityToString(priority p);
    static status StringToStatus(const std::string& str);
    static priority StringToPriority(const std::string& str);

    virtual std::size_t JobType() const { return JOB_TYPE_GENERAL; }
    virtual std::string JobTypeToString() const { return "General"; }
    virtual bool SupportsType(const std::size_t type) const { return type == JobType(); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent);
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent);
    virtual std::string PrintJobDetails() const;

private:
    std::size_t m_timeCreated;
    std::size_t m_timeLastEdit;
    std::string m_userAuthor;               // Author of the job (creator)
    std::string m_userWorker;               // Worker assigned to the job
    std::string m_strSCHandle;              // SC Handle for identification (could be username or custom ID)
    priority m_eJobPriority = priority::low;// Priority of the job
    status m_eJobStatus = status::open;     // Current status of the job
    GUID m_id;                              // Unique identifier for the job
};