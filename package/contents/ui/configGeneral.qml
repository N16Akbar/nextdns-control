import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as Controls
import io.github.n16akbar.nextdnscontrol

Kirigami.FormLayout {
    id: root
    Component.onCompleted: controller.refresh()

    property bool saved: false
    property bool edited: false

    DnsController {
        id: controller

        onProfileSaved: {
            root.saved = true;
            profileField.text = controller.profile;
        }

        onStateChanged: {
            if (!root.edited && !controller.busy && controller.profile.length > 0) {
                profileField.text = controller.profile;
            }
        }
    }

    Controls.TextField {
        id: profileField

        Kirigami.FormData.label: "Profile ID"
        maximumLength: 6
        placeholderText: "abcdef"
        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
        validator: RegularExpressionValidator { regularExpression: /^[0-9a-fA-F]{6}$/ }
        Accessible.name: "NextDNS profile ID"

        onTextEdited: {
            root.edited = true;
            root.saved = false;
        }

        onAccepted: controller.setProfile(text)
    }

    Controls.Button {
        Kirigami.FormData.label: ""
        text: controller.busy ? "Applying" : "Apply"
        enabled: !controller.busy && profileField.acceptableInput

        onClicked: controller.setProfile(profileField.text)
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.label: ""
        visible: controller.errorMessage.length > 0
        type: Kirigami.MessageType.Error
        text: controller.errorMessage
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.label: ""
        visible: root.saved && controller.errorMessage.length === 0
        type: Kirigami.MessageType.Positive
        text: "NextDNS profile saved."
    }
}
