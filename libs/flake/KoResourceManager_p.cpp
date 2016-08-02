/*
   Copyright (c) 2006, 2011 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>
   Copyright (c) 2011 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#include "KoResourceManager_p.h"

#include <QVariant>
#include <FlakeDebug.h>

#include "KoShape.h"

void KoResourceManager::setResource(int key, const QVariant &value)
{
    QVariant realValue = value;
    int realKey = key;
    QVariant currentValue = m_resources.value(key, QVariant());

    KoDerivedResourceConverterSP converter =
        m_derivedResources.value(key, KoDerivedResourceConverterSP());

    if (converter) {
        realKey = converter->sourceKey();
        currentValue = m_resources.value(realKey, QVariant());
        realValue = converter->toSource(value, currentValue);
    }

    if (m_resources.contains(realKey)) {
        if (!converter && currentValue == realValue)
            return;
        m_resources[realKey] = realValue;
    } else {
        m_resources.insert(realKey, realValue);
    }

    notifyResourceChanged(key, value);
}

void KoResourceManager::notifyResourceChanged(int key, const QVariant &value)
{
    emit resourceChanged(key, value);

    QMultiHash<int, KoDerivedResourceConverterSP>::const_iterator it = m_derivedFromSource.constFind(key);
    QMultiHash<int, KoDerivedResourceConverterSP>::const_iterator end = m_derivedFromSource.constEnd();

    while (it != end && it.key() == key) {
        KoDerivedResourceConverterSP converter = it.value();
        notifyResourceChanged(converter->key(), converter->fromSource(value));
        it++;
    }
}

QVariant KoResourceManager::resource(int key) const
{
    KoDerivedResourceConverterSP converter =
        m_derivedResources.value(key, KoDerivedResourceConverterSP());

    const int realKey = converter ? converter->sourceKey() : key;
    QVariant value = m_resources.value(realKey, QVariant());

    return converter ? converter->fromSource(value) : value;
}

void KoResourceManager::setResource(int key, const KoColor &color)
{
    QVariant v;
    v.setValue(color);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, const KoUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

KoColor KoResourceManager::koColorResource(int key) const
{
    if (! m_resources.contains(key)) {
        KoColor empty;
        return empty;
    }
    return resource(key).value<KoColor>();
}

KoShape *KoResourceManager::koShapeResource(int key) const
{
    if (! m_resources.contains(key))
        return 0;

    return resource(key).value<KoShape *>();
}


KoUnit KoResourceManager::unitResource(int key) const
{
    return resource(key).value<KoUnit>();
}

bool KoResourceManager::boolResource(int key) const
{
    if (! m_resources.contains(key))
        return false;
    return m_resources[key].toBool();
}

int KoResourceManager::intResource(int key) const
{
    if (! m_resources.contains(key))
        return 0;
    return m_resources[key].toInt();
}

QString KoResourceManager::stringResource(int key) const
{
    if (! m_resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

QSizeF KoResourceManager::sizeResource(int key) const
{
    if (! m_resources.contains(key)) {
        QSizeF empty;
        return empty;
    }
    return qvariant_cast<QSizeF>(resource(key));
}

bool KoResourceManager::hasResource(int key) const
{
    KoDerivedResourceConverterSP converter =
        m_derivedResources.value(key, KoDerivedResourceConverterSP());

    const int realKey = converter ? converter->sourceKey() : key;
    return m_resources.contains(realKey);
}

void KoResourceManager::clearResource(int key)
{
    // we cannot remove a derived resource
    if (m_derivedResources.contains(key)) return;

    if (m_resources.contains(key)) {
        m_resources.remove(key);
        notifyResourceChanged(key, QVariant());
    }
}

void KoResourceManager::addDerivedResourceConverter(KoDerivedResourceConverterSP converter)
{
    Q_ASSERT(!m_derivedResources.contains(converter->key()));

    m_derivedResources.insert(converter->key(), converter);
    m_derivedFromSource.insertMulti(converter->sourceKey(), converter);
}

bool KoResourceManager::hasDerivedResourceConverter(int key)
{
    return m_derivedResources.contains(key);
}

void KoResourceManager::removeDerivedResourceConverter(int key)
{
    Q_ASSERT(m_derivedResources.contains(key));

    KoDerivedResourceConverterSP converter = m_derivedResources.value(key);
    m_derivedResources.remove(key);
    m_derivedFromSource.remove(converter->sourceKey(), converter);
}
