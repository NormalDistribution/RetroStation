/*
(c) 2014 Normal Distribution
RetroStation Boot Menu

Written by Matt Wise for Normal Distribution, distributed under GNU GENERAL PUBLIC LICENSE V2.

Copyright (c) 2014 Normal Distribution.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <newt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

void drawMainForm();
void drawSettingsForm();
char* getIp(char* cInterface);
void messageBox(unsigned int uiW, unsigned int uiH, const char* pMessage);

char* cTitle = "RetroStation v0.1";
char* cCopyright = "(c) 2014 Normal Distribution";
char* cPrimaryHelpLine = "Use <UP> & <DOWN> to move selection. Press <START> to make a selection";

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

	newtDrawRootText(0, 0, cTitle);
	newtDrawRootText(0, 1, cCopyright);
	newtDrawRootText(0, 2, getIp("wlan0"));
	newtCenteredWindow(40, 13, "Main Menu");

	newtPushHelpLine(cPrimaryHelpLine);

	btnStart = newtButton(10, 1, "Start Emulation Station");
	btnSettings = newtButton(10, 5, "Settings");
	btnShutdown = newtButton(10, 9, "Shutdown");

	frmMain = newtForm(NULL, NULL, 0);
	newtFormAddComponents(frmMain, btnStart, btnSettings, btnShutdown, NULL);

	selectedButton = newtRunForm(frmMain);

	if (selectedButton == btnStart)
	{
		int iReturn = system("emulationstation");

		if (iReturn == 1)
		{
			appStarted = true;
		}
		else
		{
			messageBox(40, 13, "Unable to launch Emulation Station!!!");
			main();
		}
	}
	else if (selectedButton == btnSettings)
	{
		drawSettingsForm();
	}
	else if(selectedButton == btnShutdown)
	{
		//Exit right now. Don't shutdown
		//system("shutdown -h now");
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
	newtComponent btnNetworkSettings;
	newtComponent btnBack;
	newtComponent selectedButton;

	newtCls();

	newtDrawRootText(0, 0, cTitle);
	newtDrawRootText(0, 1, cCopyright);
	newtDrawRootText(0, 2, getIp("wlan0"));
	newtCenteredWindow(40, 13, "Settings");

	newtPushHelpLine(cPrimaryHelpLine);

	btnNetworkSettings = newtButton(10, 1, "Network Settings");
	btnImportROMs = newtButton(10, 5, "Import ROMs");
	btnBack = newtButton(10, 9, "Back");
	frmSettings = newtForm(NULL, NULL, 0);
	newtFormAddComponents(frmSettings, btnNetworkSettings, btnImportROMs, btnBack, NULL);

	selectedButton = newtRunForm(frmSettings);

	if (selectedButton == btnBack)
	{
		drawMainForm();
	}
	else if (selectedButton == btnNetworkSettings)
	{
		messageBox(40, 13, "Network Settings not implemented.");
		drawSettingsForm();
	}
	else if (selectedButton == btnImportROMs)
	{
		messageBox(40, 13, "Import ROMs not implemented.");
		drawSettingsForm();
	}

	newtFormDestroy(frmSettings);
	newtFinished();
}

void messageBox(unsigned int uiW, unsigned int uiH, const char* pMessage) 
{
	newtComponent form;
	newtComponent label;
	newtComponent button;

	newtCenteredWindow(uiW, uiH, "System Message");
	newtPopHelpLine();
	newtPushHelpLine("<Press <START> button to return>");

	label = newtLabel((uiW-strlen(pMessage))/2, uiH/4, pMessage);
	button = newtButton((uiW-6)/2, 2*uiH/3, "OK");
	form = newtForm(NULL, NULL, 0);
	newtFormAddComponents(form, label, button, NULL);
	newtRunForm(form);

	newtFormDestroy(form);
}

char* getIp(char* cInterface)
{
	int fd;
	struct ifreq ifr;
	char* cIP;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* Get IP address attached to the passed interface */
	strncpy(ifr.ifr_name, cInterface, IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* store result */
	cIP = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

	return cIP;
}
