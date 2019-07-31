/*
Start client using - ​ ./client <CLIENT_IP>:<UPLOAD_PORT> <TRACKER_IP_1>:<TRACKER_PORT_1>
<TRACKER_IP_2>:<TRACKER_PORT_2> <log_file>
*/

#include<cstring>
#include<bits/stdc++.h>
#include<iostream>
#include<ctime>
#include<string.h>
#include<openssl/sha.h>
#include<fstream>
#include<sys/stat.h>
#include<sys/socket.h>
#include<stdio.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>

#include "client_utility.h" 

using namespace std;

ofstream log_file;

vector<pair<char,string>> filestatus;

#define MAX 524288

#define piece_size 2048

pthread_mutex_t lock1;

void serve_client(int new_socket, struct sockaddr_in address, mtorrent_file mt_file) {
    char buffer[MAX] = {0};

    log_file << get_current_time() << " | " << "New client joined : " << new_socket << endl;

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 

    /*1. Recieve command send+" "+filename*/
    bzero(buffer, MAX);
    read( new_socket , buffer, MAX);
    log_file << get_current_time() << " | " << "1. Recieved command: " << buffer << endl;

    /*2. Dummy send */
    string dummy = "Message Recieved";
    send(new_socket , dummy.c_str() , dummy.size() , 0 );
    log_file << get_current_time() << " | " << "2. Sending acknowledgement: " << dummy << endl;

    vector<string> tokens;
     convertToArray(buffer,tokens);

    if(tokens[0] == "send") {
        
        ifstream file_read;
        log_file << get_current_time() << " | " << "Prepare to send : " << tokens[1] << endl;
        
        file_read.open(tokens[1], ios::binary);
        if(!file_read){
        	log_file << get_current_time() << " | " << "File : " << tokens[1] << "can't be open. Sending can't be done" << endl;
        	cout<<"File can't open. Sending can't be done"<<endl;
        	return;
        }

       // cout << "File " << tokens[1] << " is prepared to send." << endl;

        /*3. Recieve token no */
        bzero(buffer, MAX);
        read( new_socket , buffer, MAX);
        log_file << get_current_time() << " | " << "3. Recieved token number: " << buffer << endl;

        char ch;
        long int counter = 0;

        char buzz[MAX];
        string s;
        bzero(buzz, MAX);
        
        while(file_read.get(ch)) {
        	counter++;
        }

        file_read.close();

        /*4. send size of data */
        string size_to_share = to_string(counter);
        send(new_socket , size_to_share.c_str() , size_to_share.size() , 0 );
        log_file << get_current_time() << " | " << "4. Sending size of file: " << size_to_share << endl;

        log_file << get_current_time() << " | " << "Sending : " << counter << " bytes" << endl;

        /*5. recieve piece_number */
        bzero(buffer, MAX);
        read( new_socket , buffer, MAX);
        int piece_number = atoi(buffer);
        log_file << get_current_time() << " | " << "5. Received piece number: " << buffer << endl;
        //cout << endl << "Sending file..." << endl;
        
        /*6. send data */
        file_read.open(tokens[1], ios::binary);  
        file_read.seekg(piece_number * piece_size, ios::beg);
        int i=0;

        while(file_read.get(ch)){
        	buzz[i]=ch;
        	if(i==1023){
        		//cout<< " .......... " << ( bytes_send / counter ) * 100 << "%" << " file is uploaded"<<endl;
        		buzz[i+1]='\0';
        		send(new_socket , buzz, 1024, 0);
        		//log_file << get_current_time() << " | " <<( bytes_send / counter ) * 100 << "%" << " file is uploaded"<<endl;
        		i=-1;
        	}
        	i++;
        }
        if(i>0){
        	//cout<< " .......... " << ( bytes_send / counter ) * 100 << "%" << " file is uploaded"<<endl;
        	buzz[i+1]='\0';
        	send(new_socket , buzz, i, 0);
        	//log_file << get_current_time() << " | " <<( bytes_send / counter ) * 100 << "%" << " file is uploaded"<<endl;
        }

        log_file << get_current_time() << " | " << "File : "<<tokens[1]<<" "<<piece_number<<" Sent! " << endl;

        //cout << "File : "<<tokens[1]<<" "<<piece_number<<" Sent! " << endl;

        file_read.close();
    }
}

void client_as_server(mtorrent_file mt_file) {

    log_file << get_current_time() << " | " << "Client as server : started" << endl;

    char buffer[MAX] = {0};

    int server_threads = 0;
    thread t[100];

    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        log_file << get_current_time() << " | " << "Error in calling socket" << endl;
        cout<<"Error to start client as server. Restart again !"<< endl; 
        exit(EXIT_FAILURE); 
    } 
    log_file << get_current_time() << " | " << "Client as server : success in calling socket" << endl; 

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        log_file << get_current_time() << " | " << "Error in calling setsockopt" << endl; 
        cout<<"Error to start client as server. Restart again !"<< endl;
        exit(EXIT_FAILURE);
    } 
    log_file << get_current_time() << " | " << "Client as server : success in calling setsockopt" << endl; 

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(atoi(mt_file.client_port.c_str())); 

    log_file << get_current_time() << " | " << "Client as server : listening on port : " << mt_file.client_port << endl; 
    
    if (bind(server_fd, (struct sockaddr *)&address,  sizeof(address))<0) {
        log_file << get_current_time() << " | " << "Error in calling bind" << endl;
        cout<<"Error to start client as server. Restart again !"<< endl;  
        exit(EXIT_FAILURE); 
    } 
    log_file << get_current_time() << " | " << "Client as server : success in calling bind" << endl;  

    if (listen(server_fd, 3) < 0) { 
        log_file << get_current_time() << " | " << "Error in calling listen" << endl;
        cout<<"Error to start client as server. Restart again !"<< endl;   
        exit(EXIT_FAILURE); 
    }

    log_file << get_current_time() << " | " << "Client as server : success in calling listen" << endl;   

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  (socklen_t*)&addrlen)) < 0) { 
            log_file << get_current_time() << " | " << "Client as server : failure1 in create thread" << endl; 
            cout<<"Error to start client as server. Restart again !"<<endl;
            exit(EXIT_FAILURE);
        } else {
            //Launch server threads
            t[server_threads] = thread(serve_client, new_socket, address, mt_file);        
            server_threads++;    

            log_file << get_current_time() << " | " << "Client as server : success in create thread" << endl;   
              
        }
    }

    log_file << get_current_time() << " | " << "Client as server : now join" << endl;   

    //Joinng server threads
    for(int i=0; i < server_threads; i++) {
        t[i].join();
    }
}

void establish_connection_with_one_peer(vector<string> info, int piece_number, mtorrent_file mt_file, string directory_name,
										vector<pair<int, int>> pieces) {


	log_file << get_current_time() << " | " << "Inside establish_connection_with_one_peer" << endl;
	log_file << get_current_time() << " | " << "No of pieces allocated : " << pieces.size();

    for(int p = 0; p < pieces.size(); p++) {

    	log_file << get_current_time() << " | "<< "Connecting with peer for piece number : " << pieces[p].first << endl;

    	pair<string,string> seeder_ip_port;
	    string seeder_ip, seeder_port;

	    get_IP_PORT(info[2], seeder_ip_port);

	    seeder_ip=seeder_ip_port.first;
	    seeder_port=seeder_ip_port.second;

	    log_file << get_current_time() << " | " << " seeder ip : " << seeder_ip << " seeder port : " << seeder_port << endl;

	    struct sockaddr_in address; 
	    int sock = 0, valread; 
	    struct sockaddr_in server_address;

	    char buffer[MAX] = {0}; 
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	    { 
	        log_file << get_current_time() << " | " << "Error in calling socket" << endl;
	        cout<<"Error in connecting with seeder "<<endl;
	        return; 
	    } 
	   
	    log_file << get_current_time() << " | " << "Socket called successfully" << endl;

	    memset(&server_address, '0', sizeof(server_address)); 
	   
	    server_address.sin_family = AF_INET; 
	    server_address.sin_port = htons(atoi(seeder_port.c_str()));
	       
	    // Convert IPv4 and IPv6 addresses from text to binary form 
	    if(inet_pton(AF_INET, seeder_ip.c_str(), &server_address.sin_addr)<=0)  
	    { 
	        log_file << get_current_time() << " | " << "Error in address specifications" << endl; 
	        cout<<"Error in connecting with seeder "<<endl;
	        return; 
	    } 

	    log_file << get_current_time() << " | " << "inet_pton called successfully" << endl;
	   
	    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
	    { 
	        log_file << get_current_time() << " | " << "Error in calling connection" << endl;
	        cout<<"Error in connecting with seeder "<<endl;
	        return; 
	    }

	    log_file << get_current_time() << " | " << "connect called successfully" << endl;

	    /* 1. send file name */
	    string info_to_share = "send " + mt_file.filename;
	    bzero(buffer, MAX);
	    strcpy(buffer, info_to_share.c_str());
	    send(sock , buffer , strlen(buffer) , 0 );
	    log_file << get_current_time() << " | " << "1. Sending command: " << buffer << endl;

	    /*2 dummy recieve (acknowledgement)*/
	    bzero(buffer, MAX);
	    read( sock , buffer, MAX);
	    log_file << get_current_time() << " | " << "2. Recieved acknowledgement: " << buffer << endl;

	    /*3. send peer no */
	    info_to_share = "Dummy send";
	    bzero(buffer, MAX);
	    strcpy(buffer, info_to_share.c_str());
	    send(sock , buffer , strlen(buffer) , 0 );
	    log_file << get_current_time() << " | " << "3. Sending : " << buffer << endl;

	    /*4. recieved size of data */
	    int size_of_file;
	    bzero(buffer, MAX);
	    read( sock , buffer, MAX);
	    log_file << get_current_time() << " | " << "4. Recieved size of file: " << buffer << endl;
	    size_of_file = atoi(buffer);

	    ofstream file_write;
	    string file_name = mt_file.filename;

	    file_write.open( directory_name + "/" + file_name,ios::out | ios::app);

    	pthread_mutex_lock(&lock1);

    	file_write.seekp( piece_size * pieces[p].first, ios::beg);

    	/*5. send piece number  */
		info_to_share = to_string(pieces[p].first);
		bzero(buffer, MAX);
		strcpy(buffer, info_to_share.c_str());
		send(sock , buffer , strlen(buffer) , 0 );
		log_file << get_current_time() << " | " << "5. Sending piece number: " << buffer << endl;


		/*6. recieved data */
		int n = pieces[p].second / 1024;
		int r = pieces[p].second % 1024;
		int i=0;
		while(n--){
			bzero(buffer, MAX);
			read( sock , buffer, 1024);

			file_write.write(buffer, 1024);
		}
		bzero(buffer, MAX);
		read( sock , buffer, r);
		file_write.write(buffer, r);

		pthread_mutex_unlock(&lock1);

		file_write.close();
	
    }

}

void establish_connection_with_peers(string command, mtorrent_file mt_file, vector<string> seeders, string directory_name) {

    int peer_threads = 0;
    thread t[100];

    log_file << get_current_time() << " | " << "inside establish_connection_with_peers" << endl;

    log_file << get_current_time() << " | " << "size of seeders : " << seeders.size() << endl;

    cout << "No of seeders available : " << seeders.size() << endl;

    cout << "File: "<<mt_file.filename<<" is preparing to download."<<endl;

    log_file << get_current_time() << " | " << "File: "<<mt_file.filename<<" is preparing to download."<<endl;

    int filesize = atoi(mt_file.filesize.c_str());

    int n = floor(filesize * 1.0 / piece_size);
    int r = filesize % piece_size;

    for(int i=0; i < seeders.size() ; i++) {

    	log_file << get_current_time() << " | " << "Working for seeder : " << i << endl;

    	vector<pair<int, int>> pieces;
    	int no_of_seeders = seeders.size();
    	int p = i;
    	
    	while(p < n + 1) {
    		if(p == n) {
    			log_file << get_current_time() << " | " << "Pushing piece no : " << p << " & size : " << r << endl;
    			pieces.push_back(make_pair(p, r));
    		} else {
    			log_file << get_current_time() << " | " << "Pushing piece no : " << p << " & size : " << piece_size << endl;
    			pieces.push_back(make_pair(p, piece_size));
    		}

    		p += no_of_seeders;
    	}

        vector<string> info;
        convertToArray(seeders[i],info);

        log_file << get_current_time() << " | " << "Seeder : " << info[0] << " -- " << info[1] << " -- " << info[2] << endl;

        t[peer_threads] = thread(establish_connection_with_one_peer, info, i, mt_file, directory_name, pieces);        
        peer_threads++;
        
    }

    for(int i=0; i < peer_threads; i++) {
        t[i].join();
    }

    cout << "File downloaded!" << endl;

	auto it=find(filestatus.begin(),filestatus.end(),make_pair('D',mt_file.filename));
	if(it!=filestatus.end()){
		filestatus[it-filestatus.begin()]=make_pair('S',mt_file.filename);
	}
}

vector<string> establish_connection_with_tracker(string command, mtorrent_file mt_file, int &flag, int tracker_no) {

    vector<string> seeders;

    log_file << get_current_time() << " | " << "Inside establish_connection_with_tracker" << endl;

    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in server_address;

    string info_to_share; 
    
    char buffer[MAX] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        log_file << get_current_time() << " | " << "Error in calling socket" << endl;
        cout<<"Error! In connection with tracker"<<endl;
        seeders.push_back("error");

        if(tracker_no == 1)
        	flag = 101;
        else
        	flag = 102;

        return seeders; 
    } 
   
    log_file << get_current_time() << " | " << "Socket called successfully" << endl;

    memset(&server_address, '0', sizeof(server_address)); 
   
    server_address.sin_family = AF_INET; 
    if(tracker_no == 1)
    	server_address.sin_port = htons(atoi(mt_file.tracker_1_port.c_str()));
    else
       server_address.sin_port = htons(atoi(mt_file.tracker_2_port.c_str()));

    // Convert IPv4 and IPv6 addresses from text to binary form 

   	string tracker_ip;

   	if(tracker_no == 1)
   		tracker_ip = mt_file.tracker_1_ip;
   	else
   		tracker_ip = mt_file.tracker_2_ip;

    if(inet_pton(AF_INET, tracker_ip.c_str(), &server_address.sin_addr)<=0)  
    { 
        log_file << get_current_time() << " | " << "Error in address specifications" << endl;
        cout<<"Error! In connection with tracker"<<endl; 
        seeders.push_back("error");

        if(tracker_no == 1)
        	flag = 101;
        else
        	flag = 102;

        return seeders; 
    } 

    log_file << get_current_time() << " | " << "Inet_pton called successfully" << endl;
   
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    { 
        log_file << get_current_time() << " | " << "Error in calling connection" << endl;
        cout<<"Error! In connection with tracker"<<endl;
        seeders.push_back("error");

        if(tracker_no == 1)
        	flag = 101;
        else
        	flag = 102;

        return seeders; 
    }

    log_file << get_current_time() << " | " << "Connect called successfully" << endl;

    if(command == "share") {

        info_to_share = command + " "
                        + mt_file.filename + " " 
                        + mt_file.hash_string + " "
                        + mt_file.client_ip + ":"
                        + mt_file.client_port;
        log_file << get_current_time() << " | " << "Share : details sent to tracker: " <<info_to_share<< endl;

        send(sock , info_to_share.c_str() , info_to_share.size() , 0 );

        cout << "File shared with trackers!" << endl;

        flag = 1;

        return seeders; 

    } else if(command == "get") {

        log_file << get_current_time() << " | " << "get : fetching details" << endl;

        info_to_share = command + " "
                        + mt_file.filename + " ";

        log_file << get_current_time() << " | " << "get : info_to_share : " << info_to_share << endl;

        send(sock , info_to_share.c_str() , info_to_share.size() , 0 );

        log_file << get_current_time() << " | " << "get : details sent to tracker " << endl;

        read( sock , buffer, MAX);

        log_file << get_current_time() << " | " << "get : " << buffer;
        log_file << get_current_time() << " | " << "get : details recieved from tracker " << endl;

        seeders = parse_seeder_list(buffer);

        flag = 1;

        return seeders;

    } else if(command == "remove") {
    	log_file << get_current_time() << " | " << "remove : sending the filename" << endl;
    	
    	info_to_share = command + " " + mt_file.filename +" "+mt_file.client_ip+" "+mt_file.client_port;

    	log_file << get_current_time() << " | " << "remove : info to share : " << info_to_share << endl;

    	send(sock , info_to_share.c_str() , info_to_share.size() , 0 );

    	log_file << get_current_time() << " | " << "remove : details sent to tracker " << endl;

        log_file << get_current_time() << " | " << "remove : details recieved from tracker"<<endl;

        cout<<"File : "<<mt_file.filename<<" information is removed from the tracker " <<endl;

        flag = 1;

        return seeders;

    }
    return seeders;
    
}



int process_request(mtorrent_file mt_file, char buffer[MAX]) {

	int flag;
    
    log_file << get_current_time() << "| " << "Inside process_request" << endl;

    vector<string> tokens;
    
    convertToArray(buffer,tokens);

    if(strcmp(tokens[0].c_str(), "exit") == 0) {
            return -1;
    }

    string command = tokens[0];

    if(strcmp(command.c_str(), "share") == 0) {
        if(tokens.size() < 2) {
            cout << "Error : Please provide filepath!" << endl; 
            log_file << get_current_time() << " | " <<  "Error : in providing filepath!" << endl;
            cout<< "share <filepath> <filename>" << endl;
            return 0; 
        } else if(tokens.size() < 3) {
            cout << "Error : Please provide mtorrent file name!" << endl;
            log_file << get_current_time() << " | " <<  "Error : in providing filename!" << endl;
            cout<< "share <filepath> <filename>" << endl;
            return 0;
        } else {
            log_file << get_current_time() << " | " << "Sharing" << endl;

            mt_file.path=tokens[1];
            mt_file.filename=tokens[2];
        
            mtorrent_file mt_file_up = create_mtorrent_file(mt_file);

            if(mt_file_up.filename==""){
            	return -1;
            }

            vector<string> seeders = establish_connection_with_tracker(command, mt_file_up, flag, 1);

            if(flag == 101) { 		// Tracker 1 is down

            	cout << endl << "Flag value : " << flag << endl;
            	cout << endl << "Tracker 1 is down..." << endl;
            	cout << endl << "Requesting tracker 2..." << endl;

            	seeders = establish_connection_with_tracker(command, mt_file_up, flag, 2);

            	if(flag == 102) {
            		cout << endl << "Tracker 2 is also down..." << endl;
            		return -1;
            	}
            }

            if(seeders.size()>0 && seeders[0]=="error"){
            	return -1;
            }
        }
        
        return 0;
    
    } else if(strcmp(command.c_str(), "get") == 0) {
        
        if(tokens.size() < 2) {
            log_file << get_current_time() << " | " << "Error : In providing mtorrent file path!" << endl;
            cout << "Error : Please provide mtorrent file path!" << endl;
            cout<< "get <filepath_of_mtorrent> <destination_path>" << endl;
            return 0;
        } else if(tokens.size() < 3) {
            log_file << get_current_time() << " | " << "Error : Please provide destination file path!" << endl;
            cout << "Error : Please provide destination file path!" << endl;
            cout<< "get <filepath_of_mtorrent> <destination_path>" << endl;
            return 0;
        } else {
            log_file << get_current_time() << " | " << "get : Downloading" << endl;

            string mtorrent_file_path = tokens[1];

            log_file << get_current_time() << " | " << "get : create mtorrent object" << endl;

            mtorrent_file mt_file_new;
            mt_file_new.path = mtorrent_file_path;
            int index = mtorrent_file_path.find_last_of("/\\");
            mt_file_new.filename = mtorrent_file_path.substr(index+1);

            index = mt_file_new.filename.find_last_of(".");

            filestatus.push_back( make_pair('D',mt_file_new.filename.substr(0,index) ));

            mtorrent_file mt_file_up = create_mtorrent_object(mt_file_new);

            if(mt_file_up.filename==""){
            	return -1;
            }

            log_file << get_current_time() << " | " << "get : object created" << endl;

            vector<string> seeders = establish_connection_with_tracker(command, mt_file_up, flag, 1);

            if(flag == 101) { 		// Tracker 1 is down

            	cout << endl << "Tracker 1 is down..." << endl;
            	cout << endl << "Requesting tracker 2..." << endl;

            	seeders = establish_connection_with_tracker(command, mt_file_up, flag, 2);

            	if(flag == 102) {
            		cout << endl << "Tracker 2 is also down..." << endl;
            		return -1;
            	}
            }

            if(seeders[0]=="error"){
            	return -1;
            }

            log_file << get_current_time() << " | " << "get : seeders recieved" << endl;

            log_file << get_current_time() << " | " << "get : start connecting with peers" << endl;

            establish_connection_with_peers(command, mt_file_up, seeders, tokens[2]);

            log_file << get_current_time() << " | " << "get : closing connection with peers" << endl;
        }

        return 0;
    } else if(strcmp(command.c_str(), "show") == 0) {

    	log_file << get_current_time() << " | " << "show downloads" << endl;

    	cout<<"----- Files Status -----" << endl;
    	
    	if(filestatus.size()==0)
    		cout<<"EMPTY"<<endl;
    	
    	for(auto it:filestatus){
    		cout<<it.first<<" "<<it.second<<endl;
    	
    	}
    
    } else if(strcmp(command.c_str(), "remove") == 0){
    	if(tokens.size() < 2) {
            cout << "Error : Please provide mtorrent filename!" << endl; 
            log_file << get_current_time() << " | " <<  "Error : in providing mtorrent filename!" << endl;
            cout<< "remove <filename.mtorrent>" << endl;
            return 0; 
        } else {
            log_file << get_current_time() << " | " <<  "Sending the filename to tracker!" << endl;

            mtorrent_file mt_file_up;
            int index=tokens[1].find_last_of(".");
            mt_file_up.filename=tokens[1].substr(0,index);
            
            mt_file_up.tracker_1_ip=mt_file.tracker_1_ip;
            mt_file_up.tracker_1_port=mt_file.tracker_1_port;
            mt_file_up.tracker_2_ip=mt_file.tracker_2_ip;
            mt_file_up.tracker_2_port=mt_file.tracker_2_port;
            mt_file_up.client_ip=mt_file.client_ip;
            mt_file_up.client_port=mt_file.client_port;

            vector<string> seeders = establish_connection_with_tracker(command, mt_file_up, flag, 1);

            if(flag == 101) { 		// Tracker 1 is down

            	cout << endl << "Tracker 1 is down..." << endl;
            	cout << endl << "Requesting tracker 2..." << endl;

            	seeders = establish_connection_with_tracker(command, mt_file_up, flag, 2);

            	if(flag == 102) {
            		cout << endl << "Tracker 2 is also down..." << endl;
            		return -1;
            	}
            }

            if(seeders.size()>0 && seeders[0]=="error"){
            	return -1;
            }

           	remove(tokens[1].c_str());

        	log_file << get_current_time() << " | " << "remove : mtorrent file from persistent database"<<endl;

        	cout<<"File : "<<mt_file.filename<<".motorrent is removed from the database " <<endl;
            
            return 0;
        }

    }
    return 0;
}


void local_client(mtorrent_file mt_file) {
    char buffer[MAX] = {0};
    while(1) {

        bzero(buffer, MAX);
        cout << "Command$";
        cin.getline(buffer, MAX);
        int status = process_request(mt_file, buffer);

        if(status == 101) { // Tracker 1 is down

        }
    }
}
int main(int argc, char *argv[]){

	//mutex for synchronisation while writing into file
	if (pthread_mutex_init(&lock1, NULL) != 0) 
    { 
    	cout<<"Mutex init has failed"<<endl;
        return 1; 
    } 

    //client as server thread and process request thread
    int main_threads = 0;
    thread t[100];

    if(argc!=5){
        cout<<"Incomplete information."<<endl;
        cout<<"Start client using - ​ ./client <CLIENT_IP>:<UPLOAD_PORT> <TRACKER_IP_1>:<TRACKER_PORT_1> <TRACKER_IP_2>:<TRACKER_PORT_2> <log_file> "<<endl;
        return 1;
    }
    cout<<endl;
    cout<<" ---------- Client Started ---------- "<<endl;
    
    //log file
    log_file.open(argv[4],ios::out|ios::app);
    if(log_file){
        log_file<< get_current_time()<<" | "<<" Client started using: "<<argv[0]<<" "<<argv[1]<<" "<<argv[3]<<" "<<argv[4]<<endl;
    }

    pair<string,string> client_ip_port;
    pair<string,string> tracker1_ip_port;
    pair<string,string> tracker2_ip_port;
    get_IP_PORT(argv[1],client_ip_port);
    get_IP_PORT(argv[2],tracker1_ip_port);
    get_IP_PORT(argv[3],tracker2_ip_port);

    //log file maintain information 
    if(log_file){
        log_file<<get_current_time() <<" | "<<"Information in mtorrent file: "<<endl;
        log_file<<get_current_time() <<" | "<<"Client IP: "<<client_ip_port.first<<endl;
        log_file<<get_current_time() <<" | "<<"Client Port: "<<client_ip_port.second<<endl;
        log_file<<get_current_time() <<" | "<<"Tracker1 IP: "<<tracker1_ip_port.first<<endl;
        log_file<<get_current_time() <<" | "<<"Tracker1 Port: "<<tracker1_ip_port.second<<endl;
        log_file<<get_current_time() <<" | "<<"Tracker2 IP: "<<tracker2_ip_port.first<<endl;
        log_file<<get_current_time() <<" | "<<"Tracker2 IP: "<<tracker2_ip_port.second<<endl;
    }

    //mtorrent file object containing all information
    mtorrent_file mt_file;
    mt_file.client_ip=client_ip_port.first;
    mt_file.client_port=client_ip_port.second;
    mt_file.tracker_1_ip=tracker1_ip_port.first;
    mt_file.tracker_1_port=tracker1_ip_port.second;
    mt_file.tracker_2_ip=tracker2_ip_port.first;
    mt_file.tracker_2_port=tracker2_ip_port.second;

    //Launch a thread to make client behave as server
    t[main_threads]= thread(client_as_server, mt_file);
    main_threads++;

    cout << "Commands: " <<endl;
    
    cout <<"1.	To share a file with tracker" <<endl;
    cout << "Command$	share <filepath> <filename>" <<endl;

    cout<<"2.	To get a file from tracker using mtorrent file" <<endl; 
    cout << "Command$	get <filepath_of_mtorrent file> <destination_path>" <<endl;

    cout<<"3.	To show all the files that are downloading[D] and downloaded[S]" <<endl;
    cout << "Command$	show downloads" <<endl;

    cout<<"4.	To remove file from tracker and corresponding mtorrent file from database" <<endl;
    cout << "Command$	remove <filename.mottrent>" <<endl;
    
    //Launch a thread to get command to perform some process request
    t[main_threads] = thread(local_client, mt_file);        
    main_threads++;

    //Joining thread
    for(int i=0; i < main_threads; i++) {
        t[i].join();
    }

    log_file.close();


    pthread_mutex_destroy(&lock1);


    return 0;
}