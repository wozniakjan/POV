/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha 4a
// Autor: Michal Hradiš <ihradis@fit.vutbr.cz>
// Cil: Spojeni obrazu podle korespondenci lokalnich priznaku. Hleda se transformace homografie pomoci algoritmu RANSAC
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/
// * Muzete porovnat, jake jsou rozdily, kdyz na vstupu prohodite poradi obrazku.
/////////////////////////////////////////////////////////////////////////////////////////////


// Muzete si vypnout vykreslovani na obrazovku.
#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <cxcore.h>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>

using namespace std; 
using namespace cv;

// porovnani pro serazeni korespondenci od nejlepsi
bool compareDMatch( const DMatch& a, const DMatch &b){
	return a.distance < b.distance;
}

int main( int argc, char* argv[])
{
	// jmena souboru pro zpracovani
	string imageName1;
	string imageName2;


	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-i1" && i + 1 < argc){
			imageName1 = argv[ ++i];
		} else if( string(argv[ i]) == "-i2" && i + 1 < argc){
			imageName2 = argv[ ++i];
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << "  -i1 imageName1 -i2 imageName2" << endl;
			cout << "Merges two images into one. The images have to share some common area and have to be taken from one location." << endl;
			return 0;
		} else {
			cerr << "Error: Unrecognized command line parameter \"" << argv[ i] << "\" use -h to get more information." << endl;
		}
	}

	// kontrola zadani parametru
	if( imageName1.empty() || imageName2.empty()){
		cerr << "Error: Some mandatory command line options were not specified. Use -h for more information." << endl;
		return -1;
	}


	// nacteni sedotonovych obrazku 
	Mat img1 = imread( imageName1, 0);
	Mat img2 = imread( imageName2, 0);

	if( img1.data == NULL || img2.data == NULL){
		cerr << "Error: Failed to read input image files." << endl;
		return -1;
	}

	// SURF detektor lokalnich oblasti
	SurfFeatureDetector detector;

	// samotna detekce lokalnich priznaku
	vector< KeyPoint> keyPoints1, keyPoints2;
	detector.detect( img1, keyPoints1);
	detector.detect( img2, keyPoints2);
	cout << keyPoints1.size() << " " << keyPoints2.size();

	// extraktor SURF descriptoru
	SurfDescriptorExtractor descriptorExtractor;

	// samonty vypocet SURF descriptoru
	Mat descriptors1, descriptors2;
	descriptorExtractor.compute( img1, keyPoints1, descriptors1);
	descriptorExtractor.compute( img2, keyPoints2, descriptors2);

	// tento vektor je pouze pro ucely funkce hledajici korespondence
	vector< Mat> descriptorVector2;
	descriptorVector2.push_back( descriptors2);

	// objekt, ktery dokaze snad pomerne efektivne vyhledavat podebne vektory v prostorech s vysokym poctem dimenzi
	FlannBasedMatcher matcher;
	// Pridani deskriptoru, mezi kterymi budeme pozdeji hledat nejblizsi sousedy
	matcher.add( descriptorVector2);
	// Vytvoreni vyhledavaci struktury nad vlozenymi descriptory
	matcher.train();

	// nalezeni nejpodobnejsich descriptoru (z obrazku 2) pro descriptors1 (oblasti z obrazku 1)
	vector<cv::DMatch > matches;
	matcher.match( descriptors1, matches);

	// serazeni korespondenci od nejlepsi (ma nejmensi vzajemnou vzdalenost v prostoru descriptoru)
	sort( matches.begin(), matches.end(), compareDMatch);
	// pouzijeme jen 200 nejlepsich korespondenci
	matches.resize( min( 200, (int) matches.size()));

	// pripraveni korespondujicich dvojic
	Mat img1Pos( matches.size(), 1, CV_32FC2);
	Mat img2Pos( matches.size(), 1, CV_32FC2);

	// naplneni matic pozicemi
	for( int i = 0; i < (int)matches.size(); i++){
		img1Pos.at< Vec2f>( i)[0] = keyPoints1[ matches[ i].queryIdx].pt.x;
		img1Pos.at< Vec2f>( i)[1] = keyPoints1[ matches[ i].queryIdx].pt.y;
		img2Pos.at< Vec2f>( i)[0] = keyPoints2[ matches[ i].trainIdx].pt.x;
		img2Pos.at< Vec2f>( i)[1] = keyPoints2[ matches[ i].trainIdx].pt.y;
	}

	// Doplnte vypocet 3x3 matice homografie s vyuzitim algoritmu RANSAC. Pouzijte jdenu funkci knihovny OpenCV.
	/** FILL DONE **/
	Mat homography = findHomography( img1Pos, img2Pos, CV_RANSAC );


	// vystupni buffer pro vykresleni spojenych obrazku
	Mat outputBuffer( 1024, 1280, CV_8UC1);

	// Vysledny spojeny obraz budeme chtit vykreslit do outputBuffer tak, aby se dotykal okraju, ale nepresahoval je.
	// "Prilepime" obrazek 2 k prvnimu. Tuto "slepeninu" je potreba zvetsit a posunout, aby byla na pozadovane pozici.
	// K tomuto potrebujeme zjistit maximalni a minimalni souradnice vykreslenych obrazu. U obrazu 1 je to jednoduche, minima a maxima se 
	// ziskaji primo z rozmeru obrazu. U obrazku 2 musime pomoci drive ziskane homografie promitnout do prostoru obrazku 1 jeho rohove body.

	float minX = 0;
	float minY = 0;
	float maxX = (float) img1.cols;
	float maxY = (float) img1.rows;

	// rohy obrazku 2
	vector< Vec3d> corners;
	corners.push_back( Vec3d( 0, 0, 1));
	corners.push_back( Vec3d( img2.cols, 0, 1));
	corners.push_back( Vec3d( img2.cols, img2.rows, 1));
	corners.push_back( Vec3d( 0, img2.rows, 1));

	// promitnuti rohu obrazku 2 do prosotoru obrazku 1 a upraveni minim a maxim
	for( int i = 0; i < (int)corners.size();i ++){

		// Doplnte transformaci Mat( corners[ i]) do prostoru obrazku 1 pomoci homography.
		// Dejte si pozor odkud kam homography je. Podle toho pouzijte homography, nebo homography.inv().
		/**FILL ALMOST DONE**/
		Mat projResult = homography.inv() * Mat( corners[ i]);// * homography;

		minX = std::min( minX, (float) (projResult.at<double>( 0) / projResult.at<double>( 2)));
		maxX = std::max( maxX, (float) (projResult.at<double>( 0) / projResult.at<double>( 2)));
		minY = std::min( minY, (float) (projResult.at<double>( 1) / projResult.at<double>( 2)));
		maxY = std::max( maxY, (float) (projResult.at<double>( 1) / projResult.at<double>( 2)));
	}




	// Posuneme a zvetseme/zmenseme vysledny spojeny obrazek tak, by vysledny byl co nejvetsi, ale aby byl uvnitr vystupniho bufferu.

	// Zmena velikosti musi byt takova, aby se nam vysledek vesel na vysku i na sirku
	double scaleFactor = min( outputBuffer.cols / ( maxX - minX), outputBuffer.rows / ( maxY - minY));

	// Doplnte pripraveni matice, ktera zmeni velikost (scaleMatrix) o scaleFactor a druhe (translateMatrix), ktera posune vysledek o -minX a -minY. 
	// Po tomto bude obrazek ve vystupnim bufferu.
	Mat scaleMatrix = Mat::eye( 3, 3, CV_64F);
	Mat translateMatrix = Mat::eye( 3, 3, CV_64F);
	/**FILL DONE**/
    scaleMatrix.at<double>(0,0) = scaleFactor;
    scaleMatrix.at<double>(1,1) = scaleFactor;

    translateMatrix.at<double>(0,2) = -(double)minX; 
    translateMatrix.at<double>(1,2) = -(double)minY;
   
    cout << endl << minX << " " << minY << endl << translateMatrix << endl << endl;
    
	Mat centerMatrix = scaleMatrix * translateMatrix;


	// Transformace obrazku 1 
	warpPerspective( img1, outputBuffer, centerMatrix, outputBuffer.size(), 1, BORDER_TRANSPARENT);

	// Transformace obrazku 2 
	warpPerspective( img2, outputBuffer, centerMatrix * homography.inv(), outputBuffer.size(), 1, BORDER_TRANSPARENT);

	cout << "normMatrix" << endl;
	cout << centerMatrix << endl << endl;

	cout << "normMatrix" << endl;
	cout << homography << endl << endl;

#if VISUAL_OUTPUT
	imshow( "IMG1", img1);
	imshow( "IMG2", img2);
	imshow( "MERGED", outputBuffer);
	waitKey();
#endif
}
