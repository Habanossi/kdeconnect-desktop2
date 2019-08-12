//#include "matplotlibcpp.h"
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
//#include "resample.h"
#include <Eigen/Core>
//</home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/eigen-eigen-323c052e1731/Eigen/Core>
#include "AudioRead.cpp"

using namespace std;

void fingerPrint(){
	cout << "fingerPrint() start.\n";

	//INIT VARIABLES
	QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
	QString recordFile = "testRec.dat";
	QString outputfile = "fingerprint.txt";
	bool liveRecord = 0;
	int sampleRate = 16000;
	int sampleRateTarget = 16000;
	int fpAmount = 1;

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

	vector<double> dataRaw = {};
	QFile inFile(dataDirectory + recordFile);
	if(!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << "something wrong with inFile";
	}
	QTextStream inc(&inFile);
	double value;
	do{
		value = inc.readLine().toDouble();
		dataRaw.push_back(value);
	}while(!inc.atEnd());							//All data moved from testRec.dat to dataRaw, kinda unnecessary to put it in textfile, could fix
	inFile.close();

	//Make sure the sampling rate is 16kHz, resample if not
	vector<double> dataV = {};
	vector<double> dataResampled = {};
	/*if(sampleRate != sampleRateTarget){
		resample(sampleRateTarget,sampleRate,dataRaw,dataV);
	}
	else {
		for(auto i = dataRaw.begin(); i != dataRaw.end(); i++) dataV.push_back(*i);
	}*/
	for(auto i = dataRaw.begin(); i != dataRaw.end(); i++) dataV.push_back(*i);

	cout << "size of dataRaw (data from audio recording): " << dataRaw.size() << endl;
	cout << "size of dataV (data from audio recording): " << dataV.size() << endl; //43521 when prerec

	//Split the data sequence into windows, use windowing.cpp
	double frameLength = sampleRate * 0.03; 	//flattop: 0.03
	double hopSize 	= sampleRate * 0.02; 	//flattop: 0.02 (2/3 overlap)
	Eigen::MatrixXd frameMatrix = window(dataDirectory, recordFile, frameLength, hopSize, "flattop", dataV);
	cout << "size of matrix from windowing: " << frameMatrix.rows() << " * " <<  frameMatrix.cols() << endl;

	//FFTW
	QString energyfile;
	int frameStart = 0;
	for(int i = 0; i < fpAmount; i++){
		outputfile = "fingerprintdata/fingerprint" + QString::number(i) + ".txt";
		energyfile = "energydata/energy" + QString::number(i) + ".txt";
		calculations(frameMatrix, outputfile, energyfile, frameStart);
	}
}
