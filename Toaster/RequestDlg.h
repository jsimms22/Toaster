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

class GuildSettings;

//---------------------------------------------------------------------------------------------------------------------
/// \class CraftRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class CraftRequestDlg : public dpp::interaction_modal_response
{
public:
	CraftRequestDlg();
	~CraftRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component ItemDescEdit;
	dpp::component ItemQuantityEdit;
	dpp::component ItemQualityEdit;
	dpp::component PrioritySelect;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class BuildRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class BuildRequestDlg : public dpp::interaction_modal_response
{
public:
	BuildRequestDlg();
	~BuildRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component BuildDesignEdit;
	dpp::component BuildRequiresEdit;
	dpp::component BuildZoneEdit;
	dpp::component PrioritySelect;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class ComponentRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class ComponentRequestDlg : public dpp::interaction_modal_response
{
public:
	ComponentRequestDlg();
	~ComponentRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component ComponentListEdit;
	dpp::component PrioritySelect;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class ResourceRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class ResourceRequestDlg : public dpp::interaction_modal_response
{
public:
	ResourceRequestDlg();
	~ResourceRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component ResourceTypeSelect;
	dpp::component ResourceListEdit;
	dpp::component ResourceQualityEdit;
	dpp::component PrioritySelect;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class RefineryRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class RefineryRequestDlg : public dpp::interaction_modal_response
{
public:
	RefineryRequestDlg();
	~RefineryRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component RefineryTypeSelect;
	dpp::component ResourceListEdit;
	dpp::component RefinerySiteEdit;
	dpp::component PrioritySelect;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class HazardousRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class HazardousRequestDlg : public dpp::interaction_modal_response
{
public:
	HazardousRequestDlg();
	~HazardousRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component ThreatLevelSelect;
	dpp::component HazResourceZoneEdit;
	dpp::component HazResourceListEdit;
	dpp::component PrioritySelect;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AssignRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class AssignRequestDlg : public dpp::interaction_modal_response
{
public:
	AssignRequestDlg(const std::shared_ptr<const JobRequest>& job);
	~AssignRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component WorkerAssignSelect;
	dpp::component WorkerAssignSelect2;
	dpp::component WorkerAssignSelect3;
	dpp::component StatusUpdateSelect;

	std::shared_ptr<const JobRequest> m_spJob;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class StatusChangeRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class StatusChangeRequestDlg : public dpp::interaction_modal_response
{
public:
	struct StatusOption {
		std::string label;
		std::string value;
		const char* emoji;
	};

	static const std::vector<StatusOption> StatusList;

	StatusChangeRequestDlg(const std::shared_ptr<const JobRequest>& job);
	~StatusChangeRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component StatusUpdateSelect;

	std::shared_ptr<const JobRequest> m_spJob;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class PriorityChangeRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class PriorityChangeRequestDlg : public dpp::interaction_modal_response
{
public:
	struct PriorityOption {
		std::string label;
		std::string value;
		const char* emoji;
		std::string description;
	};

	static const std::vector<PriorityOption> PriorityList;

	PriorityChangeRequestDlg(const std::shared_ptr<const JobRequest>& job);
	~PriorityChangeRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component PrioritySelect;

	std::shared_ptr<const JobRequest> m_spJob;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class EditRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class EditRequestDlg : public dpp::interaction_modal_response
{
public:
	EditRequestDlg(const std::shared_ptr<const JobRequest>& job);
	~EditRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component CitizenHandleEdit;
	// For crafting
	dpp::component ItemDescEdit;
	dpp::component ItemQuantityEdit;
	// For base building
	dpp::component BuildDesignEdit;
	dpp::component BuildRequiresEdit;
	dpp::component BuildZoneEdit;
	// For component requests
	dpp::component ComponentListEdit;
	// For resource requests
	dpp::component ResourceListEdit;
	// For refining jobs
	dpp::component RefinerySiteEdit;
	// For hazard jobs
	dpp::component ThreatLevelSelect;
	dpp::component HazResourceZoneEdit;
	dpp::component HazResourceListEdit;

	std::shared_ptr<const JobRequest> m_spJob;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class DeleteRequestDlg
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class DeleteRequestDlg : public dpp::interaction_modal_response
{
public:
	DeleteRequestDlg(const std::shared_ptr<const JobRequest>& job, const std::string details);
	~DeleteRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component JobDescriptionEdit;
	dpp::component DeleteJustificationEdit;

	std::shared_ptr<const JobRequest> m_spJob;
	std::string m_strDetails;
};