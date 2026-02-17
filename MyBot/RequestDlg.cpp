#include "RequestDlg.h"
#include "Resource.h"
#include "BotUtility.h"
#include "CraftingJobRequest.h"

const std::string CreateRequestDlg::modalID = "CreateRequestModal";
const std::string CreateRequestDlg::modalDesc = "Submit Job Request";

const std::string AssignRequestDlg::modalID = "AssignRequestModal";
const std::string AssignRequestDlg::modalDesc = "Assign Job Request To Worker";

const std::string StatusChangeRequestDlg::modalID = "StatusChangeModal";
const std::string StatusChangeRequestDlg::modalDesc = "Update Status of Job Request";

const std::string PriorityChangeRequestDlg::modalID = "PriorityChangeRequestDlg";
const std::string PriorityChangeRequestDlg::modalDesc = "Update Priority of Job Request";

const std::string EditRequestDlg::modalID = "EditRequestModal";
const std::string EditRequestDlg::modalDesc = "Edit Active Job Request";

void CreateRequestDlg::InitializeControls()
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
    ItemQualityEdit.set_label("Item Quantity")
        .set_type(dpp::cot_text)
        .set_placeholder("")
        .set_min_length(1)
        .set_max_length(4)
        .set_required(true)
        .set_text_style(dpp::text_short)
        .set_id(Component_ItemQuantity);

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

void CreateRequestDlg::AddChildrenComponents()
{
    add_component(CitizenHandleEdit);
    add_component(ItemDescEdit);
    add_component(ItemQualityEdit);
    add_component(PrioritySelect);
}

void AssignRequestDlg::InitializeControls()
{

    // Create a text box component
    JobRequestIDEdit.set_label("Job Request ID")
        .set_type(dpp::cot_text)
        .set_default_value(utils::GuidToString(m_job->GetID()))
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
        .set_default_value(utils::GuidToString(m_job->GetID()))
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
        .set_default_value(utils::GuidToString(m_job->GetID()))
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
        .set_default_value(utils::GuidToString(m_job->GetID()))
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
        ItemQualityEdit.set_label("Item Quantity")
            .set_type(dpp::cot_text)
            .set_default_value(std::to_string(craft->GetQuantity()))
            .set_min_length(1)
            .set_max_length(4)
            .set_text_style(dpp::text_short)
            .set_id(Component_ItemQuantity);
    }
}

void EditRequestDlg::AddChildrenComponents()
{
    set_title(utils::GuidToString(m_job->GetID()));
    add_component(JobRequestIDEdit);
    add_component(CitizenHandleEdit);
    if (m_job->SupportsType(JOB_TYPE_CRAFTING))
    {
        add_component(ItemDescEdit);
        add_component(ItemQualityEdit);
    }
}