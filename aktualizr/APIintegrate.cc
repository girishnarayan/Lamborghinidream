#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/signals2.hpp>

#include "libaktualizr/aktualizr.h"
#include "libaktualizr/config.h"

#include "logging/logging.h"
#include "utilities/utils.h"







int main(int argc, char *argv[]) {
  logger_init();
  logger_set_threshold(boost::log::trivial::info);
  LOG_INFO << "demo-app starting";

  try {
    bpo::variables_map commandline_map = parse_options(argc, argv);
    Config config(commandline_map);

    Aktualizr aktualizr(config);

    auto f_cb = [](const std::shared_ptr<event::BaseEvent> event) { process_event(event); };
    boost::signals2::scoped_connection conn(aktualizr.SetSignalHandler(f_cb));

    if (!config.uptane.secondary_config_file.empty()) {
      try {
        initSecondaries(&aktualizr, config.uptane.secondary_config_file);
      } catch (const std::exception &e) {
        LOG_ERROR << "Failed to init Secondaries: " << e.what();
        LOG_ERROR << "Exiting...";
        return EXIT_FAILURE;
      }
    }

    aktualizr.Initialize();

    const char *cmd_list = "Available commands: SendDeviceData, CheckUpdates, Download, Install, CampaignCheck, CampaignAccept, Pause, Resume, Abort";
    std::cout << cmd_list << std::endl;

    std::vector<Uptane::Target> current_updates;
    std::string buffer;
    while (std::getline(std::cin, buffer)) {
      std::vector<std::string> words;
      boost::algorithm::split(words, buffer, boost::is_any_of("\t "), boost::token_compress_on);
      std::string &command = words.at(0);
      boost::algorithm::to_lower(command);
      if (command == "senddevicedata") {
        aktualizr.SendDeviceData().get();
      } else if (command == "checkupdates") {
        auto result = aktualizr.CheckUpdates().get();
        current_updates = result.updates;
      } else if (command == "download") {
        aktualizr.Download(current_updates).get();
      } else if (command == "install") {
        aktualizr.Install(current_updates).get();
        current_updates.clear();
      } else if (command == "campaigncheck") {
        aktualizr.CampaignCheck().get();
      } else if (command == "campaignaccept") {
        if (words.size() == 2) {
          aktualizr.CampaignControl(words.at(1), campaign::Cmd::Accept).get();
        } else {
          std::cout << "Error. Specify the campaign ID" << std::endl;
        }
      } else if (command == "pause") {
        aktualizr.Pause();
      } else if (command == "resume") {
        aktualizr.Resume();
      } else if (command == "abort") {
        aktualizr.Abort();
      } else if (!command.empty()) {
        std::cout << "Unknown command.\n";
        std::cout << cmd_list << std::endl;
      }
    }
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    LOG_ERROR << "Fatal error in demo-app: " << ex.what();
    return EXIT_FAILURE;
  }
}