#define _USE_MATH_DEFINES

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <Eigen/Core>

using namespace std;


//vector<vector<double>> window(QString directory, QString file, int frameLength, int hopSize, string windowType){
Eigen::MatrixXd window(QString directory, QString file, int frameLength, int hopSize, string windowType, vector<double> data){
	//vector<vector<double>>	frameMatrix = {};
	//vector<double> 			frame = {};
	vector<double> 			windowFunction = {};	
	vector<double> 			dataV = data;

	//QFile inFile(directory + file);
	//if(!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
	//	qDebug() << "something wrong with inFile";
	//}
	//QTextStream in(&inFile);
	//double value;
	//do{
	//	value = in.readLine().toDouble();
	//	dataV.push_back(value);							//All data moved from list.dat to dataV, kinda unnecessary to put it in textfile, could fix
	//}while(!in.atEnd());								
	//inFile.close();
	cout << "works" << hopSize <<"\n";
	int numberOfFrames = 1 + floor((dataV.size() - frameLength) / hopSize);
		cout << "numberofframes: " << numberOfFrames << endl;
	if(windowType == "hamming"){						//Implement Hamming window  w(n)=0.54−0.46cos(2πn/M−1),   0≤n≤M−1
		for(int i = 0; i < frameLength - 1; i++){ 
			double point = 0.54 - 0.46*cos((2*M_PI*i)/(frameLength-1));
			windowFunction.push_back(point);
		}						
	}
	else if(windowType == "flattop"){					//Flattop window is what we use
		double point;
		for(double i = 0.5; i < frameLength; i+=3){
			point = sin(M_PI*i / frameLength);
			windowFunction.push_back(point);
		}
		for(int i = 0; i < frameLength - hopSize; i++){
			windowFunction.push_back(1);
		}
		for(double i = frameLength; i > 0.5; i-=3){
			point =	sin(M_PI*i / frameLength);		
			windowFunction.push_back(point);
		}
	}
	else cout << "Windowing function not supported" << endl;
	std::cout << "length of windowFunction: " << windowFunction.size() << "\n";
	//Fill matrix and file with data with windowing function applied
	QFile outFile("/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/window.dat");
	if(!outFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "something wrong with outFile";
	}
	QTextStream out(&outFile);	
	//int c, e = 0;
	Eigen::MatrixXd x(numberOfFrames,frameLength);
	for(int i = 0; i < numberOfFrames; i++){ 						//for each row
		for(int j = 0; j < frameLength; j++){						//for each column
			double dataPoint = dataV[i*hopSize + j];
			//cout << dataPoint << endl;
			if(dataPoint){
				dataPoint *= windowFunction[j];						//assign correct data point * windowing function point
				x(i,j) = dataPoint;
				/*int d = c % 3;
				int arrayJStart = j + (d/3)*frameLength;
				if(arrayJStart >= frameLength){
					x(i-(e-1),arrayJStart - frameLength) = dataPoint;
				}
				else{
					x(i - e, arrayJStart) = dataPoint;
				}*/
				//frame.push_back(dataPoint); 						
				out << dataPoint << "\n";
			} else out << 0 << "\n";
		}
		//frameMatrix.push_back(frame);
		//frame.clear();
		//c += 2;
		//if(c % 3 == 2) e++;
	}
	outFile.close();

	return x;
	//return frameMatrix;
}



