#include "portability.h"

// TagResult class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : August 2004
//
// Represents an SML node which represents the result of a command with
// the tag <result>.
//

#include "sml_TagResult.h"

using namespace sml;

TagResult::TagResult(void) { SetTagNameFast(sml_Names::kTagResult); }

TagResult::~TagResult(void) {}
