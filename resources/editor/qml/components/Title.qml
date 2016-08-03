import QtQuick 2.0

Rectangle {
  property string title: "Undefined"
  id: title
  color: "#5590EE"
  anchors {
    right: parent.right
    left: parent.left
  }
  height: 30
  border.width: 1
  border.color: "#222222"
  Text {
    id: caption
    text: "Levels"
    font.family: "Helvetica"
    font.pointSize: 12
    color: "#FFFFFF"
    anchors {
      verticalCenter: parent.verticalCenter
      horizontalCenter: parent.horizontalCenter
    }
  }
}
