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
#include "PtreeExtensions.h"

#include <boost/property_tree/json_parser.hpp>
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

  bool ProjectManager::initialize(const DataNode& settings)
  {
    const std::string defaultProjectImage = settings.get("defaultProjectImage", "");

    auto templatesFolder = settings.get_optional<std::string>("templatesFolder");
    if(!templatesFolder)
    {
      LOG(ERROR) << "Failed to setup project manager: no \"templatesFolder\" field, was found in the config";
      return false;
    }
    mTemplatesFolder = templatesFolder.get();

    // TODO: maybe it is better to iterate all project templates in the templates directory
    if(auto templates = settings.get_child_optional("projectTemplates"))
    {
      for(auto& pair : templates.get())
      {
        auto templateId = pair.first;
        const std::string image = pair.second.get<std::string>("image", defaultProjectImage);
        const std::string name = pair.second.get("name", templateId);
        const std::string description = pair.second.get("description", "No description");

        mTemplates.append(
          new ProjectTemplate(
            QString::fromUtf8(templateId.c_str()),
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

    DataNode createInstructions;
    std::ifstream ifs(path.toStdString());
    try
    {
      boost::property_tree::read_json(ifs, createInstructions);
    }
    catch(boost::property_tree::json_parser_error& ex)
    {
      LOG(ERROR) << "Failed to parse template file: " << path.toStdString() << ", reason: " << ex.message();
      status = NO_SUCH_TEMPLATE;
    }
    ifs.close();
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

    auto resources = createInstructions.get_child_optional("resources");
    if (resources)
    {
      for (auto pair : resources.get())
      {
        QString resourcePath(pair.second.get_value<std::string>().c_str());
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

    DataNode projectConfig;
    if(!dumpFile(projectSettingsFile.toStdString(), projectConfig))
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

  bool ProjectManager::loadProject(QString projectFilePath, DataNode& configs)
  {
    if(!parseFile(projectFilePath.toStdString(), configs))
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
