' This code is written by JelleWho
' 

Imports System
Imports System.Threading
Imports System.IO.Ports
Imports System.ComponentModel
Public Class Serial_Data

    Dim EmergencyState As String = "!"
    Dim EndCode = "."                               'the dot is here to show the arduino that the integer has ended in the command
    Dim Fistcommand As Boolean = True               'If this is the first command (and we need to set the bars
    Dim CountError As Integer = 0
    Dim CountCommands As Integer = 0

    Dim RecievedText As String
    Dim StartBit As String = "["
    Dim StopBit As String = "]"

    'Put above this line your own strings

    '==================================================
    '==================================================
    '==========          Core V7.1           ==========
    '==================================================
    '==================================================
    Dim MSGBoxName As String
    Dim MSG As String
    Dim myPort As Array
    Dim ArduinoAnswer As String
    Dim ButtonSelected As String
    Dim PrefUSB As String

    Dim ErrorCounter As Int32
    Dim ErrorCounter2 As Int32

    Delegate Sub SetTextCallback(ByVal [text] As String) 'Added to prevent threading errors during receiveing of data
    'Action - when closing application
    Private Sub Form2_FormClosing(ByVal sender As Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles Me.FormClosing
        If ButtonConnectDisconnect.Text = "Disconnect" Then
            Threading.Thread.Sleep(100) ' 1000 milliseconds = 1 second
            Disconnect()               ' disconnect arduino, Tried to fix the Frees here
            Threading.Thread.Sleep(100) ' 1000 milliseconds = 1 second
        End If
    End Sub
    'Action - (when) Starting up
    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        ReloadUSB()
        MSGBoxName = "JelleWho"
        ButtonConnectDisconnect.Select()
        RunOnStartup()
    End Sub
    'Button - Reload USB
    Private Sub ComboBoxPoort_USBIndexChanged(sender As Object, e As EventArgs) Handles ComboBoxUSB.Click
        ReloadUSB()
    End Sub
    'Code   - Reload USB
    Sub ReloadUSB()
        PrefUSB = ComboBoxUSB.SelectedItem
        ComboBoxUSB.Items.Clear()
        On Error GoTo ErrHand
        myPort = IO.Ports.SerialPort.GetPortNames()
        ComboBoxUSB.Items.AddRange(myPort)
        ComboBoxUSB.SelectionStart.ToString()
        ComboBoxUSB.SelectedItem = PrefUSB
        If ComboBoxUSB.SelectedItem = "" Then   'Try autoselect an port
            ComboBoxUSB.SelectedItem = "COM10"
            ComboBoxUSB.SelectedItem = "COM9"
            ComboBoxUSB.SelectedItem = "COM8"
            ComboBoxUSB.SelectedItem = "COM7"
            ComboBoxUSB.SelectedItem = "COM6"
            ComboBoxUSB.SelectedItem = "COM5"
            ComboBoxUSB.SelectedItem = "COM4"
            ComboBoxUSB.SelectedItem = "COM3"
            ComboBoxUSB.SelectedItem = "COM2"
            ComboBoxUSB.SelectedItem = "COM1"
            ComboBoxUSB.SelectedItem = "COM0"
        End If
ErrHand:
    End Sub
    'Button - Send
    Private Sub ButtonSend_Click(sender As Object, e As EventArgs) Handles ButtonSend.Click
        SerialSend(TextBoxInput.Text)
    End Sub
    'Button - Enter Pressed (Send)
    Private Sub TextBoxInput_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles TextBoxInput.KeyDown
        If e.KeyCode = Keys.Enter Then
            SerialSend(TextBoxInput.Text)
        End If
    End Sub
    'Code   - Serial send
    Sub SerialSend(Text As String)
        TextBoxInput.Text = ""
        On Error GoTo ErrHand
        SerialPort1.Write(Text)
        TextBoxInput.Text = Text
        RichTextBoxInput.Text &= Text + Chr(13)
        Exit Sub
ErrHand:
        MsgBox("You couldn't resist to remove the cable didn't you?", , MSGBoxName)
        Disconnect()
    End Sub
    'Button - Connect and Disconnect
    Private Sub ButtonButtonConnectDisconnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonConnectDisconnect.Click
        If ButtonConnectDisconnect.Text = "Connect" Then
            'Connect
            Connect()
        Else
            'Disconnect
            ButtonConnectDisconnect.Text = "Connect"
            ButtonConnectDisconnect.ForeColor = Color.Green
            Disconnect()
        End If
    End Sub
    'Code   - Connect
    Sub Connect()
        If ComboBoxUSB.Text <> "" Then
            On Error GoTo ErrHand
            ''SerialPort1.Close()
            SerialPort1.PortName = ComboBoxUSB.Text
            SerialPort1.BaudRate = 9600

            'Added the next lines
            SerialPort1.DataBits = 8
            SerialPort1.Parity = Parity.None
            SerialPort1.StopBits = StopBits.One
            SerialPort1.Handshake = Handshake.None
            SerialPort1.Encoding = System.Text.Encoding.Default
            SerialPort1.ReadTimeout = 1

            SerialPort1.Open()
            ComboBoxUSB.Enabled = False
            ButtonSend.Enabled = True
            TextBoxInput.Enabled = True
            ButtonConnectDisconnect.Text = "Disconnect"
            ButtonConnectDisconnect.ForeColor = Color.Red
            TextBoxInput.Select()
            RunOnConnect()
        Else
            MsgBox("I can not connect to nothing! what where you thinking..." + Chr(13) + "Please give me a COM poort to connect to, before letting me try to connect to it", , MSGBoxName)
            ReloadUSB()
        End If
        Exit Sub
ErrHand:
        MsgBox("The COM Port your trying to use, does not seems to work anymore" + Chr(13) + "Did you remove the cable again?", , MSGBoxName)
        ReloadUSB()
        ButtonConnectDisconnect.Select()
    End Sub
    'Code   - Disconnect
    Sub Disconnect()
        RunOnDisconnect()
        ComboBoxUSB.Enabled = True
        ButtonSend.Enabled = False
        TextBoxInput.Enabled = False
        ButtonConnectDisconnect.Text = "Connect"
        ButtonConnectDisconnect.ForeColor = Color.Green
        ButtonConnectDisconnect.Select()
        On Error GoTo ErrHand
        SerialPort1.Close()
        Exit Sub
ErrHand:
        ReloadUSB()
    End Sub
    'Action - Recieved serial data
    Private Sub SerialPort1_DataReceived(ByVal sender As System.Object, ByVal e As System.IO.Ports.SerialDataReceivedEventArgs) Handles SerialPort1.DataReceived
        On Error GoTo ErrHand
        System.Threading.Thread.Sleep(10)    'Minimal 4
        ReceivedText(SerialPort1.ReadExisting())
        Exit Sub
ErrHand:
    End Sub
    'Code   - Recieved serial data
    Private Sub ReceivedText(ByVal [text] As String) 'input from ReadExisting
        If Me.RichTextBoxOutput.InvokeRequired Then
            Dim x As New SetTextCallback(AddressOf ReceivedText)
            Me.Invoke(x, New Object() {(text)})
        Else
            RecievedText = RecievedText + [text]
            Dim A = 10
            'CMD1 = StartBit
            'CMD2 = -1/0/1 = int engine 
            'CMD3 = ! or ~ = emergency state
            'CMD4 = int    = int steering
            'CMD5 = StopBit
            Do While InStr(1, RecievedText, StartBit) > 0 And InStr(2, RecievedText, StopBit) > 0 And A > 0
                A = A - 1
                Dim StartBitPos = InStr(1, RecievedText, StartBit)
                Dim StopBitPos = InStr(2, RecievedText, StopBit)
                If (StopBitPos > StartBitPos) Then
                    Dim TheText = Microsoft.VisualBasic.Left(RecievedText, StopBitPos - 1)
                    Dim TheText2 = Microsoft.VisualBasic.Right(TheText, StopBitPos - 1 - StartBitPos)
                    RecievedText = Microsoft.VisualBasic.Right(RecievedText, Microsoft.VisualBasic.Len(RecievedText) - StopBitPos)
                    RunOnDataRecieved(TheText2)
                    RichTextBoxOutputb.Text &= TheText2 + Chr(13)
                Else
                    RecievedText = Microsoft.VisualBasic.Right(RecievedText, Microsoft.VisualBasic.Len(RecievedText) - StopBitPos)
                End If
            Loop
            Me.RichTextBoxOutput.Text &= [text] 'append text to all past command list
            If AutoScroll.Checked = True Then
                RichTextBoxOutput.SelectionStart = RichTextBoxOutput.TextLength
                RichTextBoxOutput.ScrollToCaret()
                RichTextBoxInput.SelectionStart = RichTextBoxInput.TextLength
                RichTextBoxInput.ScrollToCaret()
                RichTextBoxOutputb.SelectionStart = RichTextBoxOutputb.TextLength
                RichTextBoxOutputb.ScrollToCaret()
            End If
        End If
        Exit Sub
    End Sub
    'Button - Clear output
    Private Sub RichTextBoxOutput_DoubleClick(sender As Object, e As EventArgs) Handles RichTextBoxOutput.DoubleClick
        RichTextBoxOutput.Text = ""
    End Sub
    '==================================================
    '==================================================
    '==========          Your Codes          ==========
    '==================================================
    '==================================================
    'Action - (when) starting up
    Sub RunOnStartup()
        Connect()
    End Sub
    'Action - (when) Connect
    Sub RunOnConnect()
        Emergency.Enabled = True
        SerialSend(EmergencyState + "E0.")          'Tell the Arduino we didn't understand (So it will sync up)
    End Sub
    'Action - (when) Disconnect
    Sub RunOnDisconnect()
        EmergencyOff()
        Emergency.Enabled = False
    End Sub
    'Action - (when) Serial data recieved
    Sub RunOnDataRecieved(Text As String)
        'Example = ~M100
        'CMD1 = ! or ~ = emergency state
        'CMD2 = M or S or E = Motor or Steering or Error
        'CMD3 = 0-255 = byte of state 

        'CMD3 = int    = int steering
        Try
            Dim CMD1 As String = Microsoft.VisualBasic.Left(Text, 1)
            If (CMD1 = "E") Then                        'If we retrieved and read error from the Arduino
                SerialSend(TextBoxInput.Text)           'Resend command
                TextBox1.Text = CMD1
                Return                                  'Stop code here
            End If
            Dim CMD2 As String = Microsoft.VisualBasic.Right(Microsoft.VisualBasic.Left(Text, 2), 1)
            Dim CMD3 As String = Microsoft.VisualBasic.Right(Text, Microsoft.VisualBasic.Len(Text) - 2)
            TextBox1.Text = CMD1 + "_" + CMD2 + "_" + CMD3
            Dim CMDCheck As Boolean = False             'Create a new boolean (if the code is valid)
            If (CMD1 = "~" Or CMD1 = "!") Then          'Check if the format is right (first char = emergency state)
                If (CMD2 = "S" Or CMD2 = "M") Then 'Check if the format is right (second char = Steering or Engine)
                    If (CMD3 < 256) Then                'Check if the format is right (third char = The value of above mentioned )
                        CMDCheck = True                 'Tell rest of the code the CMD is valid
                    End If
                End If
            End If
            If (CMDCheck) Then                          'If CMD is valid
                If (CMD1 = "~") Then                    'If the emergency button isn't pressed
                    EmergencyPicturePin.BackColor = Color.Green 'Show color box as Green to indicate the Emergency button has been released
                Else
                    EmergencyPicturePin.BackColor = Color.Red   'Show color box as Red to indicate the Emergency button has been pressed
                    Fistcommand = True                  'So the manual scrolbars are set
                End If
                If (CMD2 = "S") Then                    'If we are recieving an steering change
                    SteeringBar.Value = CMD3            'Set rotation number
                ElseIf (CMD2 = "M") Then
                    EngineBar.Value = CMD3              'Set engine number
                End If
                If (Fistcommand) Then                   'If this is the first command
                    SteeringBarManual.Value = SteeringBar.Value 'set the steering bar manual value
                    EngineBarManual.Value = EngineBar.Value 'set the engine bar manual value.
                    Fistcommand = False                 'First command has passed
                End If
            Else
                SerialSend(EmergencyState + "E0.")      'Tell the Arduino we didn't understand
                ErrorCounter2 = ErrorCounter2 + 1       'Add one to the error counter
                Errors2.Text = ErrorCounter2            'Update error couter
            End If
        Catch ex As Exception                           'If we have an error in the code (we couldn't cut the string as it should)
            If IgnoreError.Checked = False Then
                MSG = "Error while recieving data." + Chr(13) + "When starting up a bussy connection this error shows." + Chr(13) + "Ignore this message in future? (can be undone);" + Chr(13) + "'" + [Text] + "'"
                Dim result As Integer = MessageBox.Show(MSG, MSGBoxName, MessageBoxButtons.YesNo)
                If result = DialogResult.Yes Then
                    IgnoreError.Checked = True
                End If
            End If
            SerialSend(EmergencyState + "E0.")          'Tell the Arduino we didn't understand
            ErrorCounter = ErrorCounter + 1
            Errors.Text = ErrorCounter
        End Try
    End Sub
    '==================================================
    '==================================================
    '==========          More Codes          ==========
    '==================================================
    '==================================================
    '
    Sub EmergencyToggle()
        If Emergency.ForeColor = Color.Red Then     'If the color is red
            EmergencyOff()
        Else
            MSG = "Are you sure everything is save, and we are ready to start?" 'Show message
            Dim result As Integer = MessageBox.Show(MSG, MSGBoxName, MessageBoxButtons.YesNo)
            If result = DialogResult.Yes Then       'If responce is YES
                SteeringBarManual.Enabled = True    'Enable the use of the button (4x)
                EngineBarManual.Enabled = True
                EngineSend.Enabled = True
                SteeringSend.Enabled = True
                EmergencyState = "~"                'Set EmergencyState to that we are save
                EmergencyPicture.BackColor = Color.Green    'change the color 
                Emergency.ForeColor = Color.Red     'Change the color
                Fistcommand = True                 'Next command is the first command in line (and we need a sync on the manual bar)
            End If
        End If
        SerialSend(EmergencyState + "E0.")          'Tell the Arduino the current EmergencyState
    End Sub
    Sub EmergencyOff()
        SteeringBarManual.Enabled = False       'Disable the use of the button (4x)
        EngineBarManual.Enabled = False
        EngineSend.Enabled = False
        SteeringSend.Enabled = False
        EmergencyState = "!"                    'Set EmergencyState to BURN!
        Emergency.ForeColor = Color.Green       'Change the color
        EmergencyPicture.BackColor = Color.Red  'Change the color
    End Sub
    'Button - Help
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles ButtonHelp.Click
        MSG = "Please read these things carefully, they could come in handy" + Chr(13) + Chr(13) + " - I'm just a dick and there is no information here"
        Dim result As Integer = MessageBox.Show(MSG, MSGBoxName, MessageBoxButtons.YesNo)
        If result = DialogResult.Yes Then
            MsgBox("Well I wanted to inplent copy arduino code here..")
        End If
    End Sub
    'button emergency
    Private Sub Emergency_Click(sender As Object, e As EventArgs) Handles Emergency.Click
        EmergencyToggle()
    End Sub
    'button send steering
    Private Sub SteeringSend_Click_1(sender As Object, e As EventArgs) Handles SteeringSend.Click, SteeringBarManual.Leave
        SerialSend(EmergencyState + "R" + System.Convert.ToString(SteeringBarManual.Value))  'set steering to state 
    End Sub
    'Button send engine
    Private Sub EngineSend_Click(sender As Object, e As EventArgs) Handles EngineSend.Click, EngineBarManual.Leave
        Dim SpeedValue As Integer = EngineBarManual.Value
        If (SpeedValue >= 0) Then
            SerialSend(EmergencyState + "M" + System.Convert.ToString(SpeedValue))  'set steering to state 
        Else
            SerialSend(EmergencyState + "m" + System.Convert.ToString(Math.Abs(SpeedValue)))  'set steering to state with engine in reversed
        End If
    End Sub
    'Clear inputbox
    Private Sub RichTextBoxInput_DoubleClick(sender As Object, e As EventArgs) Handles RichTextBoxInput.DoubleClick
        RichTextBoxInput.Text = ""
    End Sub
    'Clear Outputb
    Private Sub RichTextBoxOutputb_DoubleClick(sender As Object, e As EventArgs) Handles RichTextBoxOutputb.DoubleClick
        RichTextBoxOutputb.Text = ""
    End Sub
End Class
