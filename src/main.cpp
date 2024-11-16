#include "../include/Environment.h"
#include "../include/Config.h"

// ... You may not change this file

unsigned int verbose = 0;

int main(int , char **) {

	Config con;
	Environment env(&con);

	env.populate();

	

	env.start();
	
	return 0;
}