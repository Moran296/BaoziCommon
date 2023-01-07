#ifndef BAOZI_RESCALE_H__
#define BAOZI_RESCALE_H__

#include <type_traits>

namespace Baozi {

template <typename TInput, typename TOutput>
requires std::is_arithmetic_v<TInput> && std::is_arithmetic_v<TOutput>
class BaoRescale {
public:
    BaoRescale(TInput input_min, TInput input_max, TOutput output_min, TOutput output_max) :
        m_input_min(input_min),
        m_input_max(input_max),
        m_output_min(output_min),
        m_output_max(output_max)
    {
        m_input_range = m_input_max - m_input_min;
        m_output_range = m_output_max - m_output_min;
    }

    TOutput operator()(TInput input) {
        return m_output_min + (input - m_input_min) * m_output_range / m_input_range;
    }

private:
    TInput m_input_min;
    TInput m_input_max;
    TInput m_input_range;
    TOutput m_output_min;
    TOutput m_output_max;
    TOutput m_output_range;
};


} // namespace Baozi

#endif