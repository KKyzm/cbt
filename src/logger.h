//
// Logger with console and file output.
// the console will show only warnings or worse, while the file will log all messages.
//

// code from
//   https://github.com/gabime/spdlog/wiki/1.-QuickStart#create-a-logger-with-multiple-sinks-each-sink-with-its-own-formatting-and-log-level
#include <cstdlib>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

inline void init_logger(const std::string& path = "logs/cbt.log") {
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[%M:%S %z] [%^%L%$] [thread %t] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
    file_sink->set_level(spdlog::level::trace);

    // set multi_sink logger as default logger
    auto default_logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list({console_sink, file_sink}));
    default_logger->flush_on(spdlog::level::trace);
    spdlog::set_default_logger(default_logger);

  } catch (const spdlog::spdlog_ex& ex) {
    using namespace std::string_literals;
    spdlog::error("Log initialization failed: "s + ex.what());
    exit(EXIT_FAILURE);
  }
}
