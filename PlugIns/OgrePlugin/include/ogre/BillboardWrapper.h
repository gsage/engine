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

#ifndef _BillboardSetWrapper_H_
#define _BillboardSetWrapper_H_

#include "ogre/MovableObjectWrapper.h"

#include <OgreBillboardSet.h>
#include <OgreBillboard.h>

#define BBT_POINT_ID                  "BBT_POINT"
#define BBT_ORIENTED_COMMON_ID        "BBT_ORIENTED_COMMON"
#define BBT_ORIENTED_SELF_ID          "BBT_ORIENTED_SELF"
#define BBT_PERPENDICULAR_SELF_ID     "BBT_PERPENDICULAR_SELF"
#define BBT_PERPENDICULAR_COMMON_ID   "BBT_PERPENDICULAR_COMMON"

namespace Gsage {

    class BillboardWrapper : public Serializable<BillboardWrapper>
    {
      public:
        BillboardWrapper();
        virtual ~BillboardWrapper() {};

        /**
         * Initialize billboard
         * @param dict DataProxy with values
         * @param billboardSet Ogre::BillboardSet to create billboard into
         * @returns false if fails
         */
        bool initialize(const DataProxy& dict, Ogre::BillboardSet* billboardSet);

        /**
         * Set billboard position
         * @param position Position of the billboard object
         */
        void setPosition(const Ogre::Vector3& position);

        /**
         * Get billboard position
         */
        const Ogre::Vector3& getPosition() const;

        /**
         * Set billboard width
         * @param value Billboard width
         */
        void setWidth(const float& value);

        /**
         * Get billboard width
         */
        float getWidth();

        /**
         * Set billboard height
         * @param value Billboard height
         */
        void setHeight(const float& value);

        /**
         * Get billboard height
         */
        float getHeight();

        /**
         * Set billboard colour
         * @param value Billboard colour ARGB
         */
        void setColour(const Ogre::ColourValue& value);

        /**
         * Get billboard colour
         */
        const Ogre::ColourValue& getColour() const;

        /**
         * Set billboard rotation
         * @param value Billboard rotation
         */
        void setRotation(const Ogre::Degree& value);

        /**
         * Get billboard rotation
         */
        Ogre::Degree getRotation();

        /**
         * Set Texcoord Rect
         */
        void setTexcoordRect(const Ogre::FloatRect& rect);

        /**
         * Get Texcoord Rect
         */
        const Ogre::FloatRect& getTexcoordRect() const;

        /**
         * Set Texcoord Index
         */
        void setTexcoordIndex(const unsigned int& index);

        /**
         * Get Texcoord Index
         */
        unsigned int getTexcoordIndex();
      private:
        Ogre::BillboardSet* mBillboardSet;
        Ogre::Billboard* mBillboard;

        float mWidth;
        float mHeight;
    };

    class BillboardSetWrapper : public MovableObjectWrapper<Ogre::BillboardSet>
    {
      public:
        static const std::string TYPE;

        BillboardSetWrapper();
        virtual ~BillboardSetWrapper();

        /**
         * Override default logic values reading
         */
        bool read(const DataProxy& dict);

        /**
         * Set common up vector of the billboard
         * @param vector Vector3
         */
        void setCommonUpVector(const Ogre::Vector3& vector);

        /**
         * Get common up vector of the billboard
         */
        const Ogre::Vector3& getCommonUpVector() const;

        /**
         * Set common direction vector of the billboard
         * @param vector Vector3
         */
        void setCommonDirection(const Ogre::Vector3& vector);

        /**
         * Get direction vector of the billboard
         */
        const Ogre::Vector3& getCommonDirection() const;

        /**
         * Set billboard type
         * @param type Type id
         * @see Ogre::BillboardType for the list of supported ids. It is the same as it is in the enum
         */
        void setBillboardType(const std::string& type);

        /**
         * Get billboard type
         */
        std::string getBillboardType();

        /**
         * Set billboard material
         * @param id Material id
         */
        void setMaterialName(const std::string& id);

        /**
         * Get billboard material
         */
        const std::string& getMaterialName() const;

        /**
         * Add billboards to the billboard set
         */
        void setBillboards(const DataProxy& dict);

        /**
         * Get billboards to the billboard set
         */
        DataProxy getBillboards();

      private:
        /**
         * Convert ogre type enum to string
         * @param type Enum value of the type
         */
        static std::string mapBillboardType(const Ogre::BillboardType type);
        /**
         * Convert string type id to ogre type enum
         * @param type String id
         */
        static Ogre::BillboardType mapBillboardType(const std::string& type);

        Ogre::BillboardSet* mBillboardSet;

        typedef std::vector<BillboardWrapper> Billboards;
        Billboards mBillboards;
    };
}

#endif
