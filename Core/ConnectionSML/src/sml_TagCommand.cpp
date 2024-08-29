#include "portability.h"

// TagCommand class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : August 2004
//
// Represents an SML node which represents a command and has
// the tag <command>.
//

#include "sml_TagCommand.h"

using namespace sml;

TagCommand::TagCommand() { SetTagNameFast(sml_Names::kTagCommand); }
