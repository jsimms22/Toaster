#include "RequestDlg.h"
#include "Resource.h"
#include "BotUtility.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"

const std::string CraftRequestDlg::modalID = "CreateRequestModal";
const std::string CraftRequestDlg::modalDesc = "Submit Item Crafting Request";

const std::string BuildRequestDlg::modalID = "BuildRequestModal";
const std::string BuildRequestDlg::modalDesc = "Submit Base Building Request";

const std::string ComponentRequestDlg::modalID = "ComponentRequestModal";
const std::string ComponentRequestDlg::modalDesc = "Submit Ship Component Request";

const std::string ResourceRequestDlg::modalID = "ResourceRequestModal";
const std::string ResourceRequestDlg::modalDesc = "Submit Resource Collection Request";

const std::string AssignRequestDlg::modalID = "AssignRequestModal";
const std::string AssignRequestDlg::modalDesc = "Assign Job Request To Worker";

const std::string StatusChangeRequestDlg::modalID = "StatusChangeModal";
const std::string StatusChangeRequestDlg::modalDesc = "Update Status of Job Request";

const std::string PriorityChangeRequestDlg::modalID = "PriorityChangeModal";
const std::string PriorityChangeRequestDlg::modalDesc = "Update Priority of Job Request";

const std::string EditRequestDlg::modalID = "EditRequestModal";
const std::string EditRequestDlg::modalDesc = "Edit Active Job Request";

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
        .set_max_length(128)
        .set_required(true)
        .set_text_style(dpp::text_short)
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

    // Create a combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .add_select_option(dpp::select_option("Low", LOW_PRIORITY_ID, "When convienent for the fulfiller.").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Medium", MED_PRIORITY_ID, "Need the item soon.").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("High", HIGH_PRIORITY_ID, "Need the item today.").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Critical", CRITICAL_PRIORITY_ID, "Need the item as soon as possible.").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Priority);
}

void CraftRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ItemDescEdit);
    add_component(ItemQuantityEdit);
    add_component(ItemQualityEdit);
    add_component(PrioritySelect);
}

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

    // Create a combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .add_select_option(dpp::select_option("Low", LOW_PRIORITY_ID, "When convienent for the fulfiller.").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Medium", MED_PRIORITY_ID, "Need the item soon.").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("High", HIGH_PRIORITY_ID, "Need the item today.").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Critical", CRITICAL_PRIORITY_ID, "Need the item as soon as possible.").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Priority);
}

void BuildRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(BuildDesignEdit);
    add_component(BuildRequiresEdit);
    add_component(BuildZoneEdit);
    add_component(PrioritySelect);
}

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
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(256)
        .set_required(true)
        .set_text_style(dpp::text_paragraph)
        .set_id(Component_CompList);

    // Create a combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .add_select_option(dpp::select_option("Low", LOW_PRIORITY_ID, "When convienent for the fulfiller.").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Medium", MED_PRIORITY_ID, "Need the item soon.").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("High", HIGH_PRIORITY_ID, "Need the item today.").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Critical", CRITICAL_PRIORITY_ID, "Need the item as soon as possible.").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Priority);
}

void ComponentRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ComponentListEdit);
    add_component(PrioritySelect);
}

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

    // Create a combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .add_select_option(dpp::select_option("Low", LOW_PRIORITY_ID, "When convienent for the fulfiller.").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Medium", MED_PRIORITY_ID, "Need the item soon.").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("High", HIGH_PRIORITY_ID, "Need the item today.").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Critical", CRITICAL_PRIORITY_ID, "Need the item as soon as possible.").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Priority);
}

void ResourceRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ResourceTypeSelect);
    add_component(ResourceListEdit);
    add_component(ResourceQualityEdit);
    add_component(PrioritySelect);
}

void AssignRequestDlg::InitializeControls()
{

    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(utils::GuidToStringNoBrackets(m_job->GetID()))
        .set_min_length(1)
        .set_max_length(128)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create a combo box component
    WorkerAssignSelect.set_label("Assign Task")
        .set_type(dpp::cot_selectmenu)
        .set_default_value(m_job->GetWorker())
        .set_id(Component_Assignment);

    for (const auto& worker : m_vWorkers)
    {
        WorkerAssignSelect.add_select_option(dpp::select_option(worker, worker, ""));
    }

    // Create a combo box component
    StatusUpdateSelect.set_label("Update Status")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Status")
        .add_select_option(dpp::select_option("Open", "open", "").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Active", "active", "").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("Hold", "hold", "").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Complete", "complete", "").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Status);

}

void AssignRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(WorkerAssignSelect);
    add_component(StatusUpdateSelect);
}

void StatusChangeRequestDlg::InitializeControls()
{

    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(utils::GuidToStringNoBrackets(m_job->GetID()))
        .set_min_length(1)
        .set_max_length(128)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create a combo box component
    StatusUpdateSelect.set_label("Update Status")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Status")
        .add_select_option(dpp::select_option("Open", "open", "").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Active", "active", "").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("Hold", "hold", "").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Complete", "complete", "").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Status);
}

void StatusChangeRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(StatusUpdateSelect);
}

void PriorityChangeRequestDlg::InitializeControls()
{

    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(utils::GuidToStringNoBrackets(m_job->GetID()))
        .set_min_length(1)
        .set_max_length(128)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create a combo box component
    PrioritySelect.set_label("Select Priority")
        .set_type(dpp::cot_selectmenu)
        .set_placeholder("Select Priority")
        .add_select_option(dpp::select_option("Low", LOW_PRIORITY_ID, "When convienent for the fulfiller.").set_emoji(dpp::unicode_emoji::green_circle))
        .add_select_option(dpp::select_option("Medium", MED_PRIORITY_ID, "Need the item soon.").set_emoji(dpp::unicode_emoji::yellow_circle))
        .add_select_option(dpp::select_option("High", HIGH_PRIORITY_ID, "Need the item today.").set_emoji(dpp::unicode_emoji::orange_circle))
        .add_select_option(dpp::select_option("Critical", CRITICAL_PRIORITY_ID, "Need the item as soon as possible.").set_emoji(dpp::unicode_emoji::red_circle))
        .set_id(Component_Priority);
}

void PriorityChangeRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(PrioritySelect);
}

void EditRequestDlg::InitializeControls()
{
    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(utils::GuidToStringNoBrackets(m_job->GetID()))
        .set_min_length(1)
        .set_max_length(128)
        .set_text_style(dpp::text_short)
        .set_id(Component_RequestID);

    // Create a text box component
    CitizenHandleEdit.set_label("Requestee SC Handle (In Game)")
        .set_type(dpp::cot_text)
        .set_default_value(m_job->GetSCHandle())
        .set_min_length(1)
        .set_max_length(128)
        .set_text_style(dpp::text_short)
        .set_id(Component_CitizenID);

    if (m_job->SupportsType(JOB_TYPE_CRAFTING))
    {
        std::shared_ptr<CraftingJobRequest> craft = std::dynamic_pointer_cast<CraftingJobRequest>(m_job);
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
            .set_default_value(std::to_string(craft->GetQuantity()))
            .set_min_length(1)
            .set_max_length(4)
            .set_text_style(dpp::text_short)
            .set_id(Component_ItemQuantity);
    }
    else if (m_job->SupportsType(JOB_TYPE_BUILDING))
    {
        std::shared_ptr<BuildingJobRequest> bldg = std::dynamic_pointer_cast<BuildingJobRequest>(m_job);
        // Create a text box component
        BuildDesignEdit.set_label("Type of Building(s)")
            .set_type(dpp::cot_text)
            .set_placeholder(bldg->GetBuildDesign())
            .set_min_length(1)
            .set_max_length(128)
            .set_required(true)
            .set_text_style(dpp::text_short)
            .set_id(Component_BuildDesign);

        // Create a text box component
        BuildRequiresEdit.set_label("Build Requirements")
            .set_type(dpp::cot_text)
            .set_placeholder(bldg->GetBuildRequirments())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_BuildRequires);

        // Create a text box component
        BuildZoneEdit.set_label("Building Location or Zone")
            .set_type(dpp::cot_text)
            .set_placeholder(bldg->GetBuildZone())
            .set_min_length(1)
            .set_max_length(128)
            .set_required(true)
            .set_text_style(dpp::text_short)
            .set_id(Component_BuildZone);
    }
    else if (m_job->SupportsType(JOB_TYPE_COMPONENT))
    {
        std::shared_ptr<ComponentJobRequest> comp = std::dynamic_pointer_cast<ComponentJobRequest>(m_job);
        // Create a text box component
        ComponentListEdit.set_label("Component List")
            .set_type(dpp::cot_text)
            .set_placeholder(comp->GetComponentList())
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_CompList);
    }
    else if (m_job->SupportsType(JOB_TYPE_RESOURCE))
    {
        std::shared_ptr<ResourceJobRequest> craft = std::dynamic_pointer_cast<ResourceJobRequest>(m_job);
        // Create a text box component
        ResourceListEdit.set_label("Resource List")
            .set_type(dpp::cot_text)
            .set_placeholder("")
            .set_min_length(1)
            .set_max_length(256)
            .set_required(true)
            .set_text_style(dpp::text_paragraph)
            .set_id(Component_ResourceList);
    }
    else if (m_job->SupportsType(JOB_TYPE_REFINERY))
    {
        //std::shared_ptr<CraftingJobRequest> craft = std::dynamic_pointer_cast<CraftingJobRequest>(m_job);
    }
}

void EditRequestDlg::AddChildrenComponents()
{
    add_component(JobRequestIDEdit);
    add_component(CitizenHandleEdit);
    if (m_job->SupportsType(JOB_TYPE_CRAFTING))
    {
        add_component(ItemDescEdit);
        add_component(ItemQuantityEdit);
    }
    if (m_job->SupportsType(JOB_TYPE_BUILDING))
    {
        add_component(BuildDesignEdit);
        add_component(BuildRequiresEdit);
        add_component(BuildZoneEdit);
    }
    if (m_job->SupportsType(JOB_TYPE_COMPONENT))
    {
        add_component(ComponentListEdit);
    }
    if (m_job->SupportsType(JOB_TYPE_RESOURCE))
    {
        add_component(ResourceListEdit);
    }
    if (m_job->SupportsType(JOB_TYPE_REFINERY))
    {
    }
}