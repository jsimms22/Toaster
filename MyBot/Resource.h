#pragma once

/*------------------------------COMPONENT IDS-----------------------------------*/
static constexpr const char* Component_RequestID	{ "request_id" };
static constexpr const char* Component_CitizenID	{ "citizen_id" };
static constexpr const char* Component_ItemDesc		{ "desc_id" };
static constexpr const char* Component_ItemQuantity	{ "quantity_id" };
static constexpr const char* Component_Priority		{ "priority_id" };
static constexpr const char* Component_Assignment	{ "assign_id" };
static constexpr const char* Component_Status		{ "status_id" };
/*------------------------------COMPONENT IDS-----------------------------------*/

/*-------------------------------COMMAND IDS------------------------------------*/
static constexpr const char* Command_Hello			{ "hello" };
static constexpr const char* Command_JobRequest		{ "job_request" };
static constexpr const char* Command_ModifyRequest	{ "modify_request" };
static constexpr const char* Command_ShowRequest	{ "show_request" };
static constexpr const char* Command_MyRequests		{ "my_requests" };
static constexpr const char* Command_ShowQueue		{ "show_queue" };
/*-------------------------------COMMAND IDS------------------------------------*/

/*----------------------------COMMAND OPTION IDS--------------------------------*/
// Command Options for Command_JobRequest
static constexpr const char* Option_ItemCrafting	{ "item" };
static constexpr const char* Option_BaseBuidling	{ "building" };
static constexpr const char* Option_ComponentRequest{ "component" };
static constexpr const char* Option_ResourceCollect	{ "resource" };
static constexpr const char* Option_RefineryJob		{ "refinery" };

// Command Options for Command_ModifyRequest
static constexpr const char* Option_Edit			{ "edit" };
static constexpr const char* Option_Assign			{ "assign" };
static constexpr const char* Option_Status			{ "status" };
static constexpr const char* Option_Priority		{ "priority" };
static constexpr const char* Option_Delete			{ "delete" };
/*----------------------------COMMAND OPTION IDS--------------------------------*/

/*--------------------------COMMAND PARAMETER IDS-------------------------------*/
static constexpr const char* Parameter_Cmd{ "cmd" };
static constexpr const char* Parameter_Id{ "id" };
/*--------------------------COMMAND PARAMETER IDS-------------------------------*/