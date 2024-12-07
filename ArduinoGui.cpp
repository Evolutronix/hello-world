#include "ArduinoGui.h"

using namespace System;
using namespace System::Windows::Forms;
[STAThread]
void main()
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	PCA9685::ArduinoGui form;
	Application::Run(% form);
}