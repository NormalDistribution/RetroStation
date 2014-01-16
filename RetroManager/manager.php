<html>
	<head><title>RetroManager</title></head>
	<body>
	<center>
		<img src="../images/header.png" />
		<br />
		<?php 
			$romType = $_GET["rom"];
			$scanDir = "";

			if ($romType == "nes")
			{
				$scanDir = "/home/pi/RetroPie/roms/nes/";
				echo "<b>Manage NES Roms</b>";
			}
			elseif ($romType == "snes")
			{
				$scanDir = "/home/pi/RetroPie/roms/snes/";
				echo "<b>Manage SNES roms</b>";
			}
			elseif ($romType == "smd")
			{
				$scanDir = "/home/pi/RetroPie/roms/megadrive/";
				echo "<b>Manage Sega Genesis roms</b>";
			}
		?>
		<table>
			<?php 
				$romList = scandir($scanDir);

				foreach ($romList as &$romFile) 
				{
					if ($romFile != "." && $romFile != "..")
					{
						echo '<tr><td>' . $romFile . '</td><td><a href="#">Delete</a></td></tr>';
					}
				}

				unset($romFile);
			?>
		</table>
		<br />
		<a href="index.php">Home</a>
	</center>
	</body>
</html>
