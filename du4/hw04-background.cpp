/*
 * hw04-background.cpp
 *		Domácí úkol 4a do předmětu POV
 *      Author: Ales Lanik
 *      Zadani: Tématem úkolu je práce s modelem pozadí a extrakcí kontur pohybujících se objektů.
 *      		Doplňte kód na vyznačených čtyřech místech.
 *      		Testovací video a demonstrační výstup naleznete na http://www.fit.vutbr.cz/~ilanik/POV/
 *              Dotazy posílejte autorovi.
 */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

CvScalar colors[] = { cvScalar(255, 0, 0, 255), cvScalar(0, 255, 0, 255),
		cvScalar(0, 0, 255, 255), cvScalar(255, 255, 255, 255) };

int main(int ac, char** av) {

	if (ac != 2) {
		cerr << "Param error\nExample:du4 videofile.avi\n" << endl;
		return 1;
	}
	std::string arg = av[1];
	VideoCapture capture(arg);

	if (!capture.isOpened()) {
		cerr << "Failed to open a video file!\n" << endl;
		return 1;
	}

	//Deklarace oken
	namedWindow("input", CV_WINDOW_KEEPRATIO);
	namedWindow("bg model", CV_WINDOW_KEEPRATIO);
	namedWindow("fg_mask", CV_WINDOW_KEEPRATIO);
	namedWindow("contours", CV_WINDOW_KEEPRATIO);

	Mat frame, fg_mask;
	BackgroundSubtractorMOG2 bg_model;

	for (;;) {
		//Získaní snímku videa
		capture >> frame;
		if (frame.empty())
			break;
		imshow("input", frame);

		//Updatování modelu pozadí
//--1---Provedte update modelu pozadi
        /** FILL DONE **/
		bg_model(frame, fg_mask);

		if (!fg_mask.empty()) {
//--2-------Vyfiltrujte šum v masce popředí pomocí morfologických operací dilatace a eroze (jako strukturní element zadejte volání konstruktoru Mat() ))
			/**FILL DONE**/
            dilate(fg_mask, fg_mask, Mat());
            erode(fg_mask, fg_mask, Mat());
			/**FILL DONE**/
			imshow("fg_mask", fg_mask);

			//Zobrazení aktuálního modelu pozadí
			Mat bgimg;
			bg_model.getBackgroundImage(bgimg);
			imshow("bg model", bgimg);

			//Nalezení kontur blobů
			vector<vector<Point> > contours;
			findContours(fg_mask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
			Mat contour_img;
			contour_img.create(frame.size(), frame.type());

			for (size_t k = 0; k < contours.size(); k++) {
//--3-----------Upravte podminku tak aby se vykreslovaly kontury s obsahem větším jak 100 px^2
				if (contours[k].size()>100) {
					CvScalar color = colors[k % 4];
					drawContours(contour_img, contours, k, color);

//--4---------------Vypočtěte ohraničující obálku kontury (bounding box) a vykreslete ji
					Rect boundingBox;
                    /**FILL DONE**/
					boundingBox = boundingRect(contours[k]);
					//Vykresleni bounding boxu
					rectangle(contour_img, boundingBox, color);
				}
			}
			imshow("contours", contour_img);
		}

		//Odchyt klávesy q nebo ESC
		char key = (char) waitKey(10);
		switch (key) {
		case 'q':
		case 'Q':
		case 27:
			return 0;
		case 'p':
			while(((char)waitKey(-1))!='p');
			break;
		}

	}
	return 0;
}

