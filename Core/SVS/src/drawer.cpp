#include "drawer.h"

#include <cstdlib>
#include <iostream>

#include "common.h"
#include "platform_specific.h"
#include "sgnode.h"

std::ostream& write_vec3(std::ostream& os, const vec3& v) {
  os << v(0) << " " << v(1) << " " << v(2);
  return os;
}

class ipcsocket {
 public:
  ipcsocket() : sock(INVALID_SOCK) {}

  ~ipcsocket() {
    if (sock != INVALID_SOCK) {
      close_tcp_socket(sock);
    }
  }

  bool connect(const std::string& path) {
    if (sock != INVALID_SOCK) {
      close_tcp_socket(sock);
    }
    sock = get_tcp_socket(path);
    return (sock != INVALID_SOCK);
  }

  void disconnect() {
    close_tcp_socket(sock);
    sock = INVALID_SOCK;
  }

  bool send(const std::string& s) {
    if (sock == INVALID_SOCK) {
      return false;
    }
    return tcp_send(sock, s);
  }

 private:
  SOCK sock;
};

drawer::drawer() : connected(false) { sock = new ipcsocket(); }

drawer::~drawer() { delete sock; }

bool drawer::connect(const std::string& path) {
  connected = sock->connect(path);
  return connected;
}

void drawer::disconnect() {
  if (connected) {
    sock->disconnect();
  }
  connected = false;
}

void drawer::add(const std::string& scn, const sgnode* n) {
  if (!connected || !n->get_parent()) {
    return;
  }
  change(scn, n, SHAPE | POS | ROT | SCALE);
}

void drawer::del(const std::string& scn, const sgnode* n) {
  if (!connected) {
    return;
  }

  std::stringstream ss;
  ss << scn << " -" << n->get_id() << std::endl;
  send(ss.str());
}

void drawer::change(const std::string& scn, const sgnode* n, int props) {
  if (!connected) {
    return;
  }

  vec3 p, s;
  vec4 q;
  std::stringstream ss;

  n->get_world_trans().to_prs(p, q, s);
  ss << "+" << scn << " +" << n->get_id() << " ";
  if (props & SHAPE) {
    std::string shape;
    n->get_shape_sgel(shape);
    ss << " " << shape << " ";
  }
  if (props & POS) {
    ss << " p ";
    write_vec3(ss, p);
  }
  if (props & ROT) {
    ss << " r " << q(0) << " " << q(1) << " " << q(2) << " " << q(3) << " ";
  }
  if (props & SCALE) {
    ss << " s ";
    write_vec3(ss, s);
  }
  ss << std::endl;
  send(ss.str());
}

void drawer::delete_scene(const std::string& scn) {
  if (!connected) {
    return;
  }

  send(std::string("-") + scn + "\n");
}

void drawer::send(const std::string& s) {
  if (!connected) {
    return;
  }
  if (s[s.size() - 1] != '\n') {
    connected = sock->send(s + '\n');
  } else {
    connected = sock->send(s);
  }
}
