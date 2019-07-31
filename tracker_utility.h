#include<string.h>

#ifndef INC_TRACKER_UTILITY_H
#define INC_TRACKER_UTILITY_H

using namespace std;

class mtorrent_file{
public:
	string tracker_1_ip;
	string tracker_1_port;
	string tracker_2_ip;
	string tracker_2_port;
	string seeder_file;
};
string get_current_time();
void convertToArray(string str,vector<string> &arr);
void getTrackerIP_PORT(string tracker_info,pair<string,string> &tracker_ip_port);
#endif