#include "JobRequest.h"

#include "BotUtility.h"
// d++
#include <dpp/cache.h>
#include <dpp/user.h>
#include <dpp/unicode_emoji.h>
// fmt
#include <fmt/format.h>
// std library
#include <chrono>
#include <stdexcept>
#include <string>

namespace xmlRequest
{
    constexpr const char* pszXMLCreation{ "Created" };
    constexpr const char* pszXMLLastEdit{ "LastEdit" };
    constexpr const char* pszXMLRequestUser{ "Requester" };
    constexpr const char* pszXMLJobWorker{ "Worker" };
    constexpr const char* pszXMLRequestSCHandle{ "GameHandle" };
    constexpr const char* pszXMLJobPriority{ "Priority" };
    constexpr const char* pszXMLJobStatus{ "Status" };
    constexpr const char* pszXMLJobGUID{ "GUID" };
    constexpr const char* pszXMLJobType{ "Type" };
    constexpr const char* pszXMLSubscribed{ "Subscribed" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string JobRequest::PriorityToString(const JobRequest::priority p)
{
    switch (p) {
    case JobRequest::priority::low:         return "low";
    case JobRequest::priority::medium:      return "medium";
    case JobRequest::priority::high:        return "high";
    case JobRequest::priority::critical:    return "critical";
    default: throw std::invalid_argument("Unexpected priority enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
JobRequest::priority JobRequest::StringToPriority(const std::string& str)
{
    if (str == "low")       return JobRequest::priority::low;
    if (str == "medium")    return JobRequest::priority::medium;
    if (str == "high")      return JobRequest::priority::high;
    if (str == "critical")  return JobRequest::priority::critical;
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
    case JobRequest::status::open:      return "open";
    case JobRequest::status::stalled:   return "stalled";
    case JobRequest::status::assigned:  return "assigned";
    case JobRequest::status::active:    return "active";
    case JobRequest::status::hold:      return "on hold";
    case JobRequest::status::complete:  return "complete";
    default: throw std::invalid_argument("Unexpected status enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
JobRequest::status JobRequest::StringToStatus(const std::string& str)
{
    if (str == "open")      return JobRequest::status::open;
    if (str == "stalled")   return JobRequest::status::stalled;
    if (str == "assigned")  return JobRequest::status::assigned;
    if (str == "active")    return JobRequest::status::active;
    if (str == "on hold")   return JobRequest::status::hold;
    if (str == "complete")  return JobRequest::status::complete;
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
JobRequest::JobRequest() 
    : m_id(utils::CreateGUID()) 
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
    xmlNode->SetAttribute(xmlRequest::pszXMLCreation, m_timeCreated);
    xmlNode->SetAttribute(xmlRequest::pszXMLLastEdit, m_timeLastEdit);
    xmlNode->SetAttribute(xmlRequest::pszXMLJobGUID, utils::GuidToString(m_id).c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLJobPriority, static_cast<int>(m_eJobPriority));
    xmlNode->SetAttribute(xmlRequest::pszXMLJobStatus, static_cast<int>(m_eJobStatus));
    xmlNode->SetAttribute(xmlRequest::pszXMLJobType, JobType());
    xmlNode->SetAttribute(xmlRequest::pszXMLJobWorker, m_idWorker);
    xmlNode->SetAttribute(xmlRequest::pszXMLRequestUser, m_idCustomer);
    xmlNode->SetAttribute(xmlRequest::pszXMLRequestSCHandle, m_strSCHandle.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLSubscribed, m_bNotifyCustomer);

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

    m_timeCreated = xmlNode->Unsigned64Attribute(xmlRequest::pszXMLCreation, 0);
    m_timeLastEdit = xmlNode->Unsigned64Attribute(xmlRequest::pszXMLLastEdit, 0);
    const char* pszJobGUID = xmlNode->Attribute(xmlRequest::pszXMLJobGUID);
    if (pszJobGUID)
    {
        try
        {
            m_id = utils::StringToGuid(pszJobGUID);
        }
        catch (std::exception e)
        {
            m_id = GUID_NULL;
        }
    }

    m_eJobPriority = static_cast<priority>(xmlNode->IntAttribute(xmlRequest::pszXMLJobPriority, priority::low));
    m_eJobStatus = static_cast<status>(xmlNode->IntAttribute(xmlRequest::pszXMLJobStatus, status::open));
    m_idWorker = xmlNode->Unsigned64Attribute(xmlRequest::pszXMLJobWorker, USERID_NULL);
    m_idCustomer = xmlNode->Unsigned64Attribute(xmlRequest::pszXMLRequestUser, USERID_NULL);
    const char* pszHandle = xmlNode->Attribute(xmlRequest::pszXMLRequestSCHandle);
    if (pszHandle)
    {
        m_strSCHandle = pszHandle;
    }

    m_bNotifyCustomer = xmlNode->BoolAttribute(xmlRequest::pszXMLSubscribed, false);

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
std::string JobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    fmt::memory_buffer buffer;

    return fmt::format(
        "**ID**: {}\n"
        "**Customer**: {}\n"
        "**Assigned**: {}\n"
        "**Type**: {}\n"
        "**Status**: {}\n"
        "**Priority**: {}\n"
        "**Created**: <t:{}:F>\n"
        "**Last Edit**: <t:{}:F>\n",
        utils::GuidToStringNoBrackets(m_id),
        GetCustomerName(cluster, idGuild),
        GetWorkerName(cluster, idGuild),
        JobTypeToString(),
        StatusToString(m_eJobStatus),
        PriorityToString(m_eJobPriority),
        m_timeCreated,
        m_timeLastEdit);
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
const std::string JobRequest::GetWorkerName(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    return utils::FindPreferredNameByID(cluster, m_idWorker, idGuild);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobRequest::AddNote(const dpp::snowflake& id, const JobRequest::NoteMetaData& meta)
{
    const std::size_t timestamp = utils::GetEpochTimestamp();
    m_notes[id].emplace_back(std::move(meta));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
const std::string JobRequest::PrintNoteHistory(dpp::cluster& cluster) const
{
    fmt::memory_buffer buffer;

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
            return a.second.timestamp < b.second.timestamp;
        });

    // Format each note in UTC
    for (const auto& [userID, meta] : allNotes)
    {
        std::time_t tt = static_cast<std::time_t>(meta.timestamp);
        std::tm tm{};

        // Thread-safe UTC conversion
#if defined(_MSC_VER)
        gmtime_s(&tm, &tt);   // MSVC-safe
#else
        tm = *std::gmtime(&tt); // POSIX
#endif

        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M UTC", &tm);

        // Lookup username
        std::string username = utils::FindPreferredNameByID(cluster, userID, meta.guildID);

        // Append to fmt buffer
        fmt::format_to(std::back_inserter(buffer), "{} | {}:\n{}\n", timeBuf, username, meta.note);
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

        std::time_t tt = static_cast<std::time_t>(meta.timestamp);
        std::tm tm{};

        // Thread-safe UTC conversion
#if defined(_MSC_VER)
        gmtime_s(&tm, &tt);   // MSVC-safe
#else
        tm = *std::gmtime(&tt); // POSIX
#endif

        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M UTC", &tm);

        // Lookup username
        std::string username = utils::FindPreferredNameByID(cluster, userID, meta.guildID);

        // Append to fmt buffer
        fmt::format_to(std::back_inserter(buffer), "{} | {}:\n{}\n", timeBuf, username, meta.note);
    }

    return fmt::to_string(buffer);
}