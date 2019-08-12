/**
 * Copyright 2013 Hermanni Viljanen <hermanni.viljanen@gmail.com>
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
#include <iostream>
#include "helloplugin.h"
#include <QFile>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDebug>
#include <QDBusConnection>
#include <QLoggingCategory>

#include <core/device.h>
#include <core/daemon.h>

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_hello.json", registerPlugin< HelloPlugin >(); )

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_HELLO, "kdeconnect.plugin.hello")

HelloPlugin::HelloPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin constructor for device" << device()->name();
}

HelloPlugin::~HelloPlugin()
{
//     qCDebug(KDECONNECT_PLUGIN_PING) << "Ping plugin destructor for device" << device()->name();
}

bool HelloPlugin::receivePacket(const NetworkPacket& np)
{
    Daemon::instance()->sendSimpleNotification(QStringLiteral("pingReceived"), device()->name(), np.get<QString>(QStringLiteral("message"),i18n("Hello!")), QStringLiteral("dialog-ok"));
	QString qs = np.get<QString>(QStringLiteral("message"));
	std::cout << "message received\n";
	QFile dataFile("/home/hermanni/kdeconnect-kde-1.3.4/plugins/ping/fingerprinter/data/phonedata.txt");
	if(!dataFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "Cannot open dataFile";
	}
	QTextStream data(&dataFile);	
	data << qs;	
	dataFile.close();
	//np.set(QStringLiteral("message"), "");
    return true;
}

void HelloPlugin::sendHello()
{
	QFile fpFile("fingerprint.txt");
	fpFile.open(QIODevice::ReadOnly);
	QString fingerPrint = "";
	while(!fpFile.atEnd()){
		fingerPrint = fpFile.readLine();
	}
    NetworkPacket np(PACKET_TYPE_HELLO);
	np.set(QStringLiteral("message"), fingerPrint);	
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_HELLO) << "sendHello:" << success;
	fpFile.close();
}

void HelloPlugin::sendHello(const QString& customMessage)
{
    NetworkPacket np(PACKET_TYPE_HELLO);
    if (!customMessage.isEmpty()) {
        np.set(QStringLiteral("message"), customMessage);
    }
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_HELLO) << "sendHello:" << success;
}

QString HelloPlugin::dbusPath() const
{
    return "/modules/kdeconnect/devices/" + device()->id() + "/hello";
}

#include "helloplugin.moc"

