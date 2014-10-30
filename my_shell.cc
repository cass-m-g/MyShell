// =============== BEGIN ASSESSMENT HEADER ================
// assn#6 my_shell.cc
// Assignment #6 CS100
// Cassandra Garner
// SID: 861076158
// ================== END ASSESSMENT HEADER ===============

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sstream>
#include <sys/wait.h>
#include <libgen.h>
#include <signal.h>

using namespace std;

void child(string str, int time, bool inPipe);
void parent(int pid);
vector<string> paths;
struct sigaction act;
int pid;

void sighandler(int signum){
	if(signum == SIGINT&& pid> 0 ){
		kill(pid, 9);
	}
	pid = -2;
	cout << endl;
}

char* get_program(string str){
	/*int i = 0, index = 0;
	for(;i < str.length() && str.at(i) != ' ' ; ++i){
		if(str.at(i) == '/'){
			index = i;
		}
	}

	string ret = str.substr(index, str.find(" ") - index);
	return ret;
	*/
	return getenv(str.c_str());
}

int input_redirect(string file){
	int in = open(file.c_str(), O_RDONLY);
	if(in < 0){
		cout << file << " did not open" << endl;
		return in;
	}


	if(dup2(in, 0 ) < 0){
		cerr << " error " << endl;
	}



	return in;
}

int output_redirect(string file){
	int in = open(file.c_str(), O_CREAT | O_WRONLY, S_IRWXU);
	if(in < 0){
		cout << file << " did not open" << endl;
		return in;
	}

	if(dup2(in, 1 ) < 0){
		cerr << " error " << endl;
	}

	return in;

}

int check_redirect(string str, string& f){
	int index = 0, index2 = 0;
	index = str.find("<");
	index2 = str.find(">");
	if(index > 0){
		stringstream ss;
		ss << str.substr(index+1);
		ss >> f;
		input_redirect(f);
	}
	if(index2 > 0 ){
		stringstream ss;
		ss << str.substr(index2+1);
		string file;
		ss >> file;
		return output_redirect(file);
	}
}

string getArgs(string str){
	stringstream ss;
	ss << str;
	string s;
	ss >> s;
	if(ss >> s && s != ">" && s!= "<")
		return s;

	return "";
}

void check_back(string str){
	int index = str.find("&");

	if(index >= 0 ){
		int pid = fork();
		switch(pid){
			case -1:
				cout << "Fork Failed." << endl;
				break;
			case 0:
				child(str, 1, false);
				exit(0);
				break;
			default:
				cout << pid << endl;
				parent(pid);
				break;
		}
		
	}

}


bool check_pipe(string str, int time){
	vector<int> v;
	int index = 0;
	v.push_back(-1);
	while(str.find("|", index +1) != -1){
		index = str.find("|", index +1);
		v.push_back(index);
	}
	if(v.size() == 1){
		return false;
	}

	v.push_back(v.size() -1);

	for(int i = 2 ; i < v.size(); ++i){
		string first = str.substr(v.at(i -2) + 1, v.at(i - 1) - v.at(i -2) -1);
		string second = str.substr(v.at(i -1) + 1, v.at(i) - v.at(i -1) - 1); 

		int pfd[2];
		char buf;
		if(pipe(pfd) == -1){
			cout << "Pipeing error." << endl;
			return true;
		}
		
		int cpid = fork();
		pid = cpid;
		switch(cpid){
			case -1:
				cout << "fork failed." << endl;
				return true;
			case 0:
				close(pfd[1]);
				if(dup2(pfd[0], 0) < 0){
					cerr << " error " << endl;
				}

				child(second, time, true);
				close(pfd[0]);
				exit(0);
				pid = -2;
				break;
			default:
				close(pfd[0]);
				if(dup2(pfd[1], 1) < 0){
					cerr << " error " << endl;
				}
				child(first, time, true);
				close(pfd[1]);
				parent(cpid);
				break;
		}
	}

	return true;

}



void child(string str, int time, bool inPipe){
	stringstream ss;
	ss << str;
	string front;
	ss >> front;
	string begin= front;
	char* front1 = basename((char*)begin.c_str());
	string args = getArgs(str);
	
	string arg2 = "";
	ss >> arg2;
	ss >> arg2;


	if(time == 0){
		check_back(str);
	}
	
	if(inPipe || !check_pipe(str, time) ){
		string f = "";
		int file = check_redirect(str, f);
		if(file > 0)
			close(file);

		if(args != "" && execl(begin.c_str(), front1, args.c_str(),  (char*) 0) != -1)
			cout << str << ": command not found" << endl;

		else if(execl(begin.c_str(), front1, (char*) 0) != -1)
			cout << str << ": command not found" << endl;
		else{
			for(int i = 0; i< paths.size(); ++i){
				string new_front = paths.at(i) + "/" + begin;
				if(args != "" && execl(new_front.c_str(), front1, args.c_str(),  (char*) 0) != -1)
					return;

				else if(execl(new_front.c_str(), front1, (char*) 0) != -1)
					return;
			}
			cout << str << ": command not found" << endl;
		}

	}
	
}

void parent(int pid){
	int stat;
	do{
		int child_pid = waitpid(pid, &stat, WUNTRACED | WCONTINUED);
	}while(!WIFEXITED(stat) && !WIFSIGNALED(stat));
}

void handle_command(string str){
		if(str == "quit" || str == "q")
			exit(0);
		if(str == "")
			return;
		pid = fork();
		switch(pid){
			case -1:
				cout << "Fork Failed." << endl;
				break;
			case 0:
				child(str, 0, false);
				exit(0);
				pid = -2;
				break;
			default:
				parent(pid);
				break;
		}	

		pid = -2;

}

vector<string> get_paths(char* ch){
	string str = string(ch);
	
	vector<string> vec;
	int found = -1, first = 0;

	do{
		first = found+1;
		found = str.find(":", found+1);
		vec.push_back(str.substr(first, found - first));
	}while(found > 0); 

	
	return vec;
}
		


int main(){
	string command;
//	act.sa_sigaction = sighandler;
	act.sa_handler = sighandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);
	pid = -2;

	char* path =  getenv("PATH");
	paths = get_paths(path);
	while(1){
		cin.clear();
		cout << "%";
		getline(cin, command);
		handle_command(command);
	}

	return 0;
}
