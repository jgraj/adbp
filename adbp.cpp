#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>

void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

void pack_asset_file(const char* asset_path, const char* asset_id, FILE* out_file, int* current_pos, int* asset_count) {
	FILE* file = fopen(asset_path, "rb");
	if (!file) {
		panic("Failed to open file '%s': %s\n", asset_path, strerror(errno));
	}

	const size_t buffer_size = 1024 * 1024;
	char buffer[buffer_size];

	int current_size = 0;
	size_t bytes_read;
	while ((bytes_read = fread(buffer, 1, buffer_size, file)) > 0) {
		current_size += bytes_read;
		if (fwrite(buffer, 1, bytes_read, out_file) != bytes_read) {
			fclose(file);
			panic("Failed to write to output file");
		}
	}

	if (ferror(file)) {
		fclose(file);
		panic("Error reading file '%s'\n", asset_path);
	}

	fclose(file);

	printf("Asset::make(\"%s\", %i, %i),\n", asset_id, *current_pos, current_size);
	
	*current_pos += current_size;
	*asset_count += 1;
}

void pack_assets_dir(const char* dir_path, const char* asset_id, FILE* out_file, int* current_pos, int* asset_count) {
	DIR* dir = opendir(dir_path);
	if (!dir) {
		panic("Failed to open directory '%s': %s\n", dir_path, strerror(errno));
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		// Skip '.' and '..'
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// Build the new file path
		char file_path[PATH_MAX];
		snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

		// Build the asset ID
		char new_asset_id[PATH_MAX];
		if (asset_id == NULL) {
			snprintf(new_asset_id, sizeof(new_asset_id), "%s", entry->d_name);
		} else {
			snprintf(new_asset_id, sizeof(new_asset_id), "%s/%s", asset_id, entry->d_name);
		}

		struct stat path_stat;
		stat(file_path, &path_stat);

		if (S_ISDIR(path_stat.st_mode)) {
			pack_assets_dir(file_path, new_asset_id, out_file, current_pos, asset_count);
		} else {
			pack_asset_file(file_path, new_asset_id, out_file, current_pos, asset_count);
		}
	}
	closedir(dir);
}

void pack_assets(const char* assets_path, const char* out_path) {
	FILE* out_file = fopen(out_path, "wb");
	if (!out_file) {
		if (out_file) fclose(out_file);
		panic("Failed to open output file");
	}

	printf("const Asset asset_list[] = {\n");

	int current_pos = 0;
	int asset_count = 0;
	pack_assets_dir(assets_path, NULL, out_file, &current_pos, &asset_count);

	printf("};\nconstexpr size_t asset_count = %i;", asset_count);

	fclose(out_file);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		panic("expected 2 args");
	}
	pack_assets(argv[1], argv[2]);
	return 0;
}