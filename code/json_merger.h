#pragma once

#include <cassert>
#include <fstream>
#include <string>
#include <cctype>

#include <boost/filesystem/path.hpp>
#include <boost/utility/string_view.hpp>

class JsonMerger {
public:
	JsonMerger(const boost::filesystem::path &target_path) :
		target_path(target_path),
		json_ofs(this->target_path.string() + ".json", std::ofstream::out | std::ofstream::trunc),
		csv_ofs(this->target_path.string() + ".csv", std::ofstream::out | std::ofstream::trunc) {
		this->json_ofs << "[";
	}

	~JsonMerger() {
		this->json_ofs << "]";
	}

	bool addFile(const boost::filesystem::path &path) {
		std::string ijson;
		{
			std::ifstream ifs(path.string());
			if (!ifs.is_open()) {
				return false;
			}
			ijson = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		std::string::iterator begin = ijson.begin();
		std::string::iterator end = ijson.end();
		trim(begin, end);
		assert(begin != end && *begin == '[');
		++begin;
		assert(begin != end);
		--end;
		assert(*end == ']');
		if (begin != end) {
			trim(begin, end);
			{
				std::ptrdiff_t distance = std::distance(begin, end);
				if (distance > 0) {
					if (this->json_needs_comma == true) {
						this->json_ofs << ",\n";
					}
					this->json_ofs << boost::string_view(&*begin, distance);
					this->json_needs_comma = true;
				} else {
					this->json_needs_comma = false;
				}
			}
			{
				auto part_begin = begin;
				auto part_end = begin;
				while (part_end != end) {
					while (*part_begin != '[') {
						assert(part_begin != end);
						++part_begin;
					}
					++part_begin;
					part_end = part_begin;
					while (*part_end != ']') {
						assert(part_end != end);
						++part_end;
					}
					std::ptrdiff_t distance = std::distance(part_begin, part_end);
					if (distance != 0) {
						this->csv_ofs << boost::string_view(&*part_begin, distance);
						this->csv_ofs << "\n";
					}
					assert(part_end != end);
					++part_end;
					part_begin = part_end;
				}
			}
		}
		return true;
	}


private:
	static void trim(std::string::iterator &begin, std::string::iterator &end) {
		while (begin != end && std::isspace(static_cast<unsigned char>(*begin)) != 0) {
			++begin;
		}
		if (begin == end) {
			return;
		}
		--end;
		while (begin != end && std::isspace(static_cast<unsigned char>(*end)) != 0) {
			--end;
		}
		++end;
	}

	const boost::filesystem::path target_path;

	bool json_needs_comma = false;

	std::ofstream json_ofs;
	std::ofstream csv_ofs;
};
