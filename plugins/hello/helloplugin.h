/**
 * Copyright 2019 Hermanni Viljanen <hermanniviljanen@gmail.com>
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

#ifndef HELLOPLUGIN_H
#define HELLOPLUGIN_H

#include <QObject>

#include <core/kdeconnectplugin.h>
#include "string.h"
#define PACKET_TYPE_HELLO QStringLiteral("kdeconnect.hello")

class Q_DECL_EXPORT HelloPlugin
    : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.hello")

public:
    explicit HelloPlugin(QObject* parent, const QVariantList& args);
    ~HelloPlugin() override;

    Q_SCRIPTABLE void sendHello();
    Q_SCRIPTABLE void sendHello(const QString& customMessage);

    bool receivePacket(const NetworkPacket& np) override;
    void connected() override {}

    QString dbusPath() const override;

private:
	bool compareKeys(std::string key1, std::string key2 );
};

#endif
