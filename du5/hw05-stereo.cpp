/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha X
// Autor: Marek Solony <isolony@fit.vutbr.cz>
// Cil: zoznamenie sa s kalibraciou stereo-kamier a vypoctom hlbkovej mapy
// * extrakcia bodov sachovnice
// * kalibracia stereo systemu
// * rektifikacia obrazkov z kamier
// * vypocet hlbkovej mapy
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/.
// * Vetsinou se jedna o volani jedne funkce nebo metody.
// * Vysledny kod musi byt prelozitelny na serveru merlin. Nesmite vyuzivat zadne dalsi knihovny krome OpenCV.
// * Odevzdava se pouze tento zdojovy kod.
// * S dotazy a pripominkami se obracejte na autora.
// * Pokud narazite na nejaky vetsi problem, je mozne domluvit i osobni konzultaci.
/////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <iterator>
#include <string>

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

// Muzete si vypnout vykreslovani na obrazovku nastavenim na 0.
#define VISUAL_OUTPUT 1

#define NUM_IMAGES  13		//pocet kalibranych obrazkov pre kazdu kameru

static bool readStringList( const string& filename, vector<string>& l );

int main(int argc, char* argv[])
{
	//Kalibracia stereosystemu prebieha pomocou kalibracneho telesa (sachovnice)
	//ktory musi byt v zabere oboch kamier zaroven.
	//
	//V kazdom z kalibracnych obrazkov (v lavych aj pravych) detekujeme sachovnicu.
	//Nasledne spravime zoznam 3D bodov sachovnice presne ako v minulej ulohe (AR)
	//Zavolame kalibracu funkciu pre tieto tri zoznamy.


	//nacitanie zoznamu obrazkov kalibracneho telesa z lavej a z pravej kamery
	Mat imgs[2][NUM_IMAGES];	//prvy index urcuje ci sa jedna o lavy [0] alebo pravy [1] snimok
								//druhy index bude urcovat cislo dvojice zodpovedajucich si obrazkov

	vector<string> imagelist;
	//subor imagelist.xml obsahuje nazvy obrazkov ktore budeme nacitavat
	bool ok = readStringList( "imagelist.xml", imagelist);
    if(!ok || imagelist.empty())
	{
        cout << "can not open imagelist.xml or the string list is empty" << endl;
        return 0;
    }

	for( int a = 0; a < 2; a++ ) 
	{
		for( int b = 0; b < NUM_IMAGES; b++ ) 
		{
			imgs[a][b] = imread( imagelist[b + NUM_IMAGES*a], 0 );	//nacitanie

			if( imgs[a][b].empty() )
			{
				cout << "failed to load images, aborting" << endl;
				return 0;
			}
		}
	}



	//pre kalibraciu potrebujeme zoznam vnutornych rohov sachovnice z lavej a pravej kamery
	//rohy z kazdeho kalibracneho obrazku si ulozime do zoznamu
	//prvy zoznam bude obsahovat rohy z lavych snimkov
	//druhy zoznam bude obsahovat rohy z pravych snimkov
	//zoznamy bodov
	vector<vector<Point2f> > imagePoints[2];
	imagePoints[0].resize(NUM_IMAGES);			//body z lavych snimkov
	imagePoints[1].resize(NUM_IMAGES);			//body z pravych snimkov
	//definujeme velkost sachovnice
	Size chess(9, 6);

	//v kazdom z nacitanych obrazkov potrebujeme najst rohy sachovnice
	for( int a = 0; a < 2; a++ ) 
	{
		for( int b = 0; b < NUM_IMAGES; b ++ )
		{
			//doplnete podla dokumentacie OpenCV
			/** FILL DONE **/
			bool found = findChessboardCorners(	imgs[a][b], 
												chess, 
												imagePoints[a][b], 
												CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
			if( !found )
			{
				cout << "nenasla sa sachovnica v obrazku (nemalo by sa v nasom pripade stat)" << endl;
				return 0;
			}

			//spresneinie polohy bodov na subpixlovej urovni
			//doplnte podla dokumentacie OpenCV
			/** FILL DONE **/
			cornerSubPix(	imgs[a][b], 
							imagePoints[a][b], 
							Size(5,5), 
							Size(-1,-1),
							TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,30, 0.01));

			//vykreslenie rohov
			if( VISUAL_OUTPUT ) {
				drawChessboardCorners( imgs[a][b], chess, imagePoints[a][b], found );
				imshow( "1", imgs[a][b] );
				waitKey( 0 );
			}
		}
	}



	//vytvorime si 3D body zodpovedajuce detekovanym bodom sachovnice
	//(podobne ako v minulej ulohe (AR))
	//len tento krat ich budeme potrebovat pre kazdu dvojicu kalibracnych snimkov
	vector<vector<Point3f> > objectPoints;
	objectPoints.resize(NUM_IMAGES);
	for( int i = 0; i < NUM_IMAGES; i++ )
    {
		for( int j = 0; j < chess.height; j++ )
			for( int k = 0; k < chess.width; k++ )
                /** FILL DONE **/
				objectPoints[i].push_back(Point3f(	k, 
													j, 
													0));
    }



	//vytvorenie parametrov kamier pre lavu a pravu kameru
	Mat D_left;	 //parametre skreslenia
	Mat A_left;  //matica vnutornych parametrov [fx 0 cx; 0 fy cy; 0 0 1]

	Mat D_right; //parametre skreslenia
	Mat A_right; //matica vnutornych parametrov [fx 0 cx; 0 fy cy; 0 0 1]

	Mat R, T, E, F;
	//parametre nasledovne vypocitame spolu aj s maticou R a vektorom T,
	//ktory udava vzajomnu polohu dvoch kamier
	//matice E a F nebudeme potrebovat, ak o nich chcete nieco vediet, pozrite
	//si dokumentaciu
	//doplnte podla dokumentacie OpenCV
    /** FILL DONE **/
	stereoCalibrate(objectPoints, 
					imagePoints[0], 
					imagePoints[1],
					A_left, D_left,
					A_right, D_right,
					imgs[0][0].size(), 
					R, T, E, F,
					TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),
					CV_CALIB_ZERO_TANGENT_DIST);


	//v nasledujucej casti vypocitame parametre rektifikacie
	//rektifikacia snimkov zkresli snimky tak, aby epipolarne ciary boli rovnobezne
	//( jednoducho povedane aby zodpovedajuce body lezali na tej istej Y pozicii )
	//
	//parametre rektifikacie
	Mat R1, R2, P1, P2, Q;

	//vypocet parametrov rektifikacie
	//doplnte podla dokumentacie OpenCV
    /** FILL DONE **/
	stereoRectify(	A_left,
					D_left,
					A_right,
					D_right,
					imgs[0][0].size(), 
					R,
					T,
					R1,
					R2,
					P1,
					P2,
					Q,
					CALIB_ZERO_DISPARITY);

	//predpocet map na premapovanie vstupnych obrazkov
	Mat rmapL[2];
	Mat rmapR[2];
	initUndistortRectifyMap(A_left, D_left, R1, P1, imgs[0][0].size(), CV_16SC2, rmapL[0], rmapL[1]);	//pre lavy snimok
	initUndistortRectifyMap(A_right, D_right, R2, P2, imgs[0][0].size(), CV_16SC2, rmapR[0], rmapR[1]);	//pre pravy snimok

	//nacitame obrazky s ktorymi budeme pracovat, pre ktore budeme pocitat hlbkovu mapu
	Mat left, 
		right, 
		left_rectified,		//rektifikovany lavy obrazok
		right_rectified;	//rektifikovany pravy obrazok
	left = imread( "left07.jpg", 1);
	right = imread( "right07.jpg", 1);

	//samotna rektifikacia obrazkov
	remap( left, left_rectified, rmapL[0], rmapL[1], CV_INTER_LINEAR );
	remap( right, right_rectified, rmapR[0], rmapR[1], CV_INTER_LINEAR );

	//zobrazime si rektifikovane (zarovnane obrazky)
	if( VISUAL_OUTPUT) {
		Mat canva( imgs[0][0].rows, imgs[0][0].cols*2, CV_8UC3 );
		Mat canvasPart1 = canva( Rect( 0, 0, imgs[0][0].cols, imgs[0][0].rows ) );
		resize(left_rectified, canvasPart1, canvasPart1.size(), 0, 0, CV_INTER_AREA);
		Mat canvasPart2 = canva( Rect( imgs[0][0].cols, 0, imgs[0][0].cols, imgs[0][0].rows ) );
		resize(right_rectified, canvasPart2, canvasPart2.size(), 0, 0, CV_INTER_AREA);
		for( int j = 0; j < canva.rows; j += 16 )
			line( canva, Point(0, j), Point(canva.cols, j), Scalar(0, 255, 0), 1, 8 );
		imshow( "win", canva );
		waitKey( 0 );
	}



	//teraz nastavime parametre vypoctu hlbkovej mapy
	//vypocet prebieha pomocou algoritmu semi-global block matching algorithm
	//mozete skusit hybat s parametrami ale odovzdavajte subor s povodnymi!
	StereoSGBM sgbm;
	sgbm.preFilterCap = 30;
	sgbm.SADWindowSize = 10;
	sgbm.P1 = 8*sgbm.SADWindowSize*sgbm.SADWindowSize;
	sgbm.P2 = 32*sgbm.SADWindowSize*sgbm.SADWindowSize;
	sgbm.minDisparity = 0;
	sgbm.numberOfDisparities = 128;
	sgbm.uniquenessRatio = 5;
	sgbm.speckleWindowSize = 150;
	sgbm.speckleRange = 32;
	sgbm.fullDP = true;

	//samotny vypocet depth mapy
	Mat disparity;	//obrazok do ktoreho ulozime hlbkovu mapu
	//doplnte podla dokumentacie OpenCV
	/** FILL DONE **/
	sgbm( left, right, disparity );

	//konverzia na float hodnoty
	Mat disparityF( left_rectified.size(), CV_32FC1 );
	convertScaleAbs( disparity, disparityF, 1.0/16, 0.0 );

	//zbavime sa small-scale sumu (erozia + dilatacia)
	erode( disparityF, disparityF, Mat(), Point(-1, -1), 2, BORDER_CONSTANT, morphologyDefaultBorderValue() );
	dilate( disparityF, disparityF, Mat(), Point(-1, -1), 2, BORDER_CONSTANT, morphologyDefaultBorderValue() );

	//nemenit
	cout << disparity.at<short>(150, 270) << endl;
	cout << disparity.at<short>(240, 320) << endl;
	cout << disparity.at<short>(400, 400) << endl;
	cout << disparity.at<short>(400, 500) << endl;

	//vykreslenie hlbkovej mapy
	if( VISUAL_OUTPUT ) {
		imshow( "disparity", disparityF );
		waitKey( 0 );
	}

	return 0;
}

//funkcia nacitava zoznam stringov zo suboru
static bool readStringList( const string& filename, vector<string>& l )
{
    l.resize(0);
    FileStorage fs(filename, FileStorage::READ);
    if( !fs.isOpened() )
        return false;
    FileNode n = fs.getFirstTopLevelNode();
    if( n.type() != FileNode::SEQ )
        return false;
    FileNodeIterator it = n.begin(), it_end = n.end();
    for( ; it != it_end; ++it )
        l.push_back((string)*it);
    return true;
}