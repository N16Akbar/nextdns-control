pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.notification
import io.github.n16akbar.nextdnscontrol

PlasmoidItem {
    id: root
    property bool profileEdited: false

    Plasmoid.icon: controller.enabled ? "security-high-symbolic" : "security-low-symbolic"
    toolTipMainText: "NextDNS Switcher"
    toolTipSubText: controller.busy ? "Changing DNS mode..." : controller.errorMessage.length > 0
                                      ? controller.errorMessage : controller.warningMessage.length
                                        > 0 ? controller.warningMessage : !controller.stateKnown
                                              ? "DNS mode could not be determined" :
                                                controller.enabled ? "Using NextDNS" :
                                                                     "Using default network DNS"

    Notification {
        id: operationNotification
        componentName: "plasma_workspace"
        eventId: "notification"
        iconName: "preferences-system-network-server-dns"
    }
    DnsController {
        id: controller
        onNotificationRequested: function (title, text) {
            operationNotification.title = title;
            operationNotification.text = text;
            operationNotification.sendEvent();
        }

        onProfileSaved: {
            root.profileEdited = false;
        }
    }

    onExpandedChanged: function () {
        if (root.expanded) {
            controller.refresh();
        }
    }

    fullRepresentation: PlasmaExtras.Representation {
        collapseMarginsHint: true
        implicitWidth: Kirigami.Units.gridUnit * 20
        implicitHeight: contentLayout.implicitHeight

        ColumnLayout {
            id: contentLayout
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0

            PlasmaComponents.ItemDelegate {
                Layout.fillWidth: true
                hoverEnabled: false
                topPadding: Kirigami.Units.largeSpacing
                bottomPadding: Kirigami.Units.largeSpacing
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing

                contentItem: RowLayout {
                    spacing: Kirigami.Units.gridUnit

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        PlasmaComponents.Label {
                            Layout.fillWidth: true
                            text: "DNS mode"
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                        }

                        PlasmaComponents.Label {
                            Layout.fillWidth: true
                            text: !controller.stateKnown ? "Unknown" : controller.enabled
                                                           ? "NextDNS" : "Default network DNS"
                            color: Kirigami.Theme.disabledTextColor
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                        }

                        PlasmaComponents.Label {
                            Layout.fillWidth: true
                            text: controller.stateKnown ? controller.dnsSummary : ""
                            visible: text.length > 0
                            color: Kirigami.Theme.disabledTextColor
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.Wrap
                        }
                    }

                    PlasmaComponents.Switch {
                        checkable: false
                        checked: controller.enabled
                        enabled: !controller.busy && controller.stateKnown
                                 && controller.profile.length === 6
                        Accessible.name: "Use NextDNS"
                        onClicked: controller.setEnabled(!controller.enabled)
                    }
                }
            }

            PlasmaComponents.ItemDelegate {
                Layout.fillWidth: true
                hoverEnabled: false

                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing

                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        text: "NextDNS profile"
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        PlasmaComponents.TextField {
                            id: profileField

                            Layout.fillWidth: true
                            text: root.profileEdited ? text : controller.profile
                            maximumLength: 6
                            placeholderText: "abcdef"
                            enabled: !controller.busy
                            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                            validator: RegularExpressionValidator {
                                regularExpression: /^[0-9a-fA-F]{6}$/
                            }
                            onTextEdited: root.profileEdited = true
                            Accessible.name: "NextDNS profile ID"
                            onAccepted: controller.setProfile(text)
                        }

                        PlasmaComponents.Button {
                            text: controller.busy ? "Saving..." : "Save"
                            enabled: !controller.busy && profileField.acceptableInput
                                     && profileField.text.toLowerCase() !== controller.profile
                            Accessible.name: "Save NextDNS profile ID"
                            onClicked: controller.setProfile(profileField.text)
                        }
                    }
                    Kirigami.InlineMessage {
                        Layout.fillWidth: true
                        visible: controller.stateKnown && controller.profile.length === 0
                        type: Kirigami.MessageType.Information
                        text: "Enter and save your NextDNS profile ID, then enable NextDNS."
                    }
                }
            }

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                visible: controller.errorMessage.length > 0
                type: Kirigami.MessageType.Error
                text: controller.errorMessage
            }

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                visible: controller.warningMessage.length > 0
                type: Kirigami.MessageType.Warning
                text: controller.warningMessage
            }

            PlasmaComponents.BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                visible: controller.busy
                running: visible
            }
        }
    }
}
