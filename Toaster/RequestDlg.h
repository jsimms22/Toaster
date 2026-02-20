#pragma once
#include "JobRequest.h"
#include "Resource.h"

#include <dpp/appcommand.h>

#include <memory>
#include <string>

class CraftRequestDlg : public dpp::interaction_modal_response
{
public:
	CraftRequestDlg()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls(); 
		AddChildrenComponents();
	}
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

class BuildRequestDlg : public dpp::interaction_modal_response
{
public:
	BuildRequestDlg()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
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

class ComponentRequestDlg : public dpp::interaction_modal_response
{
public:
	ComponentRequestDlg()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
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

class ResourceRequestDlg : public dpp::interaction_modal_response
{
public:
	ResourceRequestDlg()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
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

class RefineryRequestDlg : public dpp::interaction_modal_response
{
public:
	RefineryRequestDlg()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
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

class AssignRequestDlg : public dpp::interaction_modal_response
{
public:
	AssignRequestDlg(const std::shared_ptr<JobRequest>& job)
		: dpp::interaction_modal_response(), m_job(job)
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
	~AssignRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component WorkerAssignSelect;
	dpp::component StatusUpdateSelect;

	std::shared_ptr<JobRequest> m_job;
	const std::vector<std::string> m_vWorkers{ "aimx83", "mike.d.spectre", "linealign", "leaf1318" };
};

class StatusChangeRequestDlg : public dpp::interaction_modal_response
{
public:
	StatusChangeRequestDlg(const std::shared_ptr<JobRequest>& job)
		: dpp::interaction_modal_response(), m_job(job)
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
	~StatusChangeRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component StatusUpdateSelect;

	std::shared_ptr<JobRequest> m_job;
};

class PriorityChangeRequestDlg : public dpp::interaction_modal_response
{
public:
	PriorityChangeRequestDlg(const std::shared_ptr<JobRequest>& job)
		: dpp::interaction_modal_response(), m_job(job)
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
	~PriorityChangeRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component JobRequestIDEdit;
	dpp::component PrioritySelect;

	std::shared_ptr<JobRequest> m_job;
};

class EditRequestDlg : public dpp::interaction_modal_response
{
public:
	EditRequestDlg(const std::shared_ptr<JobRequest>& job)
		: dpp::interaction_modal_response(), m_job(job)
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls();
		AddChildrenComponents();
	}
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

	std::shared_ptr<JobRequest> m_job;
};