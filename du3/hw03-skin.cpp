/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha 3a
// Autor: Michal Hradiš <ihradis@fit.vutbr.cz>
// Cil: Segmentace obrazu podle barvy (a odstinu) - Hue a saturation z HSV modelu.
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/
// * Odhad rozlozeni barvy objektu zajmu z oznacenych vyrezu. 
// * Odhad rozlozeni barvy ve videu.
// * Vypocet pravdepodobnosti podle Bayessova vzorce, ze se u daneho pixelu jedna o objekt zajmu.
// * Pouzity pravdepodobnostni pristup neni dokonaly, nektere hodnoty jsou rucne nastavene,
//   evidence (background model) neni uplne korektrni a likelihood (model barvy objektu)
//   nebude pomoci rucniho oznacovani take presny. Z techto duvodu je nutne vysledky 
//   interpretovat opatrne a vysledne pravdepodobnosti brat s rezervou. Na druhou stranu 
//   jsou tyto vysledky lepe interpretovatelne, nez primo hodnoty z modelu popredi.
// * Vysledky segmentace budou hodne zaviset na kvalite zaznamu. Komprese velmi silne mrvi 
//   barevne slozky obrazu a sum pri nizsich intenzitach barev ma silny vliv na slozky H a S.
//   Idealni je dobra kamera a nekomprimovany vstup.
/////////////////////////////////////////////////////////////////////////////////////////////


// Muzete si vypnout vykreslovani na obrazovku.
#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std; 
using namespace cv;


// Spocita histogram z prvnich dvou kanalu (H a S) img a pripocita je k hodnotam v histogram.
// Predpoklad je, ze 
// Tato iplementace je pomerne neefektivni.
void addHistogram( Mat img, Mat histogram)
{
	assert( img.channels() == 3 && img.depth() == CV_8U);
	assert( histogram.channels() == 1 && histogram.depth() == CV_64F);

	for( int y = 0; y < img.rows; y++){
		for( int x = 0; x < img.cols; x++){
			Vec3b val = img.at< Vec3b>( y, x);
			histogram.at<double>( val.val[0] * histogram.rows / 256, val.val[ 1] * histogram.cols / 256) += 1;
		}
	}
}

// Obdelnik je zadany.
bool filled = false;
// Obdelnik je zadavan - zmacknute leve tlacitko mysi.
bool filling = false;
// Souradnice obdelniku zadaneho mysi.
int px1, py1, px2, py2;

// Callback pro obsluhu mysi.
void onMouse( int currentEvent, int x, int y, int, void* )
{
	if( currentEvent == CV_EVENT_LBUTTONDOWN){
		filling = true;
		px1 = x;
		py1 = y;
		px2 = x;
		py2 = y;
	} else if( currentEvent == CV_EVENT_MOUSEMOVE && filling){
		filling = true;
		px2 = x;
		py2 = y;
	} else if( currentEvent == CV_EVENT_LBUTTONUP && filling){
		filled = true;
		px2 = x; 
		py2 = y;
	}
}


// prevod do barevneho modelu, na jehot prvnich dvou slozkach bude probihat segmentace podle barvy
const int cvtColorCode = CV_RGB2HSV;


int main( int argc, char* argv[])
{
	// jmeno souboru pro zpracovani
	string inputVideoName;
	// jmeno souboru ze ktereho muze byt vytvoren prvnotni model hledane barvy
	string positiveImage;
	int binCount = 128;
	// Má být model pozadí pøizpùsobován sledovanému videu?
	bool updateBackgroundModel = true;

	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-i" && i + 1 < argc){
			inputVideoName = argv[ ++i];
		} else if( string(argv[ i]) == "-p" && i + 1 < argc){
			positiveImage = argv[ ++i];
		} else if( string(argv[ i]) == "-bf"){
			updateBackgroundModel = false;
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << "  -i inputVideoName [-p positiveImageName -bf]" << endl;
			cout << "When running use 't' to stop video playback and use left mouse button and dragging to select a region with object of iterest to build color model." << endl;
			cout << "-bf        Background model is fixed and will not be addapted to the video." << endl;
			return 0;
		} else {
			cerr << "Error: Unrecognized command line parameter \"" << argv[ i] << "\" use -h to get more information." << endl;
		}
	}

	// kontrola zadani parametru
	if( inputVideoName.empty()){
		cerr << "Error: Some mandatory command line options were not specified." << endl;
		return -1;
	}

	// Otevreni videa
	VideoCapture capture( inputVideoName);

	if( !capture.isOpened()){
		cerr << "Error: Unable to open input video file \"" << inputVideoName << "\"." << endl;
        return -1;
	}

	// histogram hledane barvy
	Mat positiveHistogram( binCount, binCount, CV_64F);
	// histogram barvy celeho obrzu - je pouzito jako evidence v Baysove vzorci
	Mat backgroundHistogram( binCount, binCount, CV_64F);

	// pozitivni histogram vynulovat
	positiveHistogram.setTo( 0);

	// vyplneni nenulovou hodnotou pro rozumne chovani - i barvy, ktere jsme nevideli maji nenulovou pravdepodobnost.
	backgroundHistogram.setTo( 1);

	// vyplneni pozitivniho histogramu ze zadaneho obrazku
	if( !positiveImage.empty()){

		Mat img = imread( positiveImage, CV_LOAD_IMAGE_COLOR);

		// budeme pracovat v jinem barevnem modelu
		Mat hsvImage;
		cvtColor( img, hsvImage, cvtColorCode);

		if( img.data == NULL){
			cerr << "Error: Unable to read image file \"" << positiveImage << "\"." << endl;
		}

		// pridani do histogramu
		addHistogram( hsvImage, positiveHistogram);
	}

	bool finished = false;
	Mat grayImage;
	Mat frame;
	
	// ronormalizovany likelihood (to nahore v Bayesove vzorci)
	Mat likelihood;
	// normalizovana evidence (to dole v Bayesove vzorci)
	Mat evidence;
	// rozlozeni posteriorni pravdepodobnosti (to, co vyjde z Bayesova vzorce)
	Mat posterior;
	// Prostor pro posteriorni pravdepodobnosti jednotlivych pixelu
	Mat objectProb( (int) capture.get( CV_CAP_PROP_FRAME_HEIGHT), (int) capture.get( CV_CAP_PROP_FRAME_WIDTH), CV_64F);

	if( VISUAL_OUTPUT){
		// vytvoreni okna a pridani callbacku, ktery umozni oznacovat obdelniky pro vytvareni barevneho modelu
		namedWindow( "Input");
		setMouseCallback( "Input", onMouse, 0 );
	}

	while( !finished){ // 
		
		// nacteni dalsiho snimku
		bool success = capture.read( frame);

		finished |= !success;

		if( success){

			// budeme pracovat v jinem barevnem modelu
			Mat hsvImage;
			cvtColor( frame, hsvImage, cvtColorCode);

			// update histogramu pozadi
			if( updateBackgroundModel){
				addHistogram( hsvImage, backgroundHistogram);
			}


			// normalizace histogramu pro ziskani likelihood a evidence rozlozeni
			likelihood = positiveHistogram / sum( positiveHistogram).val[0];
			evidence = backgroundHistogram / sum( backgroundHistogram).val[0];

			// vylazeni rozlozeni, protoze je mame v celkev vysokem rozliseni
			//GaussianBlur( likelihood, likelihood, Size( 7, 7), 0, 0);
			//GaussianBlur( evidence, evidence, Size( 7, 7), 0, 0);

			// Bayesovsky vzorec - vypocet pravdepodobnosti, ze urcita barva patri hledanemu objektu.
			// Prior je nastaveny na 0.1, coz odpovida tomu, ze ocekavam, ze hledany objekt zabere v prumeru 10 % obrazu.
			posterior = likelihood * 0.1 / ( 0.9 * evidence + likelihood * 0.1);


			// 2D lookup tabuka 
			// Doplnte prevod barev pixelu na posteriorni pravdepodobnost.
			// Kazdy pixel v objectProb nastavte na hodnotu z posterior odpovidajici barve kanalu 0 a 1 daneho pixelu v hsvImage.
			// Vypocet pozice v posterior by mel odpovidat tomu v addHistogram().
			/**FILL DONE**/
            for(int x = 0; x < hsvImage.rows; x++){
                for(int y= 0; y < hsvImage.cols; y++){
                    objectProb.at<double>(x,y) = 
                            posterior.at<double>(hsvImage.at<cv::Vec3b>(x,y)[0]/2, hsvImage.at<cv::Vec3b>(x,y)[1]/2);
                }
            }

			// Vypocet plochy u ktere je pravdepodobnost, ze se jedna o hledanou barvu vetsi nez 0.5.
			Mat segmentedColor;
			compare( objectProb, 0.5, segmentedColor, CMP_GE);
			cout << "Object area: " << countNonZero( segmentedColor) << endl;

			if( VISUAL_OUTPUT){

				// evidence je lepe videt v mensim dynamickem rozsahu
				log( evidence, evidence);

				// normalizace do rozsahu 0-1 pro zobrazeni
				// Posterior a objectProb se nenormalizuji, protoze vzdy musi byt v rozsahu 0 az 1.
				Mat normLikelihood;
				Mat normEvidence;
				normalize( likelihood, normLikelihood, 0, 1, NORM_MINMAX);
				normalize( evidence, normEvidence, 0, 1, NORM_MINMAX);
				

				imshow( "Input", frame);
				imshow( "Output", objectProb);
				imshow( "Posterior (probability of being foreground)", posterior);
				imshow( "Likelihood (foreground color model)", normLikelihood);
				imshow( "Evidence (bakground color model)", normEvidence);

				bool finishInnerLoop = false;

				switch( waitKey( 2) & 255){
					case 'x':
					case 'X':
					case 'q':
					case 'Q':
						finished = true;
						break;

					case 't': // zastaveni prehravani a moznost oznacit objekt mysi

						filling = false;
						filled = false;
						
						while( !finishInnerLoop){

							// kopie obrazu
							Mat frameCopy = frame.clone();

							// vykresleni obdelniku, pokud tahnu mysi
							if( filling){
								rectangle( frameCopy, Point( px1, py1), Point( px2, py2), Scalar( 255), 2);
							}

							// update modelu po zadani obdelniku
							if( filled){
								filling = false;
								filled = false;

								// s mysi se da tahnout i mimo obraz
								px1 = min( hsvImage.cols - 1,  max( 0, px1));
								px2 = min( hsvImage.cols - 1,  max( 0, px2));
								py1 = min( hsvImage.rows - 1,  max( 0, py1));
								py2 = min( hsvImage.rows - 1,  max( 0, py2));

								// aktualizace barevneho modelu objektu
								addHistogram( Mat( hsvImage, Rect( min( px1, px2), min( py1, py2), abs( px1 - px2), abs( py1 - py2))), positiveHistogram);
							}


							imshow( "Input", frameCopy);

							switch( waitKey(2) & 255){
								case 't':
								case ' ':
									finishInnerLoop = true;
									break;
								case 'x':
								case 'X':
								case 'q':
								case 'Q':
									finished = true;
									finishInnerLoop = true;
									break;
							}
						}
						break;
				}
			}
		}
	}
}