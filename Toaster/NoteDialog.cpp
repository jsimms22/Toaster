#include "NoteDialog.h"

#include "BotUtility.h"
#include "JobRequest.h"
// fmt
#include <fmt/format.h>
// std library
#include <string>

const std::string NoteDialog::modalID = "NoteDialogModal";
const std::string NoteDialog::modalDesc = "Add a Note to the Request";

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void NoteDialog::InitializeControls()
{
	// Create a text box component
	JobRequestIDEdit.set_label("Job Request ID")
		.set_type(dpp::cot_text)
		.set_default_value(utils::GuidToStringNoBrackets(m_spJob->GetID()))
		.set_min_length(0)
		.set_max_length(128)
		.set_text_style(dpp::text_short)
		.set_id(Component_RequestID);

	const std::string history = m_spJob->PrintLastTwoNotes(m_ctx.cluster);
	NoteHistoryEdit.set_label("Recent Notes")
		.set_type(dpp::cot_text)
		.set_placeholder("No notes.")
		.set_default_value(!history.empty() ? history : "")
		.set_min_length(0)
		.set_max_length(512)
		.set_text_style(dpp::text_paragraph)
		.set_id(Component_NoteHistory);

	AddNewNoteEdit.set_label("Add New Note")
		.set_type(dpp::cot_text)
		.set_placeholder("Add note...")
		.set_min_length(0)
		.set_max_length(256)
		.set_text_style(dpp::text_paragraph)
		.set_id(Component_AddNewNote);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void NoteDialog::AddChildrenComponents()
{
	add_component(JobRequestIDEdit);
	add_component(NoteHistoryEdit);
	add_component(AddNewNoteEdit);
}