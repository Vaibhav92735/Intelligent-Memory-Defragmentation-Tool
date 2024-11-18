#include "../include/Files.h"
#include "../include/Config.h"
#include <algorithm>
#include <iostream>
#include <pthread.h> 
#include "../include/GlobalVariables.h"
#include <thread>
#include <chrono>
#include <mutex>

std::mutex globalLock;
using namespace chrono;

// global variables
std::map<std::pair<int, int>, std::pair<int, File *>> BTF;
std::map<std::pair<int, int>, std::pair<int, std::string>> BT;
std::queue<std::pair<File*, std::vector<std::pair<std::pair<int, int>, int>>>> overflowQueue;
int BS;
int rows, cols;
int rowFrag, colFrag;

// <BaseFile>

BaseFile::BaseFile(std::string name) : name(name) {}

BaseFile::~BaseFile() {}

std::string BaseFile::getName() const
{
	return this->name;
}

void BaseFile::setName(std::string newName)
{
	this->name = newName;
}

// </BaseFile>

// <File>

File::File(std::string name, int size) : BaseFile(name), size(size)
{

	blocks.resize(size / BS + int(size % BS == 0 ? 0 : 1));
}

int File::getSize()
{
	return this->size;
}

int File::getInternalFrag(){
	return this->internalFrag;
}

void File::updateInternalFrag(){
	this->internalFrag = BS-(this->size % BS);
}

// </File>

// <Directory>

Directory::Directory(std::string name, Directory *parent, Config *setting) : BaseFile(name), children(), parent(parent)
{
	con = setting;
	if (parent == NULL)
	{
		BS = setting->blocksize;
		BT = setting->blockTable;

		rows = setting->rows;
		cols = setting->cols;

		rowFrag = setting->rowFrag;
		colFrag = setting->colFrag;

		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				BTF[make_pair(i, j)].second = NULL;
				BTF[make_pair(i, j)].first = -1;
			}
		}
	}
}

Directory::Directory(std::string name, Directory *parent) : BaseFile(name), children(), parent(parent)
{
}

// Rule of five
// copy constructor
Directory::Directory(const Directory &dir) : BaseFile(dir.getName()), children(dir.children), parent(dir.parent)
{
	this->duplicate(dir);
	if (verbose == 1 || verbose == 3)
		std::cout << __PRETTY_FUNCTION__ << std::endl;
}

// copy assignment operator
Directory &Directory::operator=(const Directory &dir)
{
	if (this != &dir)
	{
		clean_up();
		duplicate(dir);
	}
	if (verbose == 1 || verbose == 3)
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	return *this;
}

// destructor
Directory::~Directory()
{
	this->clean_up();
	if (verbose == 1 || verbose == 3)
		std::cout << __PRETTY_FUNCTION__ << std::endl;
}

// move constructor
Directory::Directory(Directory &&dir) : BaseFile(dir.getName()), children(dir.children), parent(dir.parent)
{
	steal(dir);
	if (verbose == 1 || verbose == 3)
		std::cout << __PRETTY_FUNCTION__ << std::endl;
}

// move assignment operator
Directory &Directory::operator=(Directory &&dir)
{
	if (this != &dir)
	{
		clean_up();
		steal(dir);
	}
	if (verbose == 1 || verbose == 3)
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	return *this;
}

Directory *Directory::getParent() const
{
	return this->parent;
}

void Directory::setParent(Directory *newParent)
{
	this->parent = newParent;
}

void Directory::addBlocks(BaseFile *file)
{
	File *f = dynamic_cast<File *>(file);
	// cout << BS << " ";
	// cout << f->blocks.size() << " ";

	int curr = 0;
	for (auto &i : BT)
	{
		if (i.second.second == "NULL")
		{
			i.second.first = curr;
			i.second.second = f->getName();
			f->blocks[curr] = make_pair(i.first.first, i.first.second);
			BTF[make_pair(i.first.first, i.first.second)].first = curr;
			BTF[make_pair(i.first.first, i.first.second)].second = f;
			curr++;
		}

		if (curr >= f->blocks.size())
			break;
	}

	// for (auto i : f->blocks)
	// {
	// 	cout << "Block index: " << i.first << " " << i.second << "\n";
	// 	cout << BT[{i.first, i.second}].first << " " << BT[{i.first, i.second}].second << "\n";
	// 	cout << BTF[{i.first, i.second}].first << " " << BTF[{i.first, i.second}].second->getName() << "\n";
	// }

	if (curr < f->blocks.size())
	{
		cerr << "Memory Full...\n";
		return;
	}
}

void Directory::addFile(BaseFile *file)
{
	if (!childExists(file->getName()))
	{

		if (File *fle = dynamic_cast<File *>(file))
			addBlocks(fle);

		this->children.push_back(file);
		if (Directory *dir = dynamic_cast<Directory *>(file))
			dir->parent = this;
	}
}

void Directory::removeBlocks(BaseFile *file)
{
	File *f = dynamic_cast<File *>(file);
	vector<pair<int, int>> rBlocks;
	for (auto i : BTF)
	{
		if (i.second.second == f)
		{
			rBlocks.push_back(make_pair(i.first.first, i.first.second));
		}
	}
	// for (auto i : rBlocks)
	// {
	// 	cout << i.first << " " << i.second << "\n";
	// }
	for (auto i : rBlocks)
	{
		// cout << BTF[make_pair(i.first, i.second)].first << " " << BTF[make_pair(i.first, i.second)].second->getName() << " " << f->getName() << endl;
		BTF[make_pair(i.first, i.second)].first = -1;
		BTF[make_pair(i.first, i.second)].second = NULL;
		BT[make_pair(i.first, i.second)].first = -1;
		BT[make_pair(i.first, i.second)].second = "NULL";
	}

	for (auto &i : f->blocks)
	{
		i.first = -1;
		i.second = -1;
	}

// 	for (auto &i : f->blocks)
// 	{
// 		cout << "Block index: " << i.first << " " << i.second << "\n";
// 		cout << BT[{i.first, i.second}].first << " " << BT[{i.first, i.second}].second << "\n";
// 	}
}

void DefragClass::PrintHDD()
{
    printf("In use");
	for (int i = 0; i < rows; i++)
	{
		if (i % rowFrag == 0)
		{
			for (int j = 0; j < (15 * (cols)) + 3 * (colFrag); j++)
				cout << "Y";
			cout << "\n";
		}

		for (int j = 0; j < cols; j++)
		{
			if (j % colFrag == 0)
				cout << " Y ";


			cout << " " << BT[{i, j}].second << " " << BT[{i, j}].first << " ";

			for (int k = 0; k < (12 - (BT[{i, j}].second).size() - (to_string(BT[{i, j}].first)).size()); k++)
				cout << " ";
		}
		cout << "\n";
	}
}

void DefragClass::fullDefragmentation(int i, int j)
{

	map<File *, vector<pair<pair<int, int>, int>>> fileFragments;


	for (int row = i; row < i + rowFrag; row++)
	{
		for (int col = j; col < j + colFrag; col++)
		{
			if (BTF[{row, col}].second != NULL)
				fileFragments[BTF[{row, col}].second].push_back({{row, col}, BTF[{row, col}].first});
		}
	}

	int curr = 0;
	for (auto file : fileFragments)
	{

		File *f = file.first;
		for (int it = 0; it < file.second.size(); it++)
		{
			int cr = i + curr / rowFrag, cc = j + curr % colFrag;

			f->blocks[it] = {cr, cc};
			BTF[{cr, cc}] = {file.second[it].second, f};
			BT[{cr, cc}] = {file.second[it].second, f->getName()};

			curr++;
		}
	}

	for (; curr < rowFrag * colFrag; curr++)
	{
		int cr = i + curr / rowFrag, cc = j + curr % colFrag;
		BTF[{cr, cc}] = {-1, NULL};
		BT[{cr, cc}] = {-1, "NULL"};
	}

	// for (int curr = 0; curr < rowFrag * colFrag; curr++)
	// {
	// 	int cr = i + curr / rowFrag, cc = j + curr % colFrag;

	// 	cout << BT[{cr, cc}].first << " " << BT[{cr, cc}].second << "\n";
	// }

	// cout << "--------------this segment defragmented----------------\n";

}

void DefragClass::defragmentation(int r, int c)
{
	cout << "...............virtul HDD view...............\n";
	PrintHDD();

	cout<< ".......................Before Defragmentation......................"<<endl;
	evaluateFragmentation();
	if (r != -1)
	{
		rowFrag = r;
		colFrag = c;

		if (rows % rowFrag != 0 || cols % colFrag != 0)
		{
			cerr << "Invalid choice... \n";
			rowFrag = colFrag = 1;
			cout << "subgrid size is auto selected to 1 x 1.... \n";
		}
	}

	cout << "Starting Defragmentation.... \n";

	for (int i = 0; i < rows; i += rowFrag)
	{
		for (int j = 0; j < cols; j += colFrag)
		{
			fullDefragmentation(i, j);
		}
	}

	cout<< ".......................After Defragmentation......................"<<endl;
	evaluateFragmentation();

	int totalInternalFrag = calcInternalFrag();
	cout<< "Total Internal Fragmentation is (in Bytes): "<< totalInternalFrag<<endl;

	cout << "...............virtul HDD view...............\n";
	PrintHDD();
}

int DefragClass::calcInternalFrag(){

	int totalIntFrag = 0;
	set<File*> visited;

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			File* currFile = BTF[{i,j}].second;
			if(currFile == nullptr) continue;
			if(visited.find(currFile) != visited.end()) continue;

			//update totalFrag and visited
			currFile->updateInternalFrag();
			cout<< currFile->getName() <<" "<<currFile->getInternalFrag()<<endl;
			totalIntFrag += currFile->getInternalFrag();
			visited.insert(currFile);
		}
	}

	return totalIntFrag;
}

// void DefragClass::updateInternalFrag(){

// 	int totalIntFrag = 0;
// 	set<File*> visited;

// 	for(int i = 0; i < rows; i++){
// 		for(int j = 0; j < cols; j++){
// 			File* currFile = BTF[{i,j}].second;
// 			if(currFile == nullptr) continue;
// 			if(visited.find(currFile) != visited.end()) continue;

// 			//update totalFrag and visited
// 			currFile->updateInternalFrag();
// 			// cout<<currFile->getInternalFrag()<<endl;
// 			// totalIntFrag += currFile->getInternalFrag();
// 			visited.insert(currFile);
// 		}
// 	}
// }

// Function to calculate Manhattan distance
int DefragClass::calculateDistance(pair<int, int> a, pair<int, int> b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}

// Function to compute frag size
int DefragClass::calculateFragSize(const vector<pair<int, int>> &blocks) {
    int fragSize = 1; // At least one contiguous segment
    for (size_t i = 1; i < blocks.size(); i++) {
        // Check if blocks are adjacent (row-major order)
        if (blocks[i].first != blocks[i - 1].first || blocks[i].second != blocks[i - 1].second + 1) {
            fragSize++;
        }
    }
    return (blocks.size()/fragSize)*BS;
}

// Function to compute frag distance
int DefragClass::calculateFragDistance(const vector<pair<int, int>> &blocks) {
    int fragDistance = 0;
	int fragSize = 1;
    for (size_t i = 1; i < blocks.size(); i++) {
		if (blocks[i].first != blocks[i - 1].first || blocks[i].second != blocks[i - 1].second + 1) {
			fragSize++;
            fragDistance += calculateDistance(blocks[i - 1], blocks[i]);
        }
    }
    return fragDistance*BS/fragSize;
}

// Main function to evaluate fragmentation metrics
void DefragClass::evaluateFragmentation() {
    cout << "File Name | Frag Size | Frag Distance\n";
    cout << "-----------------------------------\n";

	set<File*> visited;

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			File* currFile = BTF[{i,j}].second;
			if(currFile == nullptr) continue;
			if(visited.find(currFile) != visited.end()) continue;

			//impelement the analysis metrics
			string fileName = currFile->getName();
			auto blocks = currFile->blocks;

			//// Calculate frag size and frag distance
			int fragSize = calculateFragSize(blocks);
			int fragDistance = calculateFragDistance(blocks);

			// Display results
			cout << fileName << "       | " << fragSize << "         | " << fragDistance << "\n";

			visited.insert(currFile);
		}
	}

    cout << "-----------------------------------\n";
}

void DefragClass::slidingDefragmentation()
{
	auto start = high_resolution_clock::now();
    int currRow = 0, currCol = 0; // Starting position for compact placement

    // Traverse the grid to compact blocks
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            auto &block = BTF[{row, col}];
            File *f = block.second;
            if (f != NULL) // Check if the current block is in use
            {
                int blockValue = block.first;

                // If it's not at the current compacted position, move it
                if (row != currRow || col != currCol)
                {
                    f->blocks.push_back({currRow, currCol}); // Add the new block to the file's block list
                    BTF[{currRow, currCol}] = {blockValue, f}; // Update the block table with the new position
                    BT[{currRow, currCol}] = {blockValue, f->getName()}; // Update the block name table

                    // Mark the old position as free
                    BTF[{row, col}] = {-1, NULL};
                    BT[{row, col}] = {-1, "NULL"};
                }

                // Move to the next column in the current row
                currCol++;

                // If we've reached the end of the row, move to the next row and reset the column
                if (currCol >= cols)
                {
                    currCol = 0;
                    currRow++;
                }
            }
        }
    }

    // Mark all remaining blocks as free after compacting in-use blocks
    for (int row = currRow; row < rows; row++)
    {
        for (int col = (row == currRow ? currCol : 0); col < cols; col++)
        {
            BTF[{row, col}] = {-1, NULL};
            BT[{row, col}] = {-1, "NULL"};
        }
    }
    auto end = high_resolution_clock::now();
    
    // Calculate duration
    auto duration = duration_cast<nanoseconds>(end - start);
    cout << "Defragmentation completed in " << duration.count() << " nanoseconds.\n";

	cout<< ".......................After Defragmentation......................"<<endl;
	evaluateFragmentation();

	int totalInternalFrag = calcInternalFrag();
	cout<< "Total Internal Fragmentation is (in Bytes): "<< totalInternalFrag<<endl;

    cout << "Defragmentation completed using 1x(cols) sliding window.\n";
    PrintHDD(); // Optionally, print the final disk state
}


void Directory::recursivedel(BaseFile *file)
{
	if (File *fle = dynamic_cast<File *>(file))
	{
			removeBlocks(file);
			return;
	}
		
	Directory *dir = dynamic_cast<Directory *>(file);
	for(int i = dir->children.size() - 1; i >= 0; i--)
	{
		recursivedel(dir->children[i]);
	}
}

void Directory::removeFile(std::string name)
{
	for (int i = this->children.size() - 1; i >= 0; i--)
	{
		if (this->children[i]->getName().compare(name) == 0)
		{
			
			recursivedel(this->children[i]);

			this->children.erase(this->children.begin() + i);
		}
	}
}

void Directory::removeFile(BaseFile *file)
{
	for (int i = this->children.size() - 1; i >= 0; i--)
	{
		if (this->children[i] == file)
		{
			recursivedel(this->children[i]);
			this->children.erase(this->children.begin() + i);
		}
	}
}

// for sorting by name
struct name_predicate
{
	inline bool operator()(BaseFile *bf1, BaseFile *bf2)
	{
		return (bf1->getName().compare(bf2->getName()) < 0);
	}
};

// for sorting by size
struct size_predicate
{
	inline bool operator()(BaseFile *bf1, BaseFile *bf2)
	{
		return (bf1->getSize() < bf2->getSize());
	}
};

void Directory::sortByName()
{
	std::sort(this->children.begin(), this->children.end(), name_predicate());
}

void Directory::sortBySize()
{
	std::sort(this->children.begin(), this->children.end(), size_predicate());
}

std::vector<BaseFile *> Directory::getChildren()
{
	return this->children;
}

int Directory::getSize()
{
	int size = 0;
	for (BaseFile *child : this->children)
		size += child->getSize();
	return size;
}

// TODO: check while loop condition
std::string Directory::getAbsolutePath()
{
	if (this->parent == NULL)
		return this->getName();
	if (parent->getName().compare("/") != 0)
		return parent->getAbsolutePath() + '/' + this->getName();
	return parent->getAbsolutePath() + this->getName();
}

void Directory::steal(Directory &dir)
{
	this->setName(dir.getName());
	this->parent = dir.parent;
	dir.parent = NULL;
	for (BaseFile *bf : dir.children)
	{
		this->children.push_back(bf);
		if (Directory *d = dynamic_cast<Directory *>(bf))
			d->setParent(this);
	}
}

void Directory::clean_up()
{
	std::vector<BaseFile *>::iterator iterator = this->children.begin();
	while (!this->children.empty() && iterator != this->children.end())
	{
		this->removeFile(*iterator);
	}
	this->children.clear();
	this->parent = NULL;
}

void Directory::duplicate(const Directory &dir)
{
	this->children.clear();
	this->setName(dir.getName());
	this->setParent(dir.getParent());
	for (BaseFile *bf : dir.children)
	{
		if (Directory *d = dynamic_cast<Directory *>(bf))
		{
			Directory *new_d = new Directory(*d);
			new_d->setParent(this);
			this->children.push_back(new_d);
		}
		else if (File *f = dynamic_cast<File *>(bf))
		{
			File *new_f = new File(*f);
			this->children.push_back(new_f);
		}
	}
}

bool Directory::childExists(std::string name)
{
	for (BaseFile *child : this->children)
		if (child->getName().compare(name) == 0)
			return true;
	return false;
}

bool Directory::childDirectoryExists(std::string name)
{
	for (BaseFile *child : this->children)
		if (child->getName().compare(name) == 0)
		{
			if (dynamic_cast<Directory *>(child))
				return true;
			return false;
		}
	return false;
}

BaseFile *Directory::getChildByName(std::string name)
{
	for (BaseFile *child : this->children)
		if (child->getName().compare(name) == 0)
			return child;
	return NULL;
}

// </Directory>