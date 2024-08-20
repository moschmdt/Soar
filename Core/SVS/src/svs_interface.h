#ifndef SVS_INTERFACE_H
#define SVS_INTERFACE_H

#include <string>

class svs_interface {
 public:
  virtual ~svs_interface() {}
  virtual void state_creation_callback(Symbol* goal) = 0;
  virtual void state_deletion_callback(Symbol* goal) = 0;
  virtual void output_callback() = 0;
  virtual void input_callback() = 0;
  virtual void add_input(const std::string& in) = 0;
  virtual std::string get_output() const = 0;
  virtual std::string svs_query(const std::string& in) = 0;
  virtual bool do_cli_command(const std::vector<std::string>& args,
                              std::string& output) = 0;
  virtual bool is_enabled() = 0;
  virtual void set_enabled(bool newSetting) = 0;
  virtual bool is_enabled_in_substates() = 0;
  virtual void set_enabled_in_substates(bool newSetting) = 0;
  /**
   * Indicates that the current top of the state stack is for a subgoal/substate
   */
  virtual bool is_in_substate() = 0;
};

svs_interface* make_svs(agent* a);

#endif
