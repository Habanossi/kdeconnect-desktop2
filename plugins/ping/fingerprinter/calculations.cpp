#include <iostream>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <vector>
#include <QString>
#include "string.h"
#include <fftw3.h>
#include <Eigen/Core>

using namespace std;

void calculations(Eigen::MatrixXd frameMatrix, QString outputfile, QString energyfile, int frameStart){

	QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
	bool avg = 1;
	int sampleRate = 16000;
	int frameLength = sampleRate * 0.03; 	//flattop: 0.03
	int hopSize 	= sampleRate * 0.02; 	//flattop: 0.02 (2/3 overlap)	



	double matrixSize = frameMatrix.rows() * frameMatrix.cols();					//total size of frameMatrix (n x m)
	//FFT
	matrixSize = 99*480;

	cout << "matrixSize: " << frameMatrix.rows() << " x " << frameMatrix.cols() << endl;
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
    p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_MEASURE);					//init fft-plan
	for(int i = frameStart; i < 99 + frameStart; i++){					//for each column in frameMatrix, set it as fft-input
		for(int j = 0; j < frameLength; j++){							//
			in[j] = frameMatrix(i,j);									//
		}																//
		fftw_execute(p);												//execute fft for each column
		for(int j = N/2+1; j > 0; j--){									//
			fft << 20*log10(sqrt(out[j][0]*out[j][0] + out[j][1]*out[j][1])) << endl;	//send output to textfile
			outVCol.push_back(20*log10(sqrt(out[j][0]*out[j][0] + out[j][1]*out[j][1])));	//abs(out[j][0] + out[j][1]))) if want errored
		}
		outV.push_back(outVCol);
		outVCol.clear();
	}   
	
	fftFile.close();		//close outputfile
	fftw_destroy_plan(p);	//destruct fft_plan
    fftw_free(in); 			//free allocated memory
	fftw_free(out);			//




	//Energy bands



	cout << "SIZE OF MATRIX: " << outV.size() << " x " << outV[0].size() << endl;
	int nEnerBands = 32;
	vector<vector<double>> enerOut = {};
	vector<double> enerFrame = {};
	int nfft = N/2 + 1;			//513
	int nframes = outV.size(); 	//99
	int bandsPerEner = floor(nfft/nEnerBands); // 16
	unsigned int count = 0;

	for(int i = nfft-1; i > 0; i--){
		if(count % bandsPerEner == 0){
			double mean = 0;
			for(int j = 0; j < nframes; j++){
				double sum = 0;
				for (int k = 0; k < bandsPerEner; k++){
					if(i-k >= 0) sum += outV[j][i-k];
				} 
				sum = log10(abs(sum));
				mean += sum;										
				enerFrame.push_back(sum);	
				//cout << sum << endl;												
			}	
			
			mean /= enerFrame.size();
			//cout << "mean: " << mean << " enerframesize: " << enerFrame.size() <<  endl;
			for(unsigned int k = 0; k < enerFrame.size(); k++){			
				enerFrame[k] -= mean;
				//enerFrame[k] /= 2;
			}	
			enerOut.push_back(enerFrame);
			enerFrame.clear();	
		}	
		count++;	
	}
	cout << "starting to average time intervals.\n";
	//AVGENERTIME, enerout[32][99]
	//double avgEner[32][20] = {};
	vector<vector<double>> avgEner = {};
	vector<double> avgEnerFrame = {};
	if(avg){
		int avgLen = 5;
		for(int i = 0; i < 32; i++){
			count = 0;
			for(int j = 98; j > 0; j--){
				if(count % avgLen == 0){// && count != 0){
					double sum = 0;	
					for(int k = 0; k < avgLen; k++){
						//cout << k << endl;
						if(j-k >= 0) sum += enerOut[i][j-k];
					}
					sum /= avgLen;
					avgEnerFrame.push_back(sum);
					//avgEner[j][floor(count/avgLen)] = sum;
				}
				count++;
			}
			avgEner.push_back(avgEnerFrame);
			avgEnerFrame.clear();
			/*unsigned int idxEnd;
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
			avgEnerFrame.clear();*/
		}
	}

	//Transfer energyData/avgEnerData to textfile
	QFile oEnerFile(dataDirectory + energyfile);
	if(!oEnerFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open oEnerFile";
	}
	nframes = 20; //avgener
	QTextStream oEner(&oEnerFile);
	for(int i = 0; i < nframes; i++){ //99
		for(int j = 0; j < nEnerBands; j++){ //32
		//cout << avgEner[j][i] << endl;
		oEner << avgEner[j][i] << "\n";	//avgEner
		}
	}
	oEnerFile.close();

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
	double tMatData[30][9][9] = {};
	QString a, b ,c;
	double d, e, f;

	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 30; j++){
			for(int k = 0; k < 9; k++){
				if(!iTMat.atEnd()){
					iTMat >> a;
					d = a.toDouble();
					tMatData[j][i][k] = d;
				}	
			}
		}
	}
	Eigen::MatrixXd m(9,9);
	for(int i = 0; i < 30; i++){
		for(int j = 0; j < 9; j++){
			for(int k = 0; k < 9; k++){
				m(j, k)= tMatData[i][j][k];
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
	for(unsigned int i = 0; i < ctxShapeData.size(); i++){
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

	cout << "yMin = " << yMin << "\n";
	cout << "yMax = " << yMax << "\n";
	cout << "xMin = " << xMin << "\n";
	cout << "xMax = " << xMax << "\n";

	int nEnerBandsUsed = nEnerBands - yMin - yMax;
	cout << nEnerBandsUsed << " is the size of nEnerBandsUsed\n";
	int nFramesUsed = avgEner[0].size()/*enerOut[0].size()*/ - xMin - xMax;
	cout << nFramesUsed << " is the size of nFramesUsed\n";
	double ctxBands[nFramesUsed][ctxShapeData.size()/2][nEnerBandsUsed] = {};
	double multAux [nFramesUsed][ctxShapeData.size()/2][nEnerBandsUsed] = {};
	double decorMat[nFramesUsed][nEnerBandsUsed] = {};
	Eigen::VectorXd v(9); 
	Eigen::MatrixXd x;
	cout << "enerOut.size: " << enerOut.size() << endl;
	cout << "enerOut[0].size: " << enerOut[0].size() << endl;

	for(int i = 0; i < nEnerBandsUsed; i++){								//30
		for(int j = 0; j < nFramesUsed; j++){ 								//97
			int mean = 0;	
			for(unsigned int k = 0; k < ctxShapeData.size()/2; k++){		//9
				int ctxIdx = ctxOptData[k*nEnerBandsUsed + i] - 1;
				int idy = j + yMin + ctxShapeData[ctxIdx*2 + 1];
				int idx = i + xMin + ctxShapeData[ctxIdx*2];
				ctxBands[j][k][i] = avgEner[idx][idy];//enerOut[idx][idy];
				//cout << "enerout: " << enerOut[idx][idy] << "\n";
				v(k) = ctxBands[j][k][i];
				//cout << v(k) << "\n";
				mean += v(k);
				//cout << "mean: " << mean << endl;
			}
			mean /= 9;
			for(int k = 0; k < 9; k++){
				v(k) -= mean;
			}
			//cout << "round\n";
			//v.transpose();
			//cout << tRealMatData[i] << endl;
			tRealMatData[i].transposeInPlace();
			x = tRealMatData[i] * v;
			//x.transposeInPlace();
			//cout << x << endl;
			for(unsigned int k = 0; k < ctxShapeData.size()/2; k++){
				multAux[j][k][i] = x(k);
			}
			decorMat[j][i] = multAux[j][0][i];
			//cout << "decormat: " << decorMat[j][i] << endl;
		}		
	}
	QFile outputDecorFile(dataDirectory + outputfile);
	if(!outputDecorFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open outputDecorFile.";
	}
	QTextStream outputDecor(&outputDecorFile);	

	cout << "Size of x: " << x.rows() << "x" << x.cols() << endl;
	for(int i = 0; i < nEnerBandsUsed; i++){
		for(int j = 0; j < nFramesUsed; j++){
			//cout << decorMat[j][i] << endl;
			if(decorMat[j][i] > 0){
				outputDecor << 1 << endl;
			//	cout << 1 << endl;			
			}
			else {
				outputDecor << 0 << endl;
			//	cout << 0 << endl;
			}
		}
	}
	outputDecorFile.close();
	cout << "All Done.\n";
	return;
}
