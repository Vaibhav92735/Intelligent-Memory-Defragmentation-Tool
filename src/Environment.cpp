#include <iostream>
#include "../include/Environment.h"
#include "../include/StringTools.h"
#include "../include/Commands.h"
#include "../include/GlobalVariables.h"
#include "../include/Config.h"

// #include "../include/Files.h"

// #include "./Files.cpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

Environment::Environment(Config *setting) : commandsHistory(), fs(setting)
{
	this->fs.setWorkingDirectory(&this->fs.getRootDirectory());
	this->con = setting;
}

void Environment::start()
{
	string input;

	// get first input
	cout << this->fs.getWorkingDirectory().getAbsolutePath() << ">";
	getline(cin, input);
	input = StringTools::trim(input);

	// loop until the input is "exit"
	while (input.compare("exit") != 0)
	{
		// handle input
		handleInput(input);

		// get next input
		cout << this->fs.getWorkingDirectory().getAbsolutePath() << ">";
		getline(cin, input);
		input = StringTools::trim(input);
	}
}


void Environment::populate()
{
	std::cout << "If you want to populate files then enter the 1 else 0/>";
	int mode;
	// cin >> mode;
	mode = 1;

	if (mode)
	{
		std::string filePath;
		std::cout << "enter the filepath/>";
		// std::cin >> filePath;
		filePath = "src/example.txt" ;
		// std::cout << filePath << "\n";
		std::ifstream file(filePath);

		if (!file)
		{
			std::cerr << "Error: Could not open file...." << filePath << std::endl;
			return;
		}

		std::string line;
		std::vector<string> files;
		std::vector<std::vector<std::string>> folders;
		std::vector<int> fileSizes;

		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string fullPath;
			int size;

			if (!(iss >> fullPath >> size))
			{
				std::cerr << "Error: Invalid input format on line: " << line << std::endl;
				continue;
			}

			std::vector<std::string> folderHierarchy;
			size_t lastSlash = fullPath.find_last_of('/');
			if (lastSlash == std::string::npos)
			{
				std::cerr << "Error: Invalid path format on line: " << line << std::endl;
				continue;
			}

			std::string path = fullPath.substr(0, lastSlash);
			std::string fileName = fullPath.substr(lastSlash + 1);
			std::stringstream pathStream(path);
			std::string folder;
			while (std::getline(pathStream, folder, '/'))
			{
				folderHierarchy.push_back(folder);
			}

			// FileEntry entry = { fileName, size };
			files.push_back(fileName);
			fileSizes.push_back(size);
			folders.push_back(folderHierarchy);

			// std::cout <<line << " "<< size << "\n";
		}

		file.close();

		// std::cout << "Parsed file entries:\n";
		// for (size_t i = 0; i < files.size(); ++i)
		// {
		// 	std::cout << "File: " << files[i] << ", Size: " << fileSizes[i] << " bytes\n";
		// 	std::cout << "Folders: ";
		// 	for (const auto &folder : folders[i])
		// 	{
		// 		std::cout << folder << " ";
		// 	}
		// 	std::cout << "\n";
		// }

		for (int i = 0; i < files.size(); i++)
		{

			for (int j = 0; j < folders[i].size(); j++)
			{
				string cmd;
				cmd += "mkdir ";
				cmd += folders[i][j];
				string input = StringTools::trim(cmd);
				handleInput(input);

				cmd = "cd ";
				cmd += folders[i][j];
				input = StringTools::trim(cmd);
				handleInput(input);
			}

			string cmd;
			cmd += "mkfile ";
			cmd += files[i];
			cmd += (" " + to_string(fileSizes[i]));

			string input = StringTools::trim(cmd);
			handleInput(input);

			cmd = "cd /";
			input = StringTools::trim(cmd);
			handleInput(input);
		}
	}

	cout << "Succesfully Populated....... \n";
}

void Environment::addToHistory(BaseCommand *command)
{
	this->commandsHistory.push_back(command);
}

const vector<BaseCommand *> &Environment::getHistory() const
{
	return this->commandsHistory;
}

void Environment::handleInput(const string &input)
{
	if (verbose == 2 || verbose == 3)
		std::cout << input << std::endl;

	string first_word = input.substr(0, input.find(' '));
	string args = input.substr(first_word.size(), input.size() - first_word.size());

	switch (CommandTools::stringToCommand(first_word))
	{
	case CommandType::EmptyCommandType:
		// do and print nothing
		break;
	case CommandType::CdCommandType:
	{
		CdCommand cd_command(first_word, args);
		cd_command.execute(this->fs);
		this->addToHistory(new CdCommand(cd_command));
	}
	break;
	case CommandType::CpCommandType:
	{
		CpCommand cp_command(first_word, args);
		cp_command.execute(this->fs);
		this->addToHistory(new CpCommand(cp_command));
	}
	break;
	case CommandType::ExecCommandType:
	{
		ExecCommand exec_command(first_word, args, this->getHistory());
		exec_command.execute(this->fs);
		this->addToHistory(new ExecCommand(exec_command));
	}
	break;
	case CommandType::HistoryCommandType:
	{
		HistoryCommand history_command(first_word, args, this->getHistory());
		history_command.execute(this->fs);
		this->addToHistory(new HistoryCommand(history_command));
	}
	break;
	case CommandType::LsCommandType:
	{
		LsCommand ls_command(first_word, args);
		ls_command.execute(this->fs);
		this->addToHistory(new LsCommand(ls_command));
	}
	break;
	case CommandType::MkdirCommandType:
	{
		MkdirCommand mkdir_command(first_word, args);
		mkdir_command.execute(this->fs);
		this->addToHistory(new MkdirCommand(mkdir_command));
	}
	break;
	case CommandType::MkfileCommandType:
	{
		MkfileCommand mkfile_command(first_word, args);
		mkfile_command.execute(this->fs);
		this->addToHistory(new MkfileCommand(mkfile_command));
	}
	break;
	case CommandType::MvCommandType:
	{
		MvCommand mv_command(first_word, args);
		mv_command.execute(this->fs);
		this->addToHistory(new MvCommand(mv_command));
	}
	break;
	case CommandType::PwdCommandType:
	{
		PwdCommand pwd_command(first_word, args);
		pwd_command.execute(this->fs);
		this->addToHistory(new PwdCommand(pwd_command));
	}
	break;
	case CommandType::RenameCommandType:
	{
		RenameCommand rename_command(first_word, args);
		rename_command.execute(this->fs);
		this->addToHistory(new RenameCommand(rename_command));
	}
	break;
	case CommandType::RmCommandType:
	{
		RmCommand rm_command(first_word, args);
		rm_command.execute(this->fs);
		this->addToHistory(new RmCommand(rm_command));
	}
	break;
	case CommandType::VerboseCommandType:
	{
		VerboseCommand verbose_command(first_word, args);
		verbose_command.execute(this->fs);
		this->addToHistory(new VerboseCommand(verbose_command));
	}
	break;
	case CommandType::DefragCommandType:
	{
		DefragCommand defrag_command(first_word, args);
		defrag_command.execute(this->fs);
		this->addToHistory(new DefragCommand(defrag_command));
	}
	break;
	case CommandType::slidingfragCommandType:
	{
		slidingfragCommand rename_command(first_word, args);
		rename_command.execute(this->fs);
		this->addToHistory(new slidingfragCommand(rename_command));
	}
	break;
	case CommandType::FDefragCommandType:
	{
		FDefragCommand fdefrag_command(first_word, args);
		fdefrag_command.execute(this->fs);
		this->addToHistory(new FDefragCommand(fdefrag_command));
	}
	break;
	case CommandType::UnknownCommandType:
	default:
		ErrorCommand error_command(first_word, args);
		error_command.execute(this->fs);
		this->addToHistory(new ErrorCommand(error_command));
		break;
	}
}

// rule of five

// copy constructor
Environment::Environment(const Environment &env_src) : commandsHistory(env_src.commandsHistory), fs(env_src.fs)
{
	duplicate(env_src);
	if (verbose == 1 || verbose == 3)
		cout << __PRETTY_FUNCTION__ << endl;
}

// copy assignment operator
Environment &Environment::operator=(const Environment &env_src)
{
	if (&env_src != this)
	{
		clean_up();
		duplicate(env_src);
	}
	if (verbose == 1 || verbose == 3)
		cout << __PRETTY_FUNCTION__ << endl;
	return *this;
}

// destructor
Environment::~Environment()
{
	clean_up();
	if (verbose == 1 || verbose == 3)
		cout << __PRETTY_FUNCTION__ << endl;
}

// move constructor
Environment::Environment(Environment &&env_src) : commandsHistory(env_src.commandsHistory), fs(env_src.fs)
{
	steal(env_src);
	if (verbose == 1 || verbose == 3)
		cout << __PRETTY_FUNCTION__ << endl;
}

// move assignment operator
Environment &Environment::operator=(Environment &&env_src)
{
	if (&env_src != this)
	{
		clean_up();
		steal(env_src);
	}
	if (verbose == 1 || verbose == 3)
		cout << __PRETTY_FUNCTION__ << endl;
	return *this;
}

void Environment::clean_up()
{
	for (BaseCommand *cmd : this->commandsHistory)
	{
		delete cmd;
	}
	commandsHistory.clear();
}

void Environment::duplicate(const Environment &env_src)
{
	this->commandsHistory = env_src.commandsHistory;
	this->fs = env_src.fs;
}

void Environment::steal(Environment &env_src)
{
	for (BaseCommand *cmd : env_src.getHistory())
	{
		this->commandsHistory.push_back(cmd);
	}
	this->fs = env_src.fs;
}