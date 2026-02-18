#include "MyBot.h"
#include "Resource.h"
#include "BotUtility.h"
#include "RequestDlg.h"
#include "JobQueue.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <string>

auto main() -> int
{
    const std::string BOT_TOKEN = utils::LoadBotToken("../token.txt");
    dpp::cluster bot(BOT_TOKEN);

    //bot.on_log(dpp::utility::cout_logger());
    const std::string log_name = "mybot.log";
    // /* Set up spdlog logger */
    std::shared_ptr<spdlog::logger> log;
    spdlog::init_thread_pool(8192, 2);
    std::vector<spdlog::sink_ptr> sinks;
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_name, 1024 * 1024 * 5, 10);
    sinks.push_back(stdout_sink);
    sinks.push_back(rotating);
    log = std::make_shared<spdlog::async_logger>("logs", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(log);
    log->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
    log->set_level(spdlog::level::level_enum::debug);

    /* Integrate spdlog logger to D++ log events */
    bot.on_log([&bot, &log](const dpp::log_t& event) {
        switch (event.severity) {
        case dpp::ll_trace:
            log->trace("{}", event.message);
            break;
        case dpp::ll_debug:
            log->debug("{}", event.message);
            break;
        case dpp::ll_info:
            log->info("{}", event.message);
            break;
        case dpp::ll_warning:
            log->warn("{}", event.message);
            break;
        case dpp::ll_error:
            log->error("{}", event.message);
            break;
        case dpp::ll_critical:
        default:
            log->critical("{}", event.message);
            break;
        }
        });

    // Repopulate queue from file
    JobQueue* queue = new JobQueue();

    bot.on_slashcommand([&bot, queue](const dpp::slashcommand_t& event) 
        {
            if (event.command.get_command_name() == Command_MyRequests)
            {
                const dpp::user author = event.command.get_issuing_user();
                // Reply to the user, but only let them see the response. 
                if (queue->GetQueueSize() == 0)
                {
                    // If no requests are found, send a message saying no requests exist
                    event.reply(dpp::message("You have no requests in queue.").set_flags(dpp::m_ephemeral));
                }
                else
                {
                    // Send the formatted list of requests back to the user
                    dpp::message msg;
                    std::string header = "Here are your current requests (ordered by priority):\n\n";
                    std::string result = queue->PrintQueueByUser(author.username);
                    result = !result.empty() ? result : "";
                    msg.set_content(!result.empty() ? header + result : "You have no requests in queue.").set_flags(dpp::m_ephemeral);
                    event.reply(msg);
                }

                bot.log(dpp::ll_info, author.username + " called " + event.command.get_command_name());
            }
            else if (event.command.get_command_name() == Command_ShowQueue)
            {
                const dpp::user author = event.command.get_issuing_user();
                // Reply to the user, but only let them see the response. 
                if (queue->GetQueueSize() == 0)
                {
                    // If no requests are found, send a message saying no requests exist
                    event.reply(dpp::message("Request queue is currently empty.").set_flags(dpp::m_ephemeral));
                }
                else
                {
                    // Send the formatted list of requests back to the user
                    dpp::message msg;
                    const std::string header = "Here is the request queue (ordered by priority):\n\n";
                    msg.set_content(header + queue->PrintQueue()).set_flags(dpp::m_ephemeral);
                    event.reply(msg);
                }

                bot.log(dpp::ll_info, author.username + " called " + event.command.get_command_name());
            }
            else if (event.command.get_command_name() == "hello")
            {
                const dpp::user author = event.command.get_issuing_user();
                // Reply to the user, but only let them see the response. 
                event.reply(dpp::message("Hello! How are you today?").set_flags(dpp::m_ephemeral));

                bot.log(dpp::ll_info, author.username + " called " + event.command.get_command_name());
            }
        });

    bot.on_interaction_create([&bot, queue](const dpp::interaction_create_t& event) 
        {
            if (event.command.get_command_name() == Command_JobRequest)
            {
                const dpp::user author = event.command.get_issuing_user();
                const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

                if (strCmdID == Option_ItemCrafting)
                {
                    CraftRequestDlg modal;
                    event.dialog(modal);
                }
                else if (strCmdID == Option_ComponentRequest)
                {
                    ComponentRequestDlg modal;
                    event.dialog(modal);
                }
                else if (strCmdID == Option_BaseBuidling)
                {
                    BuildRequestDlg modal;
                    event.dialog(modal);
                }
                else if (strCmdID == Option_ResourceCollect)
                {
                    ResourceRequestDlg modal;
                    event.dialog(modal);
                }
                else if (strCmdID == Option_RefineryJob)
                {
                    CraftRequestDlg modal;
                    event.dialog(modal);
                }

                bot.log(dpp::ll_info, author.username + " called " + event.command.get_command_name());
            }
            else if (event.command.get_command_name() == Command_ModifyRequest)
            {
                const dpp::user author = event.command.get_issuing_user();
                const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
                const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

                const std::shared_ptr<JobRequest> job = queue->GetJobByGUID(strJobID);
                if (!job)
                {
                    return;
                }

                if (strCmdID == Option_Edit)
                {
                    EditRequestDlg editModal(job);
                    event.dialog(editModal);
                }
                else if (strCmdID == Option_Assign)
                {
                    AssignRequestDlg assignModal(job);
                    event.dialog(assignModal);
                }
                else if (strCmdID == Option_Status)
                {
                    StatusChangeRequestDlg statusModal(job);
                    event.dialog(statusModal);
                }
                else if (strCmdID == Option_Priority)
                {
                    PriorityChangeRequestDlg statusModal(job);
                    event.dialog(statusModal);
                }
                else if (strCmdID == Option_Delete)
                {
                    const dpp::user author = event.command.get_issuing_user();
                    const bool result = queue->DeleteJobByGUID(strJobID);

                    const std::string header = author.username + " deleted job request: \n";
                    event.reply(dpp::message(header + strJobID).set_flags(dpp::m_ephemeral));

                    queue->SaveQueueToFile();
                }

                bot.log(dpp::ll_info, author.username + " called " + event.command.get_command_name());
            }
            else if (event.command.get_command_name() == Command_ShowRequest)
            {
                const dpp::user author = event.command.get_issuing_user();
                const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));

                const std::shared_ptr<JobRequest> job = queue->GetJobByGUID(strJobID);
                if (job)
                {
                    const std::string header = "Here is the request:\n\n";
                    event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));
                }
                else
                {
                    event.reply(dpp::message("This id does not exist in queue.").set_flags(dpp::m_ephemeral));
                }

                bot.log(dpp::ll_info, author.username + " called " + event.command.get_command_name());
            }
        });

    // This event handles form submission for the modal dialog we create above 
    bot.on_form_submit([&bot, queue](const dpp::form_submit_t& event)
        {
            if (event.custom_id == CraftRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strDesc = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
                const std::string strQuantity = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
                const std::string strQuality = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
                const std::string strPriority = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

                auto jobCraft = std::make_shared<CraftingJobRequest>();
                jobCraft->SetAuthor(author.username);
                jobCraft->SetSCHandle(strSCHandle);
                jobCraft->SetItemDesc(strDesc);
                jobCraft->SetQuantity(std::stoull(strQuantity));
                jobCraft->SetQualityThres(strQuality);
                jobCraft->SetPriority(JobRequest::StringToPriority(strPriority));

                // Send a reply with the details of the request before we time out
                const std::string header = author.username + " submitted job request: \n";
                event.reply(dpp::message(header + jobCraft->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                queue->AddToQueue(std::move(jobCraft));

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == BuildRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strDesign = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
                const std::string strRequires = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
                const std::string strZone = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
                const std::string strPriority = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

                auto jobBuild = std::make_shared<BuildingJobRequest>();
                jobBuild->SetAuthor(author.username);
                jobBuild->SetSCHandle(strSCHandle);
                jobBuild->SetBuildDesign(strDesign);
                jobBuild->SetBuildRequirments(strRequires);
                jobBuild->SetBuildZone(strZone);
                jobBuild->SetPriority(JobRequest::StringToPriority(strPriority));

                // Send a reply with the details of the request before we time out
                const std::string header = author.username + " submitted job request: \n";
                event.reply(dpp::message(header + jobBuild->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                queue->AddToQueue(std::move(jobBuild));

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == ComponentRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strCompList = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
                const std::string strPriority = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

                auto jobComp = std::make_shared<ComponentJobRequest>();
                jobComp->SetAuthor(author.username);
                jobComp->SetSCHandle(strSCHandle);
                jobComp->SetComponentList(strCompList);
                jobComp->SetPriority(JobRequest::StringToPriority(strPriority));

                // Send a reply with the details of the request before we time out
                const std::string header = author.username + " submitted job request: \n";
                event.reply(dpp::message(header + jobComp->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                queue->AddToQueue(std::move(jobComp));

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == ResourceRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strResState = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
                const std::string strResList = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
                const std::string strQuality = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
                const std::string strPriority = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

                auto jobRes = std::make_shared<ResourceJobRequest>();
                jobRes->SetAuthor(author.username);
                jobRes->SetSCHandle(strSCHandle);
                jobRes->SetResourceState(ResourceJobRequest::StringToState(strResState));
                jobRes->SetResourcelist(strResList);
                jobRes->SetQualityThres(strQuality);
                jobRes->SetPriority(JobRequest::StringToPriority(strPriority));

                // Send a reply with the details of the request before we time out
                const std::string header = author.username + " submitted job request: \n";
                event.reply(dpp::message(header + jobRes->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                queue->AddToQueue(std::move(jobRes));

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == EditRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strSCHandle = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
                // These parameters are input that depend on the dialog form's order as defined by the type of the job
                const std::string strParam1 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
                const std::string strParam2 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
                const std::string strParam3 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

                std::shared_ptr<JobRequest> job = queue->GetJobByGUID(strID);
                if (job->SupportsType(JOB_TYPE_CRAFTING))
                {
                    std::shared_ptr<CraftingJobRequest> craft = std::dynamic_pointer_cast<CraftingJobRequest>(job);
                    craft->SetSCHandle(strSCHandle);
                    craft->SetItemDesc(strParam1);
                    craft->SetQuantity(std::stoull(strParam2));

                    const std::string header = author.username + " edited job request: \n";
                    event.reply(dpp::message(header + craft->PrintJobDetails()).set_flags(dpp::m_ephemeral));
                }
                else if (job->SupportsType(JOB_TYPE_BUILDING))
                {
                    std::shared_ptr<BuildingJobRequest> bldg = std::dynamic_pointer_cast<BuildingJobRequest>(job);
                    bldg->SetSCHandle(strSCHandle);
                    bldg->SetBuildDesign(strParam1);
                    bldg->SetBuildRequirments(strParam2);
                    bldg->SetBuildZone(strParam3);

                    const std::string header = author.username + " edited job request: \n";
                    event.reply(dpp::message(header + bldg->PrintJobDetails()).set_flags(dpp::m_ephemeral));
                }
                else if (job->SupportsType(JOB_TYPE_COMPONENT))
                {
                    std::shared_ptr<ComponentJobRequest> comp = std::dynamic_pointer_cast<ComponentJobRequest>(job);
                    comp->SetSCHandle(strSCHandle);
                    comp->SetComponentList(strParam1);

                    const std::string header = author.username + " edited job request: \n";
                    event.reply(dpp::message(header + comp->PrintJobDetails()).set_flags(dpp::m_ephemeral));
                }
                else if (job->SupportsType(JOB_TYPE_RESOURCE))
                {
                    std::shared_ptr<ResourceJobRequest> resrc = std::dynamic_pointer_cast<ResourceJobRequest>(job);
                    resrc->SetSCHandle(strSCHandle);
                    resrc->SetResourcelist(strParam1);

                    const std::string header = author.username + " edited job request: \n";
                    event.reply(dpp::message(header + resrc->PrintJobDetails()).set_flags(dpp::m_ephemeral));
                }
                else if (job->SupportsType(JOB_TYPE_RESOURCE))
                {
                }

                if (job)
                {
                    queue->SaveQueueToFile();
                }

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == AssignRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strWorker = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
                const std::string strStatus = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

                std::shared_ptr<JobRequest> job = queue->GetJobByGUID(strID);
                if (job)
                {
                    job->SetWorker(strWorker);
                    job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

                    const std::string header = author.username + " assigned job request: \n";
                    event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                    queue->SaveQueueToFile();
                }

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == StatusChangeRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

                std::shared_ptr<JobRequest> job = queue->GetJobByGUID(strID);
                if (job)
                {
                    job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

                    const std::string header = author.username + " updated job status: \n";
                    event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                    queue->SaveQueueToFile();
                }

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
            else if (event.custom_id == PriorityChangeRequestDlg::modalID)
            {
                const dpp::user author = event.command.get_issuing_user();

                const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
                const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

                std::shared_ptr<JobRequest> job = queue->GetJobByGUID(strID);
                if (job)
                {
                    job->SetPriority(JobRequest::StringToPriority(strPriority));

                    const std::string header = author.username + " updated job priority: \n";
                    event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));

                    queue->SaveQueueToFile();
                }

                bot.log(dpp::ll_info, author.username + " called " + event.custom_id);
            }
        });

    bot.on_ready([&bot](const dpp::ready_t& event)
        {
            //if (dpp::run_once<struct clear_bot_commands>()) 
            {
                //bot.guild_bulk_command_delete(1472034166869852287);
                //bot.global_bulk_command_delete();
            }
            if (dpp::run_once<struct register_bot_commands>())
            {
                // Create a slash command and register it as a global command 
                dpp::slashcommand request(Command_JobRequest, "Submit a job request to the queue.", bot.me.id);
                request.add_option(dpp::command_option(dpp::co_string, Parameter_Cmd, "Select the action to take.", true)
                    .add_choice(dpp::command_option_choice("Item Crafting", Option_ItemCrafting))
                    .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
                    .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
                    .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
                    .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)));

                dpp::slashcommand myrequests(Command_MyRequests, "Retrieve a list of your requests and status.", bot.me.id);

                dpp::slashcommand seequeue(Command_ShowQueue, "Retrieve the current list of requests in the queue by a priority.", bot.me.id);

                dpp::slashcommand showrequest(Command_ShowRequest, "Display the information about the request.", bot.me.id);
                showrequest.add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true));

                dpp::slashcommand modifyrequest(Command_ModifyRequest, "Edit a job request in queue by its id.", bot.me.id);
                modifyrequest.add_option(dpp::command_option(dpp::co_string, Parameter_Cmd, "Select the action to take.", true)
                    .add_choice(dpp::command_option_choice("Edit Job", Option_Edit))
                    .add_choice(dpp::command_option_choice("Assign Worker", Option_Assign))
                    .add_choice(dpp::command_option_choice("Update Status", Option_Status))
                    .add_choice(dpp::command_option_choice("Change Priority", Option_Priority))
                    .add_choice(dpp::command_option_choice("Delete Job", Option_Delete)));
                modifyrequest.add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true));

                dpp::slashcommand emphemcommand(Command_Hello, "Hello there!", bot.me.id);

                bot.guild_bulk_command_create({ request, myrequests, seequeue, showrequest, modifyrequest, emphemcommand }, 1472034166869852287);
                //bot.global_bulk_command_create({ pingcommand, pongcommand, dingcommand, dongcommand, modalcommand, emphemcommand });

                bot.log(dpp::ll_debug, "Registered commands with guild");
            }
        });

    // Start bot 
    bot.start(dpp::st_wait);

    queue->SaveQueueToFile();
    delete(queue);

    return 0;
}
