//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Resource.h"
#include "RequestID.h"
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
// tinyxml
#include "tinyxml2.h"
// bsoncxx
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>
// std library
#include <algorithm>
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

    static const std::size_t NOTES_PER_PAGE;

    using UserNotes = std::vector<NoteMetaData>;
    using NoteHistory = std::unordered_map<dpp::snowflake, UserNotes>;
    using WorkerList = std::vector<std::string>;

    explicit JobRequest(const dpp::snowflake& guildID, const dpp::snowflake& customerID);
    virtual ~JobRequest() = default;

    const std::size_t GetCreatedTime() const { return m_timeCreated; }
    void SetCreatedTime(const std::size_t time) { m_timeCreated = time; }

    const std::size_t GetLastEditTime() const { return m_timeLastEdit; }
    void SetLastEditTime(const std::size_t time) { m_timeLastEdit = time; }

    const dpp::snowflake& GetCustomerID() const { return m_idCustomer; }
    void SetCustomerID(const dpp::snowflake& id) { m_idCustomer = id; }

    const std::vector<dpp::snowflake>& GetWorkerIDs() const { return m_idWorkerList; }
    bool AddWorkerID(const dpp::snowflake& id) 
    {
        if (m_idWorkerList.end() == std::find(m_idWorkerList.begin(), m_idWorkerList.end(), id))
        {
            m_idWorkerList.emplace_back(id);
            return true;
        }

        return false;
    }
    bool RemoveWorkerID(const dpp::snowflake& id)
    {
        bool bResult = false;

        m_idWorkerList.erase(
            std::remove_if( m_idWorkerList.begin(), m_idWorkerList.end(), [&](const auto& worker) 
                { 
                    if (worker == id)
                        bResult = true;
                    return worker == id;
                }), 
            m_idWorkerList.end()
        );

        return bResult;
    }
    bool IsWorker(const dpp::snowflake& user) const;

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

    const RequestID GetID() const { return m_id; }
    const void SetID(const RequestID rID) { m_id = rID; }

    const NoteHistory GetNoteHistory() const { return m_notes; }
    const UserNotes GetNoteHistory(const dpp::snowflake user) const { return m_notes.at(user); }
    void AddNote(const dpp::snowflake& id, const JobRequest::NoteMetaData& meta);
    const std::size_t NoteHistorySize() const;
    const std::string PrintNoteHistory(dpp::cluster& cluster, const std::size_t page) const;
    const std::string PrintLastTwoNotes(dpp::cluster& cluster) const;

    bool IsCustomerSubscribed() const { return m_bNotifyCustomer; }
    void SubscribeCustomer(const bool bUpdate) { m_bNotifyCustomer = bUpdate; }

    virtual std::uint64_t JobType() const { return JOB_TYPE_GENERAL; }
    virtual std::string JobTypeToString() const { return "General"; }
    virtual bool SupportsType(const std::uint64_t type) const { return type == JobType(); }

    // Serialization + Deserialization: XML
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode);
    virtual void WriteChildren(tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const;
    virtual void ReadChildren(tinyxml2::XMLElement* xmlNode);
    
    // Serialization + Deserialization: BSON
    virtual bsoncxx::builder::basic::document WriteAttributesBSON() const;
    virtual void ReadAttributesBSON(const bsoncxx::document::view& doc);

    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bPrintNames = false) const;
    const std::string PrintJobDetailsCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bPrintNames = false) const;
    const std::string GetCustomerName(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;
    const WorkerList GetWorkerNames(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;

private:
    RequestID m_id;                         // Unique identifier for the job
    dpp::snowflake m_idGuild = ID_NULL;
    std::uint64_t m_timeCreated = 0;
    std::uint64_t m_timeLastEdit = 0;
    dpp::snowflake m_idCustomer = ID_NULL;  // Customer id of who submitted the job
    std::vector<dpp::snowflake> m_idWorkerList;// Id of the worker assigned to the job
    std::string m_strSCHandle = "n/a";      // SC Handle for identification (could be username or custom ID)
    priority m_eJobPriority = priority::low;// Priority of the job
    status m_eJobStatus = status::open;     // Current status of the job
    NoteHistory m_notes;
    bool m_bNotifyCustomer = false;
};