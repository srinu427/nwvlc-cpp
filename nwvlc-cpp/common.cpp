#include "common.h"
#include <fstream>
#include <iostream>
#include <cpr/cpr.h>


bool write_config_njson(std::string url, std::string mname, std::string uname, int nw_poll_interval)
{
	try {
		
		json cjson;
		cjson["url"] = url;
		cjson["media_name"] = mname;
		cjson["uname"] = uname;
		cjson["nwpoll_interval_ms"] = nw_poll_interval;

		std::ofstream fw("config.json");
		fw << std::setw(4) << cjson << std::endl;
		fw.close();
		return true;
	}
	catch(int err) {
		return false;
	}
}

json read_config_njson()
{
	json cjson;
	cjson["url"] = "http://127.0.0.1:4270/poll_status";
	cjson["media_name"] = "ex_media_name";
	cjson["uname"] = "ex_user_name";
	cjson["nwpoll_interval_ms"] = 1000;

	try {
		if (!std::filesystem::exists("config.json")) {
			write_config_njson();
		}
		json rjson;
		std::ifstream fr("config.json");
		fr >> rjson;
		fr.close();

		cjson["url"] = rjson["url"];
		cjson["media_name"] = rjson["media_name"];
		cjson["uname"] = rjson["uname"];
		cjson["nwpoll_interval_ms"] = rjson["nwpoll_interval_ms"];

		return cjson;
	}
	catch (int err) {
		return cjson;
	}
	
}

bool check_nurl_valid(std::string url)
{
	try {
		cpr::Response res = cpr::Post(cpr::Url{ url }, cpr::Body{ {} });
		return (res.status_code == 200);
	}
	catch (int err) {
		return false;
	}
}