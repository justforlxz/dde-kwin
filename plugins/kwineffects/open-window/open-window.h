/*
 * Copyright (C) 2020 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     tanfang <tanfang@uniontech.com>
 *
 * Maintainer: tanfang <tanfang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _DEEPIN_OPEN_WINDOW_H
#define _DEEPIN_OPEN_WINDOW_H

#include <kwineffects.h>

using namespace KWin;

/**
 *  Deepin OpenWindow View Effect
 **/
class Q_DECL_HIDDEN OpenWindowEffect : public Effect
{
    Q_OBJECT
public:
    OpenWindowEffect(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    virtual ~OpenWindowEffect();

    static bool supported();
    virtual void reconfigure(ReconfigureFlags) override;

    // Screen painting
    virtual void prePaintScreen(ScreenPrePaintData &data, int time) override;
    virtual void postPaintScreen() override;

    // Window painting
    virtual void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time) override;
    virtual void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;

    virtual bool isActive() const override;
    int requestedEffectChainPosition() const override {
        return 52;
    }

private slots:
    void slotWindowDeleted(KWin::EffectWindow *w);
    void slotWindowAdded(KWin::EffectWindow *w);
private:
     std::chrono::milliseconds m_duration;
     //support to opening multiple windows at the same time
     QHash<const EffectWindow*, TimeLine> m_animations;
};

#endif /* ifndef _DEEPIN_OPEN_WINDOW_H */

