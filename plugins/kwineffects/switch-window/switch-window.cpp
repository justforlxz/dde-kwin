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

#include "switch-window.h"
#include <QMouseEvent>

namespace  SwitchConsts{
    const QEasingCurve TOGGLE_MODE =  QEasingCurve::InOutQuint;// AnimationMode.EASE_OUT_QUINT;
    static const int FADE_DURATION = 500;
    static const int SWITCH_SPACE = 50;
}

SwitchWindowEffect::SwitchWindowEffect(QObject *, const QVariantList &):
    Effect(),
    m_activated(false),
    m_currentEffectWindow(nullptr)
{
    connect(effects, &EffectsHandler::windowAdded, this, &SwitchWindowEffect::onWindowAdded);
    connect(effects, &EffectsHandler::windowDeleted, this, &SwitchWindowEffect::onWindowDeleted);
    // Load all other configuration details
    reconfigure(ReconfigureAll);
}

SwitchWindowEffect::~SwitchWindowEffect()
{
    m_moveingEffectWindows.clear();
}

bool SwitchWindowEffect::supported()
{
    bool supported = effects->isOpenGLCompositing() && effects->animationsSupported();
    return supported;
}

void SwitchWindowEffect::reconfigure(Effect::ReconfigureFlags)
{
    m_toggleTimeline.setDuration(SwitchConsts::FADE_DURATION);
    m_toggleTimeline.setEasingCurve(SwitchConsts::TOGGLE_MODE);
    updateSwitchingWindows();
}

void SwitchWindowEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    if (isActive()){
        if(m_activated && m_moving){
            int new_time = m_toggleTimeline.currentTime() + (m_activated ? time: -time);
            m_toggleTimeline.setCurrentTime(new_time);
            //qDebug()<<__func__<<"m_toggleTimeline:"<<m_toggleTimeline.currentTime();
            data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
            m_windowMotionManager.calculate(time / 2.f); // moving
        }

        if (!m_activated && m_toggleTimeline.currentValue() == 0) {
            cleanup();
        }
    }
    // this makes blurred windows work, should we need it ?
    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant(true));
    }
    effects->prePaintScreen(data, time);
}

void SwitchWindowEffect::paintScreen(int mask, QRegion region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
}

void SwitchWindowEffect::postPaintScreen()
{
    //qDebug()<<__func__<<"m_toggleTimeline:"<<m_toggleTimeline.currentTime()<<":"<<m_toggleTimeline.currentValue();
    if ((m_activated && m_toggleTimeline.currentValue() != 1)
            || (!m_activated && m_toggleTimeline.currentValue() != 0))
        effects->addRepaintFull();

    if(m_activated && m_toggleTimeline.currentValue() == 1) {
        setActive(false);
    }
    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant());
    }
    effects->postPaintScreen();
}

// Window painting
void SwitchWindowEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    //must have ,else paintwindow it not effect
    data.mask |= PAINT_WINDOW_TRANSFORMED;

    if (m_activated) {
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);   // Display always
    }
    w->enablePainting(EffectWindow::PAINT_DISABLED);
    if (!(w->isDock() || w->isDesktop() || isRelevantWithPresentWindows(w))) {
        w->disablePainting(EffectWindow::PAINT_DISABLED);
        w->disablePainting(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
    }

    effects->prePaintWindow(w, data, time);
}

void SwitchWindowEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (!m_activated) {
        effects->paintWindow(w, mask, region, data);
        return;
    }
    if (m_windowMotionManager.isManaging(w) || w->isDesktop()) {
        auto area = effects->clientArea(ScreenArea, 0, 0);
        WindowPaintData d = data;
        if (w->isDesktop()) {
            d.setBrightness(0.4);
            effects->paintWindow(w, mask, area, d);

        }else if (!w->isDesktop()) {
            auto geo = m_windowMotionManager.transformedGeometry(w);
            d += QPoint(qRound(geo.x()), qRound(geo.y()));
            d.setScale(QVector2D((float)geo.width() / w->width(), (float)geo.height() / w->height()));
            effects->paintWindow(w, mask, area, d);
        }

    }else {
        effects->paintWindow(w, mask, region, data);
    }
}

bool SwitchWindowEffect::isActive() const
{
    return (m_activated || m_toggleTimeline.currentValue() != 0) && !effects->isScreenLocked();
}

void SwitchWindowEffect::setActive(bool active)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this)
        return;

    if (m_activated == active)
        return;

    if (!m_activated && m_toggleTimeline.currentValue() != 0) {
        // closing animation is still in effect
        return;
    }

    m_activated = active;
    if(active) {
        effects->setShowingDesktop(false);
        effects->setActiveFullScreenEffect(this);
    }
    else {
       foreach (EffectWindow* w, m_windowMotionManager.managedWindows()) {
          m_windowMotionManager.moveWindow(w, w->geometry());
       }
       cleanup();
    }
}

void SwitchWindowEffect::moveToPreWindow()
{
    if(!isActive()) {
        return;
    }

    if(m_moveingEffectWindows.size() <= 1) {
        m_toggleTimeline.setCurrentTime(1000);
        effects->addRepaintFull();
        return;
    }
    m_moving = true;
    m_windowMotionManager.reset();
    m_windowMotionManager.unmanageAll();
    m_toggleTimeline.setCurrentTime(0);
    m_moveingEffectWindows.move(0, m_moveingEffectWindows.size() - 1);

    for (int i = 0; i<m_moveingEffectWindows.size(); ++i) {
        EffectWindow *w = m_moveingEffectWindows[i];
        m_windowMotionManager.manage(w);
        int transindex = i + 1;
        int topindex = i;
        if(i == m_moveingEffectWindows.size() - 1) {
            transindex = 0;
            topindex = -1;
        }
        int direct = 1;
        if(i % 2 != 0) {
            direct = -1;
        }
        if(topindex == 0) {
            m_currentEffectWindow = w;
        }
        QPoint topleft((w->geometry().width() *topindex), w->y());
        QRect transformedGeom(QPoint((w->geometry().width()) * transindex + SwitchConsts::SWITCH_SPACE * direct, w->y()),
                              QSize(w->geometry().width(), w->geometry().height()));
        m_windowMotionManager.setTransformedGeometry(w, transformedGeom);
        m_windowMotionManager.moveWindow(w, topleft);
    }
    effects->addRepaintFull();
}

void SwitchWindowEffect::moveToNextWindow()
{
    if(!isActive()) {
        return;
    }

    if(m_moveingEffectWindows.size() <= 1) {
        m_toggleTimeline.setCurrentTime(1000);
        effects->addRepaintFull();
        return;
    }

    m_moving = true;
    m_windowMotionManager.reset();
    m_windowMotionManager.unmanageAll();
    m_toggleTimeline.setCurrentTime(0);
    m_moveingEffectWindows.move(m_moveingEffectWindows.size() - 1, 0);

    for (int i = 0; i<m_moveingEffectWindows.size(); ++i) {
        EffectWindow* w = m_moveingEffectWindows[i];
        m_windowMotionManager.manage(w);
        if(i == 0) {
            m_currentEffectWindow = w;
        }
        QPoint topleft((w->geometry().width() * i), w->y());
        int direct = 1;
        if(i % 2 != 0) {
            direct = -1;
        }
        QRect transformedGeom(QPoint((w->geometry().width()) *(i - 1) - SwitchConsts::SWITCH_SPACE * direct,
                                     w->y()), QSize(w->geometry().width() ,w->geometry().height()));
        m_windowMotionManager.setTransformedGeometry(w ,transformedGeom);
        m_windowMotionManager.moveWindow(w ,topleft);
    }
    effects->addRepaintFull();
}

void SwitchWindowEffect::onWindowAdded(EffectWindow *w)
{
    if (!isRelevantWithPresentWindows(w))
        return; // don't add
    m_moveingEffectWindows.insert(0,w);
}

void SwitchWindowEffect::onWindowDeleted(EffectWindow *w)
{
    if (m_moveingEffectWindows.contains(w)) {
        m_moveingEffectWindows.removeOne(w);
        if (m_moveingEffectWindows.size() > 0) {
            EffectWindow * w = m_moveingEffectWindows.first();
            if (!w->isMinimized()) {
                effects->activateWindow(w);
                effects->addRepaintFull();
            }
        }
    }
}

bool SwitchWindowEffect::isRelevantWithPresentWindows(EffectWindow *w) const
{
    if (w->isSpecialWindow() || w->isUtility()) {
        return false;
    }

    if (w->isDock()) {
        return false;
    }

    if (w->isSkipSwitcher()) {
        return false;
    }

    if (w->isDeleted()) {
        return false;
    }

    if (!w->acceptsFocus()) {
        return false;
    }

    if (!w->isCurrentTab()) {
        return false;
    }

    if (!w->isOnCurrentActivity()) {
        return false;
    }

    return true;
}

void SwitchWindowEffect::updateSwitchingWindows()
{
    m_moveingEffectWindows.clear();
    EffectWindowList windows = effects->stackingOrder();
    for (int i = windows.size() - 1; i >= 0 ; --i) {
        EffectWindow *w = windows[i];
        if (isRelevantWithPresentWindows(w)) {
            m_moveingEffectWindows.append(w);
        }
    }
}

void SwitchWindowEffect::cleanup()
{
    if (m_activated) {
        return;
    }
    effects->setActiveFullScreenEffect(0);
    m_windowMotionManager.unmanageAll();
    m_toggleTimeline.setCurrentTime(0); // end moving
    if(m_currentEffectWindow) {
        effects->activateWindow(m_currentEffectWindow);
    }
    m_currentEffectWindow = nullptr;
    m_moving = false;
    effects->addRepaintFull();
}

