#include "RequestDlg.h"

#include "BotUtility.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"

#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
#include "HazardousRequest.h"
// d++
#include <dpp/unicode_emoji.h>

const std::string CraftRequestDlg::modalID = "CreateRequestModal";
const std::string CraftRequestDlg::modalDesc = "Submit Item Crafting Request";

const std::string BuildRequestDlg::modalID = "BuildRequestModal";
const std::string BuildRequestDlg::modalDesc = "Submit Base Building Request";

const std::string ComponentRequestDlg::modalID = "ComponentRequestModal";
const std::string ComponentRequestDlg::modalDesc = "Submit Ship Component Request";

const std::string ResourceRequestDlg::modalID = "ResourceRequestModal";
const std::string ResourceRequestDlg::modalDesc = "Submit Resource Collection Request";

const std::string RefineryRequestDlg::modalID = "RefineryRequestModal";
const std::string RefineryRequestDlg::modalDesc = "Submit Refinery Job";

const std::string HazardousRequestDlg::modalID = "HazardousRequestModal";
const std::string HazardousRequestDlg::modalDesc = "Submit Hazardous Item Retrieval Request";

const std::string AssignRequestDlg::modalID = "AssignRequestModal";
const std::string AssignRequestDlg::modalDesc = "Assign Job Request To Worker";

const std::string StatusChangeRequestDlg::modalID = "StatusChangeModal";
const std::string StatusChangeRequestDlg::modalDesc = "Update Status of Job Request";
const std::vector<StatusChangeRequestDlg::StatusOption> StatusChangeRequestDlg::StatusList{
    { "Open", JobRequest::StatusToString(JobRequest::status::open), JobRequest::StatusToEmoji(JobRequest::status::open) },
    { "Stalled", JobRequest::StatusToString(JobRequest::status::stalled), JobRequest::StatusToEmoji(JobRequest::status::stalled) },
    { "Assigned", JobRequest::StatusToString(JobRequest::status::assigned), JobRequest::StatusToEmoji(JobRequest::status::assigned) },
    { "Active", JobRequest::StatusToString(JobRequest::status::active), JobRequest::StatusToEmoji(JobRequest::status::active) },
    { "Hold", JobRequest::StatusToString(JobRequest::status::hold), JobRequest::StatusToEmoji(JobRequest::status::hold) },
    { "Complete", JobRequest::StatusToString(JobRequest::status::complete), JobRequest::StatusToEmoji(JobRequest::status::complete) }
};

const std::string PriorityChangeRequestDlg::modalID = "PriorityChangeModal";
const std::string PriorityChangeRequestDlg::modalDesc = "Update Priority of Job Request";
const std::vector<PriorityChangeRequestDlg::PriorityOption> PriorityChangeRequestDlg::PriorityList{
    { "Low", JobRequest::PriorityToString(JobRequest::priority::low), JobRequest::PriorityToEmoji(JobRequest::priority::low), "When convenient." },
    { "Medium", JobRequest::PriorityToString(JobRequest::priority::medium), JobRequest::PriorityToEmoji(JobRequest::priority::medium), "Need the item soon." },
    { "High", JobRequest::PriorityToString(JobRequest::priority::high), JobRequest::PriorityToEmoji(JobRequest::priority::high), "Need the item today." },
    { "Critical", JobRequest::PriorityToString(JobRequest::priority::critical), JobRequest::PriorityToEmoji(JobRequest::priority::critical), "Need the item as soon as possible." }
};

const std::string EditRequestDlg::modalID = "EditRequestModal";
const std::string EditRequestDlg::modalDesc = "Edit Active Job Request";

const std::string DeleteRequestDlg::modalID = "DeleteRequestModal";
const std::string DeleteRequestDlg::modalDesc = "Delete Job Request";

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
CraftRequestDlg::CraftRequestDlg()
    : dpp::interaction_modal_response()
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void CraftRequestDlg::InitializeControls()
{
    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    // Create a text box component
    ItemDescEdit.set_label("Item Description or Name")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_ItemDesc);

    // Create a text box component
    ItemQuantityEdit.set_label("Item Quantity")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(4)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_ItemQuantity);

    // Create a text box component
    ItemQualityEdit.set_label("Expected Item Quality Range")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Quality")
        .add_select_option(dpp::select_option("Any", "any", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("500-600", "500-600", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("600-700", "600-700", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("700-800", "700-800", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("800-900", "800-900", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("900+", "900+", "Qaulity Rating of the Item."))
        .set_id(Component_ItemQuality);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void CraftRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ItemDescEdit);
    add_component(ItemQuantityEdit);
    add_component(ItemQualityEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
BuildRequestDlg::BuildRequestDlg()
    : dpp::interaction_modal_response()
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildRequestDlg::InitializeControls()
{
    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    // Create a text box component
    BuildDesignEdit.set_label("Type of Building(s)")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_BuildDesign);

    // Create a text box component
    BuildRequiresEdit.set_label("Build Requirements")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_BuildRequires);

    // Create a text box component
    BuildZoneEdit.set_label("Building Location or Zone")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_BuildZone);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(BuildDesignEdit);
    add_component(BuildRequiresEdit);
    add_component(BuildZoneEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
ComponentRequestDlg::ComponentRequestDlg()
    : dpp::interaction_modal_response()
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentRequestDlg::InitializeControls()
{
    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    // Create a text box component
    ComponentListEdit.set_label("Component List")
        .set_type(dpp::cot_text)
        .set_placeholder("Class / Size / System / Grade or Name / Qty")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_CompList);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ComponentListEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
ResourceRequestDlg::ResourceRequestDlg()
    : dpp::interaction_modal_response()
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ResourceRequestDlg::InitializeControls()
{
    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    // Create a combo box component
    ResourceTypeSelect.set_label("Select Requested State of the Resources")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select State")
        .add_select_option(dpp::select_option("Mineable", ResourceJobRequest::StateToString(ResourceJobRequest::state::UnrefinedMineable), ""))
        .add_select_option(dpp::select_option("Refined Mineable", ResourceJobRequest::StateToString(ResourceJobRequest::state::RefinedMineable), ""))
        .add_select_option(dpp::select_option("Salvage", ResourceJobRequest::StateToString(ResourceJobRequest::state::UnrefinedSalvage), ""))
        .add_select_option(dpp::select_option("Refined Salvage", ResourceJobRequest::StateToString(ResourceJobRequest::state::RefinedSalvage), ""))
        .add_select_option(dpp::select_option("Harvestable", ResourceJobRequest::StateToString(ResourceJobRequest::state::UnrefinedHarvest), ""))
        .add_select_option(dpp::select_option("Refined Harvestable", ResourceJobRequest::StateToString(ResourceJobRequest::state::RefinedHarvest), ""))
        .set_id(Component_ResourceType);

    // Create a text box component
    ResourceListEdit.set_label("Resource List")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_ResourceList);

    // Create a text box component
    ResourceQualityEdit.set_label("Expected Resource Quality Range")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Quality")
        .add_select_option(dpp::select_option("Any", "any", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("500-600", "500-600", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("600-700", "600-700", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("700-800", "700-800", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("800-900", "800-900", "Qaulity Rating of the Item."))
        .add_select_option(dpp::select_option("900+", "900+", "Qaulity Rating of the Item."))
        .set_id(Component_ResourceQuality);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ResourceRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ResourceTypeSelect);
    add_component(ResourceListEdit);
    add_component(ResourceQualityEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
RefineryRequestDlg::RefineryRequestDlg()
    : dpp::interaction_modal_response()
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void RefineryRequestDlg::InitializeControls()
{
    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    // Create a combo box component
    RefineryTypeSelect.set_label("Select The Resource Category")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select State")
        .add_select_option(dpp::select_option("Refined Mineable", RefineryJobRequest::StateToString(RefineryJobRequest::state::RefinedMineable), ""))
        .add_select_option(dpp::select_option("Refined Salvage", RefineryJobRequest::StateToString(RefineryJobRequest::state::RefinedSalvage), ""))
        .add_select_option(dpp::select_option("Refined Harvestable", RefineryJobRequest::StateToString(RefineryJobRequest::state::RefinedHarvest), ""))
        .set_id(Component_ResourceType);

    // Create a text box component
    ResourceListEdit.set_label("Resource For Refining")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_ResourceList);
    
    // Create a text box component
    RefinerySiteEdit.set_label("Refinery Site or Location")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_RefinerySite);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void RefineryRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(RefineryTypeSelect);
    add_component(ResourceListEdit);
    add_component(RefinerySiteEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
HazardousRequestDlg::HazardousRequestDlg()
    : dpp::interaction_modal_response()
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void HazardousRequestDlg::InitializeControls()
{
    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    // Create a combo box component
    ThreatLevelSelect.set_label("Expected Threat Level")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Threat Level")
        .add_select_option(dpp::select_option("Permissive", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Permissive), "No threats expected.").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Minimal", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Minimal), "Minimal to low threat expected.").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Uncertain", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Uncertain), "Unknown hostility level.").set_emoji(dpp::unicode_emoji::black_circle))
        .add_select_option(dpp::select_option("Hostile", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Hostile), "High likelihood of combat").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_ThreatLevel);

    // Create a text box component
    HazResourceZoneEdit.set_label("Hazardous Item Location")
        .set_type(dpp::cot_text)
        .set_placeholder("Zone or Region")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_hazItemZone);

    // Create a text box component
    HazResourceListEdit.set_label("Hazardous Item List")
        .set_type(dpp::cot_text)
        .set_placeholder("Item descriptions, quantities, and expected quality levels.")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_HazItemList);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void HazardousRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ThreatLevelSelect);
    add_component(HazResourceZoneEdit);
    add_component(HazResourceListEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
AssignRequestDlg::AssignRequestDlg(const std::shared_ptr<const JobRequest>& job)
    : dpp::interaction_modal_response(), m_spJob(job)
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void AssignRequestDlg::InitializeControls()
{
    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(ToString(m_spJob->GetID()))
        .set_min_length(0)
        .set_max_length(16)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create a combo box component
    WorkerAssignSelect.set_label("Assign Task To")
        .set_type(dpp::cot_text)
        .set_placeholder("Enter Worker ID")
        .set_min_length(0)
        .set_max_length(40)
        .set_text_style(dpp::text_short)
        .set_id(Component_Assignment);

    WorkerAssignSelect2.set_label("Assign Additional")
        .set_type(dpp::cot_text)
        .set_placeholder("Enter Worker ID 2")
        .set_min_length(0)
        .set_max_length(40)
        .set_text_style(dpp::text_short)
        .set_id(fmt::format("{}:{}", Component_Assignment, 2));

    WorkerAssignSelect3.set_label("Assign Additional")
        .set_type(dpp::cot_text)
        .set_placeholder("Enter Worker ID 3")
        .set_min_length(0)
        .set_max_length(40)
        .set_text_style(dpp::text_short)
        .set_id(fmt::format("{}:{}", Component_Assignment, 3));

    StatusUpdateSelect.set_label("Update Status")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Status")
        .set_id(Component_Status);

    // Add all status options dynamically
    for (const auto& s : StatusChangeRequestDlg::StatusList) {
        StatusUpdateSelect.add_select_option(
            dpp::select_option(s.label, s.value, "").set_emoji(s.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void AssignRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(WorkerAssignSelect);
    add_component(WorkerAssignSelect2);
    add_component(WorkerAssignSelect3);
    add_component(StatusUpdateSelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------

StatusChangeRequestDlg::StatusChangeRequestDlg(const std::shared_ptr<const JobRequest>& job)
    : dpp::interaction_modal_response(), m_spJob(job)
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void StatusChangeRequestDlg::InitializeControls()
{

    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(ToString(m_spJob->GetID()))
        .set_min_length(0)
        .set_max_length(16)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    StatusUpdateSelect.set_label("Update Status")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Status")
        .set_id(Component_Status);

    // Add all status options dynamically
    for (const auto& s : StatusChangeRequestDlg::StatusList) {
        StatusUpdateSelect.add_select_option(
            dpp::select_option(s.label, s.value, "").set_emoji(s.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void StatusChangeRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(StatusUpdateSelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
PriorityChangeRequestDlg::PriorityChangeRequestDlg(const std::shared_ptr<const JobRequest>& job)
    : dpp::interaction_modal_response(), m_spJob(job)
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void PriorityChangeRequestDlg::InitializeControls()
{
    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(ToString(m_spJob->GetID()))
        .set_min_length(0)
        .set_max_length(16)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create the combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .set_id(Component_Priority);

    // Add all priority options dynamically
    for (const auto& p : PriorityChangeRequestDlg::PriorityList) {
        PrioritySelect.add_select_option(
            dpp::select_option(p.label, p.value, p.description).set_emoji(p.emoji)
        );
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void PriorityChangeRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(PrioritySelect);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
EditRequestDlg::EditRequestDlg(const std::shared_ptr<const JobRequest>& job)
    : dpp::interaction_modal_response(), m_spJob(job)
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void EditRequestDlg::InitializeControls()
{
    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(ToString(m_spJob->GetID()))
        .set_min_length(0)
        .set_max_length(16)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create a text box component
    CitizenHandleEdit.set_label("Star Citizen Handle")
        .set_type(dpp::cot_text)
        .set_default_value(m_spJob->GetSCHandle())
        .set_min_length(1)
        .set_max_length(128)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    if (m_spJob->SupportsType(JOB_TYPE_CRAFTING))
    {
        const std::shared_ptr<const CraftingJobRequest> craft = std::dynamic_pointer_cast<const CraftingJobRequest>(m_spJob);
        // Create a text box component
        ItemDescEdit.set_label("Item Description or Name")
            .set_type(dpp::cot_text)
            .set_default_value(craft->GetItemDesc())
            .set_min_length(1)
            .set_max_length(128)
            .set_text_style(dpp::text_short)
            .set_id(Component_ItemDesc);

        // Create a text box component
        ItemQuantityEdit.set_label("Item Quantity")
            .set_type(dpp::cot_text)
            .set_default_value(craft->GetQuantity())
            .set_min_length(1)
            .set_max_length(4)
            .set_text_style(dpp::text_short)
            .set_id(Component_ItemQuantity);
    }
    else if (m_spJob->SupportsType(JOB_TYPE_BUILDING))
    {
        const std::shared_ptr<const BuildingJobRequest> bldg = std::dynamic_pointer_cast<const BuildingJobRequest>(m_spJob);
        // Create a text box component
        BuildDesignEdit.set_label("Type of Building(s)")
            .set_type(dpp::cot_text)
            .set_default_value(bldg->GetBuildDesign())
            .set_min_length(1)
            .set_max_length(128)
            .set_required(true)
            .set_text_style(dpp::text_short)
            .set_id(Component_BuildDesign);

        // Create a text box component
        BuildRequiresEdit.set_label("Build Requirements")
            .set_type(dpp::cot_text)
            .set_default_value(bldg->GetBuildRequirments())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_BuildRequires);

        // Create a text box component
        BuildZoneEdit.set_label("Building Location or Zone")
            .set_type(dpp::cot_text)
            .set_default_value(bldg->GetBuildZone())
            .set_min_length(1)
            .set_max_length(128)
            .set_required(true)
            .set_text_style(dpp::text_short)
            .set_id(Component_BuildZone);
    }
    else if (m_spJob->SupportsType(JOB_TYPE_COMPONENT))
    {
        const std::shared_ptr<const ComponentJobRequest> comp = std::dynamic_pointer_cast<const ComponentJobRequest>(m_spJob);
        // Create a text box component
        ComponentListEdit.set_label("Component List")
            .set_type(dpp::cot_text)
            .set_default_value(comp->GetComponentList())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_CompList);
    }
    else if (m_spJob->SupportsType(JOB_TYPE_RESOURCE))
    {
        const std::shared_ptr<const ResourceJobRequest> resource = std::dynamic_pointer_cast<const ResourceJobRequest>(m_spJob);
        // Create a text box component
        ResourceListEdit.set_label("Resource List")
            .set_type(dpp::cot_text)
            .set_default_value(resource->GetResourcelist())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_ResourceList);
    }
    else if (m_spJob->SupportsType(JOB_TYPE_REFINERY))
    {
        const std::shared_ptr<const RefineryJobRequest> refine = std::dynamic_pointer_cast<const RefineryJobRequest>(m_spJob);
        // Create a text box component
        ResourceListEdit.set_label("Resource List")
            .set_type(dpp::cot_text)
            .set_default_value(refine->GetResourcelist())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_ResourceList);

        // Create a text box component
        RefinerySiteEdit.set_label("Refinery Site or Location")
            .set_type(dpp::cot_text)
            .set_default_value(refine->GetRefinery())
            .set_min_length(1)
            .set_max_length(128)
            .set_required(true)
            .set_text_style(dpp::text_short)
            .set_id(Component_RefinerySite);
    }
    else if (m_spJob->SupportsType(JOB_TYPE_HAZARD))
    {
        const std::shared_ptr<const HazardousRequest> hazmat = std::dynamic_pointer_cast<const HazardousRequest>(m_spJob);
        // Create a combo box component
        ThreatLevelSelect.set_label("Expected Threat Level")
            .set_type(dpp::cot_selectmenu)
            .add_select_option(dpp::select_option("Permissive", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Permissive), "No threats expected.")
                .set_emoji(dpp::unicode_emoji::green_circle)
                .set_default(hazmat->GetThreatLevel() == HazardousRequest::ThreatLevel::Permissive))
            .add_select_option(dpp::select_option("Minimal", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Minimal), "Minimal to low threat expected.")
                .set_emoji(dpp::unicode_emoji::orange_circle)
                .set_default(hazmat->GetThreatLevel() == HazardousRequest::ThreatLevel::Minimal))
            .add_select_option(dpp::select_option("Uncertain", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Uncertain), "Unknown hostility level.")
                .set_emoji(dpp::unicode_emoji::black_circle)
                .set_default(hazmat->GetThreatLevel() == HazardousRequest::ThreatLevel::Uncertain))
            .add_select_option(dpp::select_option("Hostile", HazardousRequest::ThreatToString(HazardousRequest::ThreatLevel::Hostile), "High likelihood of combat")
                .set_emoji(dpp::unicode_emoji::red_circle)
                .set_default(hazmat->GetThreatLevel() == HazardousRequest::ThreatLevel::Hostile))
            .set_id(Component_ThreatLevel);

        // Create a text box component
        HazResourceZoneEdit.set_label("Hazardous Item Location")
            .set_type(dpp::cot_text)
            .set_default_value(hazmat->GetItemLocation())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_short)
            .set_id(Component_hazItemZone);

        // Create a text box component
        HazResourceListEdit.set_label("Hazardous Item List")
            .set_type(dpp::cot_text)
            .set_default_value(hazmat->GetItemList())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_HazItemList);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void EditRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(CitizenHandleEdit);
    if (m_spJob->SupportsType(JOB_TYPE_CRAFTING))
    {
        add_component(ItemDescEdit);
        add_component(ItemQuantityEdit);
    }
    if (m_spJob->SupportsType(JOB_TYPE_BUILDING))
    {
        add_component(BuildDesignEdit);
        add_component(BuildRequiresEdit);
        add_component(BuildZoneEdit);
    }
    if (m_spJob->SupportsType(JOB_TYPE_COMPONENT))
    {
        add_component(ComponentListEdit);
    }
    if (m_spJob->SupportsType(JOB_TYPE_RESOURCE))
    {
        add_component(ResourceListEdit);
    }
    if (m_spJob->SupportsType(JOB_TYPE_REFINERY))
    {
        add_component(ResourceListEdit);
        add_component(RefinerySiteEdit);
    }
    if (m_spJob->SupportsType(JOB_TYPE_HAZARD))
    {
        add_component(ThreatLevelSelect);
        add_component(HazResourceZoneEdit);
        add_component(HazResourceListEdit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
DeleteRequestDlg::DeleteRequestDlg(const std::shared_ptr<const JobRequest>& job, const std::string details)
    : dpp::interaction_modal_response(), m_spJob(job), m_strDetails(details)
{
    set_custom_id(fmt::format("{}:{}", modalID, utils::GetEpochTimestamp()));
    set_title(modalDesc);
    InitializeControls();
    AddChildrenComponents();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void DeleteRequestDlg::InitializeControls()
{
    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(ToString(m_spJob->GetID()))
        .set_min_length(0)
        .set_max_length(16)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create the combo box component
    JobDescriptionEdit.set_label("Details")
        .set_type(dpp::cot_text)
        .set_default_value(m_strDetails)
        .set_min_length(0)
        .set_max_length(500)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_JobDescription);

    // Create the combo box component
    DeleteJustificationEdit.set_label("Reason")
        .set_type(dpp::cot_text)
        .set_placeholder("Enter reason here...")
        .set_min_length(2)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_DeleteJustification);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void DeleteRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(JobDescriptionEdit);
    add_component(DeleteJustificationEdit);
}