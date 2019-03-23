#include "SplitString.h"

SplitString::SplitString()
{
}

std::vector<std::string> SplitString::split(std::string a1, std::string a2)
{
	std::vector<std::string> vstrings;

	char *str = new char[a1.length() + 1];
	strcpy(str, a1.c_str());

	char * pch;
	pch = strtok(str, a2.c_str());
	while (pch != NULL) {
		vstrings.push_back(pch);
		pch = strtok(NULL, a2.c_str());
	}

	delete[] str;
	delete[] pch;
	return vstrings;
}

bool SplitString::isInteger(const std::string & s){
	if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

   char * p;
   strtol(s.c_str(), &p, 10);

   return (*p == 0);
}
