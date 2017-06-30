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

#ifndef _ProjectManager_H_
#define _ProjectManager_H_

#include "GsageDefinitions.h"
#include "DataProxy.h"
#include <QObject>
#include <QQmlListProperty>

namespace Gsage {

  class ProjectTemplate : public QObject
  {
      Q_OBJECT
      Q_PROPERTY(QString image READ image CONSTANT)
      Q_PROPERTY(QString name READ name CONSTANT)
      Q_PROPERTY(QString id READ id CONSTANT)
      Q_PROPERTY(QString description READ description CONSTANT)
    public:
      ProjectTemplate(const QString& id = "", const QString& name = "", const QString& image = "", const QString& description = "");

      /**
       * Project template image to display
       */
      QString image() const
      {
        return mImage;
      }

      /**
       * Project template human readable name
       */
      QString name() const
      {
        return mName;
      }

      /**
       * Project file id
       */
      QString id() const
      {
        return mId;
      }

      /**
       * Project description
       */
      QString description() const
      {
        return mDescription;
      }
    private:
      QString mImage;
      QString mName;
      QString mId;
      QString mDescription;
  };

  class ProjectManager : public QObject
  {
      Q_OBJECT
      Q_PROPERTY(QQmlListProperty<Gsage::ProjectTemplate> templates READ templates NOTIFY templatesChanged)
      Q_PROPERTY(QString defaultFolder READ defaultFolder CONSTANT)
    public:
      enum CreationStatus
      {
        UNKNOWN,
        SUCCESS,
        NO_SUCH_TEMPLATE,
        PROJECT_EXISTS,
        RESOURCE_COPYING_FAILED,
        PROJECT_FILE_CREATION_FAILED
      };
      Q_ENUMS(CreationStatus)


      ProjectManager();
      virtual ~ProjectManager();

      /**
       * Create project from template
       * @param templateFile Project creation template file
       * @param projectSettings Project settings
       */
      Q_INVOKABLE CreationStatus create(QString templateFile, QVariantMap projectSettings);

      /**
       * Get currently opened project levels list
       */
      Q_INVOKABLE QStringList getProjectLevels();

      /**
       * Initialize project manager:
       * - load all project templates
       * - load recent projects
       * @param settings DataProxy with all project manager settings
       */
      bool initialize(const DataProxy& settings);

      /**
       * Load project
       * @param projectFilePath Project file path
       * @param configs Project will be read in this DataProxy
       */
      bool loadProject(QString projectFilePath, DataProxy& configs);

      /**
       * Get project manager default working directory
       */
      QString defaultFolder() const;

      /**
       * Get list of loaded project templates
       */
      QQmlListProperty<ProjectTemplate> templates();

    signals:

      /**
       * Fired when template list is modified
       */
      void templatesChanged();

    private:
      bool copyFiles(const QString& src, const QString& dst);

      QList<ProjectTemplate*> mTemplates;

      std::string mTemplatesFolder;

      QString mEditorFolder;
      QString mCurrentProjectFolder;

  };
}
#endif
