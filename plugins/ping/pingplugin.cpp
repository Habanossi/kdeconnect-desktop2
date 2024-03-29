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
QString message;
long timeAtStart = 0;
std::string recordStartingTime = "";
QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
QString isC = "N00%";
int matchcount = 0;
int tick = 1;
bool calc = false;
bool first = false;

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
    Daemon::instance()->sendSimpleNotification(QStringLiteral("pingReceived"), device()->name(), np.get<QString>(QStringLiteral("message"),i18n("Pingman!")), QStringLiteral("dialog-ok"));
	message = np.get<QString>(QStringLiteral("message"));
	phonedata = message;
	cout << "message received.\n";
	qDebug() << message << "\n";

	if(message == "first"){
		sendFunction();
		first = true;
	}
	
	else if(tick == 1){
		tick = 0;
		QString startTime = message.right(13);
		qDebug() << "start at: " << startTime << "\n";
		timeval tv;
		gettimeofday(&tv, 0);
		std::string s = to_string(tv.tv_usec);
		cout << s << endl;
		std::string time = to_string(tv.tv_sec) + s[0] + s[1];
		while (time.size() < 13){
			time += "0";
		}
		long l = stol(time);
		long t = startTime.toLong();
		std::this_thread::sleep_for(std::chrono::milliseconds(t-l + 480));
		fingerPrint();
		QFile fpFile(dataDirectory + "fingerprintdata/fingerprint0.txt");
		if(!fpFile.exists()){ qDebug() << "FILE DOES NOT EXIST"; exit(1); }
		if(!fpFile.open(QIODevice::ReadOnly)){
			qDebug() << "something wrong with fpFile";
		}
		QTextStream in(&fpFile);
		QString fingerPrint;
		while(!in.atEnd()){
			fingerPrint += in.readLine();
		}
		fpFile.close();
		laptopdata = fingerPrint;
		sendFunction();
	}else if(tick == 0){
		while(calc == 1){
			cout << "waiting for own calculations before receiving fingerprint\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		qDebug() << "This fingerprint: " <<laptopdata << "\n";
		qDebug() << "Other fingerprint: " << phonedata << "\n";
		for(int j = 0; j < laptopdata.length(); j++){
			if(phonedata[j] == laptopdata[j]){	
				matchcount++;
			}
		}		
		cout << "matchcount: " << matchcount << endl;	
		cout << to_string(matchcount/540.0 *100) << " percent match." << endl;
		if(matchcount > 330){
			cout << "pairing is secure and private\n";
			isC = "Y" + QString::number(matchcount);
		}else{
			cout << "can't establish a private connection\n";
			isC = "N" + QString::number(matchcount);
		}
		matchcount = 0;
		tick = 1;
		sendFunction();
	}
    return true;
}

void PingPlugin::sendFunction(){
	if(tick == 1){
		tick = 0;
		calc = true;
		timeval tv;
		gettimeofday(&tv, 0);
		std::string s = to_string(tv.tv_usec);
		cout << s << endl;
		std::string time = to_string(tv.tv_sec) + s[0] + s[1];
		while (time.size() < 13){
			time += "0";
		}
		const QString& tTime = QString::fromStdString(time);
		cout << "Current time, sendFunction, tick 1: " << time << endl;
		NetworkPacket np(PACKET_TYPE_PING);
		np.set(QStringLiteral("message"), isC + tTime);	
		cout << "sending with with tick = 1" << endl;	
		sendPacket(np);
		std::this_thread::sleep_for(std::chrono::milliseconds(3480));
		fingerPrint(); // Record audio, calculate fingerprint
		QFile fpFile(dataDirectory + "fingerprintdata/fingerprint0.txt");
		if(!fpFile.exists()){ qDebug() << "FILE DOES NOT EXIST"; exit(1); }
		if(!fpFile.open(QIODevice::ReadOnly)){
			qDebug() << "something wrong with fpFile";
		}
		QTextStream in(&fpFile);
		QString fingerPrint;
		while(!in.atEnd()){
			fingerPrint += in.readLine();
		}
		fpFile.close();
		laptopdata = fingerPrint;
		calc = false;
	}else if(tick == 0){
		NetworkPacket np(PACKET_TYPE_PING);
		np.set(QStringLiteral("message"), laptopdata);	
		cout << "sending with with tick = 0" << endl;	
		sendPacket(np);
		tick = 1;
	}
}

void PingPlugin::sendPing(){
	if(tick != 1 || first){
		tick = 1;
		first = false;
		cout << "Fixing variables" << endl;
	}else{
		NetworkPacket np(PACKET_TYPE_PING);
		np.set(QStringLiteral("message"), "first");
		sendPacket(np);
		first = true;
	}
}

void PingPlugin::sendPing(const QString& customMessage){
    NetworkPacket np(PACKET_TYPE_PING);
    if (!customMessage.isEmpty()) {
        np.set(QStringLiteral("message"), customMessage);
    }
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
}

QString PingPlugin::dbusPath() const{
    return "/modules/kdeconnect/devices/" + device()->id() + "/ping";
}

#include "pingplugin.moc"

