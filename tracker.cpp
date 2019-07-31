#include<iostream>
#include<string>
#include<bits/stdc++.h>
#include<fstream>
#include<sys/socket.h>
#include<stdio.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>

#include "tracker_utility.h"

#define MAX 524288
using namespace std;

ofstream log_file;
static int server_thread_no = 0;
thread server_threads[100];

void establish_connection_with_other_tracker(string command,vector<string> tokens,mtorrent_file mt_file){

    log_file << get_current_time() << " | " << "Inside establish_connection_with_other_tracker" << endl;

    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in server_address;

    string info_to_share; 
    
    char buffer[MAX] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        log_file << get_current_time() << " | " << "Error in calling socket" << endl;
        return;
    } 
   
    log_file << get_current_time() << " | " << "Socket called successfully" << endl;

    memset(&server_address, '0', sizeof(server_address)); 
   
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(mt_file.tracker_2_port.c_str()));

    // Convert IPv4 and IPv6 addresses from text to binary form 

    string tracker_ip;

    tracker_ip = mt_file.tracker_2_ip;

    if(inet_pton(AF_INET, tracker_ip.c_str(), &server_address.sin_addr)<=0)  
    { 
        log_file << get_current_time() << " | " << "Error in address specifications" << endl;
        return; 
    } 

    log_file << get_current_time() << " | " << "Inet_pton called successfully" << endl;
   
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    { 
        log_file << get_current_time() << " | " << "Error in calling connection" << endl;
        return;
        
    }

    log_file << get_current_time() << " | " << "Connect called successfully" << endl;

    if(command == "share") {
        log_file<<get_current_time()<<" | "<<"Share command found"<<endl;

        string file=tokens[1];
        string hash_string=tokens[2];
        string client_ip_port=tokens[3];
        info_to_share = "sync "
                        +command + " "
                        + file + " " 
                        + hash_string + " "
                        + client_ip_port;
        log_file << get_current_time() << " | " << "Share : details sent to other_tracker: " <<info_to_share<< endl;

        send(sock , info_to_share.c_str() , info_to_share.size() , 0 );

        cout << "File shared with other trackers!" << endl;
    }
}

void createClientThread(int new_socket, struct sockaddr_in tracker1, mtorrent_file mt_file){
    log_file<<get_current_time()<<" | "<<"Creating a thread"<<endl;

    char buffer[1024]={0};

    log_file<<get_current_time()<<" | "<<"New client joined "<<new_socket<<endl;

    tracker1.sin_family=AF_INET;
    tracker1.sin_addr.s_addr=INADDR_ANY;    
    
    bzero(buffer,1024);
    read(new_socket,buffer,1024);
    
    log_file<<get_current_time()<<" | "<<"new command found : "<<buffer;
    
    vector<string> tokens;
    convertToArray(buffer,tokens);
    
    if(tokens.size()==0){
    	log_file<<get_current_time()<<" | "<<"No command found!"<<endl;
    	return;
    }
    
    string command=tokens[0];
    log_file<<get_current_time()<<" | "<<"Command: "<<command<<endl;

    if(command=="exit"){
    	log_file<<get_current_time()<<" | "<<"Exit command found"<<endl;
    	return;
    }
    else if(command=="share"){
    	log_file<<get_current_time()<<" | "<<"Share command found"<<endl;
    	log_file<<get_current_time()<<" | "<<"Wrtting in seeder_file: "<<mt_file.seeder_file<<endl;

    	ofstream seeder_file;
        seeder_file.open(mt_file.seeder_file.c_str(), ios::out | ios::app);
        for(int i=1; i < tokens.size(); i++) {
            seeder_file<< tokens[i]<<" ";
            log_file<<get_current_time()<<" | "<< tokens[i]<<" "<< endl;  
        }
        
        seeder_file << endl;
        seeder_file.close();
        bzero(buffer, 1024);

        establish_connection_with_other_tracker(command,tokens,mt_file);
    }
    else if(command=="get"){
    	log_file << get_current_time() <<"get command found"<<endl;
    	string requested_file = tokens[1];

        ifstream file_read;
        file_read.open(mt_file.seeder_file);
        string info_to_share;

        while (file_read.peek() != EOF) {
            bzero(buffer, 1024);
            file_read.getline(buffer, 1024);
            vector<string> seeder;
            convertToArray(buffer,seeder);

            if(requested_file == seeder[0]) {
                info_to_share = info_to_share + buffer + "\n";
            }
        }
        
        send(new_socket , info_to_share.c_str() , info_to_share.size() , 0 ); 
    }
    else if(command=="remove"){
        log_file << get_current_time() << " | "<<"remove command found "<<endl;

        string filename=tokens[1];
        string client_ip=tokens[2];
        string client_port=tokens[3];

        log_file << get_current_time() << " | "<<"remove : removing file "<<filename<<" from seeder list."<<endl;

        //to read seeder file
        ifstream fin;
        fin.open(mt_file.seeder_file.c_str(),ios::in);

        //to write onto new file
        ofstream fout;
        fout.open("temp.txt",ios::out);

        string line;
        int line_no=0;
        while (getline(fin, line)) {
            if (line.find(filename) != string::npos && line.find(client_ip) != string::npos && line.find(client_port) != string::npos) {
                continue;
            }
            else
                fout << line << endl;
        }
        
        fin.close();
        fout.close();
        
        remove(mt_file.seeder_file.c_str());
        rename("temp.txt",mt_file.seeder_file.c_str());

        bzero(buffer, 1024);

    }
    else if(command=="sync"){
        if(tokens[1]=="share"){
        log_file<<get_current_time()<<" | "<<"Share command found to sync with tracker"<<endl;
        log_file<<get_current_time()<<" | "<<"Wrtting in seeder_file: "<<mt_file.seeder_file<<endl;

        ofstream seeder_file;
        seeder_file.open(mt_file.seeder_file.c_str(), ios::out | ios::app);
        for(int i=2; i < tokens.size(); i++) {
            seeder_file<< tokens[i]<<" ";
            log_file<<get_current_time()<<" | "<< tokens[i]<<" "<< endl;  
        }
        
        seeder_file << endl;
        seeder_file.close();
        bzero(buffer, 1024);
        }
    }
}
int main(int argc,char *argv[]){
	if(argc!=5){
		cout<<"Incomplete Information."<<endl;
		cout<<"Start Tracker 1 using: ./tracker <my_tracker_ip>:<my_tracker_port> <other_tracker_ip>:<other_tracker_port> <seederlist_file> <log_file>."<<endl;
		exit(0);
	}
	//log file for tracker starting
	log_file.open(argv[4],ios::out|ios::app);
	if(log_file){
		log_file<<get_current_time()<<" | "<<"Start Tracker using: "<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<" "<<argv[4]<<endl;
	}

    cout<<" ---------- Tracker started ---------- "<< endl;

	string seeder_file=argv[3];
	pair<string,string> tracker_ip_port;
	pair<string,string> other_tracker_ip_port;
	getTrackerIP_PORT(argv[1],tracker_ip_port);
	getTrackerIP_PORT(argv[2],other_tracker_ip_port);

	//log file maintain information 
	if(log_file){
		log_file<<get_current_time()<<" | "<<"Tracker IP: "<<tracker_ip_port.first<<endl;
		log_file<<get_current_time()<<" | "<<"Tracker Port: "<<tracker_ip_port.second<<endl;
		log_file<<get_current_time()<<" | "<<"Other Tracker IP: "<<other_tracker_ip_port.first<<endl;
		log_file<<get_current_time()<<" | "<<"Other Tracker Port: "<<other_tracker_ip_port.second<<endl;
		log_file<<get_current_time()<<" | "<<"Seeder_file: "<<seeder_file<<endl;

	}

	//m_torrent file for tracker
	mtorrent_file mt_file;
	mt_file.tracker_1_ip=tracker_ip_port.first;
	mt_file.tracker_1_port=tracker_ip_port.second;
	mt_file.tracker_2_ip=other_tracker_ip_port.first;
	mt_file.tracker_2_port=other_tracker_ip_port.second;
	mt_file.seeder_file=seeder_file;

	//connecting multiple clients with tracker
    int k;
    socklen_t len;
    int sock_desc,temp_sock_desc;
    struct sockaddr_in tracker1,client;

    memset(&tracker1,0,sizeof(tracker1));
    memset(&client,0,sizeof(client));

    sock_desc=socket(AF_INET,SOCK_STREAM,0);
    if(sock_desc==-1)
    {
        log_file<<get_current_time()<<" | "<<"Error in socket creation"<<endl;
        exit(EXIT_FAILURE);
    }

    log_file<<get_current_time()<<" | "<<"Success in calling socket"<<endl;

    tracker1.sin_family=AF_INET;
    tracker1.sin_addr.s_addr=INADDR_ANY;    
    //tracker1.sin_addr.s_addr=inet_addr("192.168.0.113");
    //tracker1.sin_addr.s_addr=inet_addr(tracker_ip_port.first.c_str);
    tracker1.sin_port=htons( atoi( tracker_ip_port.second.c_str() ) );

    k=bind(sock_desc,(struct sockaddr*)&tracker1,sizeof(tracker1));
    if(k==-1)
    {
        log_file<<get_current_time()<<" | "<<"Error in binding"<<endl;
        exit(EXIT_FAILURE);
    }
    log_file<<get_current_time()<<" | "<<"Bind successfully"<<endl;
    k=listen(sock_desc,20);
    if(k==-1)
    {
        log_file<<get_current_time()<<" | "<<"Error in listening"<<endl;
        exit(EXIT_FAILURE);
    }
    log_file<<get_current_time()<<" | "<<"Listen called successfully"<<endl;

    len=sizeof(client);//VERY IMPORTANT

    while(1)
    {
    	temp_sock_desc = accept(sock_desc,(struct sockaddr*)&client,&len);
        if(temp_sock_desc==-1)
    	{
        	log_file<<get_current_time()<<" | "<<"Error in temporary socket creation"<<endl;
        	exit(EXIT_FAILURE);
    	}
    	log_file<<get_current_time()<<" | "<<"Temporary socket created successfully"<<endl;

    	server_threads[server_thread_no] = thread(createClientThread, temp_sock_desc, tracker1, mt_file);        
        server_thread_no++;
    }

	for(int i=0; i<server_thread_no; i++) {
        server_threads[i].join();
    }
    
    log_file.close();

	return 0;
}