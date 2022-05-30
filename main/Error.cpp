#include "Error.h"

//--------------------------------------------------
//					UserError
//--------------------------------------------------

UserError::UserError(const std::string &what_arg): std::runtime_error(what_arg) {
}



//--------------------------------------------------
//					RXError
//--------------------------------------------------

RXError::RXError(const std::string &what_arg): std::runtime_error(what_arg) {
}