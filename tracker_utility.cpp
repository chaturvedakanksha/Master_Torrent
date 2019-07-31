#include<string>
#include<bits/stdc++.h>
#include<ctime>

#include "tracker_utility.h"

using namespace std;

string get_current_time(){

    time_t now = time(0);
    string time;

    tm *ltm = localtime(&now);

    
    time = time + to_string(ltm->tm_mday) + "/" + to_string(1 + ltm->tm_mon) 
            + "/" + to_string(1900 + ltm->tm_year)
            + " | " + to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) 
            + ":" + to_string(ltm->tm_sec);

    return time;
}

void convertToArray(string str,vector<string> &arr){
	stringstream ss(str);
	string word;
	while(ss>>word){
		arr.push_back(word);
	}
}

void getTrackerIP_PORT(string tracker_info,pair<string,string> &tracker_ip_port){
	int index=tracker_info.find(":");
	string tracker_ip=tracker_info.substr(0,index);
	string tracker_port=tracker_info.substr(index+1);
	tracker_ip_port.first=tracker_ip;
	tracker_ip_port.second=tracker_port;

}
