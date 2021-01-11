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


#ifndef _DEEPIN_SWITCH_CLIENT_H
#define _DEEPIN_SWITCH_CLIENT_H

#include <kwineffects.h>
#include <kwinglplatform.h>

using namespace KWin;

/**
 *  Deepin SwitchWindow View Effect
 **/
class Q_DECL_HIDDEN SwitchWindowEffect : public Effect
{
    Q_OBJECT
public:
    SwitchWindowEffect(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    virtual ~SwitchWindowEffect();

    static bool supported();
    virtual void reconfigure(ReconfigureFlags) override;

    // Screen painting
    virtual void prePaintScreen(ScreenPrePaintData &data, int time) override;
    virtual void postPaintScreen() override;

    // Window painting
    virtual void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time) override;
    virtual void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;

    virtual bool isActive() const override;

public Q_SLOTS:
    void setActive(bool active);
    void toggleActive()  {
        setActive(!m_activated);
    }
    void moveToPreWindow();
    void moveToNextWindow();
private slots:
    void onWindowAdded(KWin::EffectWindow *);
    void onWindowDeleted(KWin::EffectWindow *);
    void onWindowUnminimized(KWin::EffectWindow *);
    void onWindowMminimized(KWin::EffectWindow *);
private:
    bool isRelevantWithPresentWindows(EffectWindow *w) const;
    void updateSwitchingWindows();
    EffectWindow *getNextWindow() const;
    EffectWindow *getPreWindow() const;
    // close animation finished
    void cleanup();
private:

     bool m_activated {false};
     bool m_moving {false};
     QList<EffectWindow *> m_moveingEffectWindows;
     EffectWindow *m_currentEffectWindow {nullptr};
     EffectWindow *m_preEffectWindow {nullptr};
     EffectWindow *m_nextEffectWindow {nullptr};
     //animation time duration;
     std::chrono::milliseconds m_duration;
     //animation line
     TimeLine m_animationTimeline;
};




#endif /* ifndef _DEEPIN_SWITCH_CLIENT_H */

