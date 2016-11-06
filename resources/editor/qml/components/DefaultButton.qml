import QtQuick.Controls 1.1
import QtQuick 2.0
import QtQuick.Controls.Styles 1.1

Button {
  style: ButtonStyle {
    label: Text {
      verticalAlignment: Text.AlignVCenter
      horizontalAlignment: Text.AlignHCenter
      font.family: "Helvetica"
      text: control.text
    }
  }
}
