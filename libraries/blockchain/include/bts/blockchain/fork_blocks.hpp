#pragma once

#include <stdint.h>
#include <vector>

#define BTSX_MARKET_FORK_TO_UNIX_TIME_LIST

#define FORK_25 13000

#define BTS_CHECK_CANONICAL_SIGNATURE_FORK_BLOCK_NUM FORK_25

namespace bts { namespace blockchain {
  uint32_t estimate_last_known_fork_from_git_revision_timestamp(uint32_t revision_time);
  std::vector<uint32_t> get_list_of_fork_block_numbers();
} }
