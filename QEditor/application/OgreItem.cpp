/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreItem.h"
#include "OgreNode.h"
#include "GsageFacade.h"

#include <QPoint>

namespace Gsage {
  OgreItem::OgreItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mTimerID(0)
    , mFacade(0)
  {
    setFlag(ItemHasContents);
    setSmooth(false);

    startTimer(16);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    grabMouse();

    setCursor( QCursor( Qt::BlankCursor ));

    mKeyMap[0] = KeyboardEvent::KC_UNASSIGNED;
    mKeyMap[Qt::Key_Escape] = KeyboardEvent::KC_ESCAPE;
    mKeyMap[Qt::Key_1] = KeyboardEvent::KC_1;
    mKeyMap[Qt::Key_2] = KeyboardEvent::KC_2;
    mKeyMap[Qt::Key_3] = KeyboardEvent::KC_3;
    mKeyMap[Qt::Key_4] = KeyboardEvent::KC_4;
    mKeyMap[Qt::Key_5] = KeyboardEvent::KC_5;
    mKeyMap[Qt::Key_6] = KeyboardEvent::KC_6;
    mKeyMap[Qt::Key_7] = KeyboardEvent::KC_7;
    mKeyMap[Qt::Key_8] = KeyboardEvent::KC_8;
    mKeyMap[Qt::Key_9] = KeyboardEvent::KC_9;
    mKeyMap[Qt::Key_0] = KeyboardEvent::KC_0;
    mKeyMap[Qt::Key_Minus] = KeyboardEvent::KC_MINUS;
    mKeyMap[Qt::Key_Equal] = KeyboardEvent::KC_EQUALS;
    mKeyMap[Qt::Key_Backspace] = KeyboardEvent::KC_BACK;
    mKeyMap[Qt::Key_Tab] = KeyboardEvent::KC_TAB;
    mKeyMap[Qt::Key_Q] = KeyboardEvent::KC_Q;;
    mKeyMap[Qt::Key_W] = KeyboardEvent::KC_W;;
    mKeyMap[Qt::Key_E] = KeyboardEvent::KC_E;;
    mKeyMap[Qt::Key_R] = KeyboardEvent::KC_R;;
    mKeyMap[Qt::Key_T] = KeyboardEvent::KC_T;;
    mKeyMap[Qt::Key_Y] = KeyboardEvent::KC_Y;;
    mKeyMap[Qt::Key_U] = KeyboardEvent::KC_U;;
    mKeyMap[Qt::Key_I] = KeyboardEvent::KC_I;;
    mKeyMap[Qt::Key_O] = KeyboardEvent::KC_O;;
    mKeyMap[Qt::Key_P] = KeyboardEvent::KC_P;;
    mKeyMap[Qt::Key_BracketLeft] = KeyboardEvent::KC_LBRACKET;
    mKeyMap[Qt::Key_BracketRight] = KeyboardEvent::KC_RBRACKET;
    mKeyMap[Qt::Key_Return] = KeyboardEvent::KC_RETURN;
    mKeyMap[Qt::Key_Control] = KeyboardEvent::KC_LCONTROL;
    mKeyMap[Qt::Key_A] = KeyboardEvent::KC_A;;
    mKeyMap[Qt::Key_S] = KeyboardEvent::KC_S;;
    mKeyMap[Qt::Key_D] = KeyboardEvent::KC_D;;
    mKeyMap[Qt::Key_F] = KeyboardEvent::KC_F;;
    mKeyMap[Qt::Key_G] = KeyboardEvent::KC_G;;
    mKeyMap[Qt::Key_H] = KeyboardEvent::KC_H;;
    mKeyMap[Qt::Key_J] = KeyboardEvent::KC_J;;
    mKeyMap[Qt::Key_K] = KeyboardEvent::KC_K;;
    mKeyMap[Qt::Key_L] = KeyboardEvent::KC_L;;
    mKeyMap[Qt::Key_Semicolon] = KeyboardEvent::KC_SEMICOLON;
    mKeyMap[Qt::Key_Apostrophe] = KeyboardEvent::KC_APOSTROPHE;
    mKeyMap[Qt::Key_Dead_Grave] = KeyboardEvent::KC_GRAVE;
    mKeyMap[Qt::Key_Shift] = KeyboardEvent::KC_LSHIFT;
    mKeyMap[Qt::Key_Backslash] = KeyboardEvent::KC_BACKSLASH;
    mKeyMap[Qt::Key_Z] = KeyboardEvent::KC_Z;;
    mKeyMap[Qt::Key_X] = KeyboardEvent::KC_X;;
    mKeyMap[Qt::Key_C] = KeyboardEvent::KC_C;;
    mKeyMap[Qt::Key_V] = KeyboardEvent::KC_V;;
    mKeyMap[Qt::Key_B] = KeyboardEvent::KC_B;;
    mKeyMap[Qt::Key_N] = KeyboardEvent::KC_N;;
    mKeyMap[Qt::Key_M] = KeyboardEvent::KC_M;;
    mKeyMap[Qt::Key_Comma] = KeyboardEvent::KC_COMMA;
    mKeyMap[Qt::Key_Period] = KeyboardEvent::KC_PERIOD;
    mKeyMap[Qt::Key_Slash] = KeyboardEvent::KC_SLASH;
    mKeyMap[Qt::Key_multiply] = KeyboardEvent::KC_MULTIPLY;
    mKeyMap[Qt::Key_Menu] = KeyboardEvent::KC_LMENU;
    mKeyMap[Qt::Key_Space] = KeyboardEvent::KC_SPACE;
    mKeyMap[Qt::Key_CapsLock] = KeyboardEvent::KC_CAPITAL;
    mKeyMap[Qt::Key_F1] = KeyboardEvent::KC_F1;;
    mKeyMap[Qt::Key_F2] = KeyboardEvent::KC_F2;;
    mKeyMap[Qt::Key_F3] = KeyboardEvent::KC_F3;;
    mKeyMap[Qt::Key_F4] = KeyboardEvent::KC_F4;;
    mKeyMap[Qt::Key_F5] = KeyboardEvent::KC_F5;;
    mKeyMap[Qt::Key_F6] = KeyboardEvent::KC_F6;;
    mKeyMap[Qt::Key_F7] = KeyboardEvent::KC_F7;;
    mKeyMap[Qt::Key_F8] = KeyboardEvent::KC_F8;;
    mKeyMap[Qt::Key_F9] = KeyboardEvent::KC_F9;;
    mKeyMap[Qt::Key_F10] = KeyboardEvent::KC_F10;;
    mKeyMap[Qt::Key_NumLock] = KeyboardEvent::KC_NUMLOCK;
    mKeyMap[Qt::Key_ScrollLock] = KeyboardEvent::KC_SCROLL;
    mKeyMap[Qt::Key_F11] = KeyboardEvent::KC_F11;;
    mKeyMap[Qt::Key_F12] = KeyboardEvent::KC_F12;;
    mKeyMap[Qt::Key_F13] = KeyboardEvent::KC_F13;;
    mKeyMap[Qt::Key_F14] = KeyboardEvent::KC_F14;;
    mKeyMap[Qt::Key_F15] = KeyboardEvent::KC_F15;;
    mKeyMap[Qt::Key_Kana_Shift] = KeyboardEvent::KC_KANA;
    mKeyMap[Qt::Key_yen] = KeyboardEvent::KC_YEN;
    mKeyMap[Qt::Key_MediaPrevious] = KeyboardEvent::KC_PREVTRACK;
    mKeyMap[Qt::Key_Colon] = KeyboardEvent::KC_COLON;
    mKeyMap[Qt::Key_Underscore] = KeyboardEvent::KC_UNDERLINE;
    mKeyMap[Qt::Key_Kanji] = KeyboardEvent::KC_KANJI;
    mKeyMap[Qt::Key_Stop] = KeyboardEvent::KC_STOP;
    mKeyMap[Qt::Key_MediaNext] = KeyboardEvent::KC_NEXTTRACK;
    mKeyMap[Qt::Key_VolumeMute] = KeyboardEvent::KC_MUTE;
    mKeyMap[Qt::Key_Calculator] = KeyboardEvent::KC_CALCULATOR;
    mKeyMap[Qt::Key_MediaTogglePlayPause] = KeyboardEvent::KC_PLAYPAUSE;
    mKeyMap[Qt::Key_MediaStop] = KeyboardEvent::KC_MEDIASTOP;
    mKeyMap[Qt::Key_VolumeDown] = KeyboardEvent::KC_VOLUMEDOWN;
    mKeyMap[Qt::Key_VolumeUp] = KeyboardEvent::KC_VOLUMEUP;
    mKeyMap[Qt::Key_HomePage] = KeyboardEvent::KC_WEBHOME;
    mKeyMap[Qt::Key_division] = KeyboardEvent::KC_DIVIDE;
    mKeyMap[Qt::Key_SysReq] = KeyboardEvent::KC_SYSRQ;
    mKeyMap[Qt::Key_Pause] = KeyboardEvent::KC_PAUSE;
    mKeyMap[Qt::Key_Home] = KeyboardEvent::KC_HOME;
    mKeyMap[Qt::Key_Up] = KeyboardEvent::KC_UP;
    mKeyMap[Qt::Key_PageUp] = KeyboardEvent::KC_PGUP;
    mKeyMap[Qt::Key_Left] = KeyboardEvent::KC_LEFT;
    mKeyMap[Qt::Key_Right] = KeyboardEvent::KC_RIGHT;
    mKeyMap[Qt::Key_End] = KeyboardEvent::KC_END;
    mKeyMap[Qt::Key_Down] = KeyboardEvent::KC_DOWN;
    mKeyMap[Qt::Key_PageDown] = KeyboardEvent::KC_PGDOWN;
    mKeyMap[Qt::Key_Insert] = KeyboardEvent::KC_INSERT;
    mKeyMap[Qt::Key_Delete] = KeyboardEvent::KC_DELETE;
    mKeyMap[Qt::Key_Meta] = KeyboardEvent::KC_LWIN;
    mKeyMap[Qt::Key_PowerOff] = KeyboardEvent::KC_POWER;
    mKeyMap[Qt::Key_Sleep] = KeyboardEvent::KC_SLEEP;
    mKeyMap[Qt::Key_WakeUp] = KeyboardEvent::KC_WAKE;
    mKeyMap[Qt::Key_Search] = KeyboardEvent::KC_WEBSEARCH;
    mKeyMap[Qt::Key_Favorites] = KeyboardEvent::KC_WEBFAVORITES;
    mKeyMap[Qt::Key_Refresh] = KeyboardEvent::KC_WEBREFRESH;
    mKeyMap[Qt::Key_Stop] = KeyboardEvent::KC_WEBSTOP;
    mKeyMap[Qt::Key_Forward] = KeyboardEvent::KC_WEBFORWARD;
    mKeyMap[Qt::Key_Back] = KeyboardEvent::KC_WEBBACK;
    mKeyMap[Qt::Key_Launch0] = KeyboardEvent::KC_MYCOMPUTER;
    mKeyMap[Qt::Key_LaunchMail] = KeyboardEvent::KC_MAIL;
    mKeyMap[Qt::Key_LaunchMedia] = KeyboardEvent::KC_MEDIASELECT;

    mModifierMap[Qt::ShiftModifier] = KeyboardEvent::Shift;
    mModifierMap[Qt::ControlModifier] = KeyboardEvent::Ctrl;
    mModifierMap[Qt::AltModifier] = KeyboardEvent::Alt;
  }

  QSGNode *OgreItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
  {
    if (width() <= 0 || height() <= 0 || mFacade == 0 || mFacade->isStopped()) {
      delete oldNode;
      return 0;
    }

    OgreNode* node = static_cast<OgreNode*>(oldNode);
    if (!node)
    {
      node = new OgreNode(window());
      node->setEngine(mFacade->getEngine());
    }

    onUpdate();
    node->setSize(QSize(width(), height()));
    node->update();
    // mark texture dirty, otherwise Qt will not trigger a redraw (preprocess())
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
  }

  void OgreItem::timerEvent(QTimerEvent *)
  {
    update();
  }

  void OgreItem::setEngine(GsageFacade *facade)
  {
    mFacade = facade;
  }

  void OgreItem::mousePressEvent(QMouseEvent* event)
  {
    proxyMouseEvent(event);
  }

  void OgreItem::mouseReleaseEvent(QMouseEvent* event)
  {
    proxyMouseEvent(event);
  }

  void OgreItem::mouseMoveEvent(QMouseEvent *event)
  {
    proxyMouseEvent(event);
  }

  void OgreItem::hoverMoveEvent(QHoverEvent* event)
  {
    if(mFacade == 0 || mFacade->getEngine() == 0)
      return;

    MouseEvent e(MouseEvent::MOUSE_MOVE, width(), height(), MouseEvent::None);
    QPointF p = mapToItem(this, event->posF());
    e.setAbsolutePosition(p.x() * (window()->width() / width()), p.y() * (window()->height() / height()), 0);
    e.setRelativePosition(0,0,0);
    mFacade->getEngine()->fireEvent(e);
  }

  void OgreItem::wheelEvent(QWheelEvent* event)
  {
    if(mFacade == 0 || mFacade->getEngine() == 0)
      return;

    QPointF p = mapToItem(this, QPointF(event->pos()));
    MouseEvent e(MouseEvent::MOUSE_MOVE, width(), height());
    e.setAbsolutePosition(p.x() * (window()->width() / width()), p.y() * (window()->height() / height()), 0);
    e.setRelativePosition(0, 0, event->delta());
    mFacade->getEngine()->fireEvent(e);
  }

  void OgreItem::proxyMouseEvent(QMouseEvent* event)
  {
    if(mFacade == 0 || mFacade->getEngine() == 0)
      return;
    std::string eventType;
    switch(event->type())
    {
      case QEvent::MouseMove:
        eventType = MouseEvent::MOUSE_MOVE;
        break;
      case QEvent::MouseButtonPress:
        forceActiveFocus();
        eventType = MouseEvent::MOUSE_DOWN;
        break;
      case QEvent::MouseButtonRelease:
        eventType = MouseEvent::MOUSE_UP;
        break;
      default:
        return;
    }

    MouseEvent e(eventType, width(), height(), mapButtonType(event->button()));
    QPointF p = mapToItem(this, QPointF(event->pos()));
    e.setAbsolutePosition(p.x() * (window()->width() / width()), p.y() * (window()->height() / height()), 0);
    e.setRelativePosition(0,0,0);
    mFacade->getEngine()->fireEvent(e);
  }

  MouseEvent::ButtonType OgreItem::mapButtonType(const Qt::MouseButtons& t)
  {
    MouseEvent::ButtonType res;
    switch(t)
    {
      case Qt::LeftButton:
        res = MouseEvent::Left;
        break;
      case Qt::RightButton:
        res = MouseEvent::Right;
        break;
      case Qt::MiddleButton:
        res = MouseEvent::Middle;
        break;
      default:
        res = MouseEvent::None;
    }
    return res;
  }

  void OgreItem::keyPressEvent(QKeyEvent * e)
  {
    KeyboardEvent::Key key = mKeyMap[e->key()];
    KeyboardEvent event(KeyboardEvent::KEY_DOWN, key, e->text().toUtf8().constData()[0], getModifiersState(e));
    mFacade->getEngine()->fireEvent(event);
  }

  void OgreItem::keyReleaseEvent(QKeyEvent * e)
  {
    KeyboardEvent::Key key = mKeyMap[e->key()];
    KeyboardEvent event(KeyboardEvent::KEY_UP, key, e->text().toUtf8().constData()[0], getModifiersState(e));
    mFacade->getEngine()->fireEvent(event);
  }

  unsigned int OgreItem::getModifiersState(QKeyEvent * e)
  {
    int modifiers[] = {
      Qt::ShiftModifier,
      Qt::ControlModifier,
      Qt::AltModifier
    };

    unsigned int res = 0;
    for(int i = 0; i < 3; i++)
    {
      if(e->modifiers() & modifiers[i])
          res |= mModifierMap[modifiers[i]];
    }
  }
}
