#include <cstddef>
#include "Hook.h"

#include "Events/OFootBlockEventListener.h"
#include "Events/EquipEventSink.h"
#include "Events/InputEventSink.h"
#include "Events/CellWatcher.h"
#include "Utils/BoneUtils.h"

using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    /**
     * Setup logging.
     *
     * <p>
     * Logging is important to track issues. CommonLibSSE bundles functionality for spdlog, a common C++ logging
     * framework. Here we initialize it, using values from the configuration file. This includes support for a debug
     * logger that shows output in your IDE when it has a debugger attached to Skyrim, as well as a file logger which
     * writes data to the standard SKSE logging directory at <code>Documents/My Games/Skyrim Special Edition/SKSE</code>
     * (or <code>Skyrim VR</code> if you are using VR).
     * </p>
     */
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }

    ///**
    // * Initialize the SKSE cosave system for our plugin.
    // *
    // * <p>
    // * SKSE comes with a feature called a <em>cosave</em>, an additional save file kept alongside the original Skyrim
    // * save file. SKSE plugins can write their own data to this file, and load it again when the save game is loaded,
    // * allowing them to keep custom data along with a player's save. Each plugin must have a unique ID, which is four
    // * characters long (similar to the record names used by forms in ESP files). Note however this is little-endian,
    // so
    // * technically the 'SMPL' here ends up as 'LPMS' in the save file, unless we use a byte order swap.
    // * </p>
    // *
    // * <p>
    // * There can only be one serialization callback for save, revert (called on new game and before a load), and load
    // * for the entire plugin.
    // * </p>
    // */
    void InitializeSerialization() {
        log::trace("Initializing cosave serialization...");
        auto* serde = GetSerializationInterface();
        serde->SetUniqueID(_byteswap_ulong('OFB'));
        log::trace("Cosave serialization initialized.");
    }

    ///**
    // * Register to listen for messages.
    // *
    // * <p>
    // * SKSE has a messaging system to allow for loosely coupled messaging. This means you don't need to know about or
    // * link with a message sender to receive their messages. SKSE itself will send messages for common Skyrim
    // lifecycle
    // * events, such as when SKSE plugins are done loading, or when all ESP plugins are loaded.
    // * </p>
    // *
    // * <p>
    // * Here we register a listener for SKSE itself (because we have not specified a message source). Plugins can send
    // * their own messages that other plugins can listen for as well, although that is not demonstrated in this example
    // * and is not common.
    // * </p>
    // *
    // * <p>
    // * The data included in the message is provided as only a void pointer. It's type depends entirely on the type of
    // * message, and some messages have no data (<code>dataLen</code> will be zero).
    // * </p>
    // */
    void InitializeMessaging() {
        if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
                switch (message->type) {
                    // Skyrim lifecycle events.
                    // Called after all plugins have finished running
                    // SKSEPlugin_Load. It is now safe to do multithreaded
                    // operations, or operations against other plugins.
                    case MessagingInterface::kPostLoad:
                    // Called after all kPostLoad message handlers have run.
                    case MessagingInterface::kPostPostLoad:
                    // Called when all game data has been found.
                    case MessagingInterface::kInputLoaded: {
                        auto& input = Events::InputEventSink::GetSingleton();
                        input.SetToggleKeys(42, 64);
                        input.Register();
                        break;
                    }
                    // All ESM/ESL/ESP plugins have loaded, main menu is now active.
                    // It is now safe to access form data.
                    case MessagingInterface::kDataLoaded: {
                        Hook::CharacterHook::Install();
                        auto ofbe = Events::OFootBlockEventListener::GetSingleton();
                        ofbe->Register();

                        auto ees = Events::EquipListener::GetSingleton();
                        ees->Register();

                        auto watcher = Events::MenuWatcher::GetSingleton();
                        watcher->Register();

                        auto cellwatcher = Events::CellWatcher::GetSingleton();
                        cellwatcher->Register();
                        break;
                    }


                    // Skyrim game events.
                    case MessagingInterface::kNewGame:      // Player starts a new game from main menu.
                    case MessagingInterface::kPreLoadGame:  // Player selected a game to load, but it hasn't loaded yet.
                                                            // Data will be the name of the loaded save.
                    {
                        auto cachemgr = Utils::BoneUtils::BoneCache::GetSingleton();
                        cachemgr->Clear();
                        break;
                    }
                    case MessagingInterface::kPostLoadGame:  // Player's selected save game has finished loading.
                                                             // Data will be a boolean indicating whether the load was
                                                             // successful.
                    case MessagingInterface::kSaveGame:      // The player has saved a game.
                                                             // Data will be the save name.
                    case MessagingInterface::kDeleteGame:  // The player deleted a saved game from within the load menu.
                        break;
                }
            })) {
            stl::report_and_fail("Unable to register message listener.");
        }
    }
}  // namespace
/**
 * This if the main callback for initializing your SKSE plugin, called just before Skyrim runs its main function.
 *
 * <p>
 * This is your main entry point to your plugin, where you should initialize everything you need. Many things can't be
 * done yet here, since Skyrim has not initialized and the Windows loader lock is not released (so don't do any
 * multithreading). But you can register to listen for messages for later stages of Skyrim startup to perform such
 * tasks.
 * </p>
 */
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion(REL::Version("1.0.0.0"sv));
    v.PluginName("OFootBlock");
    v.AuthorName("Shyuecc");
    v.UsesAddressLibrary();
    v.UsesNoStructs();

    return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info) {
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "OFootBlock";
    a_info->version = 0x01000000;

    return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);
    InitializeMessaging();
    InitializeSerialization();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
