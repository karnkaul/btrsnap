#pragma once
#include "btrsnap/instance.hpp"
#include "common/mock_btrfs.hpp"
#include "common/test_directory.hpp"

namespace btrsnap::test {
class TestInstance : public Instance {
  public:
	explicit TestInstance() : Instance(std::make_unique<MockBtrfs>()) {}

  private:
	TestDirectory m_test_dir{};
};
} // namespace btrsnap::test
