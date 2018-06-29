<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class Serial_Data
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(Serial_Data))
        Me.Errors = New System.Windows.Forms.Label()
        Me.AA = New System.Windows.Forms.Label()
        Me.EmergencyPicture = New System.Windows.Forms.PictureBox()
        Me.RichTextBoxInput = New System.Windows.Forms.RichTextBox()
        Me.EngineSend = New System.Windows.Forms.Button()
        Me.EngineBarManual = New System.Windows.Forms.HScrollBar()
        Me.EngineBar = New System.Windows.Forms.HScrollBar()
        Me.SteeringSend = New System.Windows.Forms.Button()
        Me.SteeringBarManual = New System.Windows.Forms.HScrollBar()
        Me.EmergencyPicturePin = New System.Windows.Forms.PictureBox()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.SteeringBar = New System.Windows.Forms.HScrollBar()
        Me.IgnoreError = New System.Windows.Forms.CheckBox()
        Me.RichTextBoxOutput = New System.Windows.Forms.RichTextBox()
        Me.ButtonSend = New System.Windows.Forms.Button()
        Me.ComboBoxUSB = New System.Windows.Forms.ComboBox()
        Me.TextBoxInput = New System.Windows.Forms.TextBox()
        Me.ButtonConnectDisconnect = New System.Windows.Forms.Button()
        Me.SerialPort1 = New System.IO.Ports.SerialPort(Me.components)
        Me.Emergency = New System.Windows.Forms.Button()
        Me.AutoScroll = New System.Windows.Forms.CheckBox()
        Me.ButtonHelp = New System.Windows.Forms.Button()
        Me.Errors2 = New System.Windows.Forms.Label()
        Me.RichTextBoxOutputb = New System.Windows.Forms.RichTextBox()
        Me.TextBox1 = New System.Windows.Forms.TextBox()
        CType(Me.EmergencyPicture, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.EmergencyPicturePin, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'Errors
        '
        resources.ApplyResources(Me.Errors, "Errors")
        Me.Errors.Cursor = System.Windows.Forms.Cursors.Default
        Me.Errors.ForeColor = System.Drawing.Color.Red
        Me.Errors.Name = "Errors"
        '
        'AA
        '
        resources.ApplyResources(Me.AA, "AA")
        Me.AA.Cursor = System.Windows.Forms.Cursors.WaitCursor
        Me.AA.Name = "AA"
        Me.AA.UseWaitCursor = True
        '
        'EmergencyPicture
        '
        Me.EmergencyPicture.BackColor = System.Drawing.Color.Red
        Me.EmergencyPicture.Cursor = System.Windows.Forms.Cursors.Default
        resources.ApplyResources(Me.EmergencyPicture, "EmergencyPicture")
        Me.EmergencyPicture.Name = "EmergencyPicture"
        Me.EmergencyPicture.TabStop = False
        '
        'RichTextBoxInput
        '
        resources.ApplyResources(Me.RichTextBoxInput, "RichTextBoxInput")
        Me.RichTextBoxInput.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.RichTextBoxInput.Name = "RichTextBoxInput"
        Me.RichTextBoxInput.ReadOnly = True
        Me.RichTextBoxInput.TabStop = False
        '
        'EngineSend
        '
        resources.ApplyResources(Me.EngineSend, "EngineSend")
        Me.EngineSend.Cursor = System.Windows.Forms.Cursors.Hand
        Me.EngineSend.Name = "EngineSend"
        Me.EngineSend.UseVisualStyleBackColor = True
        '
        'EngineBarManual
        '
        resources.ApplyResources(Me.EngineBarManual, "EngineBarManual")
        Me.EngineBarManual.Cursor = System.Windows.Forms.Cursors.Hand
        Me.EngineBarManual.LargeChange = 1
        Me.EngineBarManual.Maximum = 255
        Me.EngineBarManual.Minimum = -255
        Me.EngineBarManual.Name = "EngineBarManual"
        '
        'EngineBar
        '
        resources.ApplyResources(Me.EngineBar, "EngineBar")
        Me.EngineBar.Cursor = System.Windows.Forms.Cursors.Default
        Me.EngineBar.LargeChange = 1
        Me.EngineBar.Maximum = 255
        Me.EngineBar.Minimum = -255
        Me.EngineBar.Name = "EngineBar"
        '
        'SteeringSend
        '
        resources.ApplyResources(Me.SteeringSend, "SteeringSend")
        Me.SteeringSend.Cursor = System.Windows.Forms.Cursors.Hand
        Me.SteeringSend.Name = "SteeringSend"
        Me.SteeringSend.UseVisualStyleBackColor = True
        '
        'SteeringBarManual
        '
        resources.ApplyResources(Me.SteeringBarManual, "SteeringBarManual")
        Me.SteeringBarManual.Cursor = System.Windows.Forms.Cursors.Hand
        Me.SteeringBarManual.Maximum = 256
        Me.SteeringBarManual.Name = "SteeringBarManual"
        '
        'EmergencyPicturePin
        '
        Me.EmergencyPicturePin.BackColor = System.Drawing.Color.Red
        Me.EmergencyPicturePin.Cursor = System.Windows.Forms.Cursors.Default
        resources.ApplyResources(Me.EmergencyPicturePin, "EmergencyPicturePin")
        Me.EmergencyPicturePin.Name = "EmergencyPicturePin"
        Me.EmergencyPicturePin.TabStop = False
        '
        'Label2
        '
        resources.ApplyResources(Me.Label2, "Label2")
        Me.Label2.Cursor = System.Windows.Forms.Cursors.Default
        Me.Label2.Name = "Label2"
        '
        'Label1
        '
        resources.ApplyResources(Me.Label1, "Label1")
        Me.Label1.Cursor = System.Windows.Forms.Cursors.Default
        Me.Label1.Name = "Label1"
        '
        'SteeringBar
        '
        resources.ApplyResources(Me.SteeringBar, "SteeringBar")
        Me.SteeringBar.Cursor = System.Windows.Forms.Cursors.Default
        Me.SteeringBar.Maximum = 256
        Me.SteeringBar.Name = "SteeringBar"
        '
        'IgnoreError
        '
        resources.ApplyResources(Me.IgnoreError, "IgnoreError")
        Me.IgnoreError.Cursor = System.Windows.Forms.Cursors.Hand
        Me.IgnoreError.Name = "IgnoreError"
        Me.IgnoreError.UseMnemonic = False
        Me.IgnoreError.UseVisualStyleBackColor = True
        '
        'RichTextBoxOutput
        '
        resources.ApplyResources(Me.RichTextBoxOutput, "RichTextBoxOutput")
        Me.RichTextBoxOutput.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.RichTextBoxOutput.Name = "RichTextBoxOutput"
        Me.RichTextBoxOutput.ReadOnly = True
        Me.RichTextBoxOutput.TabStop = False
        '
        'ButtonSend
        '
        Me.ButtonSend.Cursor = System.Windows.Forms.Cursors.Hand
        resources.ApplyResources(Me.ButtonSend, "ButtonSend")
        Me.ButtonSend.Name = "ButtonSend"
        Me.ButtonSend.UseVisualStyleBackColor = True
        '
        'ComboBoxUSB
        '
        Me.ComboBoxUSB.Cursor = System.Windows.Forms.Cursors.Hand
        Me.ComboBoxUSB.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        resources.ApplyResources(Me.ComboBoxUSB, "ComboBoxUSB")
        Me.ComboBoxUSB.FormattingEnabled = True
        Me.ComboBoxUSB.Name = "ComboBoxUSB"
        '
        'TextBoxInput
        '
        Me.TextBoxInput.Cursor = System.Windows.Forms.Cursors.IBeam
        resources.ApplyResources(Me.TextBoxInput, "TextBoxInput")
        Me.TextBoxInput.Name = "TextBoxInput"
        '
        'ButtonConnectDisconnect
        '
        Me.ButtonConnectDisconnect.Cursor = System.Windows.Forms.Cursors.Hand
        resources.ApplyResources(Me.ButtonConnectDisconnect, "ButtonConnectDisconnect")
        Me.ButtonConnectDisconnect.ForeColor = System.Drawing.Color.Green
        Me.ButtonConnectDisconnect.Name = "ButtonConnectDisconnect"
        Me.ButtonConnectDisconnect.UseVisualStyleBackColor = True
        '
        'SerialPort1
        '
        Me.SerialPort1.PortName = "COM6"
        '
        'Emergency
        '
        Me.Emergency.Cursor = System.Windows.Forms.Cursors.Hand
        resources.ApplyResources(Me.Emergency, "Emergency")
        Me.Emergency.ForeColor = System.Drawing.Color.Green
        Me.Emergency.Name = "Emergency"
        Me.Emergency.UseVisualStyleBackColor = False
        '
        'AutoScroll
        '
        resources.ApplyResources(Me.AutoScroll, "AutoScroll")
        Me.AutoScroll.Checked = True
        Me.AutoScroll.CheckState = System.Windows.Forms.CheckState.Checked
        Me.AutoScroll.Cursor = System.Windows.Forms.Cursors.Hand
        Me.AutoScroll.Name = "AutoScroll"
        Me.AutoScroll.UseMnemonic = False
        Me.AutoScroll.UseVisualStyleBackColor = True
        '
        'ButtonHelp
        '
        Me.ButtonHelp.Cursor = System.Windows.Forms.Cursors.Hand
        resources.ApplyResources(Me.ButtonHelp, "ButtonHelp")
        Me.ButtonHelp.Name = "ButtonHelp"
        Me.ButtonHelp.UseVisualStyleBackColor = True
        '
        'Errors2
        '
        resources.ApplyResources(Me.Errors2, "Errors2")
        Me.Errors2.Cursor = System.Windows.Forms.Cursors.Default
        Me.Errors2.ForeColor = System.Drawing.Color.Red
        Me.Errors2.Name = "Errors2"
        '
        'RichTextBoxOutputb
        '
        resources.ApplyResources(Me.RichTextBoxOutputb, "RichTextBoxOutputb")
        Me.RichTextBoxOutputb.Cursor = System.Windows.Forms.Cursors.IBeam
        Me.RichTextBoxOutputb.Name = "RichTextBoxOutputb"
        Me.RichTextBoxOutputb.ReadOnly = True
        Me.RichTextBoxOutputb.TabStop = False
        '
        'TextBox1
        '
        Me.TextBox1.Cursor = System.Windows.Forms.Cursors.IBeam
        resources.ApplyResources(Me.TextBox1, "TextBox1")
        Me.TextBox1.Name = "TextBox1"
        '
        'Serial_Data
        '
        resources.ApplyResources(Me, "$this")
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.Controls.Add(Me.TextBox1)
        Me.Controls.Add(Me.RichTextBoxOutputb)
        Me.Controls.Add(Me.Errors2)
        Me.Controls.Add(Me.Errors)
        Me.Controls.Add(Me.AA)
        Me.Controls.Add(Me.EmergencyPicture)
        Me.Controls.Add(Me.RichTextBoxInput)
        Me.Controls.Add(Me.EngineSend)
        Me.Controls.Add(Me.EngineBarManual)
        Me.Controls.Add(Me.EngineBar)
        Me.Controls.Add(Me.SteeringSend)
        Me.Controls.Add(Me.SteeringBarManual)
        Me.Controls.Add(Me.EmergencyPicturePin)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.SteeringBar)
        Me.Controls.Add(Me.IgnoreError)
        Me.Controls.Add(Me.RichTextBoxOutput)
        Me.Controls.Add(Me.ButtonSend)
        Me.Controls.Add(Me.ComboBoxUSB)
        Me.Controls.Add(Me.TextBoxInput)
        Me.Controls.Add(Me.ButtonConnectDisconnect)
        Me.Controls.Add(Me.Emergency)
        Me.Controls.Add(Me.AutoScroll)
        Me.Controls.Add(Me.ButtonHelp)
        Me.Name = "Serial_Data"
        CType(Me.EmergencyPicture, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.EmergencyPicturePin, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents Errors As Label
    Friend WithEvents AA As Label
    Friend WithEvents EmergencyPicture As PictureBox
    Public WithEvents RichTextBoxInput As RichTextBox
    Friend WithEvents EngineSend As Button
    Friend WithEvents EngineBarManual As HScrollBar
    Friend WithEvents EngineBar As HScrollBar
    Friend WithEvents SteeringSend As Button
    Friend WithEvents SteeringBarManual As HScrollBar
    Friend WithEvents EmergencyPicturePin As PictureBox
    Friend WithEvents Label2 As Label
    Friend WithEvents Label1 As Label
    Friend WithEvents SteeringBar As HScrollBar
    Friend WithEvents IgnoreError As CheckBox
    Public WithEvents RichTextBoxOutput As RichTextBox
    Friend WithEvents ButtonSend As Button
    Friend WithEvents ComboBoxUSB As ComboBox
    Friend WithEvents TextBoxInput As TextBox
    Friend WithEvents ButtonConnectDisconnect As Button
    Friend WithEvents SerialPort1 As IO.Ports.SerialPort
    Friend WithEvents Emergency As Button
    Friend WithEvents AutoScroll As CheckBox
    Friend WithEvents ButtonHelp As Button
    Friend WithEvents Errors2 As Label
    Public WithEvents RichTextBoxOutputb As RichTextBox
    Friend WithEvents TextBox1 As TextBox
#Disable Warning BC40004 ' Member conflicts with member in the base type and should be declared 'Shadows'
#Enable Warning BC40004 ' Member conflicts with member in the base type and should be declared 'Shadows'
End Class
