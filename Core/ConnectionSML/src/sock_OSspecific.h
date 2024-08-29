// OSSpecific class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : ~2001
//
// The parts of the socket code that are specific to the operating system.
//

#ifndef CT_OS_SPECIFIC_H
#define CT_OS_SPECIFIC_H

namespace sock {

bool InitializeOperatingSystemSocketLibrary();
bool TerminateOperatingSystemSocketLibrary();
bool MakeSocketNonBlocking(SOCKET hSock);
}  // namespace sock

#endif  // CT_OS_SPECIFIC_H
