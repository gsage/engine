import QtQuick 2.0
import Ogre.Viewer 1.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.1
import "views"

Rectangle {
  id: ogre
  color: "black"
  property var levels:[]

  function setEngineRunning(engineRunning) {
      wizard.visible = !engineRunning;
      renderWindow.visible = engineRunning;
      editor.setEnginePaused(!engineRunning);

      // reload levels
      if(engineRunning) {
        levels = projectManager.getProjectLevels();
      }

      levelsBrowser.width = engineRunning ? 200 : 0;
      levelsBrowser.visible = engineRunning;
  }

  Component.onCompleted: {
    editor.systemsInitialized.connect(function() {
      setEngineRunning(true);
    })
  }

  Rectangle {
    id: topPanel
    height: 0
    anchors{
      left: parent.left
      right: parent.right
    }
    color: "#5590EE"
  }

  MainPanel {
    anchors{
      top: topPanel.bottom
      bottom: parent.bottom
    }
    id: mainPanel
    onSelected: {
      switch(selection) {
        case "home":
          setEngineRunning(false)
          wizard.state = "initial";
          break;
      }
    }
  }

  Rectangle
  {
    id: mainWindow
      focus: true
    anchors
    {
      top: topPanel.bottom
      bottom: parent.bottom
      left: mainPanel.right
      right: levelsBrowser.left
    }

    OgreItem
    {
      id: renderWindow
      objectName: "renderWindow"
      visible: false
      anchors.fill: parent
      Behavior on opacity { NumberAnimation { } }
      Behavior on width { NumberAnimation { } }
      Behavior on height { NumberAnimation { } }
    }
    Text {
    color: "white"

        text: activeFocus ? "I have active focus!" : "I do not have active focus"
      }

    Wizard
    {
      id: wizard
      anchors.fill: parent
      onFinished: {
        editor.loadProject(projectPath + "/project.json");
      }
    }
  }

  LevelsBrowser {
    id: levelsBrowser
    model: levels
    width: 0
    anchors
    {
      top: parent.top
      bottom: parent.bottom
      right: parent.right
    }

    onSelected: {
      editor.load(selection)
    }
  }
}
