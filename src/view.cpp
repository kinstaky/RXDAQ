#include "include/view.h"

namespace rxdaq {

template <typename Value>
View<Value>::View(
	std::vector<Value> values,
	std::vector<std::string> parameters,
	std::vector<unsigned short> modules,
	std::vector<unsigned short> channels
)
: values_(values)
, parameters_(parameters)
, modules_(modules)
, channels_(channels) {

	size_t values_size = parameters_.size() * modules_.size();
	values_size *= channels_.size() ? channels_.size() : 1;
	if (values_size != values.size()) {
		throw RXError("value size invalid");
	}

	if (channels.empty()) {
		type_ = kMoudleParameter;
	} else {
		type_ = parameters.size() > 1 ? ViewType::kServeralChannelParameter
			: ViewType::kOneChannelParameter;
	}
}



template <typename Value>
std::ostream& operator<<(std::ostream &os, const View<Value> &view) {
	if (type_ == ViewType::kModuleParameter) {
		
		os << "module";
		for (const auto &name : parameters_) {
			os << std::setw(8) << name;
		}
		os << "\n";
		for (size_t i = 0; i < modules_.size(); ++i) {
			os << std::setw(6) << modules_[i];
			for (size_t j = 0; j < parameters_.size(); ++j) {
				if (sizeof(Value) == 1) {
					// if (values_[i*parameters_.size() + j]) {
					// 	os << std::string(7, ' ') << "*";
					// } else {
					// 	os << std::string(8, ' ');
					// }
					os << std::string(7, ' ') << " *"[values_[i*parameters_.size() + j]];
				} else {
					os << std::hex << std::setw(8)
						<< values_[i*parameters_.size() + j] << std::dec;
				}
			}
			os << "\n":
		}

	} else if (type_ == ViewType::kOneChannelParameter) {
		
		os << "module";
		for (const auto &c : channels) {
			os << std::setw(8) << "ch" + std::to_string(c);
		}
		os << "\n";
		for (size_t i = 0; i < modules_.size(); ++i) {
			os << std::setw(6) << i;
			for (size_t j = 0; j < channels_.size(); ++j) {
				os << "  " << std::setfill(' ') < std:setw(6)
					<< values_[i * channels_.size() + j];
			}
			os << "\n";
		}


	} else if (type_ == ViewType::kServeralChannelParameter) {
		
		os << "module    ch";
		for (const auto &name : parameters_) {
			os << std::setw(8) << name;
		}
		os << "\n";
		for (size_t i = 0; i < module_.size(); ++i) {
			os << std::setw(6) << i;
			for (size_t j = 0; j < channels_.size(); ++j) {
				os << std::setw(6) << i << std::setw(6) << j << std::hex;
				for (size_t k = 0; k < parameters_.size(); ++k) {
					if (sizeof(Value) == 1) {
						os << std::string(7, ' ')
							<< " *"[values_[i*parameters_.size() + j]];
					} else {
						os << std::hex << std::setw(8)
							<< values_[i*parameters_.size() + j] << std::dec;
					}
				}
			}
		}
		
	} else {
		throw RXError("invalid view type");
	}

	return os;
}

template class View<bool>;
template class View<unsigned int>;
template class View<double>;

};