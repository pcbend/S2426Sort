#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef __CINT__
#ifndef __ROOTMACRO__

#include <cstdlib> 
#include <sys/stat.h> 
#include <iostream> 
#include <fstream> 
#include <sstream>
#include <limits.h>
#include <string>
#include <vector>
#include <regex>

bool fileExists(const char *filename){
  //std::ifstream(filename);
  struct stat buffer;
  return (stat(filename,&buffer)==0);
}

std::vector<std::string> tokenizeString(std::string path,char delimiter='/') { 
  std::istringstream ss(path);
  std::string token;
  std::vector<std::string> parts;
  //printf("fullpath = %s\n",path.c_str());
  while(std::getline(ss,token,delimiter)) {
    //printf("token = %s\n",token.c_str());
    parts.push_back(token);
  }
  return parts;
}

void getRunNumber(std::string infile,int &run,int &subrun) {
  std::regex pattern("run([0-9]{5})_([0-9]{3})\\.mid");
  std::smatch matches;

  if (std::regex_search(infile, matches, pattern)) {
    // We should have 3 matches: the full string, capture group 1, and capture group 2
    if (matches.size() == 3) {
      // Convert the captured strings to integers
      run = std::stoi(matches.str(1));    // Group 1: 5-digit number
      subrun = std::stoi(matches.str(2)); // Group 2: 3-digit number
      return; // Success, return from the function
    }  
  }
  run = -1;
  subrun = -1;
  return;
};

#ifdef __LINUX__
#include <unistd.h>
std::string programPath(){
  char buff[PATH_MAX+1];
  size_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
  buff[len] = '\0';

  std::string exe_path = buff;
  return exe_path.substr(0, exe_path.find_last_of('/'));
}
#endif

#ifdef __DARWIN__ 
#include <mach-o/dyld.h>
std::string programPath(){
  char buff[PATH_MAX];
  uint32_t len = PATH_MAX;
  _NSGetExecutablePath(buff,&len);
  std::string exe_path = buff;
  return exe_path.substr(0, exe_path.find_last_of('/'));
}
#endif

#endif
#endif

#endif
