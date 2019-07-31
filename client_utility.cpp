#include<string.h>
#include<ctime>
#include<iostream>
#include<vector>
#include<bits/stdc++.h>
#include<openssl/sha.h>

#include "client_utility.h"

using namespace std;

#define MAX 524288

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
vector<string> parse_seeder_list(char buffer[1024]) {

    vector<string> tokens;
    string temp;

    char ch;

    for(int i =0; i < strlen(buffer); i++) {
        ch = buffer[i];

        if(ch == '\n' || ch == '\0') {
            tokens.push_back(temp);
            temp = "";
        } else {
            temp = temp + ch;
        }

    }

    return tokens;
}
void get_IP_PORT(string info,pair<string,string> &ip_port){
    int index=info.find(":");
    string ip=info.substr(0,index);
    string port=info.substr(index+1);
    ip_port.first=ip;
    ip_port.second=port;

}
string compute_sha_of_hash_string(string hash_string) {

    char input[MAX];

    strcpy(input, hash_string.c_str());

    char buffer[SHA_DIGEST_LENGTH*2];
    unsigned char output[SHA_DIGEST_LENGTH];
    
    memset(buffer, 0x0, SHA_DIGEST_LENGTH*2);
    memset(output, 0x0, SHA_DIGEST_LENGTH);

    SHA1((unsigned char *) input, strlen(input), output);

    for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf((char*)&(buffer[i*2]), "%02x", output[i]); 
    }

    string sha(buffer, 20);

    return sha;
}

mtorrent_file create_mtorrent_file(mtorrent_file mt_file){
    //log_file<<get_current_time()<<" | "<<"Creating mtorrent file"<<endl;
    string path=mt_file.path;
    string filename=mt_file.filename;

    //creating mtorrent file
    mtorrent_file mt_file_up;
    
    //get size of file
    ifstream fin(path,ios::in|ios::binary);
    if(!fin){
    	//log_file<<get_current_time()<<" | "<<"Error! File: "<<filename<<" doesn't exist."<<endl;
        cout<<"File: "<<filename<<" doesn't exist. Enter the valid path(local path) " << endl;
        return mt_file_up;
    }
    fin.seekg(0,ios::end);
    int filesize=fin.tellg();
    fin.close();

    //generating SHA1 for a file
    char buffer[1024];
    bzero(buffer,1024);
    string hash_string;
    ifstream f_in(path,ios::in|ios::binary);
    if(!f_in){
        cout<<"File can't open";
        exit(0);
    }
    bzero(buffer,1024);
    f_in.getline(buffer,1024);
    hash_string+=buffer; 
    
    string sha=compute_sha_of_hash_string(hash_string);

    ofstream m_file;

    m_file.open(mt_file.filename+".mtorrent",ios::out);
    if(!m_file){
        cout<<"Mtorrent file can't open"<<endl;
        exit(EXIT_FAILURE);
    }

    m_file<<"Tracker 1 URL: "<<mt_file.tracker_1_ip<<" : "<<mt_file.tracker_1_port<<endl;
    m_file<<"Tracker 2 URL: "<<mt_file.tracker_2_ip<<" : "<<mt_file.tracker_2_port<<endl;
    m_file<<"Filename: "<<mt_file.filename<<endl;
    m_file<<"Filesize: "<<filesize<<endl;
    m_file<<"Hash_string: "<<sha<<endl;
    m_file.close();

    mt_file_up.tracker_1_ip=mt_file.tracker_1_ip;
    mt_file_up.tracker_1_port=mt_file.tracker_1_port;
    mt_file_up.tracker_2_ip=mt_file.tracker_2_ip;
    mt_file_up.tracker_2_port=mt_file.tracker_2_port;
    mt_file_up.client_ip=mt_file.client_ip;
    mt_file_up.client_port=mt_file.client_port;
    mt_file_up.filename=mt_file.filename;
    mt_file_up.path=mt_file.path;
    mt_file_up.filesize=filesize;
    mt_file_up.hash_string=sha;

   // log_file<<get_current_time()<<" | "<<" mtorrent file is been created. "<<endl;
    return mt_file_up;

 }

 mtorrent_file create_mtorrent_object(mtorrent_file mt_file){
    //log_file<<get_current_time()<<" | "<<"Creating mtorrent file object"<<endl;
    string path=mt_file.path;
    string filename=mt_file.filename;

    //creating mtorrent file object
    mtorrent_file mt_file_up;
    
    //get size of file
    ifstream fin(path,ios::in|ios::binary);
    if(!fin){
    	//log_file << get_current_time()<<" | "<< "mtorrent file does not exist" << endl;
        cout << "mtorrent file doesn't exist" << endl;
        return mt_file_up;
    }
    string line;
    vector<string> info,tokens;
    for(int i=0;i<5;i++){
        getline(fin,line);
        info.push_back(line);
    }
    //tracker 1 ip and port
    convertToArray(info[0],tokens);
    mt_file_up.tracker_1_ip=tokens[3];
    mt_file_up.tracker_1_port=tokens[5];
    
    //traker 2 ip and port
    vector<string> tokens2;
    convertToArray(info[1],tokens2);
    mt_file_up.tracker_2_ip=tokens2[3];
    mt_file_up.tracker_2_port=tokens2[5];
    
    //filename
    int index=filename.find_last_of(".");
    mt_file_up.filename=filename.substr(0,index);
    
    //filepath
    vector<string> tokens1;
    convertToArray(info[3],tokens1);
    mt_file_up.filesize=tokens1[1];

    //hash string
    vector<string> tokens3;
    convertToArray(info[4],tokens3);
    mt_file_up.hash_string=tokens3[1];

   //log_file<<get_current_time()<<" | "<<"mtorrent file object is been created. "<<endl;
    return mt_file_up;

 }