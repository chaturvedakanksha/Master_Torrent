#include<string.h>
#include<ctime>
#include<iostream>
#include<vector>
#include<bits/stdc++.h>
#include<string.h>

using namespace std;

#ifndef INC_CLIENT_UTILITY_H
#define INC_CLIENT_UTILITY_H

class mtorrent_file {
public:
    string tracker_1_ip;
    string tracker_1_port;
    string tracker_2_ip;
    string tracker_2_port;
    string client_ip;
    string client_port;
    string path;
    string filename;
    string filesize;
    string hash_string;
    mtorrent_file (){
        tracker_1_ip="";
        tracker_1_port="";
        tracker_2_ip="";
        tracker_2_port="";
        client_ip="";
        client_port="";
        path="";
        filename="";
        filesize="";
        hash_string="";
    }

};

string get_current_time();
void convertToArray(string str,vector<string> &arr);
vector<string> parse_seeder_list(char buffer[1024]);
void get_IP_PORT(string info,pair<string,string> &ip_port);
string compute_sha_of_hash_string(string hash_string) ;
mtorrent_file create_mtorrent_file(mtorrent_file mt_file);
 mtorrent_file create_mtorrent_object(mtorrent_file mt_file);
#endif