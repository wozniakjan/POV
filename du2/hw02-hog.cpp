/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha 1
// Autor: Michal Hradiš <ihradis@fit.vutbr.cz
// Cil: seznameni s OpenCV implementaci detektoru objektu zalozeneho na Histograms of Oriented Gradients (HOG) a linearnim klasifikatoru
//	Puvodni publikace Navneet Dalal, Bill Triggs, "Histograms of Oriented Gradients for Human Detection," Computer Vision and Pattern Recognition, IEEE Computer Society Conference on, pp. 886-893, 2005 IEEE Computer Society Conference on Computer Vision and Pattern Recognition (CVPR'05) - Volume 1, 2005
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/
// * Program umoznuje natrenovat vlastni detektor. Muzete vyzkouset na vlastni datove sade. Ukazkova datova sada je prilozena k zadani.
// * Je vyuzit Stochastic Gradient Descent pro trenovani linearniho klasifikatoru, ktery neni uplne otestovany, tak se muze nekdy chovat i podivne. V ramci domaciho ukolu s nim ale problemy nebudou.
// * Parametry uceni jsou nastaveny tak, aby to nejak fungovalo, urcite existuje i lepsi. 
// * Je mozne pouzit jakykoliv jiny algoritmus pro trenovani linearnich klasifikatoru, ale dosazene vysledky se mohou lisit.  
// * V dokumentaci OpenCV aktualne patrne chybi popis HOGDescriptor, ale je tam popis verze pro GPU, která je podobná. Vše potøebné se jde zjistit z definice HOGDescriptor v hlavickovem souboru. 
/////////////////////////////////////////////////////////////////////////////////////////////


// Muzete si vypnout vykreslovani na obrazovku.
#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

#include "sgdSVM.h"

using namespace std; 
using namespace cv;

int main( int argc, char* argv[])
{
	// jmeno video-souboru pro zpracovani
	string inputVideoName;
	// Pokud je true, pouzije se predem natrenovany linearni klasifikator z OpenCV, ktery slouzi k detekci chodcu.
	bool useDefaultDetector = false;
	// jmeno souboru se seznamem obrazku s pozitivnimi priklady objektu k detekci
	string positiveImageFile;
	// jmeno souboru se seznamem obrazku, ve kterych se objekt zajmu nevyskytuje a ze kterych se nahodne vyrezaji negativni priklady
	string negativeImageFile;
	// pocet negativnich vzorku pro trenovani detekcniho klasifikatoru
	int negativeSampleCount = 10000;
	// pocet pruchodu datovou sadou pri trenovani SVM
	int svmIterations = 50;
	// prah detektoru - jeho nastaveni je ted nepodstatne, protoze se na zacatku prepise vlastnim prahem klasifikatoru
	float threshold = 0;

	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-d"){
			useDefaultDetector = true;
		} else if( string(argv[ i]) == "-i" && i + 1 < argc){
			inputVideoName = argv[ ++i];
		} else if( string(argv[ i]) == "-p" && i + 1 < argc){
			positiveImageFile = argv[ ++i];
		} else if( string(argv[ i]) == "-n" && i + 1 < argc){
			negativeImageFile = argv[ ++i];
		} else if( string(argv[ i]) == "-nc" && i + 1 < argc){
			stringstream str( argv[ ++i]);
			str >> negativeSampleCount;
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << " -i inputVideoName -p positiveImageFile -n negativeImageFile [-d -nc 10000]" << endl;
			cout << "-d		Default OpenCV pedestrian detector will be used. -p, -n and -nc have no effect in this case." << endl;
			return 0;
		} else {
			cerr << "Error: Unrecognized command line parameter \"" << argv[ i] << "\" use -h to get more information." << endl;
		}
	}


	// kontrola zadani parametru
	if( !useDefaultDetector && (inputVideoName.empty() || positiveImageFile.empty() || negativeImageFile.empty())){
		cerr << "Error: Some mandatory command line options were not specified." << endl;
		return -1;
	}


	// objekt detektoru / HOG descriptoru
	HOGDescriptor descriptor;


	if( useDefaultDetector){
		// Doplnte nastaveni default klasifikatoru HOGDescriptor::getDefaultPeopleDetector() jako detekcniho kasifikatoru pro descriptor.
		/** FILL DONE **/
		descriptor.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
	} else {

		// pouze pro vypisovani tecek behem nacitani dat
		int generalCounter = 0;

		// otevreni souboru se seznamem obrazku s pozitivnimi priklady
		ifstream posFile( positiveImageFile.c_str());

		if( !posFile.good()){
			cerr << "Error: Unable to open file \"" << positiveImageFile << "\"." << endl;
		}

		// trenovaci priznakove vektory
		vector< vector< float> > trainingData;

		// trenovaci tridy - pro SVM jsou to +1, -1
		vector< double> trainingLabels;

		// spocitat priznakove vektory pro pozitivni trenovaci vzorky
		cout << "Preparing possitive examples:" << flush;
		while( posFile.good()){

			// nacteni jednoho jmena ze souboru
			string imgName;
			posFile >> imgName;

			// nacteni sedotonoveho obrazku
			Mat img = imread( imgName, 0);

			if( img.data == NULL){
				cerr << "Error: Unable to read image file \"" << imgName << "\"." << endl;
				continue;
			}

			// zmena velikosti na velikost okna detektoru
			Mat normedImage;
		
			resize( img, normedImage, descriptor.winSize);

			// Doplnte vypocet priznakoveho vektoru pro obrazek normedImage.
			vector< float> desc;
			/**FILL DONE PERHAPS**/
            descriptor.compute(normedImage, desc);

			trainingData.push_back( desc);
			trainingLabels.push_back( +1);

			if( generalCounter++ % 10 == 0){
				cout << "." << flush;
			}
		}
		cout << " DONE " << endl;


		// otevreni souboru se seznamem obrazku s negativnimi priklady
		ifstream negFile( negativeImageFile.c_str());

		if( !negFile.good()){
			cerr << "Error: Unable to open file \"" << negativeImageFile << "\"." << endl;
			return -1;
		}

		// nacteni negativnich obrazku
		cout << "Reading background images " << flush;
		vector< Mat> negativeImages;
		while( negFile.good()){

			// nacteni jednoho jmena ze souboru
			string imgName;
			negFile >> imgName;

			// nacteni sedotonoveho obrazku 
			Mat img = imread( imgName, 0);

			if( img.data == NULL){
				cerr << "Error: Unable to read image file \"" << imgName << "\"." << endl;
				continue;
			}

			// obrazek musi byt vetsi nez okno detektoru, aby z nej sly extrahovat nahodne vyrezy
			if( img.cols < descriptor.winSize.width || img.rows <= descriptor.winSize.height){
				cerr << "Error: Unable image file \"" << imgName << "\" resolution is too small." << endl;
				continue;
			}

			// pridat do sady negativnich obrazku
			negativeImages.push_back( img);

			if( generalCounter++ % 10 == 0){
				cout << "." << flush;
			}
		}
		cout << " DONE " << endl;

		trainingData.reserve( trainingData.size() + negativeSampleCount); 

		// Vypocet priznakovych vektoru pro nahodne vyrezy z nactenych obrazku.
		// Tyto vzorky budou pouzity jako negativni priklady pro natrenovani detektoru.
		cout << "Preparing negative examples:" << flush;
		for( int i = 0; i < negativeSampleCount; i++){

			// vyber nahodneho obrazku a pozice
			const int imgID = rand() % negativeImages.size();
			const int x = rand() % ( negativeImages[ imgID].cols - descriptor.winSize.width);
			const int y = rand() % ( negativeImages[ imgID].rows - descriptor.winSize.height);

			// hlavicka pro vyrez - nekopiruji se data
			Mat cutImage( negativeImages[ imgID], Rect( x, y, descriptor.winSize.width, descriptor.winSize.height));

			// Doplnte vypocet priznakoveho vektoru z cutImage.
			vector< float> desc;
            /** FILL DONE **/
			descriptor.compute(cutImage, desc);

			// Pridat do sady trenovacich vzorku tentokrat s negativni tridou
			trainingData.push_back( desc);
			trainingLabels.push_back( -1);

			if( generalCounter++ % 100 == 0){
				cout << "." << flush;
			}
		}

		// priprava dat do formatu pro trenovaci algoritmus
		const int dimension = (int) trainingData[0].size();
		vector< TFloatRep *> svmData;

		for( int i = 0; i < (int)trainingData.size(); i++){
			svmData.push_back( new TFloatRep[ dimension]);
			for( int j = 0; j < dimension; j++){
				svmData.back()[ j] = trainingData[ i][ j];
			}
		}
		cout << " DONE " << endl;


		// Objekt pro trenovani klasifikatoru.
		TSVMSgd svm( dimension, 0.001, "HINGELOSS");

		cout << "Training SVM with stochastic gradient descent: " << endl;
		// perform several iterations of SGD 
		for( int i = 0; i < svmIterations; i++){
			svm.train( svmData, trainingLabels);
		}
		cout << " DONE " << endl;


		// Doplnte nastaveni klasifikatoru svm.getW() pro descriptor (detektor).
		/**FILL DONE**/
        descriptor.setSVMDetector(svm.getW());
	} 

	// Otevreni videa
	VideoCapture capture( inputVideoName);

	if( !capture.isOpened()){
		cerr << "Error: Unable to open input video file \"" << inputVideoName << "\"." << endl;
        return -1;
	}



	bool finished = false;

	Mat grayImage;

	while( !finished){ // 
		
		// Doplnte nacteni dalsiho snimku.
		Mat frame;
		bool s1 = capture.grab();
		bool success = capture.retrieve( frame);

		finished |= !success;

		if( success){

			// prevod na sedotonovy obraz
			cvtColor( frame, grayImage, CV_BGR2GRAY );

			// pro nalezene objekty
			vector< Rect> objects;

			// Doplnte detekci ve více rozlišeních s paramtery:
			// * prah detekce: threshold
			// * krok ve velikosti: 1.1
			// * jinak default parametry
			/**FILL DONE**/
            descriptor.detectMultiScale(frame, objects, threshold, Size(), Size(), 1.1);

			// vypis souradnic nalezenych objektu
			for( int i = 0; i < (int) objects.size(); i++){
				cout << '\t' << objects[ i].x << '\t' << objects[ i].y << '\t' << objects[ i].x + objects[ i].width << '\t' << objects[ i].y + objects[ i].height;
			}
			cout << endl;


			if( VISUAL_OUTPUT){
			
				// vykresleni nalezenych objektu
				for( int i = 0; i < (int) objects.size(); i++){
					rectangle( frame, objects[ i], Scalar( 255));
				}

				imshow( "Local features", frame);

				switch( waitKey( 2) & 255){
					case 'u':
						threshold += 0.1f;
						break;
					case 'j':
						threshold -= 0.1f;
						break;
					case 'i':
						threshold += 1.0f;
						break;
					case 'k':
						threshold -= 1.0f;
						break;
					case 'X':
					case 'q':
					case 'Q':
		 				finished = true;
						break;
				}
				cout << "Threshold " << threshold << endl;
			}
		}
	}
}