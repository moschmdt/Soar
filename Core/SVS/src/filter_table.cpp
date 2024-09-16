#include "filter_table.h"

#include <iomanip>
#include <iostream>

#include "filter.h"
#include "scene.h"
#include "symbol.h"

filter_table& get_filter_table() {
  static filter_table inst;
  return inst;
}

// filters/node.cpp
filter_table_entry* node_filter_entry();
filter_table_entry* all_nodes_filter_entry();
filter_table_entry* remove_node_filter_entry();
filter_table_entry* node_position_filter_entry();
filter_table_entry* node_rotation_filter_entry();
filter_table_entry* node_scale_filter_entry();
filter_table_entry* node_bbox_filter_entry();
filter_table_entry* combine_nodes_filter_entry();

// filters/distance.cpp
filter_table_entry* distance_filter_entry();
filter_table_entry* distance_select_filter_entry();
filter_table_entry* closest_filter_entry();
filter_table_entry* farthest_filter_entry();

// filters/volume.cpp
filter_table_entry* volume_filter_entry();
filter_table_entry* volume_select_filter_entry();
filter_table_entry* largest_filter_entry();
filter_table_entry* smallest_filter_entry();
filter_table_entry* larger_filter_entry();
filter_table_entry* smaller_filter_entry();
filter_table_entry* larger_select_filter_entry();
filter_table_entry* smaller_select_filter_entry();

// filters/axis_distance.cpp
filter_table_entry* axis_distance_filter_entry();
filter_table_entry* axis_distance_select_filter_entry();

// filters/axis_relation.cpp
filter_table_entry* axis_relation_select_filter_entry();

// filters/calculate_placement.cpp
filter_table_entry* calculate_placement_filter_entry();

// filters/intersect.cpp
filter_table_entry* intersect_filter_entry();
filter_table_entry* intersect_select_filter_entry();

// filters/contain.cpp
filter_table_entry* contain_filter_entry();
filter_table_entry* contain_select_filter_entry();

// filters/occlusion.cpp
filter_table_entry* occlusion_filter_entry();

// filters/overlap.cpp
filter_table_entry* overlap_filter_entry();
filter_table_entry* overlap_select_filter_entry();

// filters/monitor_object.cpp
filter_table_entry* monitor_position_filter_entry();
filter_table_entry* monitor_volume_filter_entry();

// filters/tag_select.cpp
filter_table_entry* tag_select_filter_entry();

filter_table::filter_table() {
  set_help("Prints out a list of all filter types.");

  add(node_filter_entry());
  add(all_nodes_filter_entry());
  add(remove_node_filter_entry());
  add(node_position_filter_entry());
  add(node_rotation_filter_entry());
  add(node_scale_filter_entry());
  add(node_bbox_filter_entry());
  add(combine_nodes_filter_entry());

  add(distance_filter_entry());
  add(distance_select_filter_entry());
  add(closest_filter_entry());
  add(farthest_filter_entry());

  add(volume_filter_entry());
  add(volume_select_filter_entry());
  add(largest_filter_entry());
  add(smallest_filter_entry());
  add(larger_filter_entry());
  add(smaller_filter_entry());
  add(larger_select_filter_entry());
  add(smaller_select_filter_entry());

  add(axis_distance_filter_entry());
  add(axis_distance_select_filter_entry());

  add(axis_relation_select_filter_entry());

  add(calculate_placement_filter_entry());

  add(intersect_filter_entry());
  add(intersect_select_filter_entry());

  add(contain_filter_entry());
  add(contain_select_filter_entry());

  add(occlusion_filter_entry());

  add(overlap_filter_entry());
  add(overlap_select_filter_entry());

  add(monitor_volume_filter_entry());
  add(monitor_position_filter_entry());

  add(tag_select_filter_entry());
}

void filter_table::proxy_get_children(std::map<std::string, cliproxy*>& c) {
  std::map<std::string, filter_table_entry*>::iterator i, iend;
  for (i = t.begin(), iend = t.end(); i != iend; ++i) {
    c[i->first] = i->second;
  }
}

void filter_table::proxy_use_sub(const std::vector<std::string>& args,
                                 std::ostream& os) {
  os << "====================== FILTER TABLE ======================="
     << std::endl;
  std::map<std::string, filter_table_entry*>::iterator i;
  for (i = t.begin(); i != t.end(); i++) {
    os << "  " << std::setw(22) << std::left << i->first << " | "
       << i->second->description << std::endl;
  }
  os << "==========================================================="
     << std::endl;
  os << "For specific filter info, use the command 'svs filters.filter_name'"
     << std::endl;
}

filter* filter_table::make_filter(const std::string& pred, Symbol* root,
                                  soar_interface* si, scene* scn,
                                  filter_input* input) const {
  std::map<std::string, filter_table_entry*>::const_iterator i = t.find(pred);
  if (i == t.end() || i->second->create == NULL) {
    return NULL;
  }
  return (*(i->second->create))(root, si, scn, input);
}

void filter_table::add(filter_table_entry* e) {
  assert(t.find(e->name) == t.end());
  t[e->name] = e;
}

filter_table_entry::filter_table_entry() : create(NULL), description("") {
  set_help("Reports information about this filter type.");
}

void filter_table_entry::proxy_use_sub(const std::vector<std::string>& args,
                                       std::ostream& os) {
  os << "Filter: " << name << std::endl;
  os << "  " << description << std::endl;
  os << "  Parameters:" << std::endl;
  std::map<std::string, std::string>::iterator i;
  for (i = parameters.begin(); i != parameters.end(); i++) {
    os << "    " << std::setw(15) << std::left << i->first << " | " << i->second
       << std::endl;
  }
}
/**
Example input:

(<ot> ^type on-top ^a <ota> ^b <otb>)
(<ota> ^type node ^name box1)
(<otb> ^type node ^name box2)
*/
filter* parse_filter_spec(soar_interface* si, Symbol* root, scene* scn) {
  wme_vector children, params;
  wme_vector::iterator i;
  std::string pname, ftype, itype;
  filter_input* input;
  bool fail;
  filter* f = NULL;

  if (!root->is_sti()) {
    std::string strval;
    long intval;
    double floatval;

    if (get_symbol_value(root, strval)) {
      return new const_filter<std::string>(strval);
    } else if (get_symbol_value(root, intval)) {
      return new const_filter<int>(static_cast<int>(intval));
    } else if (get_symbol_value(root, floatval)) {
      return new const_filter<double>(floatval);
    }
    return NULL;
  }

  fail = false;
  si->get_child_wmes(root, children);
  for (i = children.begin(); i != children.end(); ++i) {
    if (!get_symbol_value(si->get_wme_attr(*i), pname)) {
      continue;
    }
    Symbol* cval = si->get_wme_val(*i);
    if (pname == "type") {
      if (!get_symbol_value(cval, ftype)) {
        return NULL;
      }
    } else if (pname == "input-type") {
      if (!get_symbol_value(cval, itype)) {
        return NULL;
      }
    } else if (pname != "status" && pname != "result") {
      params.push_back(*i);
    }
  }

  // The combine type check is a bit of a hack
  if (itype == "concat" || ftype == "combine_nodes") {
    input = new concat_filter_input();
  } else if (params.size() == 0) {
    input = new null_filter_input();
  } else {
    input = new product_filter_input();
  }

  for (i = params.begin(); i != params.end(); ++i) {
    if (!get_symbol_value(si->get_wme_attr(*i), pname)) {
      continue;
    }
    Symbol* cval = si->get_wme_val(*i);
    filter* cf = parse_filter_spec(si, cval, scn);
    if (!cf) {
      fail = true;
      break;
    }
    input->add_param(pname, cf);
  }

  if (!fail) {
    if (ftype == "combine") {
      // f = new passthru_filter(root, si, input);
    } else {
      f = get_filter_table().make_filter(ftype, root, si, scn, input);
    }
  }

  if (fail || ftype == "" || f == NULL) {
    delete input;
    return NULL;
  }
  return f;
}
