#pragma once
#ifndef CONFIG_H_
#define CONFIG_H_

#include<bits/stdc++.h>
using namespace std;


class Config
{
	public:
	std::map< std::pair<int , int> , std::pair<int , std::string>> blockTable;
	int rowFrag;
	int colFrag;
	int blocksize;
    int rows  , cols ;
	Config();

};

#endif