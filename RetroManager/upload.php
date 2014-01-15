<html>
	<head><title>RetroManager - Manage ROMS</title></head>
	<body>
	<center>
		<img src="../images/header.png" />
		<br />
		<?php
			if ($_FILES["file"]["error"] > 0)
			{
				echo "Error: " . $_FILES["file"]["error"] . "<br />";
			}
			else
			{
				$path_parts = pathinfo($_FILES["file"]["name"]);
				$extension = strtolower($path_parts['extension']);
				$upload_path = "";
				$rom_type = "";
				if ($extension == "nes")
				{
					$upload_path = "/home/pi/RetroPie/roms/nes/";
					$rom_type = "NES";
				}
				elseif ($extension == "snes")
				{
					$upload_path = "/home/pi/RetroPie/roms/snes/";
					$rom_type = "SNES";
				}
				elseif ($extension == "smd")
				{
					$upload_path = "/home/pi/RetroPie/roms/megadrive/";
					$rom_type = "Sega Genesis";
				}
				else
				{
					$rom_type == "NA";
				}

				if ($rom_type != "NA")
				{
					if (file_exists($upload_path . $_FILES["file"]["name"]))
					{
						echo $_FILES["file"]["name"] . " already exists. ";
					}
	 				else
					{
						move_uploaded_file($_FILES["file"]["tmp_name"], $upload_path . $_FILES["file"]["name"]);
						echo "New " . $rom_type . " ROM uploaded.";
					}
				}
				else
				{
					echo "Unsupported ROM type.";
				}
			}
		?>
		<br />
		<a href="index.php">Home</a>
	</center>
	</body>
</html>
