/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha 3a
// Autor: Michal Hradiš <ihradis@fit.vutbr.cz>
// Cil: Segmentace obrazu podle textury
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/
// * Ukazka vyuziti histogramu LBP pro segmentaci obrazu.
// * Obraz je segmentovan do predem znamych typu oblasti, pro které jsou potreba trenovaci data.
// * Pro kazdou typ textury se trenuje jeden SVM klasifikator, ktery rozeznava tuto tridu od vsech ostatnich.
// * Priznakove vektory jsou histogramy LBP extrahovane z maleho okoli (32x32). 
//   Timto oknem se pak prochazy cely obraz a pro kazdy pixel klasifikatory rozhodnou, do ktereho typu textury patri.
// * Priklad je funkcni, ale pro prakticke pouziti by bylo potreba kalibrovat jednotlive klasifikatory tak,
//   aby napriklad hodnota 5 u vsech klasifikatoru znamenala stejnou jistotu, ze dana oblast patri dane texture.
//   Bez kalibrace se muze napriklad stat, ze se vsechny oblasti systematicky prirazuji temer pouze jedne tride.
/////////////////////////////////////////////////////////////////////////////////////////////


// Muzete si vypnout vykreslovani na obrazovku.
#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

#include "sgdSVM.h"
#include <math.h>


using namespace std; 
using namespace cv;


void LBPTransform( Mat img, Mat &lbp)
{

	assert( img.channels() == 1 && img.depth() == CV_8U);

	// vysledek budek o dva radky a dva sloupce mensi, protoze LBP neni definovane pro krajni pixely.
	lbp = Mat( img.rows - 2, img.cols - 2, CV_8U);

	// Doplnte vypocet LBP pro kazdy pixel vstupniho obrazu img a vysledek ulozte do lbp.
	// Iplementujte zakladni verzi LBP na okoli 3x3 pixelu, ktera prokuhuje 8-bit kod.
	// Popis zakladni verze LBP najdete napriklad v:
	// Topi Maenpaa: THE LOCAL BINARY PATTERN APPROACH TO TEXTURE ANALYSIS  – EXTENSIONS AND APPLICATIONS, http://herkules.oulu.fi/isbn9514270762/
	// 2.5.1 The original LBP --- na strane 26
	/**FILL DONE**/
    
    int val = 0;
    double angle = 0.0;
    for(int r = 1; r < (img.rows-1); r++){
        for(int c = 1; c < (img.cols-1); c++){
            angle = 0.0;
            val = 0;
            for(int i = 0; i<8; i++){
                val = val << 1;
                angle += M_PI/4;
                
                if((int)img.at<unsigned char>(r,c) < (int)img.at<unsigned char>(r+round(sin(angle)),c+round(cos(angle)))){
                    val += 1;
                }
            }
            lbp.at<unsigned char>(r-1,c-1) = val;
        }
    }            
}

// Vypocet 1D histogramu
void computeHistogram( Mat img, vector< TFloatRep> &hist, const TFloatRep bit)
{

	assert( img.channels() == 1 && img.depth() == CV_8U);

	hist.resize( 256);

	for( int y = 0; y < img.rows; y++){
		
		unsigned char * ptr = img.ptr( y);

		for( int x = 0; x < img.cols; x++){
			hist[ ptr[ x]] += bit;
		}
	}
}


// Vypocet LBP histogramu
void computeLBPHistogram( Mat img, vector< TFloatRep> &hist, const TFloatRep bit)
{
	
	Mat lbp;
	
	LBPTransform( img, lbp);

	computeHistogram( lbp, hist, bit);
}

// Funkce nacte obrazky ze seznamu v souboru se jmenem fileName, vypocita LBP histogramy pro nahodne vyrezy z techto obrazku a ulozi je v data
void readDataset( string fileName, const int targetSampleCount, vector< vector< TFloatRep> > &data, int windowWidth = 32, int windowHeight = 32)
{
	// otevreni souboru se seznamem obrazku 
	ifstream file( fileName.c_str());

	if( !file.good()){
		cerr << "Error: Unable to open file \"" << fileName << "\"." << endl;
		exit( -1);
	}


	// nacteni obrazku
	cout << "Reading images from \"" << fileName << "\"." << endl;
	vector< Mat> images;
	while( file.good()){

		// nacteni jednoho jmena ze souboru
		string imgName;
		file >> imgName;

		// nacteni sedotonoveho obrazku 
		Mat img = imread( imgName, 0);

		if( img.data == NULL){
			cerr << "Error: Unable to read image file \"" << imgName << "\"." << endl;
			continue;
		}

		// obrazek musi byt vetsi nez okno klasifikatoru
		if( img.cols < windowWidth || img.rows <= windowHeight){
			cerr << "Error: Unable image file \"" << imgName << "\" resolution is too small." << endl;
			continue;
		}

		// pridat do sady negativnich obrazku
		images.push_back( img);
	}
	cout << " DONE " << endl;

	data.reserve( data.size() + targetSampleCount); 

	// Vypocet priznakovych vektoru (histogramu LBP) pro nahodne vyrezy z nactenych obrazku.
	// Tyto vzorky budou pouzity jako negativni priklady pro natrenovani detektoru.
	cout << "Computing LBP histograms:" << flush;
	for( int i = 0; i < targetSampleCount; i++){

		// vyber nahodneho obrazku a pozice
		const int imgID = rand() % images.size();
		const int x = rand() % ( images[ imgID].cols - windowWidth);
		const int y = rand() % ( images[ imgID].rows - windowHeight);

		// hlavicka pro vyrez - nekopiruji se data
		Mat cutImage( images[ imgID], Rect( x, y, windowWidth, windowHeight));

		// vypocet priznakoveho vektoru
		vector< TFloatRep> desc;
		computeLBPHistogram( cutImage, desc, 1.0 / (windowWidth *windowHeight) );

		// Pridat do sady trenovacich vzorku tentokrat s negativni tridou
		data.push_back( desc);
	}
	cout << " DONE " << endl;
}



int main( int argc, char* argv[])
{
	// barevna paleta pro zobrazeni segmentace
	vector< Vec3b> palette;
	palette.push_back( Vec3b( 255, 0, 0));
	palette.push_back( Vec3b( 0, 255, 0));
	palette.push_back( Vec3b( 0, 0, 255));
	palette.push_back( Vec3b( 255, 0, 0));
	palette.push_back( Vec3b( 255, 255, 0));
	palette.push_back( Vec3b( 0, 255, 255));
	palette.push_back( Vec3b( 255, 0, 255));
	palette.push_back( Vec3b( 255, 255, 255));



	vector< string> imageNames;
	vector< string> classImageLists;

	// pocet trenovacich vzorku pro kazdou tridu
	const int targetSampleCount = 50000;
	// sirka skenovaciho okna pro segmentaci
	const int windowWidth = 32;
	// vyska skenovaciho okna pro segmentaci
	const int windowHeight = 32;
	// pocet pruchodu datovou sadou pri trenovani SVM
	const int svmIterations = 20;
	// pocet dimenzi priznakovych vektoru - LBP histogramy maji 256
	const int dimension = 256;


	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-c" && i + 1 < argc){
			classImageLists.push_back( argv[ ++i]);
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << "  -c classList1 -c classList2 [-c classList3 ...] image1 [image2 image3 ...]" << endl;
			return 0;
		} else {
			imageNames.push_back( argv[ i]);
		}
	}

	// kontrola zadani parametru
	if( (int) classImageLists.size() < 2){
		cerr << "Error: At leas two classes have to be specified." << endl;
		return -1;
	}

	// zvladame jen urcity pocet trid
	if( classImageLists.size() > palette.size()){
		cerr << "Warning: Too many classes. Restricting number of classes to " << palette.size() << "." << endl;
		classImageLists.resize( palette.size());
	}

	// pro ulozeni trenovacich dat
	vector< vector< vector< TFloatRep> > > data( classImageLists.size());

	// nacteni trenovacich dat pro vsechny tridy
	for( int i = 0; i < (int) classImageLists.size(); i++){
		readDataset( classImageLists[ i], targetSampleCount, data[ i], 32, 32);
	}

	// priprava trenovacich dat pro klasifikator
	vector< vector< double> > labels( classImageLists.size());
	vector< TFloatRep *> trainingData;

	for( int i = 0; i < (int) classImageLists.size(); i++){
		for( int j = 0; j < (int) data[ i].size(); j++){

			trainingData.push_back( &data[ i][ j][0]);

			for( int k = 0; k < (int)classImageLists.size(); k++){
				if( i == k){
					labels[k].push_back( +1);
				} else {
					labels[k].push_back( -1);
				}
			}
		}
	}

	// nahodne promichani trenovacich dat
	for( int i = 0; i < 100000; i++){
		int id1 = rand() % trainingData.size();
		int id2 = rand() % trainingData.size();
		swap( trainingData[ id1], trainingData[ id2]);

		for( int k = 0; k < (int)classImageLists.size(); k++){
			swap( labels[ k][ id1], labels[ k][ id2]);
		}
	}


	// misto pro klasifikatory jendotlivych trid
	vector< TSVMSgd *> classifiers;

	for( int i = 0; i < (int) classImageLists.size(); i++){
		
		classifiers.push_back( new TSVMSgd( 256, 0.000001, "HINGELOSS"));

		cout << "Training SVM for " << classImageLists[ i] << " with stochastic gradient descent: " << endl;
		// perform several iterations of SGD 
		for( int iter = 0; iter < svmIterations; iter++){
			classifiers.back()->train( trainingData, labels[ i]);
		}
		cout << " DONE " << endl;
	}



	// segmentace se provede pro kazdy testovaci obrazek
	for( int i = 0; i < (int) imageNames.size(); i++){

		// nacteni dalsiho obrazku
		Mat img = imread( imageNames[ i], 0);

		if( img.data == NULL){
			cerr << "Error: Unable to read image file \"" << imageNames[ i] << "\"." << endl;
			continue;
		}

		// Vypsani jmena a velikosti obrazku, aby bylo trochu poznat, ze se neco deje. Pro velke obrazky muze segmentace trvat i nekolik desitek sekund.
		cout << imageNames[ i] << ' ' << img.cols << ' ' << img.rows << endl;

		// Misto pro odezvy LBP
		Mat lbp;
		
		// Vypocet LBP
		LBPTransform( img, lbp);


		// prostor pro vysledky segmentace
		Mat segmentIDs( lbp.rows, lbp.cols, CV_8U);
		Mat segmentMap( lbp.rows, lbp.cols, CV_8UC3);
		segmentMap.setTo(0);

		// misto pro odezvy jednotlivych klasifikatoru
		vector< Mat> segmentations;
		for( int i = 0; i < (int) classifiers.size(); i++){
			segmentations.push_back( Mat( lbp.rows - windowHeight, lbp.cols - windowWidth, CV_64F));
			segmentations.back().setTo(0);
		}

		// prispevek jednoho pixelu k histogramu - Je zvoleny tak, aby suma vysledneho histogramu byla 1.
		const TFloatRep bit = 1.0 / (TFloatRep) windowWidth / (TFloatRep) windowHeight;


		// Obraz je skenovan po sloupcich. Pri pohybu skenovaciho okna na nizsi radek je vzdy 
		// dolni radek pricten k histogramu a horni radek odecten. Tento postup je mnohem rychlejsi,
		// nez pocitat histogram pro kazkou pozici okna samostatne. 
		for( int x = 0; x < lbp.cols - windowWidth; x += 1){

			// misto pro LBP histogram skenovaciho okna
			vector< TFloatRep> descriptor( 256, 0);

			// vypocet histogramu obdelniku na nultem radku
			Mat initRectangle( lbp, Rect( x, 0, windowWidth, windowHeight));
			computeHistogram( initRectangle, descriptor, bit);

			// ukazatel na horni radek
			unsigned char *ptrBack = lbp.ptr( 0, x);
			// ukazatel na dolni radek
			unsigned char *ptrFront = lbp.ptr( windowHeight, x);

			// pruchod az k poslednimu radku
			for( int y = 0; y < lbp.rows - windowHeight; y ++){

				// Doplnte aktualizace histogramu (descriptor) odectenim horniho radku skenovaciho okna a prictenim dolniho radku.
				// ptrBack ukazuje na pocatek horniho radku
				// ptrFront ukazuje na pocatek dolniho radku
				/** FILL **/             
                for(int i = 0; i < windowWidth; i += 1){
                    descriptor[ptrBack[i]] -= bit;
                    descriptor[ptrFront[i]] += bit;
                }      
                
				// Vypocet odezev klasifikatoru a uchovani nejvyssi odezvy.
				double bestResponse = -1e20;
				int bestID = 0;

				for( int i = 0; i < (int) classifiers.size(); i++){
					
					const double response = classifiers[ i]->eval( &descriptor[0]);

					if( response > bestResponse){
						bestResponse = response;
						bestID = i;
					}

					// Pozdeji zobrazime odezvy jednotlivych klasifikatoru.
					segmentations[ i].at<double>( y, x) = response;
				}


				// prirazeni barvy aktualnimu pixelu podle zjistene tridy textury
				segmentMap.at< Vec3b>( y, x) = palette[ bestID];

				// prirazeni ID textury aktualnimu pixelu
				segmentIDs.at< unsigned char>( y, x) = bestID;

				// posunutiho horniho a dolniho radku o jeden nize
				ptrBack += lbp.step1( 0);
				ptrFront += lbp.step1( 0);
			}
		}

		// Vypocet plochy jednotlivych textur
		cout << "Texture areas (px):";
		for( int i = 0; i < (int) classifiers.size(); i++){
			Mat temp = segmentIDs == i;
			cout << ' ' << countNonZero( temp);
		}
		cout << endl;

		if( VISUAL_OUTPUT){

			// zobrazeni zpracovaneho obrazku
			imshow( "original", img);
		
			// zobrazeni segmentove mapy
			imshow( "segments", segmentMap);

			// zobrazeni odezev jednotlivych klasifikatoru
			for( int i = 0; i < (int) segmentations.size(); i++){
				normalize( segmentations[ i], segmentations[ i], 0, 1, NORM_MINMAX);

				stringstream str;
				str << "Segmentation for class " << i;
				imshow( str.str(), segmentations[ i]);
			}

			waitKey();
		}
	}
}