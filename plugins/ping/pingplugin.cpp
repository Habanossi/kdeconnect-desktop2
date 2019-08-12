/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "fingerprinter/fingerPrint.cpp"
#include <QFile>
#include "pingplugin.h"
#include <QString>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QDebug>
#include <QDBusConnection>
#include <QLoggingCategory>
#include <iostream>
#include <core/device.h>
#include <core/daemon.h>
#include <sys/time.h>
#include <stdlib.h>
#include <vector>
#include <chrono>
#include <thread>

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_ping.json", registerPlugin< PingPlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PING, "kdeconnect.plugin.ping")
QString phonedata;
QString laptopdata;
QString lf0, lf1, lf2, lf3, lf4, lf5, lf6, lf7, lf8, lf9, lf10, lf11, lf12, lf13, lf14, lf15, lf16, lf17, lf18, lf19;
vector<QString> k = {lf0, lf1, lf2, lf3, lf4, lf5, lf6, lf7, lf8, lf9, lf10, lf11, lf12, lf13, lf14, lf15, lf16, lf17, lf18, lf19};
int test = 1;
long timeAtStart = 0;
int fpAmount = 1;
std::string recordStartingTime = "";
QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";

PingPlugin::PingPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin constructor for device" << device()->name();
}

PingPlugin::~PingPlugin()
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin destructor for device" << device()->name();
}

bool PingPlugin::receivePacket(const NetworkPacket& np)
{
	//int test = 1;
	//QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";

    Daemon::instance()->sendSimpleNotification(QStringLiteral("pingReceived"), device()->name(), np.get<QString>(QStringLiteral("message"),i18n("Pingman!")), QStringLiteral("dialog-ok"));
	QString qs = np.get<QString>(QStringLiteral("message"));
	phonedata = qs;
	std::cout << "message received.\n";
	if(!qs.contains("x")){
	qDebug() << qs << "\n";
		timeval tv;
		gettimeofday(&tv, 0);
		std::string time = to_string(tv.tv_sec) + to_string(tv.tv_usec/1000);
		long l = stol(time);
		std::string tim = std::to_string(l);
		//qDebug() << time << "\n";
		//long l = stol(time,nullptr);
		std::cout << "TimeStamp Laptop when received: " << tim << endl;
		int offset = (l-timeAtStart)/2;
		int waitT = 2000; + offset;
		if(waitT > 20000){
			std::cout << "WaitTime really long, abort\n";
			//exit(1);
		}
		long diffBetweenTimes = l - qs.toLong() + offset;
		std::cout << "wait time: \n" << std::to_string(waitT) << "\n";
		std::cout << "offset: \n" << std::to_string(offset) << "\n";
		std::cout << "compare timestamps: " << std::to_string(diffBetweenTimes) << "\n";
		std::cout << "Waiting...\n\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(waitT));
	std::cout << "Starting to record\n\n";
		gettimeofday(&tv, 0);
		recordStartingTime = to_string(tv.tv_sec) + to_string(tv.tv_usec/1000);
		fingerPrint(); // Record audio, calculate fingerprint
		std::cout << "start time for recording: " << recordStartingTime << " - " << std::to_string(diffBetweenTimes) << "\n";
		for(int i = 0; i < fpAmount; i++){
			cout << "copying fingerprint\n";
			QFile fpFile(dataDirectory + "fingerprintdata/fingerprint" + QString::number(i) + ".txt");
			//QFile fpFile(dataDirectory + "fingerprintdata/fingerprint.txt");
			if(!fpFile.exists()){ qDebug() << "FILE DOES NOT EXIST"; exit(1); }
			if(!fpFile.open(QIODevice::ReadOnly)){
				qDebug() << "something wrong with fpFile";
			}
			QTextStream in(&fpFile);
			QString fingerPrint;
			while(!in.atEnd()){
				fingerPrint += in.readLine();
				//fingerPrint += " ";
			}
			//const QString& fPrint = fingerPrint;
			k[i] = fingerPrint;
			laptopdata = fingerPrint;
			qDebug() << laptopdata << "\n";
		}
		//qDebug() << fingerPrint << "is the messs";
/*
   		NetworkPacket np(PACKET_TYPE_PING);
		np.set(QStringLiteral("message"), fPrint);		
    	bool success = sendPacket(np);
    	qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;

		//const QString& fPrint = fingerPrint;
		//qDebug() << fingerPrint << "is the messs";
		fpFile.close();	*/
	}
	else{
		QFile dataFile("/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/phonedata.txt");
		if(!dataFile.open(QIODevice::WriteOnly | QFile::Truncate)){
			qDebug() << "Cannot open dataFile";
		}
		QTextStream data(&dataFile);
		QString& q = qs.remove("x");
		data << q;
		phonedata = q;
		dataFile.close();
	}
	test = 2;
    return true;
}

void PingPlugin::sendPing()
{

	//QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
	if(test == 1){
		timeval tv;
		gettimeofday(&tv, 0);
		std::string time = to_string(tv.tv_sec) + to_string(tv.tv_usec/1000);
		long l = stol(time);
		timeAtStart = l;
		std::string tim = std::to_string(l);
		std::cout << "TimeStamp Laptop when sent: " << time << endl;
		NetworkPacket np(PACKET_TYPE_PING);
    	/*bool success = */sendPacket(np);	
	}
	else if (test == 2){
		for(int i = 0; i < fpAmount; i++){
			int matchcount = 0;		
			//k[i] = k[i].trimmed();
			for(int j = 0; j < k[i].length(); j++){
				//qDebug() << phonedata[j] << " vs " << k[i][j] << "\n";
				if(phonedata[j] == k[i][j]){	
					matchcount++;
				}
			}		
			cout << "matchcount: " << matchcount << "\n";	
		}
		test = 1;	
	}
	else {
		fingerPrint(); // Record audio, calculate fingerprint
		QFile fpFile(dataDirectory + "fingerprint.txt");
		if(!fpFile.exists()){ qDebug() << "FILE DOES NOT EXIST"; exit(1); }
		if(!fpFile.open(QIODevice::ReadOnly)){
			qDebug() << "something wrong with fpFile";
		}
		QTextStream in(&fpFile);
		QString fingerPrint;
		while(!in.atEnd()){
			fingerPrint += in.readLine();
			//fingerPrint += " ";
		}
		const QString& fPrint = fingerPrint;
		qDebug() << fingerPrint << "is the messs";
   		 NetworkPacket np(PACKET_TYPE_PING);
		np.set(QStringLiteral("message"), fPrint);		
    	bool success = sendPacket(np);
    	qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
		fpFile.close();
	}
}

void PingPlugin::sendPing(const QString& customMessage)
{
    NetworkPacket np(PACKET_TYPE_PING);
    if (!customMessage.isEmpty()) {
        np.set(QStringLiteral("message"), customMessage);
    }
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
}

QString PingPlugin::dbusPath() const
{
    return "/modules/kdeconnect/devices/" + device()->id() + "/ping";
}

#include "pingplugin.moc"

