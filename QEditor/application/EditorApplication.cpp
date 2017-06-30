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

#include "EditorApplication.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <QtQml>
#include <QDir>
#include <QQmlPropertyMap>
#include <QFile>
#include <QFileInfo>

#include "OgreItem.h"
#include "EditorFacade.h"

#include "QOgreRenderSystem.h"

#include "components/ScriptComponent.h"

#include "systems/RecastMovementSystem.h"
#include "systems/CombatSystem.h"
#include "systems/LuaScriptSystem.h"

#include "Logger.h"
#include "GameDataManager.h"

#include "ProjectManager.h"

namespace Gsage {

  EditorApplication::EditorApplication(QWindow* parent)
    : QQuickView(parent)
    , mProjectManager(new ProjectManager())
    , mFacade(0)
    , mSystemsInitialized(false)
    , mAreaChanged(false)
  {
    connect(this, &EditorApplication::initializedEngine, this, &EditorApplication::setUpUI);
    qmlRegisterType<Gsage::OgreItem>("Ogre.Viewer", 1, 0, "OgreItem");
    qmlRegisterType<Gsage::ProjectTemplate>("Editor.Models", 1, 0, "ProjectTemplate");
    qmlRegisterType<Gsage::ProjectManager>("Editor.Utils", 1, 0, "ProjectManager");
  }

  EditorApplication::~EditorApplication()
  {
    if (mFacade != 0) 
    {
      mFacade->halt();
      destroyFacade();
    }
    delete mProjectManager;
  }

  void EditorApplication::initialize(const std::string& configPath)
  {
    mEditorConfigPath = configPath;
    if(!readJson(configPath, mConfig))
    {
      LOG(ERROR) << "Failed to read editor config file: \"" << configPath;
      QCoreApplication::exit();
    }

    auto projectManagerConfig = mConfig.get<DataProxy>("projectManager");
    if(!projectManagerConfig.second)
    {
      LOG(ERROR) << "No projectManager configs found in config file";
      QCoreApplication::exit();
    }

    if(!mProjectManager->initialize(projectManagerConfig.first))
    {
      LOG(ERROR) << "Failed to initialize project manager";
      QCoreApplication::exit();
    }

    rootContext()->setContextProperty("editor", this);
    rootContext()->setContextProperty("projectManager", mProjectManager);

    emit(initializedEngine());
  }

  void EditorApplication::initEngine(const QString& viewId, const QString& configPath, const QString& resourcePath)
  {
    mViewItemId = viewId.toStdString();
    if(mFacade != 0) {
      disconnect(this, &EditorApplication::beforeRendering, this, &EditorApplication::update);
      OgreItem* view = rootObject()->findChild<OgreItem*>(viewId);
      if(view)
      {
        view->setEngine(0);
      }
      destroyFacade();
    }

    mGameConfigPath = configPath.toStdString();
    mResourcePath = resourcePath.toStdString();
    connect(this, &EditorApplication::beforeRendering, this, (void (EditorApplication::*)(void))&EditorApplication::loadSystems, Qt::DirectConnection);
  }

  void EditorApplication::loadSystems()
  {
    disconnect(this, &EditorApplication::beforeRendering, this, (void (EditorApplication::*)(void))&EditorApplication::loadSystems);
    mFacade = new EditorFacade();
    if(!mFacade->initialize(mGameConfigPath, mResourcePath, &mConfigOverride))
    {
      LOG(ERROR) << "Failed to initialize game engine";
      destroyFacade();
      return;
    }

    // get all ogre view objects from the view and inject engine instance to all of them
    OgreItem* view = rootObject()->findChild<OgreItem*>(mViewItemId.c_str());
    if(view)
    {
      view->setEngine(mFacade);
      systemsInitialized();
    } else {
      LOG(ERROR) << "Can't attach ogre view to the view with id \"" << mViewItemId << "\"";
    }
  }

  bool EditorApplication::loadProject(const QString& projectFilePath)
  {
    DataProxy projectConfig;
    if(!mProjectManager->loadProject(projectFilePath, projectConfig))
    {
      return false;
    }

    QFileInfo projectFileInfo(projectFilePath);
    QString gameConfig(projectConfig.get("configPath", "gameConfig.json").c_str());
    QString resourcePath(projectFileInfo.absolutePath().append(QDir::separator()).append("resources"));
    mConfigOverride.put("render", mConfig.get<DataProxy>("render", DataProxy()));

    initEngine("renderWindow", gameConfig, resourcePath);
    return true;
  }

  void EditorApplication::setEnginePaused(const bool value)
  {
    if(value) {
      disconnect(this, &EditorApplication::beforeRendering, this, &EditorApplication::update);
    } else {
      connect(this, &EditorApplication::beforeRendering, this, &EditorApplication::update, Qt::DirectConnection);
    }
    mPaused = value;
  }

  void EditorApplication::setUpUI()
  {
    setResizeMode(QQuickView::SizeRootObjectToView);
    setSource(QUrl::fromLocalFile(QString(mResourcePath.c_str()).append(RESOURCES_FOLDER).append(GSAGE_PATH_SEPARATOR).append("editor/qml/main.qml")));
  }

  void EditorApplication::update()
  {
    mEngineReloadMutex.lock();
    if(mPaused || mFacade == 0) {
      return;
    }

    if(!mFacade->update()) {
      destroyFacade();
    }

    if(mAreaChanged) {
      mAreaChanged = false;
      mFacade->loadArea(mCurrentArea);
    }
    mEngineReloadMutex.unlock();
  }

  void EditorApplication::load(const QString& name)
  {
    mEngineReloadMutex.lock();
    mFacade->reset();
    mCurrentArea = name.toStdString();
    mAreaChanged = true;
    mEngineReloadMutex.unlock();
  }

  void EditorApplication::destroyFacade() {
    if (mFacade != 0)
    {
      OgreItem* view = rootObject()->findChild<OgreItem*>(mViewItemId.c_str());
      if(view)
      {
        view->setEngine(0);
      }
      delete mFacade;
      mFacade = 0;
    }
  }
}
