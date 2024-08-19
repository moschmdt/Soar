#include "soar_interface.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <utility>

#include "decide.h"
#include "output_manager.h"
#include "portability.h"
#include "slot.h"
#include "soar_module.h"
#include "symbol.h"
#include "working_memory.h"

tc_number get_new_tc_number(agent* thisAgent);

common_syms::common_syms(soar_interface* si) : si(si) {
  svs = si->make_sym("svs");
  cmd = si->make_sym("command");
  scene = si->make_sym("spatial-scene");
  child = si->make_sym("child");
  result = si->make_sym("result");
  id = si->make_sym("id");
  status = si->make_sym("status");
}

common_syms::~common_syms() {
  si->del_sym(svs);
  si->del_sym(cmd);
  si->del_sym(scene);
  si->del_sym(child);
  si->del_sym(result);
  si->del_sym(id);
  si->del_sym(status);
}

soar_interface::soar_interface(agent* a) : thisAgent(a), cs(this) {}

soar_interface::~soar_interface() {}

void soar_interface::del_sym(Symbol* s) {
  thisAgent->symbolManager->symbol_remove_ref(&s);
}

wme* soar_interface::make_id_wme(Symbol* id, const std::string& attr) {
  Symbol* attrsym = thisAgent->symbolManager->make_str_constant(attr.c_str());
  Symbol* valsym =
      thisAgent->symbolManager->make_new_identifier(attr[0], id->id->level);
  wme* w = soar_module::add_module_wme(thisAgent, id, attrsym, valsym);
  thisAgent->symbolManager->symbol_remove_ref(&attrsym);
  thisAgent->symbolManager->symbol_remove_ref(&valsym);
  return w;
}

wme* soar_interface::make_id_wme(Symbol* id, Symbol* attr) {
  char n;
  Symbol* val;

  if (attr->symbol_type != STR_CONSTANT_SYMBOL_TYPE ||
      strlen(attr->sc->name) == 0) {
    n = 'a';
  } else {
    n = attr->sc->name[0];
  }

  val = thisAgent->symbolManager->make_new_identifier(n, id->id->level);
  wme* w = soar_module::add_module_wme(thisAgent, id, attr, val);
  thisAgent->symbolManager->symbol_remove_ref(&val);
  return w;
}

wme* soar_interface::make_svs_wme(Symbol* id) {
  Symbol* val;

  val = thisAgent->symbolManager->make_new_identifier('v', id->id->level);
  wme* w = soar_module::add_module_wme(thisAgent, id, cs.svs, val);
  thisAgent->symbolManager->symbol_remove_ref(&val);
  return w;
}

void soar_interface::remove_wme(wme* w) {
  soar_module::remove_module_wme(thisAgent, w);
}

bool soar_interface::get_child_wmes(Symbol* id, wme_vector& childs) {
  slot* s;
  wme* w;

  if (!id->is_sti()) {
    return false;
  }

  childs.clear();
  for (s = id->id->slots; s != NULL; s = s->next) {
    for (w = s->wmes; w != NULL; w = w->next) {
      childs.push_back(w);
    }
  }

  return true;
}

#include <iostream>

#include "symbol.h"

bool soar_interface::get_vec3(Symbol* id, const std::string& attr, vec3& val) {
  vec3 res;

  // First find the vec3 wme
  wme* vec3_wme;
  if (!find_child_wme(id, attr.c_str(), vec3_wme)) {
    return false;
  }
  Symbol* vec3_root = get_wme_val(vec3_wme);

  std::string vec_id_name;
  vec3_root->get_id_name(vec_id_name);

  // Then find each dimension to make up the vec3
  std::string dims[] = {"x", "y", "z"};
  for (int d = 0; d < 3; d++) {
    wme* dim_wme;
    double dim_val;
    if (!find_child_wme(vec3_root, dims[d].c_str(), dim_wme)) {
      return false;
    }
    if (!get_symbol_value(get_wme_val(dim_wme), dim_val)) {
      return false;
    }
    res[d] = dim_val;
  }
  val = res;
  return true;
}

bool soar_interface::find_child_wme(Symbol* id, const std::string& attr,
                                    wme*& w) {
  slot* s;
  wme* w1;
  std::string a;

  if (!id->is_sti()) {
    return false;
  }

  for (s = id->id->slots; s != NULL; s = s->next) {
    for (w1 = s->wmes; w1 != NULL; w1 = w1->next) {
      if (get_symbol_value(get_wme_attr(w1), a) && a == attr) {
        w = w1;
        return true;
      }
    }
  }

  return false;
}

wme* soar_interface::make_wme(Symbol* id, Symbol* attr, Symbol* val) {
  wme* w = soar_module::add_module_wme(thisAgent, id, attr, val);
  thisAgent->symbolManager->symbol_remove_ref(&val);
  return w;
}

wme* soar_interface::make_wme(Symbol* id, const std::string& attr,
                              Symbol* val) {
  wme* w;
  Symbol* attrsym = make_sym(attr);
  w = make_wme(id, attrsym, val);
  thisAgent->symbolManager->symbol_remove_ref(&attrsym);

  return w;
}

void soar_interface::print(const std::string& msg) {
  thisAgent->outputManager->printa(thisAgent, msg.c_str());
}
