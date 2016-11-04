import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

import Editor.Models 1.0
import Editor.Utils 1.0

import "../components"

Rectangle
{
  color: "white"
  anchors.fill: parent
  anchors
  {
    leftMargin: -1
  }

  id: wizard
  property color primaryTextColor: "#444444"
  property color secondaryTextColor: "#666666"
  property color highlightedTextColor: "#FFFFFF"
  property string state: "initial"
  property string projectType: ""
  property string projectDescription: ""
  property string projectPath: projectManager.defaultFolder
  property string projectName: "default"

  signal finished(string projectPath)

  border.width: 1
  border.color: "#222222"

  Rectangle
  {
    anchors.fill: parent
    anchors
    {
      topMargin: 15
      leftMargin: 15
      rightMargin: 15
      bottomMargin: 15
    }

    Text
    {
      id: create
      text: "Create new project"
      font.family: "Helvetica"
      font.pointSize: 21
      color: wizard.primaryTextColor
    }

    Component {
      id: listView
      GridView {
        id: templates
        anchors.fill: parent
        anchors
        {
          topMargin: 5
          leftMargin: 5
          rightMargin: 5
          bottomMargin: 5
        }

        cellWidth: 210
        cellHeight: 160

        model: projectManager.templates

        delegate: Column {
          Rectangle {
            id: element
            state: mouseArea.containsMouse ? "over" : "normal"
            width: 200
            height: 150
            Image {
              id: icon
              anchors.fill: parent
              source: image
            }
            Text { 
              id: title
              text: name
              font.family: "Helvetica"
              font.pointSize: 19
              color: wizard.secondaryTextColor
              y: 5
              anchors {
                horizontalCenter: parent.horizontalCenter
              }
            }

            MouseArea {
              id: mouseArea
              anchors.fill: parent
              hoverEnabled: true
              onClicked: {
                wizard.state = "create"
                wizard.projectType = id
                wizard.projectDescription = description
              }
            }

            states: [
              State  {
                name: "over"
                PropertyChanges  { target: element; color: "#5590EE"}
                PropertyChanges  { target: title; color: wizard.highlightedTextColor}
              },
              State  {
                name: "normal"
                PropertyChanges  { target: element; color: "#DFDFDF"}
                PropertyChanges  { target: title; color: wizard.secondaryTextColor}
              }
            ]

            transitions: [
              Transition  {
                from: "over"
                to: "normal"
                ColorAnimation  { target: element; duration: 100}
              },
              Transition  {
                from: "normal"
                to: "over"
                ColorAnimation  { target: element; duration: 100}
              }
            ]
          }
        }
      }
    }

    Component {
      id: createProject

      Rectangle {
        id: creationDialog
        function openFileDialog() {
          fileDialog.visible = true;
        }
        FileDialog {
          id: fileDialog
          selectFolder: true
          title: "Please select project folder"
          onAccepted: {
            var prefix = "file://";
            var projectPath = folder.toString();
            if (projectPath.indexOf(prefix) != -1) {
              projectPath = projectPath.slice(prefix.length, Infinity)
            } else if (projectPath == "file:") {
              return;
            }

            wizard.projectPath = projectPath;
          }
        }
        Grid {
          columns: 1
          spacing: 10
          anchors.fill: parent

          Text {
            id: project
            text: "Configure new " + wizard.projectType + " project"
            font.family: "Helvetica"
          }

          Row {
            width: parent.width
            height: 25
            spacing: 5

            Text {
              id: projectNameLabel
              font.family: "Helvetica"
              text: "Project name:"
              verticalAlignment: Text.AlignVCenter
              height: parent.height
            }

            TextField {
              id: projectName
              text: "default"
              font.family: "Helvetica"
              width: parent.width - projectNameLabel.width - parent.spacing
              height: parent.height
            }
          }

          Grid {
            id: inputs
            columns: 4
            rows: 2
            spacing: 5
            width: parent.width

            DefaultButton {
              id: back
              onClicked: {
                wizard.state = "initial"
              }
              text: "Back"
            }
            TextField {
              readOnly: true
              placeholderText: "Project folder"
              width: parent.width - (back.width + create.width + browse.width + inputs.spacing * 3)
              text: wizard.projectPath
              font.family: "Helvetica"
            }

            DefaultButton {
              id: browse
              onClicked: {
                creationDialog.openFileDialog()
              }
              text: "Browse"
            }
            DefaultButton {
              id: create
              enabled: wizard.projectPath.toString() != "" && projectName.text != ""
              onClicked: {
                var creationStatus = projectManager.create(wizard.projectType, {
                  projectPath: wizard.projectPath,
                  projectName: projectName.text,
                  overwrite: false
                });

                if(creationStatus == ProjectManager.PROJECT_EXISTS) {
                  overwriteDialog.visible = true;
                  return;
                }

                if(creationStatus == ProjectManager.SUCCESS) {
                  wizard.finished(wizard.projectPath + "/" + projectName.text);
                }

              }
              text: "Create"
            }
          }
          Text {
            id: projectDescription
            text: wizard.projectDescription
            font.family: "Helvetica"
            color: "#666"
          }

          Row {
            id: overwriteDialog
            width: parent.width
            height: 35
            spacing: 5
            visible: false

            Rectangle {
              width: parent.width
              height: parent.height
              color: "#FFCCCC"
              Text {
                id: alertMessage
                text: "Specified folder exists and is not empty, continue?"
                font.family: "Helvetica"
                anchors.topMargin: 10
                anchors.leftMargin: 10
                anchors.fill: parent
              }

              Row {
                layoutDirection: Qt.RightToLeft
                spacing: 5
                width: parent.width
                anchors.topMargin: 5
                anchors.rightMargin: 10
                anchors.fill: parent

                DefaultButton {
                  id: yes
                  enabled: wizard.projectPath.toString() != "" && projectName.text != ""
                  onClicked: {
                    var creationStatus = projectManager.create(wizard.projectType, {
                      projectPath: wizard.projectPath,
                      projectName: projectName.text,
                      overwrite: true
                    });
                    overwriteDialog.visible = false;
                    wizard.finished(wizard.projectPath + "/" + projectName.text);
                  }
                  text: "Yes"
                }

                DefaultButton {
                  id: no
                  enabled: wizard.projectPath.toString() != "" && projectName.text != ""
                  onClicked: {
                    overwriteDialog.visible = false;
                  }
                  text: "Cancel"
                }
              }
            }

          }
        }
      }
    }

    Loader {
      anchors.fill: parent
      anchors.topMargin: 50
      sourceComponent: wizard.state == "create" ? createProject : listView
    }
  }
}
