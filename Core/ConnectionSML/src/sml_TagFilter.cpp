#include "portability.h"

// TagFilter class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : April 2006
//
// Represents a command that is being sent/received by a command line filter.
//

#include "sml_Names.h"
#include "sml_TagFilter.h"

using namespace sml;

TagFilter::TagFilter() { this->SetTagNameFast(sml_Names::kTagFilter); }

void TagFilter::SetCommand(char const* pCommandLine) {
  this->AddAttributeFast(sml_Names::kFilterCommand, pCommandLine);
}
