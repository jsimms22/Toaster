#include "Resource.h"

#include "BotUtility.h"
#include "JobQueue.h"
#include "Toaster.h"
// d++
#include <dpp/dpp.h>
// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
// mongodb
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
// fmt
#include <fmt/base.h>
#include <fmt/format.h>
// std library
#include <exception>
#include <memory>
#include <string>

// URL Encoding guidelines for mongodb: https://www.mongodb.com/docs/atlas/troubleshoot-connection/#special-characters-in-connection-string-password

namespace mongo = mongocxx::v_noabi;
namespace bson = bsoncxx::v_noabi;

auto main() -> int
{
    const std::string log_name{ "mybot.log" };

    // Initialize and setup spdlog
    std::shared_ptr<spdlog::logger> log;
    spdlog::init_thread_pool(8192, 2);

    std::vector<spdlog::sink_ptr> sinks;
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_name, 1024 * 1024 * 5, 10);
    sinks.push_back(stdout_sink);
    sinks.push_back(rotating);
    
    log = std::make_shared<spdlog::async_logger>("logs", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(log);
    log->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
    log->set_level(spdlog::level::level_enum::debug);

    const std::string BOT_TOKEN{ utils::LoadSecret("../token.txt", "BOT_TOKEN") };
    const std::string DB_USER{ utils::LoadSecret("../token.txt", "DB_USER") };
    const std::string DB_PASS{ utils::LoadSecret("../token.txt", "DB_PASS") };
    const std::string DB_CLUSTER{ utils::LoadSecret("../token.txt", "DB_CLUSTER") };

    // Create an instance and establish our MongoDB connection
    mongo::instance mongoInstance{ }; 
    mongo::database mongoDatabase; 
    std::shared_ptr<JobQueue> spQueue;
    try
    {
        const auto uri = mongo::uri{ fmt::format("mongodb+srv://{}:{}@{}", DB_USER, DB_PASS, DB_CLUSTER) };

        // Set the version of the Stable API on the client
        mongo::options::client client_options;
        const auto api = mongo::options::server_api{ mongo::options::server_api::version::k_version_1 };
        client_options.server_api_opts(api);

        // Setup the connection and get a handle on the "admin" database.
        mongo::client conn{ uri, client_options };
        mongoDatabase = conn["admin"];

        // Ping the database.
        const auto ping_cmd = bson::builder::basic::make_document(bson::builder::basic::kvp("ping", 1));
        mongoDatabase.run_command(ping_cmd.view());
        fmt::println("Pinged your deployment. You successfully connected to MongoDB!");

        // Reconstruct our queue from persistent data
        spQueue = std::make_shared<JobQueue>();
    }
    catch (const std::exception& e)
    {
        log->critical(fmt::format("Exception: {}", e.what()));
        return 0;
    }

    std::uint8_t counter{ 0 };
    while (true)
    {
        // Reinitialize our bot to reestablish connection to discord
        dpp::cluster bot{ BOT_TOKEN };
        bot.intents = dpp::intents::i_message_content | dpp::intents::i_default_intents;

        if (!(&bot))
        {
            log->critical(fmt::format("Could not initialize the bot cluster. {} attempts remaining.", (5 - (++counter))));

            if (counter > 5) { return 1; }

            Sleep(500);
            continue;
        }
        else
        {
            counter = 0;
        }

        // Integrate spdlog logger to D++ log events
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

        ToasterBot toaster(&bot, 0, spQueue, true);
        // Register our custom event handlers
        bot.on_message_create([&toaster](const dpp::message_create_t& event) { toaster.onMessage(event); });
        bot.on_slashcommand([&toaster](const dpp::slashcommand_t& event) { toaster.onSlashCommand(event); });
        bot.on_interaction_create([&toaster](const dpp::interaction_create_t& event) { toaster.onInteractionCreate(event); });
        bot.on_button_click([&toaster](const dpp::button_click_t& event) { toaster.onButtonClick(event); });
        bot.on_form_submit([&toaster](const dpp::form_submit_t& event) { toaster.onFormSubmit(event); });
        bot.on_ready([&toaster](const dpp::ready_t& event) { toaster.onReady(event); });

        try
        {
            bot.start(dpp::st_wait);
        }
        catch (const std::exception& e)
        {
            bot.log(dpp::ll_error, fmt::format("Exiting unexpectedly and attempting to reconnect. Error: {}", e.what()));
        }

        // Reconnection delay to prevent hammering discord
        Sleep(50);
    }

    spQueue->SaveQueueToFile();

    return 0;
}
