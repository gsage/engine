#ifndef _OgreItemH_
#define _OgreItemH_

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

#include <QtQuick/QQuickItem>
#include <QtCore/QPropertyAnimation>
#include "KeyboardEvent.h"

#include "MouseEvent.h"

namespace Gsage {
  class GsageFacade;

  class OgreItem : public QQuickItem
  {
      Q_OBJECT

    public:
      OgreItem(QQuickItem *parent = 0);

      void setEngine(GsageFacade *facade);

    protected:
      virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

      void timerEvent(QTimerEvent *);

      void mouseReleaseEvent(QMouseEvent* event);

      void mousePressEvent(QMouseEvent* event);

      void mouseMoveEvent(QMouseEvent* event);

      void hoverMoveEvent(QHoverEvent* event);

      void wheelEvent(QWheelEvent* event);

      void keyPressEvent(QKeyEvent * event);

      void keyReleaseEvent(QKeyEvent * event);
    signals:
      void onUpdate();
    private:
      int mTimerID;

      GsageFacade* mFacade;

      void proxyMouseEvent(QMouseEvent* event);

      unsigned int getModifiersState(QKeyEvent * e);

      MouseEvent::ButtonType mapButtonType(const Qt::MouseButtons& t);

      typedef std::map<int, KeyboardEvent::Key> KeyIdentifierMap;
      typedef std::map<int, KeyboardEvent::Modifier> ModifierMap;
      KeyIdentifierMap mKeyMap;
      ModifierMap mModifierMap;
  };
}
#endif
