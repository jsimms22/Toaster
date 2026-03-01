//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
// std library
#include <cstdlib>
#include <string_view>

constexpr std::size_t USERID_NULL{ 0 };

constexpr std::size_t JOB_TYPE_GENERAL	{ 10 };
constexpr std::size_t JOB_TYPE_CRAFTING	{ 20 };
constexpr std::size_t JOB_TYPE_BUILDING	{ 30 };
constexpr std::size_t JOB_TYPE_RESOURCE	{ 40 };
constexpr std::size_t JOB_TYPE_REFINERY	{ 50 };
constexpr std::size_t JOB_TYPE_COMPONENT{ 60 };
constexpr std::size_t JOB_TYPE_HAZARD	{ 70 };

/*------------------------------COMPONENT IDS-----------------------------------*/
constexpr const char* Component_RequestID		{ "request_id" };
constexpr const char* Component_CitizenID		{ "citizen_id" };
constexpr const char* Component_ItemDesc		{ "desc_id" };
constexpr const char* Component_ItemQuantity	{ "quantity_id" };
constexpr const char* Component_ItemQuality		{ "quality_id" };
constexpr const char* Component_BuildDesign		{ "designate_id" };
constexpr const char* Component_BuildRequires	{ "requirements_id" };
constexpr const char* Component_BuildZone		{ "buildzone_id" };
constexpr const char* Component_CompList		{ "complist_id" };
constexpr const char* Component_ResourceType	{ "resourcetype_id" };
constexpr const char* Component_ResourceList	{ "resourcelist_id" };
constexpr const char* Component_ResourceQuality	{ "resourcequality_id" };
constexpr const char* Component_RefinerySite	{ "refinerysite_id" };
constexpr const char* Component_Priority		{ "priority_id" };
constexpr const char* Component_Assignment		{ "assign_id" };
constexpr const char* Component_Status			{ "status_id" };
constexpr const char* Component_JobDescription	{ "description_id" };
constexpr const char* Component_DeleteJustification	{ "justification_id" };
/*------------------------------COMPONENT IDS-----------------------------------*/

/*---------------------------ADMIN COMPONENT IDS--------------------------------*/
// Components for Command_ConfigChannels Dialog
constexpr const char* Component_NewChannel		{ "newchannel" };
constexpr const char* Component_EditChannel		{ "editchannel" };
constexpr const char* Component_DeletedChannel	{ "deletechannel" };
constexpr const char* Component_CompletedChannel{ "completechannel" };

// Components for Command_ConfigPing Dialog
constexpr const char* Component_PingRole		{ "pingrole" };
constexpr const char* Component_PingOnNew		{ "pingnew" };
constexpr const char* Component_PingOnUpdate	{ "pingupdate" };
constexpr const char* Component_PingOnDelete	{ "pingdelete" };
constexpr const char* Component_PingOnComplete	{ "pingcomplete" };
/*---------------------------ADMIN COMPONENT IDS--------------------------------*/

/*----------------------------ADMIN COMMAND IDS---------------------------------*/
constexpr const char* Command_ConfigChannels	{ "admin_configchannels" };
constexpr const char* Command_ConfigPing		{ "admin_configping" };
constexpr const char* Command_ConfigRoles		{ "admin_configroles" };
/*----------------------------ADMIN COMMAND IDS---------------------------------*/

/*-------------------------------COMMAND IDS------------------------------------*/
constexpr const char* Command_Help				{ "help" };
constexpr const char* Command_Admin				{ "manager_portal" };
constexpr const char* Command_Worker			{ "worker_portal" };
constexpr const char* Command_JobRequest		{ "create_request" };
constexpr const char* Command_ModifyRequest		{ "modify_request" };
constexpr const char* Command_ShowRequest		{ "show_request" };
constexpr const char* Command_MyRequests		{ "my_requests" };
constexpr const char* Command_ShowQueue			{ "show_queue" };
constexpr const char* Command_SummaryQueue		{ "show_summary" };
/*-------------------------------COMMAND IDS------------------------------------*/

/*----------------------------COMMAND OPTION IDS--------------------------------*/
// Command Options for Command_ConfigRoles
constexpr const char* Option_CraftingRole		{ "craftingrole" };
constexpr const char* Option_BuildingRole		{ "buildingrole" };
constexpr const char* Option_CompDealerRole		{ "comprole" };
constexpr const char* Option_ResourceRole		{ "resourcerole" };
constexpr const char* Option_RefiningRole		{ "refiningrole" };
constexpr const char* Option_HazmatRole			{ "hazmatrole" };
constexpr const char* Option_ManagerRole		{ "managerrole" };

// Command Options for Command_Admin
constexpr const char* Option_Bot				{ "adminbot" };
constexpr const char* Option_Queue				{ "adminqueue" };

// Command Options for Command_Worker
constexpr const char* Option_Overview			{ "workeroverview" };
constexpr const char* Option_AllAssignments		{ "workerall" };

// Command Options for Command_JobRequest
constexpr const char* Option_All				{ "all" };
constexpr const char* Option_ItemCrafting		{ "item" };
constexpr const char* Option_BaseBuidling		{ "building" };
constexpr const char* Option_ComponentRequest	{ "component" };
constexpr const char* Option_ResourceCollect	{ "resource" };
constexpr const char* Option_RefineryJob		{ "refinery" };

// Command Options for Command_ModifyRequest
constexpr const char* Option_Edit				{ "edit" };
constexpr const char* Option_Assign				{ "assign" };
constexpr const char* Option_Status				{ "status" };
constexpr const char* Option_Priority			{ "priority" };
constexpr const char* Option_Delete				{ "delete" };
/*----------------------------COMMAND OPTION IDS--------------------------------*/

/*--------------------------COMMAND PARAMETER IDS-------------------------------*/
constexpr const char* Parameter_Cmd				{ "cmd" };
constexpr const char* Parameter_Type			{ "type" };
constexpr const char* Parameter_Id				{ "id" };
/*--------------------------COMMAND PARAMETER IDS-------------------------------*/

/*--------------------------------BUTTON IDS------------------------------------*/
constexpr const char* Button_Complete			{ "btn_complete" };
constexpr const char* Button_Note				{ "btn_note" };
constexpr const char* Button_Unassign			{ "btn_unassign" };
constexpr const char* Button_Delete				{ "btn_delete" };
constexpr const char* Button_Stalled			{ "btn_summary_stalled" };
constexpr const char* Button_Unassigned			{ "btn_summary_unassigned" };
/*--------------------------------BUTTON IDS-------------------------------------*/