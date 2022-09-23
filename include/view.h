#ifndef __VIEW_H__
#define __VIEW_H__

#include <iostream>
#include <vector>
#include <string>

namespace rxdaq {

template<typename Value>
class View {
public:
	
	/// @brief constructor 
	///
	/// @param[in] values values of parameters 
	/// @param[in] parameters parameter names
	/// @param[in] modules list of modules
	/// @param[in] channels list of channels
	///
	/// @throws RXError if size of values is not equal to size of
	/// 	parameters * modules * channels
	///
	View(
		std::vector<Value> values,
		std::vector<std::string> parameters,
		std::vector<unsigned short> modules,
		std::vector<unsigned short> channels = std::vector<unsigned short>()
	);


	/// @brief print the view
	///
	/// @param[in] os ostream to output the view
	/// @param[in] view this view 
	/// @returns the ostream
	///
	friend std::ostream& operator<<(std::ostream &os, const View &view);

private:
	enum class ViewType {
		kMoudleParameter = 0,
		kOneChannelParameter,
		kServeralChannelParameter
	};

	ViewType type_;
};

};


#endif 			// __VIEW_H__