/* Copyright (C) 2019 Lewis Clark

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. */


#include "file.hpp"

std::filesystem::path glt::file::GetWorkDirectory() {
	auto home = GetHomeDirectory();

	if (home.empty()) {
		return std::filesystem::path();
	}

	return (home / "gluasteal");
}

std::filesystem::path glt::file::GetHomeDirectory() {
	std::filesystem::path path;

#if (defined(OS_LINUX) || defined(OS_MAC))
	struct passwd* pw = getpwuid(getuid());
	path = std::string(pw->pw_dir);
#elif (defined(OS_WINDOWS))
	PWSTR winpath = nullptr;
	SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &winpath);

	path = std::wstring(winpath);
#endif

	return path;
}

std::filesystem::path glt::file::GetLogFilePath() {
	return (GetWorkDirectory() / "log.txt");
}

std::filesystem::path glt::file::GetServerStorePath() {
	return (GetWorkDirectory() / "servers");
}

#if (defined(OS_LINUX) || defined(OS_MAC))
static std::vector<std::string> reserved {

};
#elif (defined(OS_WINDOWS))
static std::vector<std::string> reserved {
	"con", "prn", "aux", "nul", "com0", "com1", "com2", "com3", "com4", "com5",
	"com6", "com7", "com8", "com9", "com10", "lpt0", "lpt1", "lpt2", "lpt3",
	"lpt4", "lpt5", "lpt6", "lpt7", "lpt8", "lpt9", "lpt10"
};
#endif

static char is_bad_char(const char& c) {
	if (c >= 0x0 && c <= 0x1f) {
		return true;
	}

	if (c == '_' || c == '-' || c == '.' || c == '/' || c == '\\' || isalnum(c)) {
		return false;
	}

	return true;
}

std::filesystem::path glt::file::SanitizeLuaFilePath(std::string pathstr) {
	pathstr.erase(std::remove_if(pathstr.begin(), pathstr.end(), is_bad_char), pathstr.end());

	while (!pathstr.empty() && (pathstr.back() == ' ' || pathstr.back() == '.')) {
		pathstr.pop_back();
	}

	auto path = std::filesystem::path(pathstr).relative_path();
	path = RemoveReservedWords(path);

	if (path.string().length() >= 200) {
		return std::filesystem::path("longfilename.lua"); // TODO: Do something better than this
	}

	if (!path.has_filename() || path.filename().string().front() == '.') {
		path.replace_filename("noname");
	}

	path.replace_extension(".lua");

	return path;
}

bool glt::file::IsReserved(const std::filesystem::path& path) {
	std::string pathstr = path.string();
	std::transform(pathstr.begin(), pathstr.end(), pathstr.begin(), ::tolower);

	return (std::find(reserved.begin(), reserved.end(), pathstr) != reserved.end());
}

std::filesystem::path glt::file::RemoveReservedWords(const std::filesystem::path& path) {
	auto newpath = std::filesystem::path();

	for (const auto& e : path) {
		const auto& filename = e.filename();

		if (filename == ".." || filename == ".") {
			continue;
		}

		if (IsReserved(filename)) {
			newpath /= ("_" + e.string());
		}
		else {
			newpath /= e;
		}
	}

	if (IsReserved(newpath.stem())) {
		newpath.replace_filename("_" + newpath.filename().string());
	}

	return newpath;
}
