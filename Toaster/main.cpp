#include "Resource.h"

#include "BotUtility.h"
#include "JobQueue.h"
#include "Toaster.h"
// DISCORD++
#include <dpp/dpp.h>
// SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
// STD Library
#include <memory>
#include <exception>
#include <string>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

// URL Encoding guidelines for mongodb: https://www.mongodb.com/docs/atlas/troubleshoot-connection/#special-characters-in-connection-string-password

auto main() -> int
{
    const std::string BOT_TOKEN{ utils::LoadSecret("../token.txt", "BOT_TOKEN")};
    const std::string DB_USER{ utils::LoadSecret("../token.txt", "DB_USER") };
    const std::string DB_PASS{ utils::LoadSecret("../token.txt", "DB_PASS") };
    const std::string DB_CLUSTER{ utils::LoadSecret("../token.txt", "DB_CLUSTER") };

    try
    {
        // Create an instance.
        mongocxx::v_noabi::instance inst{};
        const auto uri = mongocxx::v_noabi::uri{ "mongodb+srv://" + DB_USER + ":" + DB_PASS + "@" + DB_CLUSTER };
        // Set the version of the Stable API on the client
        mongocxx::v_noabi::options::client client_options;
        const auto api = mongocxx::v_noabi::options::server_api{ mongocxx::v_noabi::options::server_api::version::k_version_1 };
        client_options.server_api_opts(api);
        // Setup the connection and get a handle on the "admin" database.
        mongocxx::v_noabi::client conn{ uri, client_options };
        mongocxx::v_noabi::database db = conn["admin"];
        // Ping the database.
        const auto ping_cmd = bsoncxx::v_noabi::builder::basic::make_document(bsoncxx::v_noabi::builder::basic::kvp("ping", 1));
        db.run_command(ping_cmd.view());
        std::cout << "Pinged your deployment. You successfully connected to MongoDB!" << std::endl;
    }
    catch (const std::exception& e)
    {
        // Handle errors
        std::cout << "Exception: " << e.what() << std::endl;
        return 0;
    }

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

    // Rebuild our queue from persistence (for now just xml, eventually an actual DB)
    std::shared_ptr<JobQueue> spQueue = std::make_shared<JobQueue>();

    while (true)
    {
        dpp::cluster bot{ BOT_TOKEN };

        bot.intents = dpp::intents::i_message_content | dpp::intents::i_default_intents;

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

        if (!(&bot))
            return 1;

        ToasterBot toaster(&bot, 0, spQueue, true);

        bot.on_message_create([&toaster](const dpp::message_create_t& event) { toaster.onMessage(event); });
        bot.on_slashcommand([&toaster](const dpp::slashcommand_t& event) { toaster.onSlashCommand(event); });
        bot.on_interaction_create([&toaster](const dpp::interaction_create_t& event) { toaster.onInteractionCreate(event); });
        bot.on_button_click([&toaster](const dpp::button_click_t& event) { toaster.onButtonClick(event); });
        bot.on_form_submit([&toaster](const dpp::form_submit_t& event) { toaster.onFormSubmit(event); });
        bot.on_ready([&toaster](const dpp::ready_t& event) { toaster.onReady(event); });

        // Start bot
        try
        {
            bot.start(dpp::st_wait);
        }
        catch (const std::exception& e)
        {
            bot.log(dpp::ll_error, "Exiting unexpectedly: " + std::string(e.what()));
        }

        // Reconnection delay to prevent hammering discord
        Sleep(50);
    }

    spQueue->SaveQueueToFile();

    return 0;
}
