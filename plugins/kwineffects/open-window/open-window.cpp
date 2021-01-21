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

#include "open-window.h"

OpenWindowEffect::OpenWindowEffect(QObject *, const QVariantList &)
    :Effect()
{
    reconfigure(ReconfigureAll);
    connect(effects, SIGNAL(windowDeleted(KWin::EffectWindow*)), this, SLOT(slotWindowDeleted(KWin::EffectWindow*)));
    connect(effects, SIGNAL(windowAdded(KWin::EffectWindow *)), this, SLOT(slotWindowAdded(KWin::EffectWindow *)));
}

OpenWindowEffect::~OpenWindowEffect()
{

}

bool OpenWindowEffect::supported()
{
    return effects->isOpenGLCompositing() && effects->animationsSupported();
}

void OpenWindowEffect::reconfigure(ReconfigureFlags)
{
    // TODO: Rename animationDuration to duration so we can use
    // animationTime<400>
    const int d = 400;
    m_duration = std::chrono::milliseconds(static_cast<int>(animationTime(d)));
}

void OpenWindowEffect::prePaintScreen(ScreenPrePaintData& data, int time)
{
    const std::chrono::milliseconds delta(time);

    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        (*animationIt).update(delta);
        ++animationIt;
    }

    // We need to mark the screen windows as transformed. Otherwise the
    // whole screen won't be repainted, resulting in artefacts.
    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    effects->prePaintScreen(data, time);
}

void OpenWindowEffect::prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time)
{
    // Schedule window for transformation if the animation is still in
    //  progress
    if (m_animations.contains(w)) {
        // We'll transform this window
        data.setTransformed();
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
    }

    effects->prePaintWindow(w, data, time);
}

void OpenWindowEffect::paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt != m_animations.constEnd()) {
        qreal coef = animationIt->value();
        if(m_showWindow && w == m_showWindow) {
            int screenwidth = effects->virtualScreenSize().width();
            int scrrenheight = effects->virtualScreenSize().height();

            qreal width = m_showWindowRect.width()+ (screenwidth - m_showWindowRect.width()) * coef;
            qreal height = m_showWindowRect.height()+ (scrrenheight - m_showWindowRect.height()) * coef;
            qreal x = m_showWindowRect.x() + (0 - m_showWindowRect.x()) * coef;
            qreal y = m_showWindowRect.y() + (0 - m_showWindowRect.y()) * coef;

            data += QPoint(qRound(x - (float)w->x()), qRound(y - w->y()));
            data.setScale(QVector2D(width / w->width(), (float)height / w->height()));
        } else {
            data.setOpacity(coef);
            data.setScale(QVector2D((float)coef, (float)coef));
            data.translate(w->geometry().width() * 0.5 * (1.0 - coef), w->geometry().height() * 0.5 * (1.0 - coef));
        }

    }
    // Call the next effect.
    effects->paintWindow(w, mask, region, data);
}

void OpenWindowEffect::postPaintScreen()
{
    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        if ((*animationIt).done()) {
            animationIt = m_animations.erase(animationIt);
            if(m_showWindow) {
                m_showWindow = nullptr;
            }
        } else {
            ++animationIt;
        }
    }

    effects->addRepaintFull();

    // Call the next effect.
    effects->postPaintScreen();
}

void OpenWindowEffect::slotWindowDeleted(EffectWindow* w)
{
    m_animations.remove(w);
}

void OpenWindowEffect::slotWindowAdded(EffectWindow *w)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    if (!isRelevantWithPresentWindows(w)) {
        return; // don't add
    }

    TimeLine &timeLine = m_animations[w];

    if (timeLine.running()) {
        timeLine.toggleDirection();
    } else {
        timeLine.setDirection(TimeLine::Forward);
        timeLine.setDuration(m_duration);
        timeLine.setEasingCurve(QEasingCurve::OutExpo);
    }

    effects->addRepaintFull();
}

bool OpenWindowEffect::isRelevantWithPresentWindows(EffectWindow *w) const
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

bool OpenWindowEffect::isActive() const
{
    return !m_animations.isEmpty();
}

void OpenWindowEffect::showWindow(int winId, int ox, int oy, int width, int height)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    m_showWindow = effects->findWindow(winId);
    if(!m_showWindow) {
        return;
    }

    if (!isRelevantWithPresentWindows(m_showWindow)) {
        return; // don't add
    }

    m_showWindowRect = QRect(ox, oy, width, height);
    TimeLine &timeLine = m_animations[m_showWindow];

    if (timeLine.running()) {
        timeLine.toggleDirection();
    } else {
        timeLine.setDirection(TimeLine::Forward);
        timeLine.setDuration(m_duration);
        timeLine.setEasingCurve(QEasingCurve::OutExpo);
    }
    effects->activateWindow(m_showWindow);
    effects->addRepaintFull();
}

