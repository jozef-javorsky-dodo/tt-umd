// SPDX-FileCopyrightText: (c) 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "umd/device/tlb.h"

namespace tt::umd {

bool tlb_data::check(const tlb_offsets &offset) const {
    return local_offset > ((1 << (offset.x_end - offset.local_offset)) - 1) |
           x_end > ((1 << (offset.y_end - offset.x_end)) - 1) | y_end > ((1 << (offset.x_start - offset.y_end)) - 1) |
           x_start > ((1 << (offset.y_start - offset.x_start)) - 1) |
           y_start > ((1 << (offset.noc_sel - offset.y_start)) - 1) |
           noc_sel > ((1 << (offset.mcast - offset.noc_sel)) - 1) |
           mcast > ((1 << (offset.ordering - offset.mcast)) - 1) |
           ordering > ((1 << (offset.linked - offset.ordering)) - 1) |
           linked > ((1 << (offset.static_vc - offset.linked)) - 1) |
           static_vc > ((1 << (offset.static_vc_end - offset.static_vc)) - 1);
}

// Helper lambda to handle bit packing
void pack_bits(std::uint64_t &lower, std::uint64_t &upper, std::uint64_t value, std::uint32_t offset_pos) {
    if (offset_pos < 64) {
        lower |= value << offset_pos;
        if (offset_pos != 0) {
            upper |= value >> (64 - offset_pos);
        }
    } else {
        uint64_t upper_delta = value << (offset_pos - 64);
        upper = upper | upper_delta;
    }
}

std::pair<std::uint64_t, std::uint64_t> tlb_data::apply_offset(const tlb_offsets &offset) const {
    if (this->check(offset)) {
        throw std::runtime_error("Invalid offsets for TLB index");
    }

    std::uint64_t lower = 0;
    std::uint64_t upper = 0;

    pack_bits(lower, upper, local_offset, offset.local_offset);
    pack_bits(lower, upper, x_end, offset.x_end);
    pack_bits(lower, upper, y_end, offset.y_end);
    pack_bits(lower, upper, x_start, offset.x_start);
    pack_bits(lower, upper, y_start, offset.y_start);
    pack_bits(lower, upper, noc_sel, offset.noc_sel);
    pack_bits(lower, upper, mcast, offset.mcast);
    pack_bits(lower, upper, ordering, offset.ordering);
    pack_bits(lower, upper, linked, offset.linked);
    pack_bits(lower, upper, static_vc, offset.static_vc);

    return std::make_pair(lower, upper);
}

}  // namespace tt::umd
