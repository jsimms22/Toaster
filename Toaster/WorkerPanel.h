#pragma once
#include "GeneralUserPanel.h"
// std library
#include <string>

struct CommandContext;

class WorkerPanel : public GeneralUserPanel
{
public:
	WorkerPanel(
		const std::string& OwnerName,
		const dpp::snowflake& userID,
		const std::string& jobID,
		const dpp::snowflake& workerID);

	virtual ~WorkerPanel() = default;

	static void CompleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void UnassignButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

private:
	dpp::component m_btnComplete;
	dpp::component m_btnAssign;
};