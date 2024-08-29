#include "portability.h"

// SocketLib class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : ~2001
//
// Handles initilization and termination of the socket library for
// the appropriate platform.
//

#include "sock_OSspecific.h"
#include "sock_SocketLib.h"

using namespace sock;

// Construction/Destruction

SocketLib::SocketLib() { InitializeOperatingSystemSocketLibrary(); }

SocketLib::~SocketLib() { TerminateOperatingSystemSocketLibrary(); }
