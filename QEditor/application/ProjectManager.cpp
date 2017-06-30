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

#include "ProjectManager.h"
#include "Logger.h"

#include <istream>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace Gsage {

  ProjectTemplate::ProjectTemplate(const QString& id, const QString& name, const QString& image, const QString& description)
    : mId(id)
    , mName(name)
    , mImage(image)
    , mDescription(description)
  {
  }

  ProjectManager::ProjectManager()
    : mEditorFolder(QDir::currentPath())
  {
  }

  ProjectManager::~ProjectManager()
  {
    qDeleteAll(mTemplates);
  }

  bool ProjectManager::initialize(const DataProxy& settings)
  {
    const std::string defaultProjectImage = settings.get("defaultProjectImage", "");

    auto templatesFolder = settings.get<std::string>("templatesFolder");
    if(!templatesFolder.second)
    {
      LOG(ERROR) << "Failed to setup project manager: no \"templatesFolder\" field, was found in the config";
      return false;
    }
    mTemplatesFolder = templatesFolder.first;

    // TODO: maybe it is better to iterate all project templates in the templates directory
    auto templates = settings.get<DataProxy>("projectTemplates");
    if(templates.second)
    {
      for(auto& pair : templates.first)
      {
        auto templateId = pair.first;
        const std::string image = pair.second.get<std::string>("image", defaultProjectImage);
        const std::string name = pair.second.get("name", templateId);
        const std::string description = pair.second.get("description", "No description");

        mTemplates.append(
          new ProjectTemplate(
            QString::fromUtf8(templateId.str().c_str()),
            QString::fromUtf8(name.data(), name.size()),
            QString::fromUtf8(image.data(), image.size()),
            QString::fromUtf8(description.data(), description.size())
          )
        );
      }
    }
    return true;
  }

  ProjectManager::CreationStatus ProjectManager::create(QString templateFile, QVariantMap projectSettings)
  {
    QDir::setCurrent(mEditorFolder);
    QString path = QDir(mTemplatesFolder.c_str()).filePath(templateFile.append(".json"));
    CreationStatus status = UNKNOWN;

    DataProxy createInstructions;

    if(!readJson(path.toStdString(), createInstructions)) {
      LOG(ERROR) << "Failed to parse template file: " << path.toStdString();
      status = NO_SUCH_TEMPLATE;
    }
    if (status != UNKNOWN)
    {
      return status;
    }

    LOG(INFO) << "Creating project of type: " << templateFile.toStdString();
    QString projectName = projectSettings["projectName"].toString();
    QString projectPath = projectSettings["projectPath"].toString();

    bool overwrite = projectSettings["overwrite"].toBool();

    QDir directory(projectPath.append(QDir::separator()).append(projectName));
    QString projectSettingsFile(directory.path().append(QDir::separator()).append("project.json"));
    if (directory.exists())
    {
      if(!overwrite && directory.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0 && QFile(projectSettingsFile).exists())
      {
        status = PROJECT_EXISTS;
        return status;
      }
    }
    else
    {
      directory.mkpath(".");
    }

    auto resources = createInstructions.get<DataProxy>("resources");
    if (resources.second)
    {
      for (auto pair : resources.first)
      {
        QString resourcePath(pair.second.getValue<std::string>().first.c_str());
        QStringList parts = resourcePath.split(":");
        QString resourceType("local");
        QString resourceProjectMountpoint = resourcePath;
        if (parts.size() > 1)
        {
          resourceType = parts[0];
          resourcePath = parts[1];
          resourceProjectMountpoint = resourcePath;
        }
        if (parts.size() == 3)
        {
          resourceProjectMountpoint = parts[2];
        }
        LOG(INFO) << "Getting resource for the project: " << resourceType.toUtf8().constData() << " " << resourcePath.toUtf8().constData();

        if (resourceType == "local")
        {
          if (!copyFiles(QString("%1/%2").arg("../..").arg(resourcePath),
                QString("%1/%2").arg(directory.path()).arg(resourceProjectMountpoint)))
          {
            return RESOURCE_COPYING_FAILED;
          }
        }
        else if(resourceType == "server")
        {
          // TODO: implement me
        }

      }
    }

    DataProxy projectConfig;
    if(!writeJson(projectSettingsFile.toStdString(), projectConfig))
    {
      return PROJECT_FILE_CREATION_FAILED;
    }

    return SUCCESS;
  }

  QStringList ProjectManager::getProjectLevels()
  {
    QStringList res;
    if(mCurrentProjectFolder.isEmpty())
    {
      return res;
    }

    QDir levelsDir(mCurrentProjectFolder.append(QDir::separator()).append("resources").append(QDir::separator()).append("levels"));
    QFileInfoList files = levelsDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    QStringList levels;
    foreach(QFileInfo info, files) {
      levels << info.baseName();
    }
    return levels;
  }

  bool ProjectManager::loadProject(QString projectFilePath, DataProxy& configs)
  {
    if(!readJson(projectFilePath.toStdString(), configs))
    {
      return false;
    }

    QFileInfo projectFileInfo(projectFilePath);
    mCurrentProjectFolder = projectFileInfo.absolutePath();
    QDir::setCurrent(mCurrentProjectFolder);
    return true;
  }

  QString ProjectManager::defaultFolder() const
  {
    return QString(QDir::homePath() + "/gsageProjects");
  }

  QQmlListProperty<ProjectTemplate> ProjectManager::templates()
  {
    return QQmlListProperty<ProjectTemplate>(this, mTemplates);
  }

  bool ProjectManager::copyFiles(const QString& src, const QString& dst)
  {
    QFileInfo srcFileInfo(src);
    if (srcFileInfo.isDir()) {
      QDir targetDir(dst);
      targetDir.cdUp();
      if (!targetDir.mkdir(QFileInfo(dst).fileName()))
        return false;
      QDir sourceDir(src);
      QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
      foreach (const QString &fileName, fileNames) {
        LOG(INFO) << "Copying file " << fileName.toUtf8().constData();
        const QString newSrcFilePath
          = src + QLatin1Char('/') + fileName;
        const QString newTgtFilePath
          = dst + QLatin1Char('/') + fileName;
        if (!copyFiles(newSrcFilePath, newTgtFilePath))
          return false;
      }
    } else {
      if (!QFile::copy(src, dst))
        return false;
    }
    return true;
  }
}
