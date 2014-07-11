import snack
import time

screen = snack.SnackScreen()

screen.drawRootText(0, 0, 'RetroStation v0.1')
screen.drawRootText(screen.width - 28, 0, '(c) 2014 QuantumPhysGuy')
screen.pushHelpLine("Use <UP> & <DOWN> to move selection. Press <SELECT> to make a selection")

screen.refresh()

x = snack.ButtonChoiceWindow(screen, "Main Menu", "", buttons=['Start RetroStation', 'Settings'])

screen.finish()

if x == 'Start RetroStation':
	print '[Start Retro]'
else:
	print '[Else]'