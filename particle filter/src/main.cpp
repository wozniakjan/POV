#include <iostream>

#include "tracker.h"
#include "object_stack.h"

using namespace std;

void printHelp() {
  cout << "Tracker" << endl;
  cout << "OPTIONS" << endl;
  cout << "  -h .... tato napoveda" << endl <<
          "  -i .... vstupni video soubor" << endl <<
          "  -o .... vystupni soubor s nalezenym objektem" << endl;
}


int main( int argc, char* argv[]) {
    int c;
    string inputVideoName, outputVideoName;
    
	if((argc==2) && (strcmp("-h", argv[1]) == 0))
	{
		printHelp();
		return 0;
	}
	else if(argc == 5) 
	{
		if((strcmp("-i", argv[1]) == 0) && (strcmp("-o", argv[3]) == 0))
		{
			inputVideoName = argv[2];
			outputVideoName = argv[4];
		}
		else if((strcmp("-i", argv[3]) == 0) && (strcmp("-o", argv[1]) == 0))
		{
			inputVideoName = argv[4];
			outputVideoName = argv[2];
		}
		else
		{
			cerr << "Chybne zadany parametry" << endl;
			return -1;
		}
	} else 
	{
		cerr << "Chybne zadany parametry" << endl;
		return -1;
	}
    
    Tracker t(inputVideoName, outputVideoName);
    //t.trackRedBallPF(Rect(900,480,120,120), 100, 15, 0.8);
	t.track();
}
