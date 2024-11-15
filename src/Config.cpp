#include <iostream>
#include "../include/Config.h"
#include "../include/StringTools.h"
#include "../include/Commands.h"
#include "../include/GlobalVariables.h"
// #include "../include/Files.h"

// #include "./Files.cpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

Config::Config()
{

	cout << "\033[34m" << "\n .----------------.  .----------------.  .----------------.\n| .--------------. || .--------------. || .--------------. |\n| |     ____     | || |    _______   | || |  ____  ____  | |\n| |   .'    `.   | || |   /  ___  |  | || | |_  _||_  _| | |\n| |  /  .--.  \  | || |  |  (__ \_|  | || |   \ \  / /   | |\n| |  | |    | |  | || |   '.___`-.   | || |    > `' <    | |\n| |  \  `--'  /  | || |  |`\____) |  | || |  _/ /'`\ \_  | |\n| |   `.____.'   | || |  |_______.'  | || | |____||____| | |\n| |              | || |              | || |              | |\n| '--------------' || '--------------' || '--------------' |\n'----------------'  '----------------'  '----------------'\n" <<  "\033[0m";

	std::cout << "Welcome to DISK EMULATOR, configure the virtual HW here...\n";

	std::cout << "enter the 1 for custom HDD or 0 for default HDD/>";

	int mode;
	cin >> mode;


	int n, m;
	if (mode)
	{
		std::cout << "enter the size of HDD in rows X col:/>";
		cin >> n >> m;

		std::cout << "enter the blockSize which is divisible by " << n*m << ":\n" ;
		cin >> blocksize ;
		if( (n*m*1LL)% blocksize != 0)
		{
			cerr << "Invalid blockSize\n";
			return;
		}
		std::cout << "Enter the subgrid size for disk defragmentation algorithm which is divisible by "  << n << " " << m << " " << "in rows x columns:/>";

		cin >> this->rowFrag >> this->colFrag;
		if(  n%this->rowFrag!= 0 || m%this->colFrag  != 0  )
		{
			cerr << "Invalid choice\n";
			this->rowFrag = this->colFrag = 1;
			cout << "subgrid size is auto selected to 1 x 1.....\n" ;
			return;

		}

	}
	else
	{
		n = m = 8;
		blocksize = 4;
		this->rowFrag = 4;
		this->colFrag = 4;
	}

    this->rows = n;
    this->cols = m;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
            blockTable[make_pair(i, j)].second = "NULL";
            blockTable[make_pair(i, j)].first = -1;

		}
	}

	// for (auto i : blockTable)
	// {
	// 		cout << i.first.first << "  " << i.first.second  <<  "\n";
	// }


}
