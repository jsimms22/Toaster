#pragma once
#include "Common.h"
#include "JobRequest.h"

#include <string>
#include <memory>

class CreateRequestDlg : public dpp::interaction_modal_response
{
public:
	CreateRequestDlg()
		: dpp::interaction_modal_response()
	{
		set_custom_id(modalID);
		set_title(modalDesc);
		InitializeControls(); 
		AddChildrenComponents();
	}
	~CreateRequestDlg() = default;

	static const std::string modalID;
	static const std::string modalDesc;

	void InitializeControls();
	void AddChildrenComponents();

private:
	dpp::component CitizenHandleEdit;
	dpp::component ItemDescEdit;
	dpp::component ItemQualityEdit;
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
	const std::vector<std::string> m_vWorkers{ "aim","leaf","stealth","riase","elisa" };
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
	dpp::component ItemDescEdit;
	dpp::component ItemQualityEdit;

	std::shared_ptr<JobRequest> m_job;
};