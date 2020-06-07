#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <filesystem>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

void waitOnChildren(vector<int>* pids){
  if(pids->size() == 0){
    return;//no pids
  }
  //iterate through vector of PID's, wait on all child processes
  for(int i = pids->size()-1; i >= 0;i--){//iterate in reverse order for improved performance
    int pid = pids->at(i);
    waitpid(pid,0,WNOHANG);
    pids->erase(pids->begin()+i);//erase pid from list
  }
}

void argumentFormatter(vector<string> input, char* args[]){
  //convert string to char* array. output is null terminated
  int i;
  for(i = 0; i < input.size(); i++){
    args[i] = &input[i][0];
  }
  args[i] = nullptr;
}

void printCharArray(char arr[]){
  //assume char array is null terminated
  int i = 0;
  while(arr[i] != '\0'){
  cout << arr[i];
  i++;
  }
  cout << endl;
}

void printCharArray(char* arr[]){
  //assume char pointer array is nullptr terminated
  int i = 0;
  while(arr[i] != nullptr){
  cout << *arr[i];
  i++;
  }
  cout << endl;
}

void strip(string* input){
  //delete leading whitespaces
  while(*input->begin() == ' '){
    input->erase(input->begin());
  }
  //delete trailing whitespaces
  while(*(input->end()-1) == ' '){
    input->erase(input->end()-1);
  }
}

vector<string> split(string input, char delimiter){
  vector<string> parsedString;
  stringstream check1(input);
  string tempString;
  //parse string by whitespace
  while(getline(check1, tempString, delimiter)){
    parsedString.push_back(tempString);
  }
  return parsedString;
}

bool isSleepCommand(vector<string> input){
  if(input.size() == 2){// if user passed in 2 arguments
    //convert second argument to int
    int a;
    istringstream(input.at(1)) >> a;// istringstream will return 0 if invalid argument
    if(input.at(0) == "sleep" && a != 0){//verify first 2 parameters
      return true;
    }
    else{
      return false;
    }
  }
  else if(input.size() == 3){//if user passed in 3 arguments
    //convert second argument to int
    int a;
    istringstream(input.at(1)) >> a;// istringstream will return 0 if invalid argument
    if(input.at(0) == "sleep" && a != 0 && input.at(2) == "&"){//verify first 3 parameters
      return true;
    }
    else{
      return false;
    }
  }
  else{// user entered either too many or not enough arguments
    return false;
  }
}

bool islsCommand(vector<string> input){
  if(input[0] == "ls" | input[0] == "ps"){
    return true;
  }
  else{
    return false;
  }
}

bool iscdCommand(vector<string> input){
  if(input.at(0) == "cd"){
    return true;
  }
  else{
    return false;
  }
}

bool isJobsCommand(vector<string> input){
  if(input.at(0) == "jobs"){
    return true;
  }
  else{
    return false;
  }
}

bool isAwkCommand(vector<string> input){
  if(input.at(0) == "awk"){
    return true;
  }
  else{
    return false;
  }
}

bool isEchoCommand(vector<string> input){
  if(input.at(0) == "echo"){
    return true;
  }
  else{
    return false;
  }
}

void executeSleepCommand(vector<int>* pids, vector<string> input){
  if(input.size() == 3){//if user requests a background process to sleep
    int pid = fork();
    if (pid == 0){
      //convert second argument to int
      int a;
      istringstream(input.at(1)) >> a;// istringstream will return 0 if invalid
      sleep(a);
      exit(1);
    }
    else{
      pids->push_back(pid);//push child pid to vector
      waitOnChildren(pids);//garbage collection for child processes
    }
  }
  //if the user wants the foreground process to sleep
  else{
    //convert second argument to int
    int a;
    istringstream(input.at(1)) >> a;// istringstream will return 0 if invalid argument
    sleep(a);
  }
}

void executePwdCommand(char pathName[],int pathNameSize){
    //makes a system call to get the current working directory
    char* size = getcwd(&pathName[0],pathNameSize);
    if(size == nullptr){
      cout << "getcwd failed." << endl;
    }
    else{
    }
}

void getParentDirectory(char parentdir[], int parentdirSize){
  executePwdCommand(parentdir, parentdirSize);
  int curr = 0;
  while(parentdir[curr] != '\0'){
    curr++;
  }
  while(curr >= 0){
    //get parent directory by null termianting the rightmost '/'
    if( parentdir[curr] == '/'){
      parentdir[curr] = '\0';
      break;
    }
    curr--;
  }
}

void executelsCommand(vector<int>* pids, vector<string> input){
  int pid = fork();
  if(pid == 0){
    //format command line arguments into char* array
    char* args[10];
    argumentFormatter(input, args);
    int error = execvp(args[0], args);
    if(error < 0){
      cout << "execvp error" << endl;
    }
    exit(0);
  }
  else{
    pids->push_back(pid);
    waitOnChildren(pids);//garbage collection for child processes
  }
}

void executecdCommand(vector<int>* pids, vector<string> input){
  //format command line arguments into char* array
  char* args[10];
  argumentFormatter(input, args);
  //move to parent directory
  if(input[1] == "-"){
    char parentdir[100];
    getParentDirectory(parentdir, 100);
    //chdir into parent dir
    chdir(parentdir);
  }
  //system call to change cwd
  chdir(args[1]);
}

void executeJobsCommand(vector<int>* pids, string input){
  for(int i = 0; i < pids->size(); i++){
    cout << pids->at(i) << endl;
  }
}

void executeEchoCommand(vector<int>* pids, string input){
  int pid = fork();
  if(pid == 0){//child process
    char* argv[input.size()+1];
    string echoCommand = "echo";
    argv[0] = &echoCommand[0];
    //add conditionals to ignore quotation marks
      if(input[5] == '"' | input[5] == '\''){
        argv[1] = &input[6];//ignore first quotation
      }
      else{//there is no opening quotation
      argv[1] = &input[5];
      }
      if(input[input.size()-1] == '"' | input[input.size()-1] == '\''){
        input[input.size()-1] = '\0';//overwrite last quotation with null character
      }
      argv[2] = NULL;
    int error = execvp(argv[0], argv);
    if(error < 0){
      cout << "execvp error" << endl;
    }
    exit(0);
  }
  else{
    pids->push_back(pid);
    waitOnChildren(pids);
  }
}

void executeAwkCommand(vector<int>* pids, string input){
  int pid = fork();
  if(pid == 0){
    input.erase(input.begin(), input.begin()+4);//remove command name
    //delete quotation marks
    for(int i = 0; i < input.length(); i++){
      if(input[i] == '\''){
        input.erase(input.begin()+i);
      }
    }
    cout << input << endl;
    int error = execlp("awk", "awk", input.c_str(), NULL);
    if(error < 0){
      cout << "execlp error" << endl;
    }
    exit(0);
  }
  else{
    pids->push_back(pid);
    waitOnChildren(pids);//garbage collection for child processes
  }
}

void executeGeneralCommand(vector<int>* pids, vector<string> input){
  int pid = fork();
  if(pid == 0){
    //format command line arguments into char* array
    char* args[10];
    argumentFormatter(input, args);
    int error = execvp(args[0], args);
    if(error < 0){
      cout << "execvp error" << endl;
    }
    exit(0);
  }
  else{
    pids->push_back(pid);
    waitOnChildren(pids);//garbage collection for child processes
  }
}

void IOredirect(vector<int>* pids, string* inputString){
  string input = *inputString;
  int infile = -1;
  int outfile = -1;
  //tokenize string, seperate command, input file and output file
  vector<string> GTSplit = split(input, '>');
  vector<string> splitInput;
  for(int i = 0; i < GTSplit.size(); i++){
    vector<string> LTSplit = split(GTSplit[i], '<');
    for(int j = 0; j < LTSplit.size(); j++){
      strip(&LTSplit.at(j));//strip whitespaces
      splitInput.push_back(LTSplit.at(j));
      //set index of input file and output file
      if(i == 0 && j == 1){
        infile = 1;
        outfile = 2;
      }
      if(i == 1 && j == 1){
        infile = 2;
        outfile = 1;
      }
    }
  }
  //if only one I/0 symbol is used in inputString, set indices of infile and outfile
  if(splitInput.size() < 3){
    vector<string> GTSplit = split(input, '>');
    if(GTSplit.size() > 1){
      infile = -1;
      outfile = 1;
    }
    else{
      infile = 1;
      outfile = -1;
    }
  }
if(infile != -1){
  int fd = open(splitInput[infile].c_str(), O_CREAT|O_RDONLY);//open file in read/write mode
  dup2(fd, 0);
}
if(outfile != -1){
  int fd = open(splitInput[outfile].c_str(), O_CREAT|O_WRONLY);//open file in read/write mode
  dup2(fd, 1);
}
  *inputString = splitInput[0];
}

void parseSingleCommand(vector<int>* pids, string input){
  //redirect I/O if needed
  IOredirect(pids, &input);
  vector<string> splitInput = split(input, ' ');//split input string
  //echo command
  if(isEchoCommand(splitInput)){
    executeEchoCommand(pids, input);
  }
  //sleep command
  else if(isSleepCommand(splitInput)){
    executeSleepCommand(pids, splitInput);
  }
  //ls command
  else if(islsCommand(splitInput)){
    executelsCommand(pids, splitInput);
  }
  //cd command
  else if(iscdCommand(splitInput)){
    executecdCommand(pids, splitInput);
  }
  //cd command
  else if(isAwkCommand(splitInput)){
    executeAwkCommand(pids, input);
  }
  else{
    executeGeneralCommand(pids, splitInput);
    //cout << "Invalid input!" << endl;
    return;
  }
}

int main (){
  vector<int> pids;
  int originalFDin = dup(0);
  int originalFDout = dup(1);
  while (true){
    dup2(originalFDin, 0);
    dup2(originalFDout, 1);
    cout << "Rahul's Shell$ ";
    string inputline = readline("");
    add_history(&inputline[0]);
     //get a line from standard input
    //exit shell
    if (inputline == string("exit")){
      cout << "Bye!! End of shell" << endl;
      break;
    }
    //continue if no input
    if(inputline == ""){
      continue;
    }
    //handle echo seperately so | and < > can be echo'd without errors
    vector<string> temp = split(inputline, ' ');
    if(isEchoCommand(temp)){
      executeEchoCommand(&pids, inputline);
      continue;
    }
    //handle jobs command
    vector<string> temp2 = split(inputline, ' ');
    if(isJobsCommand(temp2)){
      executeJobsCommand(&pids, inputline);
      continue;
    }
    //split list of commands into vector of commands
    vector<string>  splitInput= split(inputline, '|');
    //strip each command of leading/trailing whitespaces
    for(int i = 0; i < splitInput.size(); i++){
      strip(&splitInput.at(i));
    }

    for(int i = 0; i < splitInput.size(); i++){
      //if user entered only 1 command
      if(splitInput.size() == 1){
        parseSingleCommand(&pids, splitInput.at(0));
        break;
      }
      //set up pipe
      int fd[2];
      pipe(fd);
      int pid = fork();
      if(pid==0){//child process
        if(i < splitInput.size()-1){
          dup2(fd[1], 1);//change standard output to write pipe
        }
        parseSingleCommand(&pids, splitInput.at(i));
        //garbage collection
        close(fd[0]);
        exit(0);

      }
      else{//parent process
        if(i == splitInput.size()-1){//only wait on the last command
          waitpid(pid,0,0);
          break;
        }
        pids.push_back(pid);
        dup2(fd[0], 0);//change standard input to read pipe
        close(fd[1]);//close fd[1] each iteration to prevent hang during next iteration
      }

    }
    waitOnChildren(&pids);
  }
}
