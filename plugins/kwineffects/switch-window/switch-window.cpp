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
    const QEasingCurve TOGGLE_MODE =  QEasingCurve::OutExpo;// AnimationMode.EASE_OUT_Expo;
    static const int FADE_DURATION = 600;
    static const int SWITCH_SPACE = 50;
}

SwitchWindowEffect::SwitchWindowEffect(QObject *, const QVariantList &):
    Effect()
{
    connect(effects, &EffectsHandler::windowAdded, this, &SwitchWindowEffect::onWindowAdded);
    connect(effects, &EffectsHandler::windowDeleted, this, &SwitchWindowEffect::onWindowDeleted);
    connect(effects, &EffectsHandler::windowUnminimized, this, &SwitchWindowEffect::onWindowUnminimized);
    connect(effects, &EffectsHandler::windowMinimized, this, &SwitchWindowEffect::onWindowMminimized);
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
    m_duration = std::chrono::milliseconds(static_cast<int>(animationTime(SwitchConsts::FADE_DURATION)));
    m_animationTimeline.setDuration(m_duration);
    m_animationTimeline.setDirection(TimeLine::Forward);//0~1
    m_animationTimeline.setEasingCurve(SwitchConsts::TOGGLE_MODE);
    updateSwitchingWindows();
}

void SwitchWindowEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    if (isActive()) {
        if (m_activated && m_moving) {
            const std::chrono::milliseconds delta(time);
            m_animationTimeline.update(delta);
            data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
        }
    }
    // this makes blurred windows work, should we need it ?
    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant(true));
    }
    effects->prePaintScreen(data, time);
}

void SwitchWindowEffect::postPaintScreen()
{
    if ((m_activated && m_animationTimeline.running()))
        effects->addRepaintFull();

    if (m_activated && m_animationTimeline.done()) {
        setActive(false);
    }

    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant());
    }

    // Call the next effect.
    effects->postPaintScreen();
}

// Window painting
void SwitchWindowEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    //must have ,else paintwindow it not effect
    data.setTransformed();

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
    if (m_moving || w->isDesktop()) {
        auto area = effects->clientArea(ScreenArea, 0, 0);
        WindowPaintData d = data;
        if (w->isDesktop()) {
            d.setBrightness(0.4);
            effects->paintWindow(w, mask, area, d);

        } else if (!w->isDesktop()) {
            qreal cofe = 0;
            if (m_preEffectWindow) {
                if (w == m_currentEffectWindow) {
                    cofe = m_animationTimeline.value();
                    data.translate(w->geometry().width() * cofe, 0);
                } else if (w == m_preEffectWindow) {
                    cofe = (1 - m_animationTimeline.value()) * -1;
                    data.translate(w->geometry().width() * cofe - SwitchConsts::SWITCH_SPACE, 0);
                } else if(m_moveingEffectWindows.indexOf(w) != -1) {
                    data.translate(w->geometry().width() * -1, 0);
                }
            } else if (m_nextEffectWindow) {
                if (w == m_currentEffectWindow) {
                    cofe = -m_animationTimeline.value();
                    data.translate(w->geometry().width() * cofe, 0);
                } else if (w == m_nextEffectWindow) {
                    cofe = 1 - m_animationTimeline.value();
                    data.translate(w->geometry().width() * cofe + SwitchConsts::SWITCH_SPACE, 0);
                } else if(m_moveingEffectWindows.indexOf(w) != -1) {
                    data.translate(w->geometry().width() * -1, 0);
                }
            }
        }
    }

    effects->paintWindow(w, mask, region, data);
}

bool SwitchWindowEffect::isActive() const
{
    return m_activated && !effects->isScreenLocked();
}

void SwitchWindowEffect::setActive(bool active)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this)
        return;

    if (m_activated == active)
        return;

    if (active && !m_currentEffectWindow) {
        //windows on the desktop are minimized, so return
        return;
    }

    if (m_activated && m_animationTimeline.running()) {
        // activate the animation again but it's not over yet.
        return;
    }

    m_activated = active;
    if (active) {
        m_animationTimeline.reset(); // set vaule = 0;
        effects->setActiveFullScreenEffect(this);
    } else {
        cleanup();
    }
}

void SwitchWindowEffect::moveToPreWindow()
{
    if (!isActive()) {
        return;
    }

    m_preEffectWindow = getPreWindow();
    if (m_preEffectWindow == nullptr) {
        setActive(false);//close paint
        return;
    }

    m_moving = true;
    m_animationTimeline.reset();
    m_nextEffectWindow = nullptr;
    effects->addRepaintFull();
}

void SwitchWindowEffect::moveToNextWindow()
{
    if (!isActive()) {
        return;
    }

    m_nextEffectWindow = getNextWindow();
    if (m_nextEffectWindow == nullptr) {
        setActive(false);//close paint
        return;
    }

    m_moving = true;
    m_animationTimeline.reset();
    m_preEffectWindow = nullptr;
    effects->addRepaintFull();
}

void SwitchWindowEffect::onWindowAdded(EffectWindow *w)
{
    if (!isRelevantWithPresentWindows(w))
        return; // don't add
    m_currentEffectWindow = w;
    m_moveingEffectWindows.append(w);
}

void SwitchWindowEffect::onWindowDeleted(EffectWindow *w)
{
    if (m_moveingEffectWindows.contains(w)) {
        m_moveingEffectWindows.removeOne(w);
        m_currentEffectWindow = nullptr;
    }
}

void SwitchWindowEffect::onWindowUnminimized(EffectWindow *w)
{
    m_currentEffectWindow = w;
}

void SwitchWindowEffect::onWindowMminimized(EffectWindow *w)
{
    m_currentEffectWindow = nullptr;
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
    for (int i = 0; i < windows.size(); ++i) {
        EffectWindow *w = windows[i];
        if (isRelevantWithPresentWindows(w)) {
            m_moveingEffectWindows.append(w);
        }
    }
    if (m_moveingEffectWindows.size() > 0) {
        m_currentEffectWindow = m_moveingEffectWindows.last();
    }
}

EffectWindow *SwitchWindowEffect::getNextWindow() const
{
    if (m_currentEffectWindow) {
        if (m_currentEffectWindow == m_moveingEffectWindows.last() || m_moveingEffectWindows.size() <= 1) {
            return nullptr;//do not switch, have not next window
        }
        int cindex = m_moveingEffectWindows.indexOf(m_currentEffectWindow);
        EffectWindow *w = m_moveingEffectWindows[++cindex];//next window
        return w;

    }
    return nullptr;
}

EffectWindow *SwitchWindowEffect::getPreWindow() const
{
    if (m_currentEffectWindow){
        if (m_currentEffectWindow == m_moveingEffectWindows.first() || m_moveingEffectWindows.size() <= 1) {
            return nullptr;//do not switch, have not pre window
        }
        int cindex = m_moveingEffectWindows.indexOf(m_currentEffectWindow);
        EffectWindow *w = m_moveingEffectWindows[--cindex];//pre window
        return w;
    }
    return nullptr;
}

void SwitchWindowEffect::cleanup()
{
    if (m_activated) {
        return;
    }

    if (m_preEffectWindow) {
        m_currentEffectWindow = m_preEffectWindow;
    } else if (m_nextEffectWindow) {
        m_currentEffectWindow = m_nextEffectWindow;
    }
    if (m_currentEffectWindow) {
        effects->activateWindow(m_currentEffectWindow);
    }
    m_moving = false;
    effects->setActiveFullScreenEffect(0);
}

