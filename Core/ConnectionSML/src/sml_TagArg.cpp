#include "portability.h"

// TagArg class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : August 2004
//
// Represents an SML node which represents an argument for a command and has
// the tag <arg>.
//

#include "sml_TagArg.h"

using namespace sml;

TagArg::TagArg() { this->SetTagNameFast(sml_Names::kTagArg); }
