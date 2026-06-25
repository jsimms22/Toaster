#include "Resource.h"

#include "Commands.h"
#include "BotUtility.h"
#include "Toaster.h"
#include "TinyXmlDatabase.h"
#include "MongoDatabase.h"
#include "MongoJobRepo.h"
// d++
#include <dpp/dpp.h>
#include <dpp/intents.h>
#include <dpp/message.h>
// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
// fmt
#include <fmt/base.h>
#include <fmt/format.h>
// std library
#include <exception>
#include <memory>
#include <string>
#include <thread>

// URL Encoding guidelines for mongodb: https://www.mongodb.com/docs/atlas/troubleshoot-connection/#special-characters-in-connection-string-password

std::string CacheString(dpp::cache_policy_setting_t cache);
std::string IntentsString(uint64_t intents);

auto main() -> int
{
    const std::string log_name{ "logs/toaster_logfile.log" };

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
    log->set_level(spdlog::level::level_enum::info);

    const std::string BOT_TOKEN{ utils::LoadSecret("token.txt", "BOT_TOKEN") };
    const std::string DB_USER{ utils::LoadSecret("token.txt", "DB_USER") };
    const std::string DB_PASS{ utils::LoadSecret("token.txt", "DB_PASS") };
    const std::string DB_CLUSTER{ utils::LoadSecret("token.txt", "DB_CLUSTER") };

    // Create an instance and establish our MongoDB connection
    MongoDatabase database;
    database.SetLogger(log);
    database.connect(fmt::format("mongodb+srv://{}:{}@{}", DB_USER, DB_PASS, DB_CLUSTER));
    std::shared_ptr<MongoJobRepo> jobRepo = std::make_shared<MongoJobRepo>(database.GetDB());
    jobRepo->SetLogger(log);

    std::uint8_t counter{ 0 };
    while (true)
    {
        /* Set cache policy for D++ library
         * --------------------------------
         * User caching:     cp_aggressive
         * Emoji caching:    none
         * Role caching:     cp_aggressive
         * Channel caching:  cp_aggressive
         * Guild caching:    cp_aggressive
         */
        dpp::cache_policy_t cp = { dpp::cp_aggressive, dpp::cp_none, dpp::cp_aggressive, dpp::cp_aggressive, dpp::cp_aggressive };

        // Reinitialize our bot to reestablish connection to discord
        dpp::cluster bot{ BOT_TOKEN };
        bot.intents = dpp::intents::i_message_content | dpp::intents::i_default_intents | dpp::i_guild_members;
        bot.cache_policy = cp;

        if (log) {
            log->info("{}",
                fmt::format(
                    "Initializing bot with the following cache policy:\n"
                    "User caching:     {}\n"
                    "Emoji caching:    {}\n"
                    "Role caching:     {}\n"
                    "Channel caching:  {}\n"
                    "Guild caching:    {}\n"
                    "Intents:          {}",
                    CacheString(cp.user_policy),
                    CacheString(cp.emoji_policy),
                    CacheString(cp.role_policy),
                    CacheString(cp.channel_policy),
                    CacheString(cp.guild_policy),
                    IntentsString(bot.intents)
                )
            );
        }

        if (!(&bot))
        {
            log->critical(fmt::format("Could not initialize the bot cluster. {} attempts remaining.", (5 - (++counter))));

            if (counter > 5) { return 1; }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

        ToasterBot toaster(bot, 0, jobRepo, false);
        toaster.SetLogger(log);

        // Register our custom event handlers
        bot.on_message_create([&toaster](const dpp::message_create_t& event) { toaster.onMessage(event); });
        bot.on_slashcommand([&toaster](const dpp::slashcommand_t& event) { toaster.onSlashCommand(event); });
        bot.on_interaction_create([&toaster](const dpp::interaction_create_t& event) { toaster.onInteractionCreate(event); });
        bot.on_button_click([&toaster](const dpp::button_click_t& event) { toaster.onButtonClick(event); });
        bot.on_form_submit([&toaster](const dpp::form_submit_t& event) { toaster.onFormSubmit(event); });
        bot.on_ready([&toaster](const dpp::ready_t& event) { toaster.onReady(event); });

        if (log)
            log->info("{}", "Registered D++ callbacks for slash commands.");

        try
        {
            bot.start(dpp::st_wait);
        }
        catch (const std::exception& e)
        {
            bot.log(dpp::ll_error, fmt::format("Exiting unexpectedly and attempting to reconnect. Error: {}", e.what()));
        }

        // Reconnection delay to prevent hammering discord
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::for_each(Toaster::BotCommands.begin(), Toaster::BotCommands.end(), [](ICustomCommand* cmd) { delete cmd; });
    database.disconnect();

    return 0;
}

std::string CacheString(dpp::cache_policy_setting_t cache) {
    switch (cache)
    {
    case dpp::cp_none: return "none";
    case dpp::cp_aggressive: return "cp_aggressive";
    case dpp::cp_lazy: return "cp_lazy";
    default: return "unknown";
    }
}

std::string IntentsString(uint64_t intents) {
    fmt::memory_buffer buf;
    bool first = true;

    auto append = [&](std::string_view name) {
        if (!first) {
            fmt::format_to(std::back_inserter(buf), " | ");
        }
        fmt::format_to(std::back_inserter(buf), "{}", name);
        first = false;
        };

    if (intents & dpp::intents::i_message_content)
        append("message_content");

    if (intents & dpp::intents::i_default_intents)
        append("default_intents");

    if (intents & dpp::i_guild_members)
        append("guild_members");

    // Optional extras
    if (intents & dpp::i_guilds)
        append("guilds");

    if (intents & dpp::i_guild_messages)
        append("guild_messages");

    if (intents & dpp::i_direct_messages)
        append("direct_messages");

    if (first) {
        return "none";
    }

    return fmt::to_string(buf);
}