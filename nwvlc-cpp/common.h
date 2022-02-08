#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool write_config_njson(
	std::string url="http://127.0.0.1:4270/poll_status",
	std::string mname="ex_media_name",
	std::string uname="ex_user_name",
	int nw_poll_interval=1000
);
json read_config_njson();

bool check_nurl_valid(std::string url);
