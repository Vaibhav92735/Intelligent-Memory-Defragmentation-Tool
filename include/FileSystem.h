#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "Files.h"
#include "Config.h"

//#include "Commands.h"

class FileSystem {
private:
	Directory* rootDirectory;
	Directory* workingDirectory;
	DefragClass defragObj;
public:
	Config *con;
	FileSystem(Config *setting);
	FileSystem();

	Directory& getRootDirectory() const; // Return reference to the root directory
	Directory& getWorkingDirectory() const; // Return reference to the working directory
	void setWorkingDirectory(Directory *newWorkingDirectory); // Change the working directory of the file system

	// my additions
	Directory* getDirectoryFromPath(std::string path);
	FileSystem(const FileSystem &fs_src); // copy constructor
	FileSystem &operator=(const FileSystem & fs_src); // copy assignment operator
	~FileSystem(); // destructor
	FileSystem(FileSystem &&fs_src); // move constructor
	FileSystem &operator=(FileSystem&& other); // move assignment operator
	void duplicate(const FileSystem &fs_src);
	void steal(FileSystem &fs_src);
	void defragmenting(FileSystem &fs_src, int rows, int cols);
	void fleDefrag(FileSystem &fs_src, File* file);
	void slidingdefragmenting(FileSystem &fs_src);
	void clean_up();
};


#endif
