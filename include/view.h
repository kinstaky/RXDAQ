#ifndef __VIEW_H__
#define __VIEW_H__

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#include "include/error.h"

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
	/// @tparam ViewValue type of parameter values
	/// @param[in] os ostream to output the view
	/// @param[in] view this view 
	/// @returns the ostream
	///
	template<typename ViewValue>
	friend std::ostream& operator<<(std::ostream &os, const View<ViewValue> &view);

private:
	enum class ViewType {
		kModuleParameter = 0,
		kOneChannelParameter,
		kServeralChannelParameter
	};

	ViewType type_;
	std::vector<Value> values_;
	std::vector<std::string> parameters_;
	std::vector<unsigned short> modules_;
	std::vector<unsigned short> channels_;
};


template <typename Value>
std::ostream& operator<<(std::ostream &os, const View<Value> &view) {
	if (view.type_ == View<Value>::ViewType::kModuleParameter) {
		
		os << "module";
		for (const auto &name : view.parameters_) {
			os << std::setw(8) << name;
		}
		os << "\n";
		for (size_t i = 0; i < view.modules_.size(); ++i) {
			os << std::setw(6) << view.modules_[i];
			for (size_t j = 0; j < view.parameters_.size(); ++j) {
				if (sizeof(Value) == 1) {
					// if (values_[i*parameters_.size() + j]) {
					// 	os << std::string(7, ' ') << "*";
					// } else {
					// 	os << std::string(8, ' ');
					// }
					os << std::string(7, ' ')
						<< " *"[static_cast<size_t>(
							view.values_[
								i * view.parameters_.size() + j
							]
						)];
				} else {
					os << std::hex << std::setw(8)
						<< view.values_[i*view.parameters_.size() + j]
						<< std::dec;
				}
			}
			os << "\n";
		}

	} else if (view.type_ == View<Value>::ViewType::kOneChannelParameter) {
		
		os << "module";
		for (const auto &c : view.channels_) {
			os << std::setw(8) << "ch" + std::to_string(c);
		}
		os << "\n";
		for (size_t i = 0; i < view.modules_.size(); ++i) {
			os << std::setw(6) << i;
			for (size_t j = 0; j < view.channels_.size(); ++j) {
				os << "  " << std::setfill(' ') << std::setw(6)
					<< view.values_[i * view.channels_.size() + j];
			}
			os << "\n";
		}


	} else if (view.type_ == View<Value>::ViewType::kServeralChannelParameter) {
		
		os << "module    ch";
		for (const auto &name : view.parameters_) {
			os << std::setw(8) << name;
		}
		os << "\n";
		for (size_t i = 0; i < view.modules_.size(); ++i) {
			os << std::setw(6) << i;
			for (size_t j = 0; j < view.channels_.size(); ++j) {
				os << std::setw(6) << i << std::setw(6) << j << std::hex;
				for (size_t k = 0; k < view.parameters_.size(); ++k) {
					if (sizeof(Value) == 1) {
						os << std::string(7, ' ')
							<< " *"[static_cast<size_t>(
								view.values_[
									i * view.parameters_.size() + j
								]
							)];
					} else {
						os << std::hex << std::setw(8)
							<< view.values_[i*view.parameters_.size() + j]
							<< std::dec;
					}
				}
			}
		}
		
	} else {
		throw RXError("invalid view type");
	}

	return os;
}

};


#endif 			// __VIEW_H__