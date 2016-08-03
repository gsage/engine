import QtQuick 2.0
import "../components"

Rectangle {
  id: menu
  color: "#555555"
  border.color: "#222222"

  property var model: []

  signal selected(string selection);

  Title {
    id: title
    title: "Levels"
  }

  ListView {
    id: levelsList
    spacing: 5
    anchors {
      fill: parent
      topMargin: title.height + 5
      bottomMargin: 5
      leftMargin: 5
      rightMargin: 5
    }
    model: levels
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

        width: levelsList.width
        height: 30

        MouseArea {
          id: mouseArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: "PointingHandCursor"
          onClicked: {
            menu.selected(modelData)
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
            /*Image {
              id: icon
              anchors {
                horizontalCenter: parent.verticalCenter
              }
              source: 
            }*/
            Text {
              id: caption
              text: modelData
              font.family: "Helvetica"
              font.pointSize: 12
              color: "#FFFFFF"
            }
          }
        }
      }
    }

    focus: true
  }
}
