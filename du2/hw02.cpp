/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha 2a
// Autor: Michal Hradiš <ihradis@fit.vutbr.cz>
// Cil: Pouziti Viola&Jones detektoru objektu z OpenCV
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/
// * Dotazy zasilejte autorovi
/////////////////////////////////////////////////////////////////////////////////////////////


// Muzete si vypnout vykreslovani na obrazovku.
#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <iostream>
#include <sstream>

using namespace std; 
using namespace cv;

int main( int argc, char* argv[])
{
	// jmeno souboru pro zpracovani
	string inputVideoName;
	// jmeno souboru s detektorem 
	// V OpenCV je nadete v adresari /data/ - napr. /opencv-2.3.1/data/haarcascades
	string detectorName;

	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-i" && i + 1 < argc){
			inputVideoName = argv[ ++i];
		} else if( string(argv[ i]) == "-d" && i + 1 < argc){
			detectorName = argv[ ++i];
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << "  -i inputVideoName -d detectorName" << endl;
			return 0;
		} else {
			cerr << "Error: Unrecognized command line parameter \"" << argv[ i] << "\" use -h to get more information." << endl;
		}
	}

	// kontrola zadani parametru
	if( inputVideoName.empty() || detectorName.empty()){
		cerr << "Error: Some mandatory command line options were not specified." << endl;
		return -1;
	}

	// Otevreni videa
	VideoCapture capture( inputVideoName);

	if( !capture.isOpened()){
		cerr << "Error: Unable to open input video file \"" << inputVideoName << "\"." << endl;
        return -1;
	}

	// objekt detektoru
	CascadeClassifier cascade;

	// doplnte nacteni detektoru ze souboru detectorName
    /** FILL DONE **/
    cascade.load(detectorName);
	if(cascade.empty()){
		cerr << "ERROR: Could not load cascade classifier \"" << detectorName << "\"" << endl;
        return -1;
	}

	bool finished = false;
	Mat grayImage;
	Mat frame;

	while( !finished){ // 
		
		// Doplnte nacteni dalsiho snimku.
		bool success = capture.read( frame);

		finished |= !success;

		if( success){

			// prevedeni na sedotonovy obraz
			cvtColor( frame, grayImage, CV_BGR2GRAY );

			// pro nalezene objekty
			vector< Rect> objects;

			// doplnte detekci ve více rozlišeních s paramtery:
			// - krok ve velikosti 1.15
			// - minimalni pocet sousedu 2
			// - minimalni velikost skenovaciho okna 40x40
			// - maximalni velikost skenovaciho okna 200x200
 			/**FILL DONE**/
            cascade.detectMultiScale(frame, objects, 1.15, 2, 0, Size(40, 40));

			// vypis souradnic nalezenych objektu
			for( int i = 0; i < (int) objects.size(); i++){
				cout << '\t' << objects[ i].x << '\t' << objects[ i].y << '\t' << objects[ i].x + objects[ i].width << '\t' << objects[ i].y + objects[ i].height;
			}
			cout << endl;


			if( VISUAL_OUTPUT){
			
				// vykresleni nalezenych objektu
				for( int i = 0; i < (int) objects.size(); i++){
					
					// vykreslete nalezene objekty do frame
					rectangle(frame, objects[i].tl(), objects[i].br(), Scalar(255));
				}

				imshow( "Local features", frame);

				switch( waitKey( 2) & 255){
					case 'x':
					case 'X':
					case 'q':
					case 'Q':
						finished = true;
						break;
				}
			}
		}
	}
}