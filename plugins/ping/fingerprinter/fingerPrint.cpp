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
//#include "resample.h"
#include <Eigen/Core>
//</home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/eigen-eigen-323c052e1731/Eigen/Core>
#include "AudioRead.cpp"

using namespace std;

int fingerPrint(){

	//init variables
	QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
	QString recordFile = "testRec.dat";
	bool avg = 1;
	bool liveRecord = 1;
	int sampleRate;
	int sampleRateTarget = 16000;


	//Record audio or Read audio from file, assess sampling rate
	if(liveRecord){
		AudioRead(dataDirectory, recordFile);
		sampleRate = 16000;
	}
	else sampleRate = readWav();
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
	cout << "size of dataV (data from audio recording): " << dataV.size() << endl;

	//Split the data sequence into windows, use windowing.cpp
	int frameLength = sampleRate * 0.03; 	//flattop: 0.03
	int hopSize 	= sampleRate * 0.02; 	//flattop: 0.02 (2/3 overlap)
	Eigen::MatrixXd frameMatrix = window(dataDirectory, recordFile, frameLength, hopSize, "flattop");
	cout << "size of matrix from windowing: " << frameMatrix.rows() * frameMatrix.cols() << endl;
	//FFTW
	double matrixSize = frameMatrix.rows() * frameMatrix.cols();					//total size of frameMatrix (n x m)
	cout << "all good\n"; 
	//cout << "matrixSize: " << matrixSize << endl;
	double* in = (double*) fftw_malloc(sizeof(double) * matrixSize);	//allocate fft-input array
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * matrixSize/2 + 1);	//allocate fft-output array
    fftw_plan p;														//define fft plan
	int N = 1024;
	QFile fftFile(dataDirectory + "fft.dat");
	if(!fftFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open fftFile";
	}
	QTextStream fft(&fftFile); 										//open textfile for data to matlab for testing
	vector<vector<double>> outV = {};									//vector for fft-output data
	vector<double> outVCol = {};
    p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);					//init fft-plan
	for(unsigned int i = 0; i < frameMatrix.rows(); i++){				//for each column in frameMatrix, set it as fft-input
		for(int j = 0; j < frameLength; j++){							//
			in[j] = frameMatrix(i,j);									//
		}																//
		fftw_execute(p);												//execute fft for each column
		for(int i = N/2+1; i > 0; i--){									//
			fft << 20*log10(abs(out[i][0] + out[i][1])) << endl;	//send output to textfile
			outVCol.push_back(20*log10(abs(out[i][0] + out[i][1])));	//push output to outV - datavector
		}
		outV.push_back(outVCol);
		outVCol.clear();
	}   
	
	fftFile.close();		//close outputfile
	fftw_destroy_plan(p);	//destruct fft_plan
    fftw_free(in); 			//free allocated memory
	fftw_free(out);			//

	//Energy bands
	cout << "SIZE OF MATRIX: " << outV.size() << endl;
	unsigned int nEnerBands = 32;
	vector<vector<double>> enerOut = {};
	vector<double> enerFrame = {};
	int nfft = N/2 + 1;			//513
	int nframes = outV.size(); 	//216
	int bandsPerEner = floor(nfft/nEnerBands);
	unsigned int count = 0;

	for(int i = nfft-1; i > 0; i--){
		if(count % bandsPerEner == 0){
			int mean = 0;
			for(int j = 0; j < nframes; j++){
				int sum = 0;
				for (int k = 0; k < bandsPerEner; k++){
					if(i-k >= 0) sum += outV[j][i-k];
				} 
				sum = 10*log10(pow(abs(sum),2));
				mean += sum;										
				enerFrame.push_back(sum);													
			}	
			mean /= enerFrame.size();
			for(unsigned int k = 0; k < enerFrame.size(); k++){			
				enerFrame[k] -= mean;
			}	
			enerOut.push_back(enerFrame);
			enerFrame.clear();	
		}	
		count++;	
	}
	//Transfer energyData to textfile
	QFile oEnerFile(dataDirectory + "energy.dat");
	if(!oEnerFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open oEnerFile";
	}
	QTextStream oEner(&oEnerFile);
	for(int i = 0; i < nframes; i++){
		for(int j = 0; j < nEnerBands; j++){
		oEner << enerOut[j][i] << "\n";	
		}
	}
	oEnerFile.close();

	//AVGENERTIME
	vector<vector<double>> avgEner = {};
	vector<double> avgEnerFrame = {};
	if(avg){
		//avgLen = ceil(enerOut[0].size()/5);
		for(unsigned int i = 0; i < enerOut[0].size(); i += 5){
			unsigned int idxEnd;
			if(i+5 < enerOut[0].size()) idxEnd = i+5;
			else idxEnd = enerOut[0].size();	
			for(unsigned int j = 0; j < enerOut.size(); j++){		
				int mean = 0;
				int div = 0;
				for(unsigned int k = i; k < idxEnd; k++){			
					mean += enerOut[j][k];
					div++;
				}
				if(div != 0) mean /= div;
				avgEnerFrame.push_back(mean); 
			}
			avgEner.push_back(avgEnerFrame);
			avgEnerFrame.clear();
		}
	}

	//Decorrelation
	QFile iTMatFile(dataDirectory + "T_mat.txt");
	QFile iCtxOptFile(dataDirectory + "ctx_opt.txt");
	QFile iCtxShapeFile(dataDirectory + "ctx_shape.txt");
	
	if(!iTMatFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << "Cannot open iTMatFile";
	}
	if(!iCtxOptFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << "Cannot open iCtxOptFile";
	}
	if(!iCtxShapeFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << "Cannot open iCtxShapeFile";
	}

	QTextStream iTMat(&iTMatFile);	
	QTextStream iCtxOpt(&iCtxOptFile);
	QTextStream iCtxShape(&iCtxShapeFile);

	vector<double> ctxOptData, ctxShapeData = {};
	vector<Eigen::MatrixXd> tRealMatData;
	double tMatData[9][9][30] = {};
	QString a, b ,c;
	double d, e, f;

	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 30; j++){
			for(int k = 0; k < 9; k++){
				if(!iTMat.atEnd()){
					iTMat >> a;
					d = a.toDouble();
					tMatData[k][i][j] = d;
				}	
			}
		}
	}
	Eigen::MatrixXd m(9,9);
	for(int i = 0; i < 30; i++){
		for(int j = 0; j < 9; j++){
			for(int k = 0; k < 9; k++){
				m(j, k)= tMatData[k][j][i];
			}
		}
	tRealMatData.push_back(m.transpose()); 				//tRealMatData  = T_mat = transformation matrix in correct order
		//cout << tRealMatData[i] << "\n";
	}
	while(!iCtxOpt.atEnd()){
		iCtxOpt >> b;
		e = b.toDouble();
		ctxOptData.push_back(e);
		//cout << e << "\n";
	}
	while(!iCtxShape.atEnd()){
		iCtxShape >> c;
		f = c.toDouble();
		ctxShapeData.push_back(f);
		//cout << f << "\n";
	}
	iTMatFile.close();
	iCtxOptFile.close();
	iCtxShapeFile.close();

	//limits, first and last bands and first and last frames ignored
	int yMin = ctxShapeData[0];
	int yMax = ctxShapeData[0];
	int xMin = ctxShapeData[1];
	int xMax = ctxShapeData[1];
	for(unsigned int i = 0; i < ctxShapeData.size(); i += 2){
		if(i % 2 == 0){
			if(ctxShapeData[i] < yMin) yMin = ctxShapeData[i];
			else if(ctxShapeData[i] > yMax) yMax = ctxShapeData[i];
		}		
		else {
			if(ctxShapeData[i] < xMin) xMin = ctxShapeData[i];
			else if(ctxShapeData[i] > xMax) xMax = ctxShapeData[i];
		}	
	}
	yMin = -yMin;
	xMin = -xMin;

	int nEnerBandsUsed = nEnerBands - yMin - yMax;
	cout << nEnerBandsUsed << "\n";
	int nFramesUsed = avgEner.size()/*enerOut[0].size()*/ - xMin - xMax;
	double ctxBands[nFramesUsed][ctxShapeData.size()/2][nEnerBandsUsed] = {};
	double multAux [nFramesUsed][ctxShapeData.size()/2][nEnerBandsUsed] = {};
	double decorMat[nFramesUsed][nEnerBandsUsed] = {};
	Eigen::VectorXd v(9); 
	Eigen::MatrixXd x;


	for(int i = 0; i < nEnerBandsUsed; i++){								//30
		for(int j = 0; j < nFramesUsed-2; j++){ 							//25
			int mean = 0;	
			for(unsigned int k = 0; k < ctxShapeData.size()/2; k++){		//9
				int ctxIdx = ctxOptData[k*nEnerBandsUsed + j] - 1;
				int idx = j + xMin + ctxShapeData[ctxIdx*2 + 1];
				int idy = i + yMin + ctxShapeData[ctxIdx*2];
				ctxBands[j][k][i] = avgEner[idx][idy];						//enerOut[idy][idx];
				v(k) = ctxBands[j][k][i];
				mean += v(k);
			}
			mean /= 9;
			for(int k = 0; k < 9; k++){
				v(k) -= mean;
			}
			v.transpose();
			x = tRealMatData[i] * v;
			x.transposeInPlace();
			for(unsigned int k = 0; k < ctxShapeData.size()/2; k++){
				multAux[j][k][i] = x(k);
			}
			decorMat[j][i] = multAux[j][0][i];
		}		
	}
	QFile outputDecorFile(dataDirectory + "fingerprint.txt");
	if(!outputDecorFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open outputDecorFile.";
	}
	QTextStream outputDecor(&outputDecorFile);	

	cout << "Size of x: " << x.rows() << "x" << x.cols() << endl;
	for(int i = 0; i < nEnerBandsUsed; i++){
		for(int j = 0; j < nFramesUsed; j++){
			if(decorMat[j][i] > 0) outputDecor << 1 << "\n";
			else outputDecor << 0 << "\n";
		}
	}
	outputDecorFile.close();

	return 0;
}
