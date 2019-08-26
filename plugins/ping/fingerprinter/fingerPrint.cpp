#include <iostream>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <vector>
#include <QString>
#include "string.h"
#include "windowing.cpp"
#include "Wavreader.cpp"
#include <fftw3.h>
#include "calculations.cpp"
#include <Eigen/Core>
#include "AudioRead.cpp"
#include <thread>

using namespace std;

void fingerPrint(){	
	cout << "fingerPrint() start.\n";
	//INIT VARIABLES
	QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
	QString recordFile = "testRec.dat";
	QString energyfile;
	QString outputfile;
	bool liveRecord = true;
	int sampleRate = 16000;

	//Record audio or Read audio from file, assess sampling rate
	if(liveRecord){
		AudioRead(dataDirectory, recordFile);
		sampleRate = 16000;
	}
	else {
		recordFile = "raw.dat";
		cout << "readWav() start.\n";
		sampleRate = readWav();
	}
	cout << "Sample rate is: " << sampleRate << endl;

	//Apply windowing on data
	double frameLength = sampleRate * 0.03; 	//flattop: 0.03
	double hopSize 	= sampleRate * 0.02; 	//flattop: 0.02 (2/3 overlap)
	Eigen::MatrixXd frameMatrix = window(dataDirectory, recordFile, frameLength, hopSize, "flattop");
	cout << "size of matrix from windowing: " << frameMatrix.rows() << " * " <<  frameMatrix.cols() << endl;

	//Apply FFT, extract time-averaged energybands, apply decorrelation, extract fingerprint
	int frameStart = 0;
	outputfile = "fingerprintdata/fingerprint0.txt";
	energyfile = "energydata/energy0.txt";
	calculations(frameMatrix, outputfile, energyfile, frameStart);
}
