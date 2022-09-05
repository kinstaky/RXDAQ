#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <iostream>
#include <map>
#include <mutex>

namespace rxdaq {


class MessageOstream {
public:

	/// @brief constructor
	///
	/// @param[in] os ostream to write message
	/// @param[in] print whether to print message(since level control)
	/// @param[in] lock reference to lock of Message object
	///
	MessageOstream(std::ostream &os, bool print, std::mutex &lock);

	
	/// @brief default destructor
	///
	~MessageOstream() = default;


	/// @brief write message to ostream
	///
	/// @tparam OutputType type of output content
	/// @param[in] content content to output
	/// @returns reference to this object
	///
	template <typename OutputType>
	MessageOstream& operator<<(const OutputType &content) {
		if (print_) os_ << content;
		return *this;
	}

private:

	std::ostream &os_;
	bool print_;
	std::lock_guard<std::mutex> guard_;
};


class Message {
public:
	enum class Level {
		kNone = 0,
		kError,
		kWarning,
		kInfo,
		kDebug
	};

	/// @brief constructor
	///
	/// @param[in] level the message level
	///
	Message(Level level);


	/// @brief default destructor
	///
	~Message() = default;


	/// @brief set message level
	///
	/// @param[in] level the message level to set
	///
	inline void SetLevel(Level level) {
		intrinsic_level_ =  level;
	}


	/// @brief convert string to Level
	///
	/// @param[in] level level in string format
	/// @returns enum Message::Level
	///
	static inline Level ToLevel(const std::string &level) {
		auto search = level_map_.find(level);
		if (search == level_map_.end()) {
			std::cout << "WARNING: messageLevel " << level << " invalid.\n";
			// default level
			return Level::kWarning;
		}
		return search->second;
	}


	/// @brief let message show prefix
	///
	/// @param[in] prefix whether to show prefix
	///
	inline void SetPrefix(bool prefix = true) {
		show_prefix_ = prefix;
	}


	/// @brief let message show colorful prefix
	///
	/// @param[in] color whether to show color
	///
	inline void SetColor(bool color = true) {
		show_color_ = color;
	}


	/// @brief let message show colorful prefix
	///
	inline void SetColorfulPrefix() {
		SetPrefix();
		SetColor();
	}


	/// @brief set the printing level
	///
	/// @param[in] level level of content to print
	/// @returns reference to this Message
	///
	inline Message& operator()(Level level) {
		printing_level_ = level;
		return *this;
	}


	/// @brief overload stream output function
	///
	/// @param[in] os ostream to output 
	/// @param[in] message message object to set format
	/// @returns instance of MessageOstream object
	///
	/// @relates Message
	///
	friend MessageOstream operator<<(std::ostream &os, Message &message);


private:

	static const std::map<Level, std::string> prefix_map_;
	static const std::map<Level, std::string> color_map_;
	static const std::map<std::string, Level> level_map_;

	Level intrinsic_level_;
	Level printing_level_;
	bool show_prefix_;
	bool show_color_;
	std::mutex lock_;
};



}		// namespace rxdaq

#endif	 		// __MESSAGE_H__