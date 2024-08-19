#ifndef SGNODE_H
#define SGNODE_H

/* native scene graph implementation */

#include <list>
#include <string>
#include <vector>

#include "cliproxy.h"
#include "common.h"
#include "mat.h"

class sgnode_listener;
class group_node;
class geometry_node;

typedef std::map<std::string, std::string> tag_map;

class sgnode : public cliproxy {
  friend class group_node;

 public:
  enum change_type {
    CHILD_ADDED,
    DELETED,  // sent from destructor
    TRANSFORM_CHANGED,
    SHAPE_CHANGED,
    TAG_CHANGED,
    TAG_DELETED
  };

  sgnode(const std::string& id, bool group);
  virtual ~sgnode();

  /* copied node doesn't inherit listeners */
  virtual sgnode* clone() const;

  const std::string& get_id() const { return id; }
  void set_id(const std::string& new_id) { id = new_id; }

  group_node* get_parent() { return parent; }

  const group_node* get_parent() const { return parent; }

  bool is_group() const { return group; }

  group_node* as_group();
  const group_node* as_group() const;

  void set_trans(char type, const vec3& t);
  void set_trans(const vec3& p, const vec3& r, const vec3& s);
  vec3 get_trans(char type) const;
  void get_trans(vec3& p, vec3& r, vec3& s) const;
  const transform3& get_world_trans() const;

  void set_shape_dirty();
  void listen(sgnode_listener* o);
  void unlisten(sgnode_listener* o);
  void get_listeners(std::list<sgnode_listener*>& l) const { l = listeners; }

  const bbox& get_bounds() const;
  vec3 get_centroid() const;
  bool has_descendent(const sgnode* n) const;

  void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);

  virtual void get_shape_sgel(std::string& s) const = 0;
  virtual void walk(std::vector<sgnode*>& result) = 0;

  virtual void walk_geoms(std::vector<geometry_node*>& g) = 0;
  virtual void walk_geoms(std::vector<const geometry_node*>& g) const = 0;

  // Returns the maximum and minimum values of the node projected on the given
  // axis
  //   NOTE!: Assumes the given axis is a unit vector
  virtual double max_project_on_axis(const vec3& axis) const = 0;
  virtual double min_project_on_axis(const vec3& axis) const = 0;

  // Accessors/Mutators for tags
  const tag_map& get_all_tags() const;
  bool get_tag(const std::string& tag_name, std::string& tag_value) const;
  void set_tag(const std::string& tag_name, const std::string& tag_value);
  void delete_tag(const std::string& tag_name);

 protected:
  void set_bounds(const bbox& b);
  virtual void update_shape() = 0;
  virtual sgnode* clone_sub() const = 0;
  virtual void set_transform_dirty_sub() {}

 private:
  void set_transform_dirty();
  void update_transform() const;
  void send_update(change_type t, const std::string& update_info);
  void send_update(change_type t) {
    std::string s = "";
    send_update(t, s);
  }

  std::string id;
  group_node* parent;
  bool group;
  vec3 pos;
  vec3 rot;
  vec3 scale;
  vec3 centroid;

  mutable bool shape_dirty;

  mutable bbox bounds;
  mutable bool bounds_dirty;

  mutable transform3 wtransform;
  mutable transform3 ltransform;
  mutable bool trans_dirty;

  std::list<sgnode_listener*> listeners;

  tag_map tags;
};

class group_node : public sgnode {
 public:
  group_node(const std::string& id) : sgnode(id, true) {}
  ~group_node();

  sgnode* get_child(size_t i);
  const sgnode* get_child(size_t i) const;
  bool attach_child(sgnode* c);
  void detach_child(sgnode* c);
  void walk(std::vector<sgnode*>& result);

  size_t num_children() const { return children.size(); }

  // group nodes have no shape
  void get_shape_sgel(std::string& s) const {}

  void walk_geoms(std::vector<geometry_node*>& g);
  void walk_geoms(std::vector<const geometry_node*>& g) const;

  void proxy_get_children(std::map<std::string, cliproxy*>& c);

  double max_project_on_axis(const vec3& axis) const;
  double min_project_on_axis(const vec3& axis) const;

 private:
  void update_shape();
  void set_transform_dirty_sub();
  sgnode* clone_sub() const;

  std::vector<sgnode*> children;
};

class geometry_node : public sgnode {
 public:
  geometry_node(const std::string& id) : sgnode(id, false) {}
  virtual ~geometry_node() {}
  void gjk_support(const vec3& dir, vec3& support) const;

  void walk(std::vector<sgnode*>& result) { result.push_back(this); }

  void walk_geoms(std::vector<geometry_node*>& g);
  void walk_geoms(std::vector<const geometry_node*>& g) const;

 private:
  virtual void gjk_local_support(const vec3& dir, vec3& support) const = 0;
};

class convex_node : public geometry_node {
 public:
  convex_node(const std::string& id, const ptlist& v);

  const ptlist& get_verts() const { return verts; }
  const ptlist& get_world_verts() const;
  void set_verts(const ptlist& v);
  void get_shape_sgel(std::string& s) const;
  void gjk_local_support(const vec3& dir, vec3& support) const;

  void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);

  double max_project_on_axis(const vec3& axis) const;
  double min_project_on_axis(const vec3& axis) const;

 private:
  void set_transform_dirty_sub();
  void update_shape();
  sgnode* clone_sub() const;

  ptlist verts;
  mutable ptlist world_verts;
  mutable bool world_verts_dirty;
};

class ball_node : public geometry_node {
 public:
  ball_node(const std::string& id, double radius);
  void get_shape_sgel(std::string& s) const;

  double get_radius() const { return radius; }

  void set_radius(double r);
  void gjk_local_support(const vec3& dir, vec3& support) const;

  void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);

  double max_project_on_axis(const vec3& axis) const;
  double min_project_on_axis(const vec3& axis) const;

 private:
  void update_shape();
  sgnode* clone_sub() const;

  double radius;
};

class sgnode_listener {
 public:
  virtual void node_update(sgnode* n, sgnode::change_type t,
                           const std::string& update_info) = 0;
};

#endif
