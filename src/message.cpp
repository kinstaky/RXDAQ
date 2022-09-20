#include "include/message.h"

namespace rxdaq {


//-----------------------------------------------------------------------------
// 								MessageOstream
//-----------------------------------------------------------------------------

MessageOstream::MessageOstream(
	std::ostream &os,
	bool print,
	std::mutex &lock,
	const std::string &prefix
)
: os_(os), print_(print), guard_(lock) {
	os << prefix;
}


//-----------------------------------------------------------------------------
// 								Message
//-----------------------------------------------------------------------------

const std::map<Message::Level, std::string> Message::prefix_map_ = {
	{Level::kNone, ""},
	{Level::kError, "ERROR: "},
	{Level::kWarning, "WARNING: "},
	{Level::kInfo, "INFO: "},
	{Level::kDebug, "DEBUG: "}
};


const std::map<Message::Level, std::string> Message::color_map_ = {
	{Level::kNone, ""},							// nothing
	{Level::kError, "\033[1;31m"},				// light red
	{Level::kWarning, "\033[0;33m"},			// orange
	{Level::kInfo, "\033[0;32m"},				// green
	{Level::kDebug, "\033[0;36m"}				// cyan
};


const std::map<std::string, Message::Level> Message::level_map_ = {
	{"error", Level::kError},
	{"warning", Level::kWarning},
	{"info", Level::kInfo},
	{"debug", Level::kDebug}
};


Message::Message(Level level)
:intrinsic_level_(level), printing_level_(Level::kWarning),
	show_prefix_(false), show_color_(false) {
}


MessageOstream operator<<(std::ostream &os, Message &message) {
	std::string content = "";
	if (message.printing_level_ <= message.intrinsic_level_) {
		if (message.show_prefix_) {
			// add prefix
			content = message.prefix_map_.at(message.printing_level_);

			if (message.show_color_) {
				// add color
				content = message.color_map_.at(message.printing_level_)
					+ content + "\033[0m";
			}
		}
	}
	return MessageOstream(
		os,
		message.printing_level_ <= message.intrinsic_level_,
		message.lock_,
		content
	);
}



}