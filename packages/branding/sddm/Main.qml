import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

Rectangle {
    id: root
    width: 1920
    height: 1080
    color: "#1e1e2e"

    // Background image with overlay
    Image {
        id: bg
        anchors.fill: parent
        source: "background.png"
        fillMode: Image.Stretch
        opacity: 0.6
    }

    // Gradient overlay
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1e1e2e" }
            GradientStop { position: 0.5; color: "#181825" }
            GradientStop { position: 1.0; color: "#11111b" }
        }
    }

    // Glow effects
    Item {
        anchors.fill: parent
        
        // Top-left glow
        Rectangle {
            x: -400; y: -300
            width: 800; height: 600
            color: "#89b4fa"
            opacity: 0.08
            radius: 400
            layer.enabled: true
            layer.effect: FastBlur { radius: 100 }
        }
        
        // Center glow
        Rectangle {
            x: 560; y: 240
            width: 800; height: 600
            color: "#89b4fa"
            opacity: 0.05
            radius: 400
            layer.enabled: true
            layer.effect: FastBlur { radius: 150 }
        }
        
        // Bottom-right glow
        Rectangle {
            x: 1520; y: 780
            width: 600; height: 500
            color: "#89b4fa"
            opacity: 0.08
            radius: 300
            layer.enabled: true
            layer.effect: FastBlur { radius: 100 }
        }
    }

    // Main content
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 24
        Layout.margins: 48

        // Logo and title
        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            Text {
                text: "SHOBIKAOS"
                font.family: "JetBrains Mono"
                font.pixelSize: 48
                font.weight: Font.Bold
                color: "#89b4fa"
                letterSpacing: 8
            }

            Text {
                text: "Arch-based Linux Distribution"
                font.family: "Noto Sans"
                font.pixelSize: 18
                font.weight: Font.Light
                color: "#a6adc8"
                letterSpacing: 2
            }

            // Decorative line
            Rectangle {
                width: 200
                height: 3
                color: "#89b4fa"
                radius: 1.5
            }
        }

        // User list
        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            // User avatar and name
            Column {
                spacing: 12
                Layout.alignment: Qt.AlignHCenter

                // Avatar
                Image {
                    id: avatar
                    width: 96
                    height: 96
                    source: userModel.avatar
                    fillMode: Image.PreserveAspectCrop
                    clip: true
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Qt.resolvedUrl("avatar-mask.svg")
                    }
                }

                Text {
                    text: userModel.name
                    font.family: "Noto Sans"
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                    color: "#cdd6f4"
                }

                // Password field
                TextField {
                    id: passwordField
                    width: 300
                    height: 48
                    placeholderText: "Password"
                    echoMode: TextField.Password
                    font.family: "Noto Sans"
                    font.pixelSize: 16
                    color: "#cdd6f4"
                    background: Rectangle {
                        color: "#1e1e2e"
                        border.color: passwordField.activeFocus ? "#89b4fa" : "#313244"
                        border.width: 2
                        radius: 8
                    }
                    selectByMouse: true
                    Keys.onReturnPressed: {
                        if (text.length > 0) {
                            login(text)
                        }
                    }
                }

                // Login button
                Button {
                    id: loginBtn
                    text: "Sign In"
                    width: 300
                    height: 48
                    enabled: passwordField.text.length > 0
                    font.family: "Noto Sans"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    background: Rectangle {
                        color: loginBtn.enabled ? "#89b4fa" : "#313244"
                        border.color: "#89b4fa"
                        border.width: loginBtn.enabled ? 0 : 1
                        radius: 8
                    }
                    contentItem: Text {
                        text: loginBtn.text
                        color: loginBtn.enabled ? "#1e1e2e" : "#6c7086"
                        font: loginBtn.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: login(passwordField.text)
                }
            }
        }

        // Bottom bar with clock, session, power
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 24

            // Clock
            Column {
                spacing: 4
                Layout.alignment: Qt.AlignHCenter
                
                Text {
                    id: clockText
                    text: Qt.formatTime(new Date(), "HH:mm")
                    font.family: "JetBrains Mono"
                    font.pixelSize: 36
                    font.weight: Font.Medium
                    color: "#cdd6f4"
                }
                
                Text {
                    text: Qt.formatDate(new Date(), "dddd, MMMM d, yyyy")
                    font.family: "Noto Sans"
                    font.pixelSize: 14
                    color: "#a6adc8"
                }
            }

            // Vertical separator
            Rectangle {
                width: 1
                height: 60
                color: "#313244"
            }

            // Session selector
            Column {
                spacing: 8
                Layout.alignment: Qt.AlignHCenter

                Text {
                    text: "Session"
                    font.family: "Noto Sans"
                    font.pixelSize: 12
                    color: "#6c7086"
                }

                ComboBox {
                    id: sessionCombo
                    width: 180
                    height: 40
                    model: sessionModel
                    currentIndex: sessionModel.currentIndex
                    font.family: "Noto Sans"
                    font.pixelSize: 14
                    color: "#cdd6f4"
                    background: Rectangle {
                        color: "#1e1e2e"
                        border.color: "#313244"
                        border.width: 1
                        radius: 8
                    }
                    indicator: Canvas {
                        width: 24
                        height: 24
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.fillStyle = "#89b4fa"
                            ctx.beginPath()
                            ctx.moveTo(8, 8)
                            ctx.lineTo(16, 16)
                            ctx.lineTo(8, 16)
                            ctx.fill()
                        }
                    }
                    delegate: ItemDelegate {
                        width: 180
                        height: 40
                        text: modelData
                        font.family: "Noto Sans"
                        font.pixelSize: 14
                        color: "#cdd6f4"
                        highlighted: Rectangle {
                            color: "#89b4fa"
                            radius: 6
                        }
                    }
                    onCurrentIndexChanged: sessionModel.currentIndex = currentIndex
                }
            }

            // Vertical separator
            Rectangle {
                width: 1
                height: 60
                color: "#313244"
            }

            // Power buttons
            Column {
                spacing: 8
                Layout.alignment: Qt.AlignHCenter

                Text {
                    text: "Power"
                    font.family: "Noto Sans"
                    font.pixelSize: 12
                    color: "#6c7086"
                }

                Row {
                    spacing: 12

                    // Restart
                    Button {
                        text: "⏻"
                        width: 48
                        height: 48
                        font.pixelSize: 20
                        background: Rectangle {
                            color: "#313244"
                            border.color: "#45475a"
                            border.width: 1
                            radius: 8
                        }
                        onClicked: shutdown(1)
                    }

                    // Shutdown
                    Button {
                        text: "⏼"
                        width: 48
                        height: 48
                        font.pixelSize: 20
                        background: Rectangle {
                            color: "#313244"
                            border.color: "#f38ba8"
                            border.width: 1
                            radius: 8
                        }
                        onClicked: shutdown(0)
                    }
                }
            }
        }
    }

    // Clock timer
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: clockText.text = Qt.formatTime(new Date(), "HH:mm")
    }

    // Functions
    function login(password) {
        // Authentication handled by SDDM backend
        authenticate(userModel.name, password)
    }

    function shutdown(reboot) {
        if (reboot) {
            rebootSystem()
        } else {
            powerOff()
        }
    }
}