#include <newt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

void drawMainForm();
void drawSettingsForm();

int main(void) 
{
	newtInit();
	drawMainForm();

	return 0;
}

void drawMainForm()
{
	newtComponent frmMain;
	newtComponent btnStart;
	newtComponent btnSettings;
	newtComponent btnShutdown;
	newtComponent selectedButton;
	bool appStarted = false;

	newtCls();

	newtDrawRootText(0, 0, "RetroStation v0.1");
	newtDrawRootText(0, 1, "(c) 2014 Normal Distribution");
	newtCenteredWindow(40, 13, "Main Menu");

	newtPushHelpLine("Use <UP> & <DOWN> to move selection. Press <START> to make a selection");

	btnStart = newtButton(10, 1, "Start RetroStation");
	btnSettings = newtButton(10, 5, "Settings");
	btnShutdown = newtButton(10, 9, "Shutdown");

	frmMain = newtForm(NULL, NULL, 0);
	newtFormAddComponents(frmMain, btnStart, btnSettings, btnShutdown, NULL);

	selectedButton = newtRunForm(frmMain);
	
	if (selectedButton == btnStart)
	{
		system("aptitude");
		appStarted = true;
	}
	else if (selectedButton == btnSettings)
	{
		drawSettingsForm();
	}
	else if(selectedButton == btnShutdown)
	{
		system("shutdown -h now");
	}
	
	newtFormDestroy(frmMain);
	newtFinished();

	if (appStarted)
	{
		main();
	}
}

void drawSettingsForm()
{
	newtComponent frmSettings;
	newtComponent btnImportROMs;
	newtComponent btnWiFiSettings;
	newtComponent btnBack;
	newtComponent selectedButton;

	newtCls();

	newtDrawRootText(0, 0, "RetroStation v0.1");
	newtDrawRootText(0, 1, "(c) 2014 QuantumPhysGuy");
	newtCenteredWindow(40, 13, "Settings");

	newtPushHelpLine("Use <UP> & <DOWN> to move selection. Press <SELECT> to make a selection");

	btnWiFiSettings = newtButton(10, 1, "WiFi Settings");
	btnImportROMs = newtButton(10, 5, "Import ROMs");
	btnBack = newtButton(10, 9, "Back");
	frmSettings = newtForm(NULL, NULL, 0);
	newtFormAddComponents(frmSettings, btnWiFiSettings, btnImportROMs, btnBack, NULL);

	selectedButton = newtRunForm(frmSettings);

	if (selectedButton == btnBack)
	{
		drawMainForm();
	}	
	
	newtFormDestroy(frmSettings);
	newtFinished();
}