/***************************************************
 *
 * File: filters/volume.cpp
 *
 * Volume Filters
 *  double evaluate_volume(sgnode* a, fp* p)
 *    Returns the volume of node a
 *  bool larger_test(sgnode* a, sgnode* b, fp* p)
 *    Returns true if vol(a) > vol(b)
 *  bool smaller_test(sgnode* a, sgnode* b, fp* p)
 *    Returns true if vol(a) < vol(b)
 *
 * Volume Type
 *   bbox - the volume of the node's axis-aligned bounding box
 *   scale - the product of the dimensions in the scale transform
 *     (If the geometry is a unit size, this works well)
 *
 * Filter volume : node_evaluation_filter
 *   Parameters:
 *    sgnode a
 *    volume_type << bbox scale >> [optional - default is bbox]
 *   Returns:
 *    double - volume of a
 *
 * Filter volume_select : node_evaluation_select_filter
 *   Parameters:
 *    sgnode a
 *    double min [optional - default = -INF]
 *    double max [optional - default = +INF]
 *    volume_type << bbox scale >> [optional - default is bbox]
 *   Returns:
 *    sgnode a if min <= vol(a) <= max
 *
 *  Filter largest : node_evaluation_rank_filter
 *    Parameters:
 *      set<sgnode> a
 *    	volume_type << bbox scale >> [optional - default is bbox]
 *    Returns:
 *      sgnode - the node in input set a with the largest volume
 *
 *  Filter smallest : node_evaluation_rank_filter
 *    Parameters:
 *      set<sgnode> a
 *    	volume_type << bbox scale >> [optional - default is bbox]
 *    Returns:
 *      sgnode - the node in input set a with the smallest volume
 *
 *  Filter larger : node_test_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    	volume_type << bbox scale >> [optional - default is bbox]
 *    Returns
 *      bool - true if vol(a) > vol(b)
 *
 *  Filter smaller : node_test_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    	volume_type << bbox scale >> [optional - default is bbox]
 *    Returns:
 *      bool - True if vol(a) < vol(b)
 *
 *  Filter larger_select : node_test_select_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    	volume_type << bbox scale >> [optional - default is bbox]
 *    Returns:
 *      sgnode b if vol(a) > vol(b)
 *
 *  Filter smaller_select : node_test_select_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    	volume_type << bbox scale >> [optional - default is bbox]
 *    Returns:
 *      sgnode b if vol(a) < vol(b)
 *
 *
 *********************************************************/
#include <string>

#include "filter_table.h"
#include "filters/base_node_filters.h"
#include "scene.h"
#include "sgnode_algs.h"

double evaluate_volume(sgnode* a, const filter_params* p) {
  std::string vol_type = "bbox";
  get_filter_param(0, p, "volume_type", vol_type);
  if (vol_type == "scale") {
    return scale_volume(a);
  } else {
    return bbox_volume(a);
  }
}

bool larger_test(sgnode* a, sgnode* b, const filter_params* p) {
  if (a == b) {
    return false;
  }
  std::string vol_type = "bbox";
  get_filter_param(0, p, "volume_type", vol_type);
  if (vol_type == "scale") {
    return scale_volume(a) > scale_volume(b);
  } else {
    return bbox_volume(a) > bbox_volume(b);
  }
}

bool smaller_test(sgnode* a, sgnode* b, const filter_params* p) {
  if (a == b) {
    return false;
  }
  std::string vol_type = "bbox";
  get_filter_param(0, p, "volume_type", vol_type);
  if (vol_type == "scale") {
    return scale_volume(a) < scale_volume(b);
  } else {
    return bbox_volume(a) < bbox_volume(b);
  }
}

///// filter volume //////
filter* make_volume_filter(Symbol* root, soar_interface* si, scene* scn,
                           filter_input* input) {
  return new node_evaluation_filter(root, si, input, &evaluate_volume);
}

filter_table_entry* volume_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "volume";
  e->description = "Returns volume of each node a";
  e->parameters["a"] = "Sgnode a";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_volume_filter;
  return e;
}

///// filter volume_select //////
filter* make_volume_select_filter(Symbol* root, soar_interface* si, scene* scn,
                                  filter_input* input) {
  return new node_evaluation_select_filter(root, si, input, &evaluate_volume);
}

filter_table_entry* volume_select_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "volume_select";
  e->description = "Select a if min <= volume(a) <= max";
  e->parameters["a"] = "Sgnode a";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->parameters["min"] = "minimum volume to select";
  e->parameters["max"] = "maximum volume to select";
  e->create = &make_volume_select_filter;
  return e;
}

////// filter smallest //////
filter* make_smallest_filter(Symbol* root, soar_interface* si, scene* scn,
                             filter_input* input) {
  node_evaluation_rank_filter* f =
      new node_evaluation_rank_filter(root, si, input, &evaluate_volume);
  f->set_select_highest(false);
  return f;
}

filter_table_entry* smallest_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "smallest";
  e->description = "Select node a with the smallest volume";
  e->parameters["a"] = "Sgnode a";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_smallest_filter;
  return e;
}

////// filter largest  //////
filter* make_largest_filter(Symbol* root, soar_interface* si, scene* scn,
                            filter_input* input) {
  return new node_evaluation_rank_filter(root, si, input, &evaluate_volume);
}

filter_table_entry* largest_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "largest";
  e->description = "Select node a with the largest volume";
  e->parameters["a"] = "Sgnode a";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_largest_filter;
  return e;
}

////// filter larger //////
filter* make_larger_filter(Symbol* root, soar_interface* si, scene* scn,
                           filter_input* input) {
  return new node_test_filter(root, si, input, &larger_test);
}

filter_table_entry* larger_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "larger";
  e->description = "Returns true if volume(a) > volume(b)";
  e->parameters["a"] = "Sgnode a";
  e->parameters["b"] = "Sgnode b";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_larger_filter;
  return e;
}

////// filter smaller //////
filter* make_smaller_filter(Symbol* root, soar_interface* si, scene* scn,
                            filter_input* input) {
  return new node_test_filter(root, si, input, &smaller_test);
}

filter_table_entry* smaller_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "smaller";
  e->description = "Returns true if volume(a) < volume(b)";
  e->parameters["a"] = "Sgnode a";
  e->parameters["b"] = "Sgnode b";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_smaller_filter;
  return e;
}

////// filter larger_select //////
filter* make_larger_select_filter(Symbol* root, soar_interface* si, scene* scn,
                                  filter_input* input) {
  return new node_test_select_filter(root, si, input, &larger_test);
}

filter_table_entry* larger_select_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "larger_select";
  e->description = "Select b if volume(a) > volume(b)";
  e->parameters["a"] = "Sgnode a";
  e->parameters["b"] = "Sgnode b";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_larger_select_filter;
  return e;
}

////// filter smaller_select //////
filter* make_smaller_select_filter(Symbol* root, soar_interface* si, scene* scn,
                                   filter_input* input) {
  return new node_test_select_filter(root, si, input, &smaller_test);
}

filter_table_entry* smaller_select_filter_entry() {
  filter_table_entry* e = new filter_table_entry();
  e->name = "smaller_select";
  e->description = "Select b if volume(a) < volume(b)";
  e->parameters["a"] = "Sgnode a";
  e->parameters["b"] = "Sgnode b";
  e->parameters["volume_type"] = "Either bbox or scale";
  e->create = &make_smaller_select_filter;
  return e;
}
