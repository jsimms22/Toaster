//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Resource.h"
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
// microsoft
#include <guiddef.h>
// tinyxml
#include "tinyxml2.h"
// bsoncxx
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>
// std library
#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>

//---------------------------------------------------------------------------------------------------------------------
/// \class JobRequest
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
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

    struct NoteMetaData
    {
        dpp::snowflake guildID;
        std::size_t timestamp;
        std::string note;
    };

    using UserNotes = std::vector<NoteMetaData>;
    using NoteHistory = std::unordered_map<dpp::snowflake, UserNotes>;

    explicit JobRequest(const dpp::snowflake& guildID);
    virtual ~JobRequest() = default;

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
    static std::string PriorityToString(const priority p);
    static priority StringToPriority(const std::string& str);
    static const char* PriorityToEmoji(const priority p);

    const status GetStatus() const { return m_eJobStatus; }
    void SetStatus(const status s) { m_eJobStatus = s; }
    static std::string StatusToString(const status s);
    static status StringToStatus(const std::string& str);
    static const char* StatusToEmoji(const status s);

    const GUID& GetID() const { return m_id; }

    const NoteHistory GetNoteHistory() const { return m_notes; }
    const UserNotes GetNoteHistory(const dpp::snowflake user) const { return m_notes.at(user); }
    void AddNote(const dpp::snowflake& id, const JobRequest::NoteMetaData& meta);
    const std::string PrintNoteHistory(dpp::cluster& cluster) const;
    const std::string PrintLastTwoNotes(dpp::cluster& cluster) const;

    bool IsCustomerSubscribed() const { return m_bNotifyCustomer; }
    void SubscribeCustomer(const bool bUpdate) { m_bNotifyCustomer = bUpdate; }

    virtual std::size_t JobType() const { return JOB_TYPE_GENERAL; }
    virtual std::string JobTypeToString() const { return "General"; }
    virtual bool SupportsType(const std::size_t type) const { return type == JobType(); }

    // Serialization + Deserialization: XML
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode);
    virtual void WriteChildren(tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const;
    virtual void ReadChildren(tinyxml2::XMLElement* xmlNode);
    
    // Serialization + Deserialization: BSON
    virtual bsoncxx::builder::basic::document WriteAttributesBSON() const;
    virtual void ReadAttributesBSON(const bsoncxx::document::view& doc);

    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;
    std::string PrintJobDetailsCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;
    const std::string GetCustomerName(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;
    const std::string GetWorkerName(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;

private:
    dpp::snowflake m_idGuild = 0;
    std::size_t m_timeCreated = 0;
    std::size_t m_timeLastEdit = 0;
    dpp::snowflake m_idCustomer = USERID_NULL;  // Customer id of who submitted the job
    dpp::snowflake m_idWorker = USERID_NULL;    // Id of the worker assigned to the job
    std::string m_strSCHandle = "n/a";          // SC Handle for identification (could be username or custom ID)
    priority m_eJobPriority = priority::low;    // Priority of the job
    status m_eJobStatus = status::open;         // Current status of the job
    GUID m_id = GUID_NULL;                      // Unique identifier for the job
    NoteHistory m_notes;
    bool m_bNotifyCustomer = false;
};