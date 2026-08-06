#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

using namespace std;

struct Case
{
	int caseID;
	string param;
	int cycleTimes;
	bool isMultiInstance;
	int multiNumber;
	vector<string> multiParam;
	int timeInterval;
};

struct Runner
{
	string name;
	int totalCycleTimes;
	vector<Case> cases;
};

vector<string> splitString(string str, char ch)
{
	vector<string> str_list;
	int comma_n = 0;
	do
	{
		string tmp_s;
		comma_n = str.find(ch);
		if(-1 == comma_n)
		{
			tmp_s = str.substr(0, str.length());
			str_list.push_back(tmp_s);
			break;
		}
		tmp_s = str.substr(0, comma_n);
		str.erase(0, comma_n+1);
		str_list.push_back(tmp_s);
	}
	while(true);
	return str_list;
}


vector<Runner> readConfigFile(string filePath)
{
	vector<Runner> runners;
	Runner firstRunner;
	runners.push_back(firstRunner);
	
	int runnerIndex = 0;
	int line = 0;
	
	ifstream file(filePath.c_str());
	if(!file)
	{
		cout<<"Config file open failed!\n"<<"file path: "<<filePath<<endl;
	}
	
	string buff;
	while(getline(file, buff))
	{
		if(buff.length() < 3) // empty line means end of a runner
		{
			Runner runner;
			runners.push_back(runner);
			
			runnerIndex++;
			line = 0;
		}
		else //params still in a runner
		{
			if(line == 0) //first line in a runner means Runner name
			{
				line++;
				runners[runnerIndex].name = buff.substr(1, buff.length()-3);
			}
			else if(line == 1) //second line in a runner means Runner total cycle times
			{
				line++;
				runners[runnerIndex].totalCycleTimes = atoi(buff.substr(1, buff.length()-3).c_str());
			}
			else //other lines meas Runner's cases' param
			{
				line++;
				string caseBuff;
				if(file.eof())
				{
					caseBuff = buff.substr(1, buff.length()-2);
				}else
				{
					caseBuff = buff.substr(1, buff.length()-3);;
				}
				Case aCase;
				
				char ch = ',';
				vector<string> params = splitString(caseBuff, ch);
				if(params.size() < 6)
				{
					cout<<"case param less than 6, miss necessary param! "<<runners[runnerIndex].name<<endl;
				}
				else
				{
					aCase.caseID = atoi(params[0].c_str());
					aCase.param = params[1].substr(1, params[1].length()-2);//delete two side of \" 
					aCase.cycleTimes = atoi(params[2].c_str());
					if(params[3] == "Y")
					{
						aCase.isMultiInstance = true;
					}
					else if (params[3] == "N")
					{
						aCase.isMultiInstance = false;
					}
					aCase.multiNumber = atoi(params[4].c_str());
					for(int i=5; i < params.size()-1; i++)
					{
						aCase.multiParam.push_back(params[i].substr(1, params[i].length()-2));
					}
					aCase.timeInterval = atoi(params[params.size()-1].c_str());
				}
				runners[runnerIndex].cases.push_back(aCase);
			}
		}
	}
	file.close();
	return runners;
}

void* startCMD(void* ptr)
{
	string cmd_param = (char*)ptr;
	char ch = ' ';
	vector<string> sub_str = splitString(cmd_param,ch);
	
	char str[1024];
	switch (sub_str.size())
	{
		case 1:
			sprintf(str, ".%s", sub_str[0].c_str());
			break;
		case 2:
			sprintf(str, ".%s %s", sub_str[0].c_str(), sub_str[1].c_str());
			break;
		case 3:
			sprintf(str, ".%s %s %s", sub_str[0].c_str(), sub_str[1].c_str(), sub_str[2].c_str());
			break;
		case 4:
			sprintf(str, ".%s %s %s %s", sub_str[0].c_str(), sub_str[1].c_str(), sub_str[2].c_str(), sub_str[3].c_str());
			break;
		case 5:
			sprintf(str, ".%s %s %s %s %s", sub_str[0].c_str(), sub_str[1].c_str(), sub_str[2].c_str(), sub_str[3].c_str(), sub_str[4].c_str());
			break;
		case 6:
			sprintf(str, ".%s %s %s %s %s %s", sub_str[0].c_str(), sub_str[1].c_str(), sub_str[2].c_str(), sub_str[3].c_str(), sub_str[4].c_str(), sub_str[5].c_str());
			break;
		default:
			cout<<"too many params to run"<<endl;
			break;
	}	

	//string cmd = "cd ;" + cmd_param;
//	cout<<"start cmd: "<<str<<endl;
	string cd = "cd /";
	system(cd.c_str());
	system(str);
	return 0;
}

int startRunner(string param)
{
	pthread_t id;
	int ret;
	ret = pthread_create(& id, NULL, startCMD, (void *)param.c_str());
	if(ret != 0)
	{
		cout<<"create pthread error!\n"<<endl;
		return -1;
	}
	return 1;
}

extern "C"
{
	#include <sys/types.h>
	#include <dirent.h>
	#include <stdio.h>
	#include <signal.h>
	#include <string.h>
	int killRunner(string runnerName)
	{
		DIR* dir;
		struct dirent* ptr;
		FILE* fp;
		char filepath[1024];
		char filetext[1024];
		pid_t pid;
		dir = opendir("/proc");
		int retval;
		bool isSuccess = false;
		if(NULL != dir)
		{
			while((ptr = readdir(dir)) != NULL)
			{
				if((strcmp(ptr->d_name,".") == 0 )|| (strcmp(ptr->d_name,"..") == 0))
					continue;
				if(DT_DIR != ptr->d_type)
					continue;

				sprintf(filepath,"/proc/%s/status",ptr->d_name);
				fp = fopen(filepath,"r");
				if(NULL != fp)
				{				
					fread(filetext,1,1024,fp);
					filetext[1023] = '\0';
					if(strstr(filetext, runnerName.c_str()))
					{
						pid = atoi(ptr->d_name);
						retval = kill(pid,SIGKILL);
						if(retval == 0)
						{
							printf("runner:PID:%s\n",ptr->d_name);
							isSuccess = true;
						}
					}
					fclose(fp);
				}
			}
			closedir(dir);
		}
		
		if(isSuccess) return 1;
		else return -1;
	}
}

int runACase(Runner runner, Case aCase)
{
//	cout<<"runner's case begin to run!"<<endl;
	cout<<"***runner name: "<<runner.name<<"***case ID: "<<aCase.caseID<<"***\n"<<endl;
	//do the case according to the cycle times
	for(int i=0; i < aCase.cycleTimes; i++)
	{
		//start main instance
		if(startRunner(aCase.param) != 1)
		{
			cout<<"start case fail, case id: "<<aCase.caseID<<endl;
			return -1;
		}
		//check is multiInstance
		if(aCase.isMultiInstance)
		{
			cout<<runner.name<<" case ID:"<<aCase.caseID<<" has "<<aCase.multiNumber<<" Instances"<<endl;
			for(int j=0; j < aCase.multiNumber-1; j++)
			{
				if(startRunner(aCase.multiParam[j]) != 1)
				{
					cout<<"start multi case fail, case no: "<<j+1<<endl;
					return -1;
				}
			}
		}
		else
		{
			cout<<runner.name<<" case ID:"<<aCase.caseID<<" has "<<aCase.multiNumber<<" Instance"<<endl;
		}
		
		//after time interval kill runner
		if(aCase.timeInterval == -1)
		{
			srand((unsigned)time(NULL));
			aCase.timeInterval = rand()%10+1;
			printf("The Random Number is:%d\n",aCase.timeInterval);
		}
		sleep(aCase.timeInterval);
		killRunner(runner.name);
		sleep(2);
	}
//	cout<<"a runner's case has ended successfully!"<<endl;
//	cout<<"***runner name: "<<runner.name<<"***case ID: "<<aCase.caseID<<"***\n"<<endl;
	return 1;
}

int runARunner(Runner runner)
{
	cout<<"runner begin to run!"<<endl;
	cout<<"***runner name:"<<runner.name<<"***runner's case count: "<<runner.cases.size()<<"***\n"<<endl;
	for(int i=0; i < runner.cases.size(); i++)
	{
		if(runACase(runner, runner.cases[i]) != 1)
		{
			cout<<"run a case fail: runner name: "<<runner.name<<" runner case id: "<<runner.cases[i].caseID<<endl;
			return -1;
		}
	}	
	cout<<"runner name:"<<runner.name<<" has ended successfully!"<<endl;
//	cout<<"***runner name:"<<runner.name<<"***runner's case count: "<<runner.cases.size()<<"***\n"<<endl;
	return 1;
}

int main(int argc, char *argv[])
{
	string filePath = argv[1];
	vector<Runner> runners = readConfigFile(filePath);
	
	cout<<"runners count: "<<runners.size()<<endl;
	
	for(int i=0; i < runners.size(); i++)
	{
		Runner runner = runners[i];
		runARunner(runner);
	}
	
	cout<<"!!!!!!Killer has ended successfully!!!!!!"<<endl;
}
