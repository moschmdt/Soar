#include "portability.h"

// TagName class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : October 2004
//
// Represents an SML node which represents a named object and
// has the tag <name>
//

#include "sml_TagName.h"

using namespace sml;

TagName::TagName() { this->SetTagNameFast(sml_Names::kTagName); }
