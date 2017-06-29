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

#ifndef _EditorApplication_H_
#define _EditorApplication_H_

#include <QtQuick/QQuickView>
#include <QMutex>

#include "EditorFacade.h"
#include "DataProxy.h"


namespace Gsage {

  class ProjectManager;

  class EditorApplication : public QQuickView
  {
      Q_OBJECT

    public:

      explicit EditorApplication(QWindow* parent = 0);
      virtual ~EditorApplication();

      Q_INVOKABLE void load(const QString& name);
      Q_INVOKABLE bool loadProject(const QString& projectFilePath);
      Q_INVOKABLE void setEnginePaused(const bool paused);

      /**
       * Initialize all systems and start all systems
       * @param viewId View id to attach to
       * @param configPath Game config path to use
       * @param resourcePath Game resource folder
       */
      void initEngine(const QString& viewId, const QString& configPath, const QString& resourcePath);

      /**
       * Set editor configuration path and kick off editor initialization sequence.
       *
       * @param configPath Editor config path
       */
      void initialize(const std::string& configPath);

      /**
       * Destroy engine
       */
      void destroyFacade();

    public slots:
      /**
       * @copydoc GsageFacade::update
       */
      void update();

      /**
       * Set up qml stuff
       */
      void setUpUI();

      /**
       * Initialize engine systems
       */
      void loadSystems();
    signals:

      /**
       * Initialized engine signal
       */
      void initializedEngine();

      /**
       * Systems initialization success
       */
      void systemsInitialized();

    private:
      std::string mGameConfigPath;
      std::string mEditorConfigPath;
      std::string mResourcePath;
      std::string mViewItemId;
      std::string mCurrentArea;

      DataProxy mConfig;
      DataProxy mConfigOverride;

      bool mSystemsInitialized;
      bool mPaused;
      bool mFacadeInitialized;
      bool mAreaChanged;

      ProjectManager* mProjectManager;
      EditorFacade* mFacade;

      QMutex mEngineReloadMutex;
  };
}
#endif
