#include "GoProOverlay/data/TelemetrySample.h"

#include <type_traits>

namespace gpo
{

    static_assert(
        std::is_standard_layout_v<TelemetrySample> && std::is_trivial_v<TelemetrySample>,
        "TelemetrySample must be a POD type");
    
}