/*!
 * \file
 * \brief Class module::Probe_value.
 */
#ifndef PROBE_VALUE_HPP_
#define PROBE_VALUE_HPP_

#include <string>
#include <vector>
#include <typeindex>

#include "Module/Probe/Probe.hpp"

namespace aff3ct
{
namespace module
{
template <typename T>
class Probe_value : public Probe<T>
{
public:
	Probe_value(const int size, const std::string &col_name, tools::Reporter_probe* reporter = nullptr);

	virtual ~Probe_value() = default;

	virtual void register_reporter(tools::Reporter_probe* reporter);

protected:
	virtual void _probe(const T *in, const size_t frame_id);
};
}
}

#endif /* PROBE_VALUE_HPP_ */
