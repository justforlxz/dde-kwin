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



SwitchWindowEffect::SwitchWindowEffect(QObject *, const QVariantList &):
    Effect(),
    m_activated(false)
{

}

SwitchWindowEffect::~SwitchWindowEffect()
{

}

bool SwitchWindowEffect::supported()
{
    bool supported = effects->isOpenGLCompositing() && GLRenderTarget::supported() && GLRenderTarget::blitSupported();
    return supported;
}

void SwitchWindowEffect::reconfigure(Effect::ReconfigureFlags)
{
    m_toggleTimeline.setDuration(500);
    m_toggleTimeline.setEasingCurve(QEasingCurve::InOutElastic);
    m_toggleTimeline.setLoopCount(INT_MAX);
}

void SwitchWindowEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    if (isActive()){
        qDebug()<<__func__<<__LINE__;
        int new_time = m_toggleTimeline.currentTime() + (m_activated ? time: -time);
        m_toggleTimeline.setCurrentTime(new_time);
        qDebug()<<__func__<<"m_toggleTimeline:"<<m_toggleTimeline.currentTime();
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
        if(m_activated)
            m_windowMotionManager.calculate(time / 2.f);

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
    qDebug()<<__func__<<"m_toggleTimeline:"<<m_toggleTimeline.currentTime();
    if ((m_activated && m_toggleTimeline.currentValue() != 1)
            || (!m_activated && m_toggleTimeline.currentValue() != 0))
        effects->addRepaintFull();

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
            qDebug()<<"transformedGeo caption :" << w->caption() <<"geo:"<<geo;
            d.setScale(QVector2D((float)geo.width() / w->width(), (float)geo.height() / w->height()));
            effects->paintWindow(w, mask, area, d);
        }

    }else {
        effects->paintWindow(w, mask, region, data);
    }
}

void SwitchWindowEffect::windowInputMouseEvent(QEvent *e)
{
    switch (e->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
            break;
        case QEvent::MouseButtonRelease:
            moveToNextWindow();
            //setActive(false);
            break;

        default:
            return;
    }
}

bool SwitchWindowEffect::isActive() const
{
    //qDebug()<< "active:"<<m_activated<<"m_toggleTimeline:"<<m_toggleTimeline.currentValue() << "lock:" << effects->isScreenLocked();
    bool active = (m_activated || m_toggleTimeline.currentValue() != 0) && !effects->isScreenLocked();
    return (m_activated || m_toggleTimeline.currentValue() != 0) && !effects->isScreenLocked();
}

void SwitchWindowEffect::setActive(bool active)
{
    qDebug()<<"SwitchWindowEffect"<<__func__<<__LINE__;
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
        effects->startMouseInterception(this, Qt::PointingHandCursor);
        effects->setActiveFullScreenEffect(this);
        EffectWindowList windows = effects->stackingOrder();

        for (int i = 0, j = windows.size() - 1;  j >= 0; --j) {
            EffectWindow * w  = windows[j];
            if (!isRelevantWithPresentWindows(w)) {
                continue;
            }
            initWindowTargets(w,i);
            ++i;
        }

    }
    else {
       foreach (EffectWindow* w, m_windowMotionManager.managedWindows()) {
          m_windowMotionManager.moveWindow(w, w->geometry());
       }
       cleanup();
    }
}

void SwitchWindowEffect::moveToNextWindow()
{
    m_windowMotionManager.reset();
    m_windowMotionManager.unmanageAll();
    m_toggleTimeline.setCurrentTime(0);
    MoveEffectWindow window = m_moveingEffectWindows.dequeue();
    m_moveingEffectWindows.push_back(window);

    for (int i = 0; i<m_moveingEffectWindows.size(); ++i) {
        MoveEffectWindow & window = m_moveingEffectWindows[i];
        EffectWindow* w = window.window;
        QPoint topleft((w->geometry().width() * i) , w->y());
        m_windowMotionManager.manage(w);
        QRect transformedGeom(QPoint((w->geometry().width()) *(i - 1), w->y()),QSize(w->geometry().width(),w->geometry().height()));
        m_windowMotionManager.setTransformedGeometry(w,transformedGeom);
        m_windowMotionManager.moveWindow(w, topleft);
        qDebug()<<"target caption :" << w->caption() << "target topleft:" << topleft <<"transformedGeom" <<transformedGeom<<__func__;
        qDebug()<<"------------------";
    }

}

void SwitchWindowEffect::initWindowTargets(EffectWindow *w, uint index)
{
    auto geometry = w->geometry();
    QPoint topleft((geometry.width()) * index , w->y());
    MoveEffectWindow window;
    window.index = index;
    window.window = w;
    m_moveingEffectWindows.append(window);
    m_windowMotionManager.manage(w);
    QRect transformedGeom(QPoint((w->geometry().width()) *(index - 1), w->y()),QSize(w->geometry().width(),w->geometry().height()));
    m_windowMotionManager.setTransformedGeometry(w,transformedGeom);
    m_windowMotionManager.moveWindow(w, topleft);
    qDebug()<<"target caption :" << w->caption() << "target topleft:" << topleft <<"transformedGeom" <<transformedGeom;
    qDebug()<<"------------------";
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

void SwitchWindowEffect::cleanup()
{
    if (m_activated)
        return;
    effects->stopMouseInterception(this);
    effects->setActiveFullScreenEffect(0);
    m_windowMotionManager.unmanageAll();
    QQueue<MoveEffectWindow> empty;
    m_moveingEffectWindows.swap(empty);
    effects->addRepaintFull();
}

