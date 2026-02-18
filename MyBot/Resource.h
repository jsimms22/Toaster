#pragma once

/*------------------------------COMPONENT IDS-----------------------------------*/
constexpr const char* Component_RequestID	{ "request_id" };
constexpr const char* Component_CitizenID	{ "citizen_id" };
constexpr const char* Component_ItemDesc		{ "desc_id" };
constexpr const char* Component_ItemQuantity	{ "quantity_id" };
constexpr const char* Component_ItemQuality{ "quality_id" };
constexpr const char* Component_BuildDesign{ "designate_id" };
constexpr const char* Component_BuildRequires{ "requirements_id" };
constexpr const char* Component_BuildZone		{ "buildzone_id" };
constexpr const char* Component_CompList	{ "complist_id" };
constexpr const char* Component_ResourceType{ "resourcetype_id" };
constexpr const char* Component_ResourceList{ "resourcelist_id" };
constexpr const char* Component_ResourceQuality{ "resourcequality_id" };
constexpr const char* Component_Priority		{ "priority_id" };
constexpr const char* Component_Assignment	{ "assign_id" };
constexpr const char* Component_Status		{ "status_id" };
/*------------------------------COMPONENT IDS-----------------------------------*/

/*-------------------------------COMMAND IDS------------------------------------*/
constexpr const char* Command_Hello			{ "hello" };
constexpr const char* Command_JobRequest		{ "job_request" };
constexpr const char* Command_ModifyRequest	{ "modify_request" };
constexpr const char* Command_ShowRequest	{ "show_request" };
constexpr const char* Command_MyRequests		{ "my_requests" };
constexpr const char* Command_ShowQueue		{ "show_queue" };
/*-------------------------------COMMAND IDS------------------------------------*/

/*----------------------------COMMAND OPTION IDS--------------------------------*/
// Command Options for Command_JobRequest
constexpr const char* Option_ItemCrafting	{ "item" };
constexpr const char* Option_BaseBuidling	{ "building" };
constexpr const char* Option_ComponentRequest{ "component" };
constexpr const char* Option_ResourceCollect	{ "resource" };
constexpr const char* Option_RefineryJob		{ "refinery" };

// Command Options for Command_ModifyRequest
constexpr const char* Option_Edit			{ "edit" };
constexpr const char* Option_Assign			{ "assign" };
constexpr const char* Option_Status			{ "status" };
constexpr const char* Option_Priority		{ "priority" };
constexpr const char* Option_Delete			{ "delete" };
/*----------------------------COMMAND OPTION IDS--------------------------------*/

/*--------------------------COMMAND PARAMETER IDS-------------------------------*/
constexpr const char* Parameter_Cmd{ "cmd" };
constexpr const char* Parameter_Id{ "id" };
/*--------------------------COMMAND PARAMETER IDS-------------------------------*/