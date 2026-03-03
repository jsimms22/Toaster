//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Resource.h"
#include "CommandContext.h"
// d++
#include <dpp/appcommand.h>
// std library
#include <memory>
#include <string>

class JobRequest;

class NoteDialog : public dpp::interaction_modal_response
{
public:
	NoteDialog(CommandContext& ctx, const std::shared_ptr<const JobRequest>& job)
		: dpp::interaction_modal_response(), m_ctx{ ctx }, m_spJob{ job }
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
	~NoteDialog() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component NoteHistoryEdit; 
	dpp::component AddNewNoteEdit;
	dpp::component JobRequestIDEdit;

	CommandContext m_ctx;
	std::shared_ptr<const JobRequest> m_spJob;
};

