#include <gtest/gtest.h>
#include "EventDispatcher.h"
#include "EventSubscriber.h"

using namespace Gsage;

class TestEvent : public Event
{
  public:
    static const std::string PING;
    static const std::string ECHO;

    TestEvent(const std::string& type, int value) : Event(type), mValue(value) {};
    int getValue() { return mValue; };

    void setHandled(const std::string& name) { mHandlerName = name; };
    const std::string& getLastHandlerName() { return mHandlerName; };
  private:
    int mValue;
    std::string mHandlerName;
};

const std::string TestEvent::PING = "ping";
const std::string TestEvent::ECHO = "echo";

class TestEventDispatcher : public ::testing::Test
{

  public:
    void SetUp()
    {
      mInstance = new EventDispatcher();
    }

    void TearDown()
    {
      if(mInstance)
        delete mInstance;
    }

    EventDispatcher* mInstance;
};

typedef std::vector<TestEvent> TestEvents;

class TestEventHandler : public EventSubscriber<TestEventHandler>
{
  public:
    TestEventHandler(TestEvents& events, const std::string name = "default") : mName(name), receivedEvents(events) {};
    bool onTestEvent(EventDispatcher* sender, const Event& event)
    {
      LOG(INFO) << "Test event listener 1";
      TestEvent e = (TestEvent&)event;
      e.setHandled(mName);
      receivedEvents.push_back(e);
      LOG(INFO) << e.getType();
      return true;
    }

    bool onTestEvent2(EventDispatcher* sender, const Event& event)
    {
      LOG(INFO) << "Test event listener 2";
      TestEvent e = (TestEvent&)event;
      e.setHandled(mName);
      receivedEvents.push_back(e);
      return true;
    }

    bool onTestEvent3(EventDispatcher* sender, const Event& event)
    {
      LOG(INFO) << "Test event listener 3";
      TestEvent e = (TestEvent&)event;
      e.setHandled(mName);
      receivedEvents.push_back(e);
      return true;
    }

    TestEvents& receivedEvents;
    std::string mName;

    size_t getConnectionsCount()
    {
      return mConnections.size();
    }
};

/**
 * Tests simple event dispatcher flow:
 * 1. Create handler.
 * 2. Add TestEvent::PING event listener.
 * 3. Fire event and check, that listener received it.
 * 4. Remove event listener.
 * 5. Fire event again and check, that listener did not receive anything.
 **/
TEST_F(TestEventDispatcher, TestSimpleFlow)
{
  TestEvents receivedEvents;

  TestEventHandler handler(receivedEvents);
  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);

  mInstance->fireEvent(TestEvent(TestEvent::PING, 1));
  ASSERT_EQ(receivedEvents.size(), 1);
  ASSERT_EQ(receivedEvents[0].getType(), TestEvent::PING);
  ASSERT_EQ(receivedEvents[0].getValue(), 1);

  bool success = handler.removeEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);
  ASSERT_TRUE(success);

  mInstance->fireEvent(TestEvent(TestEvent::PING, 100));
  ASSERT_EQ(receivedEvents.size(), 1);

  ASSERT_TRUE(handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent));
  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent2);
  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent3);
  mInstance->fireEvent(TestEvent(TestEvent::PING, 100));
  ASSERT_EQ(receivedEvents.size(), 4);
}

/**
 * Tests listeners prioritizing:
 * 1. Create 2 handlers.
 * 2. Add listeners to TestEvent::PING in both: 1 priority for the first one, 0 priority for the second.
 * 3. Fire event, and check the order of how it was received by handlers, should be handler2->handler1.
 **/
TEST_F(TestEventDispatcher, TestPriority)
{
  TestEvents receivedEvents;

  TestEventHandler handler1(receivedEvents, "handler1");
  TestEventHandler handler2(receivedEvents, "handler2");

  handler1.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent, 1);
  handler2.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent, 0);
  mInstance->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_EQ(receivedEvents.size(), 2);
  ASSERT_EQ(receivedEvents[0].getLastHandlerName(), "handler2");
  ASSERT_EQ(receivedEvents[1].getLastHandlerName(), "handler1");
}

/**
 * Test different handlers flow:
 * 1. Create 2 handlers. Add listener for TestEvent::PING and TestEvent::ECHO.
 * 2. Fire ECHO event and check that it was received only by the subscribed listener.
 * 3. Remove listener for echo event.
 * 5. Fire PING event and check that it was received by the subscribed listener.
 **/
TEST_F(TestEventDispatcher, TestDifferentHandlers)
{
  TestEvents handler1events;
  TestEvents handler2events;

  TestEventHandler handler1(handler1events);
  TestEventHandler handler2(handler2events);

  handler1.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);
  handler2.addEventListener(mInstance, TestEvent::ECHO, &TestEventHandler::onTestEvent);

  mInstance->fireEvent(TestEvent(TestEvent::ECHO, 100));

  handler2.removeEventListener(mInstance, TestEvent::ECHO, &TestEventHandler::onTestEvent);

  ASSERT_EQ(handler1events.size(), 0);
  ASSERT_EQ(handler2events.size(), 1);

  mInstance->fireEvent(TestEvent(TestEvent::PING, 100));

  ASSERT_EQ(handler1events.size(), 1);
}

/**
 * Test GC flow:
 * 1. Create 2 handlers, subscribe them ot the PING event.
 * 2. Delete first one.
 * 3. Second one should still receive the event, no segfault as well.
 **/
TEST_F(TestEventDispatcher, TestHandlerDeleted)
{
  TestEvents receivedEvents;

  TestEventHandler* nullHandler = new TestEventHandler(receivedEvents);
  TestEventHandler* handler = new TestEventHandler(receivedEvents);
  nullHandler->addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent, 0);
  handler->addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent, 1);

  delete nullHandler;

  mInstance->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_EQ(receivedEvents.size(), 1);

  delete handler;

  delete mInstance;
  mInstance = 0;
}

/**
 * Test remove not existing listener
 **/
TEST_F(TestEventDispatcher, TestRemoveNotExisting)
{
  TestEvents receivedEvents;

  TestEventHandler handler(receivedEvents);
  handler.removeEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);
}

/**
 * Test remove listener flows.
 * 1. Create handler, subscribe two it's function to PING event.
 * 2. Remove one of subscriptions, the second one should still receive the event.
 **/
TEST_F(TestEventDispatcher, TestRemoveEventListener)
{
  TestEvents receivedEvents;

  TestEventHandler handler(receivedEvents);
  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);
  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent2);

  mInstance->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_EQ(receivedEvents.size(), 2);

  handler.removeEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);
  ASSERT_EQ(handler.getConnectionsCount(), 2);

  receivedEvents.clear();

  mInstance->fireEvent(TestEvent(TestEvent::PING, 100));
  ASSERT_EQ(receivedEvents.size(), 1);
  handler.removeEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent2);

  ASSERT_EQ(handler.getConnectionsCount(), 0);

  delete mInstance;
  mInstance = 0;
}

/**
 * Test addEventListener/removeEventListener
 * 1. addEventListener
 * 2. removeEventListener
 * 3. addEventListener
 * 4. Should receive event
 */
TEST_F(TestEventDispatcher, TestResubscribe)
{
  TestEvents receivedEvents;

  TestEventHandler handler(receivedEvents);
  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);
  mInstance->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_EQ(receivedEvents.size(), 1);

  handler.removeEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);

  receivedEvents.clear();

  handler.addEventListener(mInstance, TestEvent::PING, &TestEventHandler::onTestEvent);

  mInstance->fireEvent(TestEvent(TestEvent::PING, 100));
  ASSERT_EQ(receivedEvents.size(), 1);

  delete mInstance;
  mInstance = 0;
}

/**
 * Test flow when the EventDispatcher instance is deleted before the handler
 * 1. Create EventDispatcher and handler and subscribe the handler to PING event.
 * 2. Delete EventDispatcher and then delete the handler
 * 3. No segfaults expected.
 */
TEST(TestEventDispatcherDeletion, TestDispatcherDeleted)
{
  TestEvents receivedEvents;
  EventDispatcher* dispatcher = new EventDispatcher();
  TestEventHandler* handler = new TestEventHandler(receivedEvents);
  handler->addEventListener(dispatcher, TestEvent::PING, &TestEventHandler::onTestEvent);
  handler->addEventListener(dispatcher, TestEvent::ECHO, &TestEventHandler::onTestEvent);
  handler->removeEventListener(dispatcher, TestEvent::ECHO, &TestEventHandler::onTestEvent);
  delete dispatcher;

  ASSERT_EQ(handler->getConnectionsCount(), 0);
  ASSERT_EQ(receivedEvents.size(), 0);

  delete handler;
}

/**
 * test subscribing to multiple dispatchers
 * then remove one: no segfault, no errors
 */
TEST(TestEventDispatcherDeletion, TestTwoDispatchers)
{
  TestEvents receivedEvents;
  EventDispatcher* dispatcher1 = new EventDispatcher();
  EventDispatcher* dispatcher2 = new EventDispatcher();
  TestEventHandler* handler = new TestEventHandler(receivedEvents);
  handler->addEventListener(dispatcher1, TestEvent::PING, &TestEventHandler::onTestEvent);
  handler->addEventListener(dispatcher2, TestEvent::PING, &TestEventHandler::onTestEvent);

  dispatcher1->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_EQ(handler->getConnectionsCount(), 4);
  ASSERT_EQ(receivedEvents.size(), 1);

  delete dispatcher1;

  dispatcher2->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_EQ(handler->getConnectionsCount(), 2);
  ASSERT_EQ(receivedEvents.size(), 2);

  delete dispatcher2;

  ASSERT_EQ(handler->getConnectionsCount(), 0);

  delete handler;
}

class TestEventHandlerRemove : public EventSubscriber<TestEventHandlerRemove>
{
  public:
    TestEventHandlerRemove() : gotEvent(false), removed(false) {}

    bool onTestEvent(EventDispatcher* sender, const Event& event)
    {
      gotEvent = true;
      removed = removeEventListener(sender, event.getType(), &TestEventHandlerRemove::onTestEvent);
      return true;
    }

    bool gotEvent;
    bool removed;
};

/**
 * Test removing event listener from a listener
 * No segfaults expected.
 */
TEST(TestRemoveEventListener, TestRemoveInHandler)
{
  EventDispatcher* dispatcher = new EventDispatcher();
  TestEventHandlerRemove* handler = new TestEventHandlerRemove();
  handler->addEventListener(dispatcher, TestEvent::PING, &TestEventHandlerRemove::onTestEvent);

  dispatcher->fireEvent(TestEvent(TestEvent::PING, 1));

  ASSERT_TRUE(handler->gotEvent);
  ASSERT_TRUE(handler->removed);
  handler->gotEvent = false;

  dispatcher->fireEvent(TestEvent(TestEvent::PING, 1));
  ASSERT_FALSE(handler->gotEvent);

  delete handler;
  delete dispatcher;
}
