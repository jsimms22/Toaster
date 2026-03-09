//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "CommandContext.h"
#include "Resource.h"
// d++
#include <dpp/appcommand.h>
#include <dpp/snowflake.h>
// fmt
#include <fmt/format.h>
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
	AdminConfigDialog(const std::string& strCommandName, CommandContext& ctx)
		: dpp::interaction_modal_response(), m_strCommand{ strCommandName }, m_ctx{ctx}
	{
		set_custom_id(fmt::format("{}:{}", modalID, m_strCommand));
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
	~AdminConfigDialog() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	// For ping rules
	dpp::component PingRoleEdit;
	dpp::component OnNewSelect;
	dpp::component OnUpdateSelect;
	dpp::component OnDeleteSelect;
	dpp::component OnCompleteSelect;

	// For channel announcement rules
	dpp::component NewChannelEdit;
	dpp::component UpdateChannelEdit;
	dpp::component DeleteChannelEdit;
	dpp::component CompleteChannelEdit;

	CommandContext m_ctx;
	std::string m_strCommand;
};

