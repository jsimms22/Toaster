#include "JobRequest.h"

#include "BotUtility.h"
// d++
#include <dpp/cache.h>
#include <dpp/user.h>
#include <dpp/unicode_emoji.h>
// fmt
#include <fmt/chrono.h>
#include <fmt/format.h>
// bsoncxx
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/concatenate.hpp>
#include <bsoncxx/builder/basic/array.hpp>
// std library
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>

namespace serial
{
    constexpr const char* pszRequestID{ "_id" };
    constexpr const char* pszGuild{ "_guild" };
    constexpr const char* pszCreation{ "created_time" };
    constexpr const char* pszLastEdit{ "last_edit_time" };
    constexpr const char* pszRequestUser{ "customer_id" };
    constexpr const char* pszJobWorkers{ "worker_id" };
    constexpr const char* pszRequestSCHandle{ "sc_handle" };
    constexpr const char* pszJobPriority{ "priority" };
    constexpr const char* pszJobStatus{ "status" };
    constexpr const char* pszJobType{ "job_type" };
    constexpr const char* pszSubscribed{ "notify_customer" };
}

const std::size_t JobRequest::NOTES_PER_PAGE{ 6 };

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string JobRequest::PriorityToString(const JobRequest::priority p)
{
    switch (p) {
    case JobRequest::priority::low:         return "Low";
    case JobRequest::priority::medium:      return "Medium";
    case JobRequest::priority::high:        return "High";
    case JobRequest::priority::critical:    return "Critical";
    default: throw std::invalid_argument("Unexpected priority enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
JobRequest::priority JobRequest::StringToPriority(const std::string& str)
{
    if (str == "Low")       return JobRequest::priority::low;
    if (str == "Medium")    return JobRequest::priority::medium;
    if (str == "High")      return JobRequest::priority::high;
    if (str == "Critical")  return JobRequest::priority::critical;
    throw std::invalid_argument("Unexpected priority string value");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const char* JobRequest::PriorityToEmoji(const JobRequest::priority p)
{
    switch (p) {
    case JobRequest::priority::low:         return dpp::unicode_emoji::green_circle;
    case JobRequest::priority::medium:      return dpp::unicode_emoji::yellow_circle;
    case JobRequest::priority::high:        return dpp::unicode_emoji::orange_circle;
    case JobRequest::priority::critical:    return dpp::unicode_emoji::red_circle;
    default:                                return dpp::unicode_emoji::black_circle;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string JobRequest::StatusToString(const JobRequest::status s)
{
    switch (s) {
    case JobRequest::status::open:      return "Open";
    case JobRequest::status::stalled:   return "Stalled";
    case JobRequest::status::assigned:  return "Assigned";
    case JobRequest::status::active:    return "Active";
    case JobRequest::status::hold:      return "On Hold";
    case JobRequest::status::complete:  return "Complete";
    default: throw std::invalid_argument("Unexpected status enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
JobRequest::status JobRequest::StringToStatus(const std::string& str)
{
    if (str == "Open")      return JobRequest::status::open;
    if (str == "Stalled")   return JobRequest::status::stalled;
    if (str == "Assigned")  return JobRequest::status::assigned;
    if (str == "Active")    return JobRequest::status::active;
    if (str == "On Hold")   return JobRequest::status::hold;
    if (str == "Complete")  return JobRequest::status::complete;
    throw std::invalid_argument("Unexpected status string value");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const char* JobRequest::StatusToEmoji(const JobRequest::status s)
{
    switch (s) {
    case JobRequest::status::open:      return dpp::unicode_emoji::white_circle;
    case JobRequest::status::stalled:   return dpp::unicode_emoji::orange_circle;
    case JobRequest::status::assigned:  return dpp::unicode_emoji::yellow_circle;
    case JobRequest::status::active:    return dpp::unicode_emoji::green_circle;
    case JobRequest::status::hold:      return dpp::unicode_emoji::red_circle;
    case JobRequest::status::complete:  return dpp::unicode_emoji::blue_circle;
    default:                            return dpp::unicode_emoji::black_circle;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
JobRequest::JobRequest(const dpp::snowflake& guildID, const dpp::snowflake& customerID)
    : m_idGuild{ guildID }, m_idCustomer{ customerID }
{
    m_timeCreated = utils::GetEpochTimestamp();
    m_timeLastEdit = m_timeCreated;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent)
    {
        return;
    }

    // Create a new <Request> element for this submission
    xmlNode->SetAttribute(serial::pszRequestID, m_id.value);
    xmlNode->SetAttribute(serial::pszCreation, m_timeCreated);
    xmlNode->SetAttribute(serial::pszLastEdit, m_timeLastEdit);
    xmlNode->SetAttribute(serial::pszJobPriority, static_cast<int>(m_eJobPriority));
    xmlNode->SetAttribute(serial::pszJobStatus, static_cast<int>(m_eJobStatus));
    xmlNode->SetAttribute(serial::pszJobType, JobType());
    //xmlNode->SetAttribute(serial::pszJobWorkers, m_idWorker);
    xmlNode->SetAttribute(serial::pszRequestUser, m_idCustomer);
    xmlNode->SetAttribute(serial::pszRequestSCHandle, m_strSCHandle.c_str());
    xmlNode->SetAttribute(serial::pszSubscribed, m_bNotifyCustomer);

    // Write notes as child elements
    WriteChildren(xmlNode, xmlNode->GetDocument());

    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::WriteChildren(tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlParent) return;

    // Create a <Notes> container element
    tinyxml2::XMLElement* xmlNotes = doc->NewElement("Notes");

    for (const auto& [userID, notes] : m_notes)
    {
        for (const auto& note : notes)
        {
            // <Note> element for each note
            tinyxml2::XMLElement* xmlNote = doc->NewElement("Note");
            xmlNote->SetAttribute("UserID", static_cast<uint64_t>(userID));  // store as uint64
            xmlNote->SetAttribute("GuildID", static_cast<uint64_t>(note.guildID));
            xmlNote->SetAttribute("Timestamp", note.timestamp);
            xmlNote->SetText(note.note.c_str());  // store the note text as element content

            xmlNotes->InsertEndChild(xmlNote);
        }
    }

    xmlParent->InsertEndChild(xmlNotes);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    m_id = xmlNode->Unsigned64Attribute(serial::pszRequestID, ID_NULL);
    m_timeCreated = xmlNode->Unsigned64Attribute(serial::pszCreation, 0);
    m_timeLastEdit = xmlNode->Unsigned64Attribute(serial::pszLastEdit, 0);
    m_eJobPriority = static_cast<priority>(xmlNode->IntAttribute(serial::pszJobPriority, priority::low));
    m_eJobStatus = static_cast<status>(xmlNode->IntAttribute(serial::pszJobStatus, status::open));
    //m_idWorker = xmlNode->Unsigned64Attribute(serial::pszJobWorkers, ID_NULL);
    m_idCustomer = xmlNode->Unsigned64Attribute(serial::pszRequestUser, ID_NULL);
    const char* pszHandle = xmlNode->Attribute(serial::pszRequestSCHandle);
    if (pszHandle)
    {
        m_strSCHandle = pszHandle;
    }

    m_bNotifyCustomer = xmlNode->BoolAttribute(serial::pszSubscribed, false);

    // Read notes
    ReadChildren(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::ReadChildren(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode) return;

    tinyxml2::XMLElement* xmlNotes = xmlNode->FirstChildElement("Notes");
    if (!xmlNotes) return;

    for (tinyxml2::XMLElement* xmlNote = xmlNotes->FirstChildElement("Note");
        xmlNote != nullptr;
        xmlNote = xmlNote->NextSiblingElement("Note"))
    {
        NoteMetaData note;
        uint64_t userID = xmlNote->Unsigned64Attribute("UserID", 0);
        note.guildID = xmlNote->Unsigned64Attribute("GuildID", 0);
        note.timestamp = xmlNote->Unsigned64Attribute("Timestamp", 0);

        const char* text = xmlNote->GetText();
        note.note = text ? text : "";

        m_notes[dpp::snowflake{ userID }].push_back(note);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document JobRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;

    document doc{};

    doc.append(
        kvp(std::string{ serial::pszRequestID }, static_cast<std::int64_t>(m_id.value)),
        kvp(std::string{ serial::pszGuild }, static_cast<std::int64_t>(m_idGuild)),
        kvp(std::string{ serial::pszJobType }, static_cast<int>(JobType())),
        kvp(std::string{ serial::pszCreation }, static_cast<int64_t>(m_timeCreated)),
        kvp(std::string{ serial::pszLastEdit}, static_cast<int64_t>(m_timeLastEdit)),
        kvp(std::string{ serial::pszRequestUser}, static_cast<int64_t>(m_idCustomer)),
        kvp(std::string{ serial::pszJobWorkers }, [&]() {
            bsoncxx::builder::basic::array a;
            for (const auto& worker : m_idWorkerList)
                a.append(static_cast<int64_t>(worker));
            return a;
            }()),
        kvp(std::string{ serial::pszRequestSCHandle}, m_strSCHandle),
        kvp(std::string{ serial::pszJobPriority}, static_cast<int>(m_eJobPriority)),
        kvp(std::string{ serial::pszJobStatus}, static_cast<int>(m_eJobStatus)),
        kvp(std::string{ serial::pszSubscribed}, m_bNotifyCustomer)
    );

    doc.append(kvp("notes", [&](sub_array notesArray)
        {
            for (const auto& [userID, notes] : m_notes)
            {
                notesArray.append([&](sub_document userDoc)
                    {
                        userDoc.append(kvp("user_id", static_cast<int64_t>(userID)));
                        userDoc.append(kvp("notes", [&](sub_array noteArray)
                            {
                                for (const auto& note : notes)
                                {
                                    noteArray.append(make_document(
                                        kvp("guild_id", static_cast<int64_t>(note.guildID)),
                                        kvp("timestamp", static_cast<int64_t>(note.timestamp)),
                                        kvp("note", note.note)
                                    ));
                                }
                            }));
                    });
            }
        }));

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::ReadAttributesBSON(const bsoncxx::document::view& doc)
{// IDs / timestamps
    if (auto elem = doc[serial::pszRequestID]; elem && elem.type() == bsoncxx::type::k_int64)
    {
        m_id = static_cast<std::uint64_t>(elem.get_int64().value);
    }

    if (auto elem = doc[serial::pszGuild]; elem && elem.type() == bsoncxx::type::k_int64)
    {
        m_idGuild = static_cast<std::uint64_t>(elem.get_int64().value);
    }

    if (auto elem = doc[serial::pszCreation]; elem && elem.type() == bsoncxx::type::k_int64)
    {
        m_timeCreated = elem.get_int64().value;
    }

    if (auto elem = doc[serial::pszLastEdit]; elem && elem.type() == bsoncxx::type::k_int64)
    {
        m_timeLastEdit = elem.get_int64().value;
    }

    if (auto elem = doc[serial::pszRequestUser]; elem && elem.type() == bsoncxx::type::k_int64)
    {
        m_idCustomer = static_cast<std::uint64_t>(elem.get_int64().value);
    }


    // Worker list
    if (auto elem = doc[serial::pszJobWorkers]; elem && elem.type() == bsoncxx::type::k_array)
    {
        auto array = elem.get_array().value;

        for (auto&& val : array)
        {
            if (val.type() == bsoncxx::type::k_int64)
                m_idWorkerList.push_back(static_cast<std::uint64_t>(val.get_int64().value));
        }
    }


    // Strings
    if (auto elem = doc[serial::pszRequestSCHandle]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strSCHandle = std::string{ elem.get_string().value };
    }


    // Enums
    if (auto elem = doc[serial::pszJobPriority]; elem && elem.type() == bsoncxx::type::k_int32)
    {
        m_eJobPriority = static_cast<priority>(elem.get_int32().value);
    }

    if (auto elem = doc[serial::pszJobStatus]; elem && elem.type() == bsoncxx::type::k_int32)
    {
        m_eJobStatus = static_cast<status>(elem.get_int32().value);
    }


    // Bool
    if (auto elem = doc[serial::pszSubscribed]; elem && elem.type() == bsoncxx::type::k_bool)
    {
        m_bNotifyCustomer = elem.get_bool().value;
    }

    m_notes.clear();

    auto notesElem = doc["notes"];
    if (!notesElem || notesElem.type() != bsoncxx::type::k_array)
        return;

    for (auto&& userElem : notesElem.get_array().value)
    {
        auto userDoc = userElem.get_document().view();

        uint64_t userID = 0;
        if (auto uid = userDoc["user_id"])
            userID = static_cast<uint64_t>(uid.get_int64().value);

        auto userNotesElem = userDoc["notes"];
        if (!userNotesElem || userNotesElem.type() != bsoncxx::type::k_array)
            continue;

        for (auto&& noteElem : userNotesElem.get_array().value)
        {
            auto noteDoc = noteElem.get_document().view();

            NoteMetaData note{};

            if (auto g = noteDoc["guild_id"])
                note.guildID = g.get_int64().value;

            if (auto t = noteDoc["timestamp"])
                note.timestamp = t.get_int64().value;

            if (auto n = noteDoc["note"])
                note.note = std::string{ n.get_string().value };

            m_notes[dpp::snowflake{ userID }].push_back(std::move(note));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string JobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bPrintNames) const
{
    fmt::memory_buffer buffer;

    std::vector<std::string> workers = GetWorkerNames(cluster, idGuild);

    fmt::memory_buffer workerbuf;
    for (size_t i = 0; i < workers.size(); ++i)
    {
        if (i != 0)
        {
            fmt::format_to(std::back_inserter(workerbuf), ", ");
        }

        if (bPrintNames)
        {
            fmt::format_to(std::back_inserter(workerbuf), "{}", workers[i]);
        }
        else
        {
            fmt::format_to(std::back_inserter(workerbuf), "<@{}>", m_idWorkerList[i]);
        }
    }

    return fmt::format(
        "ID: **{}**\n"
        "Created: <t:{}:f>\n"
        "Last Edit: <t:{}:f>\n"
        "Priority: {}\n"
        "Status: **{}**\n"
        "Customer: {}\n"
        "Game Handle: {}\n"
        "Assigned: {}\n",
        ToString(m_id),
        m_timeCreated,
        m_timeLastEdit,
        PriorityToString(m_eJobPriority),
        StatusToString(m_eJobStatus),
        (bPrintNames ? GetCustomerName(cluster, idGuild) : "<@" + std::to_string(m_idCustomer) + ">"),
        GetSCHandle(),
        fmt::to_string(workerbuf)
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const std::string JobRequest::PrintJobDetailsCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bPrintNames) const
{
    fmt::memory_buffer buffer;

    std::vector<std::string> workers = GetWorkerNames(cluster, idGuild); 
    
    fmt::memory_buffer workerbuf;
    for (size_t i = 0; i < workers.size(); ++i) 
    {
        if (i != 0) 
        { 
            fmt::format_to(std::back_inserter(workerbuf), ", "); 
        }

        if (bPrintNames)
        {
            fmt::format_to(std::back_inserter(workerbuf), "{}", workers[i]);
        }
        else
        {
            fmt::format_to(std::back_inserter(workerbuf), "<@{}>", m_idWorkerList[i]);
        }
    }

    return fmt::format(
        "ID: **{}**\n"
        "Created: <t:{}:f>\n"
        "Priority: {}\n"
        "Status: **{}**\n"
        "Customer: {}\n"
        "Game Handle: {}\n"
        "Assigned: {}\n",
        ToString(m_id),
        m_timeCreated,
        PriorityToString(m_eJobPriority),
        StatusToString(m_eJobStatus),
        (bPrintNames ? GetCustomerName(cluster, idGuild) : "<@" + std::to_string(m_idCustomer) + ">"),
        GetSCHandle(),
        fmt::to_string(workerbuf)
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const std::string JobRequest::GetCustomerName(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    return utils::FindPreferredNameByID(cluster, m_idCustomer, idGuild);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const JobRequest::WorkerList JobRequest::GetWorkerNames(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    WorkerList vResult;
    for (const auto& worker : m_idWorkerList)
    {
        vResult.emplace_back(utils::FindPreferredNameByID(cluster, worker, idGuild));
    }

    return vResult;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::AddNote(const dpp::snowflake& id, const JobRequest::NoteMetaData& meta)
{
    m_notes[id].emplace_back(std::move(meta));
}

const std::size_t JobRequest::NoteHistorySize() const
{
    std::size_t size = 0;
    for (const auto& note : m_notes)
    {
        size += note.second.size();
    }

    return size;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const std::string JobRequest::PrintNoteHistory(dpp::cluster& cluster, const std::size_t page) const
{
    // Flatten all notes into a single vector
    std::vector<std::pair<dpp::snowflake, NoteMetaData>> allNotes;
    for (const auto& [userID, notes] : m_notes)
    {
        for (const auto& note : notes)
        {
            allNotes.push_back(std::pair{ userID, note });
        }
    }

    // Sort by timestamp ascending
    std::sort(allNotes.begin(), allNotes.end(),
        [](const std::pair<dpp::snowflake, NoteMetaData> a, const std::pair<dpp::snowflake, NoteMetaData> b)
        {
            // oldest timestamps first
            return a.second.timestamp > b.second.timestamp;
        });

    const std::size_t start_index = page * NOTES_PER_PAGE;
    const std::size_t end_index = start_index + NOTES_PER_PAGE;

    std::size_t filtered_index = 0;

    fmt::memory_buffer buffer;

    // Format each note in discord format
    for (const auto& [userID, meta] : allNotes)
    {
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            // Append to fmt buffer
            fmt::format_to(std::back_inserter(buffer), "### <t:{}:f> | <@{}>:\n{}\n", meta.timestamp, userID, meta.note);
        }

        ++filtered_index;
    }

    return fmt::to_string(buffer);
}

const std::string JobRequest::PrintLastTwoNotes(dpp::cluster& cluster) const
{
    fmt::memory_buffer buffer;

    // Flatten all notes into a single vector
    std::vector<std::pair<dpp::snowflake, NoteMetaData>> allNotes;
    for (const auto& [userID, notes] : m_notes)
    {
        for (const auto& note : notes)
        {
            allNotes.emplace_back(userID, note);
        }
    }

    if (allNotes.empty())
        return ""; // nothing to print

    // Sort by timestamp ascending
    std::sort(allNotes.begin(), allNotes.end(),
        [](const auto& a, const auto& b)
        {
            return a.second.timestamp < b.second.timestamp;
        });

    // Only take the last 2 notes
    const std::size_t startIndex = allNotes.size() > 2 ? allNotes.size() - 2 : 0;

    for (std::size_t i = startIndex; i < allNotes.size(); ++i)
    {
        const auto& [userID, meta] = allNotes[i];

        // I hate that this is UTC, this should be relative to the client's system clock timezone to match discord's timestamp API
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::time_point{ std::chrono::seconds{meta.timestamp} };
        const std::string strTimeStamp = fmt::format("{:%Y-%m-%d %H:%M UTC}", std::chrono::floor<std::chrono::seconds>(tp));

        // Lookup username
        const std::string username = utils::FindPreferredNameByID(cluster, userID, meta.guildID);

        // Append to fmt buffer
        fmt::format_to(std::back_inserter(buffer), "{} | {}:\n{}\n", strTimeStamp, username, meta.note);
    }

    return fmt::to_string(buffer);
}

bool JobRequest::IsWorker(const dpp::snowflake& user) const
{
    return m_idWorkerList.cend() != std::find_if(m_idWorkerList.cbegin(), m_idWorkerList.cend(), [&user](const dpp::snowflake& worker) -> bool { return user == worker; });
}