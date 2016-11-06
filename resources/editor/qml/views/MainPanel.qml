import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
  id: menu
  width: 100
  color: "#555555"
  border.width: 1
  border.color: "#222222"

  signal selected(string selection);

  ListView {
    id: buttons
    spacing: 5
    anchors {
      fill: parent
      topMargin: 5
      bottomMargin: 5
      leftMargin: 5
      rightMargin: 5
    }
    model:
    ListModel {
      ListElement {
        menuName: "Home"
        image: "../../images/home.png"
        selection: "home"
      }

      ListElement {
        menuName: "Game"
        image: "../../images/game.png"
        selection: "game"
      }

      ListElement {
        menuName: "Script"
        image: "../../images/script.png"
        selection: "script"
      }
    }
    delegate: Column {
      Rectangle {
        Rectangle {
          property color normalColor: "#22FFFFFF"
          property color overColor: "#33FFFFFF"
          id: bg
          anchors {
            fill: parent
          }
        }
        state: mouseArea.containsMouse ? "over" : "normal"

        width: buttons.width
        height: 70

        MouseArea {
          id: mouseArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: "PointingHandCursor"
          onClicked: {
            menu.selected(model.selection)
          }
        }

        states: [
          State  {
            name: "over"
            PropertyChanges  { target: bg; color: bg.overColor}
          },
          State  {
            name: "normal"
            PropertyChanges  { target: bg; color: bg.normalColor}
          }
        ]

        transitions: [
          Transition  {
            from: "over"
            to: "normal"
            ColorAnimation  { target: bg; duration: 200}
          },
          Transition  {
            from: "normal"
            to: "over"
            ColorAnimation  { target: bg; duration: 200}
          }
        ]

        color: "#00000000"

        Rectangle {
          width: childrenRect.width
          height: childrenRect.height

          anchors.verticalCenter: parent.verticalCenter
          anchors.horizontalCenter: parent.horizontalCenter

          color: "#00000000"
          id: element

          Column {
            spacing: 5

            Item {
              anchors {
                horizontalCenter: parent.horizontalCenter
              }

              width: 30
              height: 30

              Image {
                anchors {
                  fill: parent
                }
                id: icon
                source: image
              }

              ColorOverlay {
                anchors.fill: icon
                source: icon
                color: "#FFFFFFFF"
              }
            }
            Text {
              id: caption
              text: menuName
              font.family: "Helvetica"
              color: "#FFFFFF"
              anchors {
                horizontalCenter: parent.horizontalCenter
              }
            }
          }
        }
      }
    }

    focus: true
  }
}
