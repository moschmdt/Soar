/*
 * cli_explain.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mazzin
 */

#ifndef CLI_EXPLAIN_H_
#define CLI_EXPLAIN_H_

#include "cli_Options.h"
#include "cli_Parser.h"

#include "misc.h"
#include "sml_Events.h"

namespace cli {

class ExplainCommand : public cli::ParserCommand {
public:
  ExplainCommand(cli::CommandLineInterface &cli) : ParserCommand(), cli(cli) {}
  virtual ~ExplainCommand() {}
  virtual const char *GetString() const { return "explain"; }
  virtual const char *GetSyntax() const {
    return "Use 'explain ?' or 'help explain' to learn more about the explain "
           "command.";
  }

  virtual bool Parse(std::vector<std::string> &argv) {
    cli::Options opt;
    OptionsData optionsData[] = {{'j', "json", OPTARG_NONE},
                                 {'a', "all", OPTARG_NONE},
                                 {0, 0, OPTARG_NONE}};

    bool json_flag = false;
    bool all_flag = false;

    for (;;) {
      if (!opt.ProcessOptions(argv, optionsData)) {
        cli.SetError(opt.GetError().c_str());
        return cli.AppendError(GetSyntax());
      }

      if (opt.GetOption() == -1) {
        break;
      }

      switch (opt.GetOption()) {
      case 'j':
        json_flag = true;
        break;
      case 'a':
        all_flag = true;
        break;
      }
    }

    std::string arg, arg2;
    size_t start_arg_position = opt.GetArgument() - opt.GetNonOptionArguments();
    size_t num_args = argv.size() - start_arg_position;
    if (num_args > 0) {
      arg = argv[start_arg_position];
    }
    if (num_args > 1) {
      arg2 = argv[start_arg_position + 1];
    }
    if (num_args > 2) {
      return cli.SetError("Too many arguments for the 'explain' command.");
    }

    if (all_flag) {
      if (num_args == 0 || arg != "track-operator") {
        return cli.SetError(
            "Option -a|--all is only valid with 'explain track-operator'.");
      }
      if (num_args > 1) {
        return cli.SetError(
            "Use either an operator name or -a|--all, not both.");
      }
      arg2 = "all";
      num_args = 2;
    }

    if (num_args == 1) {
      return cli.DoExplain(&arg, nullptr, json_flag);
    }
    if (num_args == 2) {
      return cli.DoExplain(&arg, &arg2, json_flag);
    }

    // case: nothing = full configuration information
    return cli.DoExplain();
  }

private:
  cli::CommandLineInterface &cli;

  ExplainCommand &operator=(const ExplainCommand &);
};
} // namespace cli

#endif /* CLI_EXPLAIN_H_ */
