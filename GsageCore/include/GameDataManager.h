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

#ifndef _GameDataManager_H_
#define _GameDataManager_H_

#include <istream>
#include <ostream>
#include <map>
#include <thread>
#include "GsageDefinitions.h"
#include "DataProxy.h"

namespace Gsage
{
  class Engine;
  class Entity;
  class EngineSystem;

  /**
   * Class responsible for level loading
   */
  class GameDataManager
  {
    public:
      GameDataManager(Engine* engine);
      virtual ~GameDataManager();

      /**
       * Set game data manager configuration
       * @param config Config
       */
      void configure(const DataProxy& config);

      /**
       * Create initial save file
       */
      bool initGame(const std::string& templateFile);
      /**
       * Load save
       */
      bool loadSave(const std::string& saveFile);
      /**
       * Dump engine state to save
       */
      bool dumpSave(const std::string& saveFile);
      /**
       * Remove entity by id
       *
       * @param id Entity id
       */
      bool removeEntity(const std::string& id);

      /**
       * Get raw entity data by id
       * @param id Entity id
       */
      DataProxy getEntityData(const std::string& id);

      /**
       * Create entity from string
       *
       * @param json Json string to create entity from
       */
      Entity* createEntity(const std::string& json);
      /**
       * Create entity from template
       *
       * @param name Template file name
       * @param params placeholders will be replaced by the parameters
       * @return created entity pointer
       */
      Entity* createEntity(const std::string& name, const DataProxy& params);
      /**
       * Create entity from any wrapped data
       *
       * @param data DataProxy
       */
      Entity* createEntity(DataProxy data);
      /**
       * Adds character
       *
       * @param name Entity id
       * @param params Override default parameters of the character
       */
      Entity* addCharacter(const std::string& name, DataProxy* params = 0);
      /**
       * Get file extension
       */
      inline const std::string& getFileExtension() const { return mFileExtension; }
      /**
       * Get characters folder path
       */
      inline const std::string& getCharactersFolder() const { return mCharactersFolder; }
      /**
       * Get locations folder
       */
      inline const std::string& getLevelsFolder() const { return mLevelsFolder; }
      /**
       * Get save data folder
       */
      inline const std::string& getSavesFolder() const { return mSavesFolder; }

      /**
       * Load area, without loading characters
       */
      bool loadArea(const std::string& area);
    private:
      static const std::string CONFIG_SECTION;

      DataProxy* mCurrentSaveFile;

      Engine* mEngine;

      // settings
      std::string mFileExtension;
      std::string mCharactersFolder;
      std::string mLevelsFolder;
      std::string mSavesFolder;

      bool loadCharacters(const std::string& area);

      DataProxy& getSaveFile();
      void resetSaveFile();

      const std::string readFile(const std::string& path);
  };
}

#endif
