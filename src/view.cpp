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
		type_ = ViewType::kModuleParameter;
	} else {
		type_ = parameters.size() > 1 ? ViewType::kServeralChannelParameter
			: ViewType::kOneChannelParameter;
	}
}


template class View<bool>;
template class View<unsigned int>;
template class View<double>;

};