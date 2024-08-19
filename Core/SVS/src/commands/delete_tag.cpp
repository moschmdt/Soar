/**********************************************************
 *
 * File: commands/delete_tag
 * Contains:
 *  class delete_tag
 *
 *  Soar Command to delete a tag on a node
 *  Parameters:
 *     ^id <string> - id of the node to delete the tag on
 *     ^tag_name <string> - name of the tag to delete
 *
 **********************************************************/
#include <iostream>
#include <string>

#include "command.h"
#include "command_table.h"
#include "filter.h"
#include "scene.h"
#include "svs.h"
#include "symbol.h"

class delete_tag_command : public command {
 public:
  delete_tag_command(svs_state* state, Symbol* root)
      : command(state, root), root(root), first(true) {
    si = state->get_svs()->get_soar_interface();
    scn = state->get_scene();
  }

  ~delete_tag_command() {}

  std::string description() { return std::string("delete_tag"); }

  bool update_sub() {
    if (first) {
      first = false;
      if (!parse()) {
        return false;
      }
    } else {
      return true;
    }

    sgnode* n = scn->get_node(id);
    if (!n) {
      set_status(std::string("Couldn't find node ") + id);
      return false;
    }

    n->delete_tag(tag_name);
    set_status("success");

    return true;
  }

  int command_type() { return SVS_WRITE_COMMAND; }

  bool parse() {
    wme *idwme, *tagwme;

    // id - the id of the node to delete the tag from
    if (!si->find_child_wme(root, "id", idwme)) {
      set_status("no object id specified");
      return false;
    }
    if (!get_symbol_value(si->get_wme_val(idwme), id)) {
      set_status("object id must be a std::string");
      return false;
    }

    // tag_name - the name of the tag to delete
    if (!si->find_child_wme(root, "tag_name", tagwme)) {
      set_status("no tag_name specified");
      return false;
    }
    if (!get_symbol_value(si->get_wme_val(tagwme), tag_name)) {
      set_status("tag_name must be a std::string");
      return false;
    }

    return true;
  }

 private:
  Symbol* root;
  scene* scn;
  soar_interface* si;
  bool first;
  std::string id;
  std::string tag_name;
};

command* _make_delete_tag_command_(svs_state* state, Symbol* root) {
  return new delete_tag_command(state, root);
}

command_table_entry* delete_tag_command_entry() {
  command_table_entry* e = new command_table_entry();
  e->name = "delete_tag";
  e->description = "Deletes a tag from a node";
  e->parameters["id"] = "Id of the node";
  e->parameters["tag_name"] = "Name of the tag to delete";
  e->create = &_make_delete_tag_command_;
  return e;
}
