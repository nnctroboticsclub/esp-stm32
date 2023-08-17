#pragma once

#include <argtable3/argtable3.h>
#include <functional>
#include <wifi/authmode_kind.hpp>
#include <iostream>

#include <esp_console.h>

namespace cmd {

template <typename T, typename Class>
concept CommandArgs = requires(T a) {
  { a.end } -> std::convertible_to<struct arg_end*>;
  { a.obj } -> std::convertible_to<Class*>;
};

template <typename T>
concept Command = requires(T a) {
  { T::TAG } -> std::convertible_to<const char*>;
  { T::kCommandName } -> std::convertible_to<const char*>;
  { T::kHelp } -> std::convertible_to<const char*>;
  { T::args } -> CommandArgs<T>;

  { T::CmdLineHandler(0, (char**)nullptr) } -> std::same_as<int>;

  { a.InitArgs() } -> std::same_as<void>;
  { a.Handler() } -> std::same_as<int>;
};

template <Command Cmd>
void RegisterCommand(Cmd& command) {
  command.InitArgs();
  command.args.obj = &command;
  esp_console_cmd_t cmd_conf = {.command = Cmd::kCommandName,
                                .help = Cmd::kHelp,
                                .hint = NULL,
                                .func = &Cmd::CmdLineHandler,
                                .argtable = &Cmd::args};
  esp_console_cmd_register(&cmd_conf);
}

}  // namespace cmd