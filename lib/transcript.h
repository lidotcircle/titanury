#ifndef _TRANSCRIPT_H_
#define _TRANSCRIPT_H_

#include <string>


int GetPIDByProcessName(const std::string& processName);

std::string GetLastErrorAsString();


#endif // _TRANSCRIPT_H_