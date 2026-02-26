#include "hwy/highway.h"
#include <cassert>

namespace hn = hwy::HWY_NAMESPACE;

HWY_ATTR void binary_search_vectorized(int32_t* sorted, size_t sorted_size,
                                        int32_t* data, int32_t* found_idx_arr,
                                        size_t data_size) {
    const hn::ScalableTag<int32_t> i32;
    const int N = hn::Lanes(i32);
    assert(data_size % N == 0);

    const auto one = hn::Set(i32, 1);

    for (size_t i = 0; i < data_size; i += N) {
        auto key       = hn::LoadU(i32, data + i);
        auto low       = hn::Zero(i32);
        auto high      = hn::Set(i32, static_cast<int32_t>(sorted_size - 1));

        auto found_idx  = hn::Set(i32, -1);
        auto finished  = hn::MaskFalse(i32);

        // The mask `finished` contains all the lanes for which
        // the search is finished. 
        while (true) {
            // We mark as finished those lanes for which low > high
            finished = hn::Or(finished, (low > high));

            // If search is finished for all lanes, we break
            if (hn::AllTrue(i32, finished)) break;

            // mid = (low + high) / 2
            auto mid = hn::ShiftRight<1>(low + high);
            auto val = hn::GatherIndex(i32, sorted, mid);

            auto eq_mask = val == key;

            found_idx = hn::IfThenElse(eq_mask, mid, found_idx);
            // If the element was found, the search if finished for this element
            finished  = hn::Or(finished, eq_mask);

            // We don't update `high` and `low` if the search is finished
            high = hn::IfThenElse(hn::AndNot(finished, val > key) , mid - one, high);
            low  = hn::IfThenElse(hn::AndNot(finished, val < key), mid + one, low);
        }

        hn::StoreU(found_idx, i32, found_idx_arr + i);
    }
}
