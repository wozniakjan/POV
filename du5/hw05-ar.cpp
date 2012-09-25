/////////////////////////////////////////////////////////////////////////////////////////////
// Fakulta informacnich technologii Vysokeho uceni technickeho v Brne
// Pocitacove videni 
// Domaci uloha X
// Autor: Marek Solony <isolony@fit.vutbr.cz>
// Cil: zoznamenie sa s vyhladavanim sachovnicoveho vzoru, 3D projekciou a zakadom AR
// * nacitanie videa
// * extrakcia bodov sachovnice
// * zpresnenie polohy detekovanych bodov
// * projekcia 3D bodov do 2D obrazu kamery
// Navod:
// * Doplnte kod v mistech oznacenych /**FILL**/.
// * Vetsinou se jedna o volani jedne funkce nebo metody.
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
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <sstream>

using namespace std; 
using namespace cv;

bool print_first = false;

int main( int argc, char* argv[])
{
	//definujme parametre modelu kamery
	Mat cam_D;	//parametre skreslenia (matica 4x1)
	Mat cam_A;  //3x3 matica vnutornych parametrov [fx 0 cx; 0 fy cy; 0 0 1]
				//fx, fy - ohniskova vzdialenost, cx, cy - stredny bod

	//kamera bola vopred kalibrovana (vypocitane parametre ), nacitanie parametrov kamery zo suboru, like a boss
	FileStorage fs("calibration_data.xml", FileStorage::READ);
	fs["DL"] >> cam_D;
	fs["AL"] >> cam_A;
	fs.release();

	//nacitanie videa s ktorym budeme pracovat
	VideoCapture vid;
	vid.open("vid.avi");
	Mat img;	//jeden frame

	//pre vsetky frames videa
	while(true)
	{
		//nacitanie framu z videa do premennej img
		/**FILL DONE**/
		vid >> img;

		//test konca videa
		if(img.empty())
			break;

		//zobraz frame
		if( VISUAL_OUTPUT)
		{
			imshow( "win", img );
			waitKey( 0 );
		}

		//kameru budeme lokalizovat pomocou vzoru - sachovnice o rozmeroch 8x6 (vnutorne rohy)
		//sachovnica bude urcovat suradnicovu sutavu sceny, bude urcovat pociatok suradnicovej sustavy
		//a tak isto osi suradnicoveho systemu. Jednotka bude jeden sachvnicovy stvorec.
		// - lavy horny (vnutorny) roh bude pociatok suradnicovej sustavy - bude mat poziciu [0, 0, 0]
		// - kratsia hrana (na ktorej lezi pociatok sustavy) reprezentuje os Y
		// - dlhsia hrana raprezentuje os X
		// - os Z prechadza pociatkom sur. sustavy a je kolma na osi X a Y.
		// - rovina sachovnice (vsetky jej body) lezia v rovine Z (Z = 0)
		// - sustava je RIGHT-handed!
		//
		//Pre vypocet polohy kamery v takto definovanej suradnicovej sustave potrebujeme
		//najst vztahy medzi 3D bodmi sachovnice a ich 2D obrazmi.
		//Preto v obraze vyhladame 2D obrazy (vnutornych) rohov sachovnice a priradime
		//im ich skutocnu 3D poziciu (napr: lavy horny roh detekovany v obraze na pozicii [x, y] 
		//ma 3D poziciu [0, 0, 0], najblizsi roh v smere osi X ma [1, 0, 0] ... )

		//vyhladanie rohov sachovnice v obrazku
		Size chess(8, 6);		//rozmery sachovnice
		vector<Point2f> corners;	//sem ulozime pozicie detekovanych bodov

		//doplnte parametre funkcie podla dokumentacie OpenCV
		/**FILL DONE**/
		if (findChessboardCorners(	img, 
									chess, 
									corners, 
									CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE ) == 0 )
		{
			cerr << "Fail, nenasla sa sachovnica v obrazku." << endl;
			continue;
		}

		//funkcia findChessboardCorners vyhlada a zoradi v obrazku rohy sachovnice
		//polohy bodov su priblizne a daju sa este spresnit, co vzdy zlepsuje vysledok
		//o spresnenie sa stara funkcia cornerSubPix, ktora spresni pozicie
		//bodov na sub-pixlovej urovni
		//na pouzitie tejto funkcie potrebujeme najprv prekonvertovat nas frame ma grayscale

		Mat gray;	//grayscale obrazok
		/** FILL DONE **/
		cvtColor(img, gray, CV_RGB2GRAY);		//konverzia img obrazku do greyscale obrazku gray

		//doplnte podla dokumentacie OpenCV
        /** FILL ALMOST DONE **/
		cornerSubPix(	gray, 
						corners, 
						Size(11,11),		//okolie spesnovacieho okna
						Size(-1,-1),		//zero zone
						TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));	//kriteria ukoncenia spresnovania

		//zobrazenie rohov
		drawChessboardCorners( img, chess, corners, 1 );
		if( VISUAL_OUTPUT)
		{
			imshow( "win", img );
			waitKey( 0 );
		}

		//zostavme zoznam 3D pozicii ktore zodpovedaju detekovanym 2D rohom sachovnice
		//poradie v akom su radene za sebou je mozne vidiet na predchadzajucom obrazku
		//(lavy horny roh = 1. bod, a nasledovne su postupne za sebou po riadkoch)
		//(dlhsia os(X) = inkrementujeme Xovu poziciu)
		//(kratsia os(Y) = inkrementujeme Yovu poziciu)
		vector<Point3f> pts3D;		//zoznam 3D pozicii
		for( int j = 0; j < chess.height; j++ )
			for( int k = 0; k < chess.width; k++ )
                /** FILL DONE **/
				pts3D.push_back( Point3f(	k,		//X 
											j,		//Y
											0 ) );	//Z, vsetky maju Z hodnotu 0 (su v jednej rovine)


		//Teraz by sme mali mat vsetky potrebne data na vypocet pozicie kamery vzhladom na
		//sachovnicu
		//Najprv spravime zo zoznamov matice prvkov
		Mat image_pts(corners);		//3D body objektu
		Mat object_pts(pts3D);		//zodpovedajuce 2D body

		//Funkcia SolvePnP vracia rotacny vektor R a pozicny vektor T pomocou ktorych sa
		//transformuju body zo suradnicovej sustavy sceny do suradnicovej sustavy kamery
		//cize definuju vzajomnu polohu kamery a sachovnice
		Mat R, T;
		//vyplnte podla dokumenacie OpenCV
        /** FILL DONE **/
		solvePnP(	object_pts,		
					image_pts,		
					cam_A,		    //camera matrix
					cam_D,		    // dist coeffs matrix, nejspis blbe zadane
					R,				//rotacia
					T,				//translacia
					false);			


		//Vykreslenie nasho virtualneho telesa bude vyzerat nasledovne:
		//1. vytvorime si zoznam 3D bodov telesa (pracujeme v suradnicovej sustave sachovnice)
		//2. zavolame funciu ProjectPoints, ktora pomocou parametrov R a T vypocita
		//na aku 2D poziciu v obrazku sa tieto body premietnu

		//Nakreslime virtualnu kocku {FAQ: Co je to kocka? A: Kocka je krychle} s dlzkou hrany 5x5x5 
		//do zaciatku suradnicovej sustavy
		//POZOR !!! suradnicovy system je RIGHT-handed, takze kladna os Z ide v nasom pripade smerom 
		//od kamery do sceny, takze aby kocka vystupovala ku kamere, dame Z hodnoty zaporne
		vector<Point3f> cube3D;
		cube3D.push_back( Point3f(0, 0, 0) );	//	\-
		cube3D.push_back( Point3f(5, 0, 0) );	//	 \-	"Prve poschodie"
		cube3D.push_back( Point3f(5, 5, 0) );	//	 /-
		cube3D.push_back( Point3f(0, 5, 0) );	//	/-
		cube3D.push_back( Point3f(0, 0, -5) );	//	\-
		cube3D.push_back( Point3f(5, 0, -5) );	//	 \-	"Druhe poschodie"
		cube3D.push_back( Point3f(5, 5, -5) );	//	 /-
		cube3D.push_back( Point3f(0, 5, -5) );	//	/-

		Mat cb(cube3D);				//zo zoznamu bodov spravime maticu prvkov
		vector<Point2f> cube2D;		//zoznam premietnutych 2D bodov
		//funkcia projectPoints premieta 3D body sceny do zobrazovacej roviny kamery (do nasho 
		//obrazku)
		//doplnte podla dokumentacie OpenCV
		/** FILL DONE**/
		projectPoints(	object_pts, 
						R, 
						T, 
						cam_A, //camera matrix
						cam_D, //dist coeffs matrix
						cube2D);
		//v zozname cube2D sa teraz nachadzju 2D body do ktorych sa premietli 3D body kocky
		//(v rovnakom poradi ako bol vstup)

		//vystup (kvoli kontrole)
		if( !print_first )
			for( int a = 0; a < cube2D.size(); a++ )
			{
				cout << (int)cube2D.at(a).x << " " << (int)cube2D.at(a).y << endl;
				print_first = true;
			}
			

		//vykresli kocku do obrazku
		line(img, cube2D.at(0), cube2D.at(1), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(1), cube2D.at(2), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(2), cube2D.at(3), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(3), cube2D.at(0), CV_RGB(0, 255, 0), 3, 8, 0);

		line(img, cube2D.at(4), cube2D.at(5), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(5), cube2D.at(6), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(6), cube2D.at(7), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(7), cube2D.at(4), CV_RGB(0, 255, 0), 3, 8, 0);

		line(img, cube2D.at(0), cube2D.at(4), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(1), cube2D.at(5), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(2), cube2D.at(6), CV_RGB(0, 255, 0), 3, 8, 0);
		line(img, cube2D.at(3), cube2D.at(7), CV_RGB(0, 255, 0), 3, 8, 0);

// 		//zobraz vystup
 		if( VISUAL_OUTPUT)
		{
			imshow( "win", img );
			waitKey( 0 );
		}
	}

	vid.release();
}