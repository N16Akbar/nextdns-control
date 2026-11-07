pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import io.github.n16akbar.nextdnscontrol

PlasmoidItem {
    id: root

    Plasmoid.icon: controller.enabled ? "security-high-symbolic" : "security-low-symbolic"
    toolTipMainText: "NextDNS Control"
    toolTipSubText: controller.busy ? "Applying DNS mode" : controller.errorMessage.length > 0 ? controller.errorMessage : !controller.stateKnown ? "DNS mode is unknown" : controller.enabled ? "NextDNS is enabled" : "Native DNS is enabled"

    DnsController {
        id: controller
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
                            text: "DNS Mode"
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                        }

                        PlasmaComponents.Label {
                            Layout.fillWidth: true
                            text: !controller.stateKnown ? "Unknown" : controller.enabled ? "NextDNS" : "Native DNS"
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
                        text: "NextDNS Profile"
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        PlasmaComponents.TextField {
                            id: profileField

                            Layout.fillWidth: true
                            text: controller.profile
                            maximumLength: 6
                            placeholderText: "abcdef"
                            enabled: controller.enabled && !controller.busy
                            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                            Accessible.name: "NextDNS profile ID"
                            onAccepted: controller.setProfile(text)
                        }

                        PlasmaComponents.Button {
                            text: controller.busy ? "Applying" : "Apply"
                            enabled: controller.enabled && !controller.busy && profileField.text.length === 6 && profileField.text.toLowerCase() !== controller.profile
                            Accessible.name: "Apply NextDNS profile"
                            onClicked: controller.setProfile(profileField.text)
                        }
                    }

                    Kirigami.InlineMessage {
                        Layout.fillWidth: true
                        visible: controller.stateKnown && !controller.enabled
                        type: Kirigami.MessageType.Information
                        text: "Enable NextDNS before changing its profile."
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
