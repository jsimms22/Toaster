//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"
#include "Resource.h"
// d++
#include <dpp/appcommand.h>
#include <dpp/snowflake.h>
// std library
#include <memory>
#include <string_view>
#include <string>

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminConfigDialog
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class AdminConfigDialog : public dpp::interaction_modal_response
{
public:
	AdminConfigDialog()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
	~AdminConfigDialog() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls() {}
	void AddChildrenComponents() {}

private:
	dpp::component CitizenHandleEdit;
	dpp::component ItemDescEdit;
	dpp::component ItemQuantityEdit;
	dpp::component ItemQualityEdit;
	dpp::component PrioritySelect;
};

