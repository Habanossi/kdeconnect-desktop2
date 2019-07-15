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

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_ping.json", registerPlugin< PingPlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PING, "kdeconnect.plugin.ping")

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
	
	qCDebug(KDECONNECT_PLUGIN_PING) << "message: \n" ;
    Daemon::instance()->sendSimpleNotification(QStringLiteral("pingReceived"), device()->name(), np.get<QString>(QStringLiteral("message"),i18n("Pingman!")), QStringLiteral("dialog-ok"));
	QFile dataFile("/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/phonedata.txt");
	if(!dataFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open dataFile";
	}
	QTextStream data(&dataFile);	
	data << np.get<QString>(QStringLiteral("message"));	
	qDebug() << "message: " << np.get<QString>(QStringLiteral("message"));	
	dataFile.close();
    return true;
}

void PingPlugin::sendPing()
{
	QString dataDirectory = "/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/";
	fingerPrint(); // Record audio, calculate fingerprint
	QFile fpFile(dataDirectory + "fingerprint.txt");
	if(!fpFile.exists()){ qDebug() << "FILE DOES NOT EXIST"; exit(1); }

	QString errMsg;
	QFileDevice::FileError err = QFileDevice::NoError;

	if(!fpFile.open(QIODevice::ReadOnly)){
		qDebug() << "something wrong with fpFile";
		errMsg = fpFile.errorString();
		err = fpFile.error();
	}
	QTextStream in(&fpFile);
	QString fingerPrint;
	while(!in.atEnd()){
		fingerPrint += in.readLine();
		fingerPrint += " ";
	}
	const QString& fPrint = fingerPrint;
	qDebug() << fingerPrint << "is the messs";
    NetworkPacket np(PACKET_TYPE_PING);
	np.set(QStringLiteral("message"), fPrint);		
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
	fpFile.close();
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

