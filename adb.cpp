#ifndef ADB_PANIC
#define ADB_PANIC(...) std::printf(__VA_ARGS__); std::exit(1);
#endif

extern const u8 _binary_temp_assets_bin_start[];

namespace adb {
	constexpr static ctk::ar<const u8> make(size_t start, size_t size) {
		return ctk::ar<const u8>(&_binary_temp_assets_bin_start[start], size);
	}

	#include <asset_list.hpp>
}