#pragma once
#include "GeneralUserPanel.h"
// std library
#include <string>

struct CommandContext;

class CustomerPanel : public GeneralUserPanel
{
public:
	CustomerPanel(
		CommandContext& ctx,
		const std::string& OwnerName,
		const dpp::snowflake& userID,
		const std::string& jobID, 
		const bool bSubscribe);

	virtual ~CustomerPanel() = default;

	static void SubscribeButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

private:
	dpp::component m_btnSubscribe;
};