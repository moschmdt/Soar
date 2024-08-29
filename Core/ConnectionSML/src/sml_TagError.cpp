#include "portability.h"

// TagError class
//
// @author: Douglas Pearson, www.threepenny.net
// @date  : August 2004
//
// Represents an SML node which represents an error in a result with
// the tag <error>.
//

#include "sml_TagError.h"

using namespace sml;

TagError::TagError(void) { SetTagNameFast(sml_Names::kTagError); }

TagError::~TagError(void) {}
