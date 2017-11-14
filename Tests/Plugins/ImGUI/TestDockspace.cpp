#include <gtest/gtest.h>

#include "ImGuiDockspace.h"
#include "Logger.h"
#include <stdio.h>

#include <deque>

using namespace Gsage;

class TestImGuiDockspace : public ::testing::Test
{
  public:
    void SetUp()
    {
      mStyle.splitterThickness = 2.0f;
      mDockspace = ImGuiDockspace(mStyle);
    }

    void TearDown()
    {
      mDockspace.reset();
    }

    /**
     * Validate tab dimensions based on layout and location
     */
    void validateDockScale(DockPtr dock)
    {
      ImVec2 rootPos = mDockspace.getRootDock()->getPosition();
      ImVec2 rootSize = mDockspace.getRootDock()->getSize();

      ImVec2 size = dock->getSize();
      ImVec2 pos = dock->getPosition();

      DockPtr parent = dock->getParent();

      if(!parent) {
        ASSERT_EQ(pos.x, rootPos.x);
        ASSERT_EQ(pos.y, rootPos.y);
        ASSERT_EQ(size.x, rootSize.x);
        ASSERT_EQ(size.y, rootSize.y);
        return;
      }

      DockPtr tmp = dock;
      std::deque<DockPtr> chain;
      while(tmp) {
        chain.push_front(tmp);
        tmp = tmp->getParent();
      }

      ImVec2 expectedPos = rootPos;
      ImVec2 expectedSize = rootSize;
      Dock::Layout layout = Dock::None;

      for(auto d : chain) {

        if(layout != Dock::None) {
          float firstChildSize;
          float secondChildSize;

          Dock::Location location = d->getLocation();
          Dock::Location opposite = (Dock::Location)(location ^ 1);
          DockPtr p = d->getParent();
          DockPtr sibling = p->getChildAt(opposite);

          float ratio = p->getRatio();
          ImVec2 parentSize = p->getSize();

          if(!d->getOpened() || !sibling->getOpened()) {
            continue;
          }

          switch(layout) {
            case Dock::Horizontal:
              firstChildSize = parentSize.x * ratio;
              secondChildSize = parentSize.x - firstChildSize;
              break;
            case Dock::Vertical:
              firstChildSize = parentSize.y * ratio;
              secondChildSize = parentSize.y - firstChildSize;
              break;
            default:
              break;
          }

          switch(location) {
            case Dock::Right:
              expectedPos.x += firstChildSize;
              expectedSize.x -= firstChildSize;
              break;
            case Dock::Left:
              expectedSize.x -= secondChildSize + mStyle.splitterThickness;
              break;
            case Dock::Bottom:
              expectedPos.y += firstChildSize;
              expectedSize.y -= firstChildSize;
              break;
            case Dock::Top:
              expectedSize.y -= secondChildSize + mStyle.splitterThickness;
              break;
            default:
              break;
          }
        }
        layout = d->getLayout();
      }

      ASSERT_EQ(pos.x, expectedPos.x);
      ASSERT_EQ(pos.y, expectedPos.y);
      ASSERT_EQ(size.x, expectedSize.x);
      ASSERT_EQ(size.y, expectedSize.y);
    }

    /**
     * Validate that dockspace has all locations and layouts set properly
     */
    void sanityCheck(DockPtr dock)
    {
      if(dock == nullptr) {
        return;
      }

      if(dock->isContainer()) {
        ASSERT_TRUE(dock->hasBothChildren());
        DockPtr first;
        DockPtr second;
        switch(dock->getLayout()) {
          case Dock::Horizontal:
            first = dock->getChildAt(Dock::Left);
            second = dock->getChildAt(Dock::Right);

            ASSERT_EQ(first->getLocation(), Dock::Left);
            ASSERT_EQ(second->getLocation(), Dock::Right);
            break;
          case Dock::Vertical:
            first = dock->getChildAt(Dock::Top);
            second = dock->getChildAt(Dock::Bottom);

            ASSERT_EQ(first->getLocation(), Dock::Top);
            ASSERT_EQ(second->getLocation(), Dock::Bottom);
            break;
          default:
            break;
        }
        ASSERT_NE(first, second);
        ASSERT_EQ(first->getParent(), dock);
        ASSERT_EQ(second->getParent(), dock);
        sanityCheck(first);
        sanityCheck(second);
      } else if(dock->getLocation() == Dock::Tab) {
        ASSERT_NE(dock->getPreviousTab(), nullptr);
        ASSERT_EQ(dock->getPreviousTab()->getNextTab(), dock);
        sanityCheck(dock->getNextTab());
      } else if(dock->getNextTab()) {
        sanityCheck(dock->getNextTab());
      }
    }

    ImGuiDockspace mDockspace;
    ImGuiDockspaceStyle mStyle;
};

TEST(TestImGuiDockspaceSimple, TestLocation)
{
  // verify that enum values are represented by inversed integers
  ASSERT_EQ(Dock::Left, Dock::Right ^ 1);
  ASSERT_EQ(Dock::Right, Dock::Left ^ 1);
  ASSERT_EQ(Dock::Top, Dock::Bottom ^ 1);
  ASSERT_EQ(Dock::Bottom, Dock::Top ^ 1);
}

TEST_F(TestImGuiDockspace, TestDock)
{
  DockPtr dock = mDockspace.createDock("stuff");

  // not existing dock
  ASSERT_FALSE(mDockspace.dockTo("none", dock, Dock::Left));

  // attach to root
  ASSERT_TRUE(mDockspace.dockTo(nullptr, dock, Dock::Left));

  sanityCheck(mDockspace.getRootDock());

  dock = mDockspace.createDock("stuff2");
  // add as tab
  ASSERT_TRUE(mDockspace.dockTo(nullptr, dock, Dock::Tab));

  sanityCheck(mDockspace.getRootDock());

  DockPtr retrieved = mDockspace.getDock("stuff2");

  ASSERT_NE(retrieved, nullptr);
  ASSERT_EQ(retrieved, dock);

  // can't dock already docked dock
  ASSERT_FALSE(mDockspace.dockTo(nullptr, dock, Dock::Left));

  ASSERT_TRUE(mDockspace.undock(dock));

  sanityCheck(mDockspace.getRootDock());

  // now it should work
  ASSERT_TRUE(mDockspace.dockTo(nullptr, dock, Dock::Left));

  sanityCheck(mDockspace.getRootDock());

  dock = mDockspace.createDock("stuff3");

  // dock not to container
  ASSERT_TRUE(mDockspace.dockTo("stuff2", dock, Dock::Bottom));

  mDockspace.setDimensions(ImVec2(20, 20), ImVec2(400, 400));

  // now validate containers
  sanityCheck(mDockspace.getRootDock());

  ASSERT_TRUE(mDockspace.undock(dock));

  sanityCheck(mDockspace.getRootDock());

  ASSERT_TRUE(mDockspace.undock("stuff2"));

  sanityCheck(mDockspace.getRootDock());

  ASSERT_TRUE(mDockspace.undock("stuff"));

  sanityCheck(mDockspace.getRootDock());
}

TEST_F(TestImGuiDockspace, TestScales)
{
  DockPtr left = mDockspace.createDock("left");
  DockPtr rightTop = mDockspace.createDock("right-top");
  DockPtr rightBottom = mDockspace.createDock("right-bottom");

  mDockspace.dockTo(nullptr, rightTop, Dock::Right);
  mDockspace.dockTo(rightTop->getLabel(), left, Dock::Left);
  mDockspace.dockTo(rightTop->getLabel(), rightBottom, Dock::Bottom);

  // verify position
  ASSERT_EQ(left->getLocation(), Dock::Left);
  ASSERT_EQ(rightTop->getLocation(), Dock::Top);
  ASSERT_EQ(rightBottom->getLocation(), Dock::Bottom);
  ASSERT_EQ(rightBottom->getParent()->getLocation(), Dock::Right);

  ImVec2 pos = ImVec2(20, 20);
  ImVec2 size = ImVec2(400, 400);

  // verify scales
  mDockspace.setDimensions(pos, size);

  validateDockScale(left);
  validateDockScale(rightBottom);
  validateDockScale(rightTop);

  // now update some ratio
  left->getParent()->setRatio(0.2);

  validateDockScale(left);
  validateDockScale(rightBottom);
  validateDockScale(rightTop);

  // verify visibility
  left->setOpened(false);

  validateDockScale(rightBottom);
  validateDockScale(rightTop);

  left->setOpened(true);
  validateDockScale(left);
  validateDockScale(rightBottom);
  validateDockScale(rightTop);

  rightBottom->setOpened(false);
  rightTop->setOpened(false);
  validateDockScale(left);

  rightTop->setOpened(true);
  validateDockScale(left);
  validateDockScale(rightTop);

  rightBottom->setOpened(true);
  validateDockScale(left);
  validateDockScale(rightBottom);
  validateDockScale(rightTop);

  // verify undock
  ASSERT_TRUE(mDockspace.undock(left));
  validateDockScale(rightBottom);
  validateDockScale(rightTop);

  ASSERT_TRUE(mDockspace.undock(rightTop));
  validateDockScale(rightBottom);
}
