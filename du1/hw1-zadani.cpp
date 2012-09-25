/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha 1
// Autor: Michal Hradiš <ihradis@fit.vutbr.cz>
// Cil: zakladni seznameni s knihovnou OpenCV
// * nacitani/ukladani obrazku a videa
// * zakladni operace zpracovani obrazu
// * pristup k elementum obrazu
// * operace s maticemi
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/.
// * Využívejte C++ rozhraní OpenCV
// * Vetsinou se jedna o volani jedne funkce nebo metody.
// * Dokumentace k OpenCV je treba na http://opencv.itseez.com/
// * Vysledny kod musi byt prelozitelny na serveru merlin. Nesmite vyuzivat zadne dalsi knihovny krome OpenCV.
// * Odevzdava se pouze tento zdojovy kod.
// * S dotazy a pripominkami se obracejte na autora.
// * Pokud narazite na nejaky vetsi problem, je mozne domluvit i osobni konzultaci.
/////////////////////////////////////////////////////////////////////////////////////////////


// Muzete si vypnout vykreslovani na obrazovku nastavenim na 0.
#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <iostream>
#include <sstream>

using namespace std; 
using namespace cv;

void fillCheckerboard( Mat img);
Mat  customNonlinearFilter( Mat img, const float angle);
Mat  lazyConvertToGray( Mat img);

int main( int argc, char* argv[])
{
	string inputImageName;
	string inputVideoName;
	string outputDirectory;
	uint64 randSeed = 0;

	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-ii" && i + 1 < argc){
			inputImageName = argv[ ++i];
		} else if( string(argv[ i]) == "-iv" && i + 1 < argc){
			inputVideoName = argv[ ++i];
		} else if( string(argv[ i]) == "-od" && i + 1 < argc){
			outputDirectory = argv[ ++i];
		} else if( string(argv[ i]) == "-s" && i + 1 < argc){
			stringstream str( argv[ ++i]);
			str >> randSeed;
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << "  -ii inputImageName -iv inputVideoName [-od outputDirectory -s randSeed]" << endl;
			return 0;
		} else {
			cerr << "Error: Unrecognized command line parameter \"" << argv[ i] << "\" use -h to get more information." << endl;
		}
	}

	// kontrola zadani parametru
	if( inputImageName.empty() || inputVideoName.empty()){
		cerr << "Error: Some mandatory command line options were not specified." << endl;
		return -1;
	}

	// generator nahodnych cisel
	RNG rndGen( randSeed);



	// Doplnte nacteni obrazku ze souboru inputImageName do inputImage 
	// tak, aby byl zachovan pocet barevnych kanalu.
	// Nápovìda: souèást highgui
	Mat inputImage;
	inputImage = imread(inputImageName, -1);


	// Kontrola uspesneho nacteni obrazku
	if( inputImage.data == NULL)	{
		cerr <<  "Error: Failed to read image file \"" << inputImageName << "\"." << endl ;
		return -1;
	}


	// Doplnte kontrolu poctu kanalu (1, nebo 3) a bitove hloubky inputImage.
	if( (inputImage.channels() != 1 && inputImage.channels() != 3) || inputImage.depth() != CV_8U){
		cerr << "Error: Input image has wrong format." << endl;
		return -1;
	}

	// Doplnte kontrolu velikosti inputImage. 
	// Musí být minimálnì 300px siroky a 250px vysoky
	if( inputImage.cols < 300 || inputImage.rows < 250){
		cerr << "Error: Input image is too smal." << endl;
		return -1;
	}


	// Prevod na sedotonovy obraz, nebo kopie odkazu na data, pokud uz je inputImage sedotonovy
	Mat grayImage = lazyConvertToGray( inputImage);


	// Doplnte vytvoreni reference roiImage na oblast zajmu (ROI) v grayImage 
	// na pozici x=100, y=25 s sirkou 50px, vyskou 150px.
	// napoveda: vyuzijte konstruktor cv::Mat
	Mat roiImage(grayImage, Rect(100,25,50,150));
	/**FILL DONE**/

	// Vyplneni roiImage sachovnici s hodnotami 0 a 255. Funkci musite napsat.
	fillCheckerboard( roiImage);

	// Dopnte hlubokou kopi roiImage (kopie dat) do checkerboardCopy.
	// checkerboardCopy nepujde zmenit zapisem do roiImage
	// Napoveda: hledejte v Basic Structures, metody tridy Mat
	Mat checkerboardCopy = roiImage.clone();
	/**FILL DONE**/



	// Doplòte vyhlazení grayImage pomoci Gaussovskeho filtru o velikosti 11x11 a
	// se standardni odchylkou 2 v horizontalnim i vertikalnim smeru
	// Napoveda: Je na to specializovana funkce. Hledejte v "Image Filtering".
	/**FILL DONE**/
	GaussianBlur(grayImage, grayImage, Size(11, 11), 2.0, 2.0);


	// checkerboardCopy by nemelo byt ovlivneno predchozi filtraci grayImage
	imwrite( outputDirectory + "/checkerboard.jpg", checkerboardCopy);
	if( VISUAL_OUTPUT){
		imshow( "Checkerboard", checkerboardCopy);
		waitKey();
	}

	// Doplnte zmenu velikosti grayImage na 200x200 s interpolaci nejblizsiho souseda. Vysledek ulozte do resizedImage.
	// Napoveda: fce. resize
	Mat resizedImage(200, 200, CV_32F);
	/**FILL DONE**/
	resize(grayImage, resizedImage, Size(200, 200), INTER_NEAREST);


	for( int i = 0; i <= 9; i++){

		// Nas specialni filtr - gradienty v urcitem smeru. Funkci musite napsat.
		Mat filteredImage = customNonlinearFilter( resizedImage, 2 * 3.14 / 10 * i);

	
		// Normalizujte hodnoty filteredImage do rozsahu <0,255> a prevedte na CV_8U. Vysledek ulozte do outImage.
		// Napoveda: je to funkce z "Operations on Arrays"
		Mat outImage;
		/**FILL DONE**/
		normalize(filteredImage, outImage, 0, 255, NORM_MINMAX, CV_8U);

		stringstream str("");
		str << "Filtered_" << i;
		string name = outputDirectory + "/" + str.str() + ".jpg";
		imwrite( name, outImage);


		if( VISUAL_OUTPUT){
			stringstream str;
			str << "Filtered_" << i;
			imshow( str.str(), outImage);
		}
	}

	// Otevreni videa
	VideoCapture capture( inputVideoName);

	if( !capture.isOpened()){
		cerr << "Error: Unable to open input video file \"" << inputVideoName << "\"." << endl;
        return -1;
	}


	// Doplnte vypis vlastnosti videa.
	// Napoveda: Metoda VideoCapture
	/**FILL DONE**/
	cout << "Video " << inputVideoName << "\twidth " << capture.get(CV_CAP_PROP_FRAME_WIDTH) << "\theight " << capture.get(CV_CAP_PROP_FRAME_HEIGHT) 
	     << "\tFPS " << capture.get(CV_CAP_PROP_FPS) << "\tframe_count " << capture.get(CV_CAP_PROP_FRAME_COUNT) << endl;


	// Otevrete vystupni video se stejnymi vlastnostmi jako ma vstupni video
	/**FILL DONE**/
	const string outputVideoName = outputDirectory + "/videoOut.avi";
    VideoWriter writer( outputVideoName, capture.get(CV_CAP_PROP_FOURCC), capture.get(CV_CAP_PROP_FPS), Size(capture.get(CV_CAP_PROP_FRAME_WIDTH),capture.get(CV_CAP_PROP_FRAME_HEIGHT)));
   
	if( !writer.isOpened()){
		cerr << "Error: Unable to open output video file \"" << outputVideoName << "\"." << endl;
        return -1;
	}


	bool finished = false;
	while( !finished){
		
		// Doplnte nacteni dalsiho snimku.
		// Napoveda: Metoda VideoCapture
		/** FILL DONE **/
		Mat frame;
		bool success = true;
		capture >> frame;
        if(frame.empty()){
            success = false;
        }

		finished |= !success;
		
		if( success){

			// Rychly detektor vyznamnych oblasti
			StarFeatureDetector detector( 32);
			vector<KeyPoint> keypoints;

			Mat grayFrame = lazyConvertToGray( frame);
			detector.detect( frame, keypoints); 

			for( int i = 0; i < (int) keypoints.size(); i++){

				// Doplnte vykresleni kruznic znazornujicich nalezene vyznamne oblasti keypoints do frame s barvou Scalar( 255)
				// Napoveda: funkce v "Drawing Functions"
				/**FILL DONE**/
				circle(frame, keypoints[i].pt, keypoints[i].size, Scalar(255));
			}

			writer.write( frame);

			if( VISUAL_OUTPUT){
			
				imshow( "Local features", frame);

				switch( waitKey( 2)){
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


	//vytvorte matici typu CV_64F s 100000-ci radky a ctyrmi sloupci 
	//Napoveda: Konstruktor Mat 
	Mat A(100000, 4, CV_64F); /** FILL DONE **/

	// vyplneni matice hodnotami s normalnim rozlozenim
	rndGen.fill( A, RNG::NORMAL, Scalar( 0), Scalar( 1));

	// Bez volani calcCovarMatrix spocitejte kovariancni matici pro A a ulozte do covM.
	// napoveda: s maticemi se daji provadet operace +, -, *, ... pomocí pretizenych operatoru
	//           hledejte v "Basic structures" "Matrix Expressions"
	//           vleze se to na jeden radek
	//           http://en.wikipedia.org/wiki/Covariance_matrix
	Mat covM(4, 4, CV_64F);
    Scalar mean[4], stddev[4], m, d;
    
    for(int i = 0; i < 4; i++){
       meanStdDev(A.col(i), mean[i], stddev[i]);
       covM.at<double>(i,i) = pow(stddev[i][0],2);
    }
    
    for(int i = 0; i < 4; i++){
       for(int j = 0; j < i; j++){
           meanStdDev(A.col(i).mul(A.col(j)), m, d);
           covM.at<double>(i,j) = m[0] - (mean[i][0] * mean[j][0]);
           covM.at<double>(j,i) = m[0] - (mean[i][0] * mean[j][0]);
       }
    }
	// vypis kovariancni matice - mela by byt 4x4 s jednickama na diagonale a nulama mimo
	cout << covM;
}

Mat  lazyConvertToGray( Mat img)
{
	// Doplnte prevedeni na sedotonovy obraz.
	// Pokud vstupni obraz jiz je sedotonovy, proveïte mìlkou kopii bez manipulace obrazových dat
	// Napoveda: cvtColor
	/** FILL DONE **/
	if( img.channels() == 1){
		return img;
	} else {
		Mat dst;
		cvtColor(img, dst, CV_RGB2GRAY);
        return dst;
	}
}


void fillCheckerboard( Mat img)
{
	// Funkce vyplni img sachovnicovym vzorem s hodnotami 255 a 0.
	// Sirka policek sachovnice je 3px a vyska 6px.
	// V levem hornim rohu je hodnota 0.
	// Postaci, kdyz bude funkcni pouze pro jednokanalovy obraz s bitovou hloubkou CV_8U.
	unsigned int color = 0;
    unsigned int size_mod = cvCeil((float)img.cols / 3.0) % 2;   
    
	/** FILL DONE **/	 
	for(int r = 0; r < img.rows; r+=6){
		for(int c = 0; c < img.cols; c+=3){
			rectangle(img, Point(c,r), Point(c+3,r+6), color, CV_FILLED);
			if(color == 0) color = 255;
            else color = 0;
		}
		if(!size_mod){
           if(color == 0) color = 255;
           else color = 0;
        }
	}
	
}


Mat  customNonlinearFilter( Mat img, float angle)
{
	// Postaci, kdyz bude funkcni pouze pro jednokanalovy obraz a bitovou hloubkou CV_8U.
	// Muzete predpokladat, ze angle bude v rozsahu <0,2*pi>
	//
	// Nelinearni filtr, jehoz vystupem je velikost gradientu pro urcity smer normalizovany velikostmi gradientu v blizkem okoli.
	// Vystup odpovida velikosti prispeveku pixelu do jednoho binu smeroveho histogramu podobne jako 
	// v Histograms of Oriented Gradients (HOG), ktere se pouzivaji napriklad v SIFT a detekci objektu (napriklad chodcu)
	// Obdobne reprezentace obrazu se pocitaji i v lidskem vizualnim systemu.
	//
	// Filtr by mel pocitat:
	// I2(x,y) = ( I_m(x,y) * exp( -2 * ADiff( I_o( x, Y), angle)) )  / ( I_mG(x,y) + e)
	// Kde:
	// I_m(x,y) = sqrt( I_dx(x,y)^2 + I_dy(x,y)^2) - velikost gradientu v bode x,y
	// I_dx - derivace v horizontalnim smeru (konvoluce s jadrem [-0.5 0 0.5])
	// I_dx - derivace ve vertikalnim smeru (konvoluce s transponovanym jadrem [-0.5 0 0.5])
	// I_o  - uhel smeru gradientu v radianech
	// ADiff - absolutni hodnoa rozdilu dvou uhlu
	// I_mG = I_m * G - Je obraz velikosti gradientu konvoluovany s Gaussovym filtrem se standardni odchylkou 6 pixelu a velikosti 19x19px.
	// e - mala kladna hodnota, ktera omezuje sum v oblastech bez vyrazneho gradientu
	//
	// Vyuzijte v maximalni mire funkce OpenCV. To obsahuje funkce pro:
	// konvoluci, vypocet I_m a I_o z I_dx a I_dy 
	// a umoznuje standardni matematicke oparace s maticemi

	const float e = 50;
	float horKernelPtr[3] = {-0.5, 0, 0.5};
	Mat horKernel( 1, 3, CV_32F, horKernelPtr);
	Mat verKernel = horKernel.t(); // transpozice

	// vypocet I_dx, I_dy - konvoluce je fce. OpenCV
	// Napoveda: Funkce pro obecnou 2D filtraci (konvoluci) je v "Image Filtering"
	Mat I_dx(img.size(), CV_32F), I_dy(img.size(), CV_32F);

	/**FILL DONE**/ 
	filter2D(img, I_dx, -1, horKernel); 
	filter2D(img, I_dy, -1, verKernel);
    I_dx.convertTo(I_dx, CV_32F);
    I_dy.convertTo(I_dy, CV_32F);
	
	// Dopnte vypocet velikosti a uhlu gradientu z parcialnich derivaci.
	// Napoveda: OpenCV ma funkci na prevod z kartezskych do polarnich souradnic (uhel a vzdalenost/magnitude) v "Operations on Arrays"
	Mat I_m(img.size(), CV_32F), I_o(img.size(), CV_32F);
    cartToPolar(I_dx, I_dy, I_m, I_o, angle);

	// Dopnte vypocet uhlovych vzdalenosti gradientu v I_o s pozadovanym smerem angle
	// Vypocitat to pro jeden gradient jde napriklad:
	//   float diff = I_o_value - angle;
	//	 if( diff < -3.14){
	//	     diff = 2 * 3.14 + diff;
	//   } else if( diff > 3.14){
	//       diff = 2 * 3.14 - diff;
	//   }
	//	 float reult = abs( diff);
    /** FILL DONE **/
	Mat ADiff(I_dx.size(), CV_32F);
    float diff;
	for(int r = 0; r < ADiff.rows; r++){
        for(int c = 0; c < ADiff.cols; c++){
            diff = I_o.at<float>(r,c) - angle;
            if(diff < -3.14){
                diff = 2 * 3.14 + diff;
            } else if( diff > 3.14){
                diff = 2 * 3.14 - diff;
            }
            ADiff.at<float>(r,c) = abs(diff);
        }
    }
	ADiff = -2 * ADiff;

	Mat EADiff(ADiff.size(), CV_32F);
	exp( ADiff, EADiff);
	
	// Doplnte konvoluci s  Gaussovym filtrem se standardni odchylkou 6 pixelu a velikosti 19x19px
	// Napoveda: Je na to specializovana funkce. Hledejte v "Image Filtering".
	Mat I_mG(img.size(), CV_32F);
	///FILL DONE
    GaussianBlur(img, I_mG, Size(19, 19), 6, 6);
    I_mG.convertTo(I_mG, CV_32F);
    
	// doplnte vypocet  result(x,y) = I_m(x,y) * EADiff(x,y) / ( I_mG(x,y) + e)
	// napoveda: s maticemi se daji provadet operace +, -, *, ... pomocí pretizenych operatoru
	//           hledejte v "Basic structures" "Matrix Expressions"
	Mat result(I_m.size(), CV_32F);///FILL DONE
    result = I_m.mul(EADiff) / (I_mG + e);

	return result;
}

