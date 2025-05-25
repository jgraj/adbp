#include <ctk-0.19/ctk.cpp>

void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	std::vfprintf(stderr, format, args);
	va_end(args);
	std::exit(1);
}

struct ADBP {
	std::FILE* out_file;
	size_t current_pos;
	size_t asset_count;

	void create(const char* out_path) {
		this->out_file = std::fopen(out_path, "wb");
		if (this->out_file == nullptr) {
			panic("failed to open output file");
		}
		this->current_pos = 0;
		this->asset_count = 0;
	}

	void destroy() {
		std::fclose(this->out_file);
	}

	void pack_assets_dir(const char* dir_path, const char* asset_id) {
		ctk::Directory dir;
		dir.create(dir_path);
		while (true) {
			ctk::Directory::Entity ent = dir.get_next(true);
			if (ent.type == ctk::Directory::Entity::Type::None) {
				break;
			}
			ctk::ar<u8> file_path = ctk::alloc_format("%s/%s", dir_path, ent.path);
			ctk::ar<u8> new_asset_id;
			if (asset_id == nullptr) {
				new_asset_id = ctk::alloc_format("%s", ent.path);
			} else {
				new_asset_id = ctk::alloc_format("%s/%s", asset_id, ent.path);
			}
			for (size_t a = 0; a < new_asset_id.len; ++a) {
				if (new_asset_id[a] == '.') {
					new_asset_id[a] = '_';
				}
			}
			if (ent.type == ctk::Directory::Entity::Type::Dir) {
				this->pack_assets_dir((const char*)file_path.buf, (const char*)new_asset_id.buf);
			} else {
				this->pack_asset_file((const char*)file_path.buf, (const char*)new_asset_id.buf);
			}
			file_path.destroy();
			new_asset_id.destroy();
		}
		dir.destroy();
	}

	void pack_asset_file(const char* asset_path, const char* asset_id) {
		ctk::ar<u8> asset_data = ctk::load_file(asset_path);
		if (std::fwrite(asset_data.buf, 1, asset_data.len, out_file) != asset_data.len) {
			panic("failed to write to output file");
		}
		std::printf("static ctk::ar<const u8> %s = ctk::ar<const u8>(&_binary_temp_assets_bin_start[%zu], %zu);\n", asset_id, current_pos, asset_data.len);
		this->current_pos += asset_data.len;
		this->asset_count += 1;
	}
};

int main(int argc, char** argv) {
	if (argc != 3) {
		panic("expected 2 args");
	}
	ADBP adbp;
	adbp.create(argv[2]);
	adbp.pack_assets_dir(argv[1], nullptr);
	adbp.destroy();
	return 0;
}