<html>
	<head><title>RetroManager</title></head>
	<body>
	<center>
		<img src="../images/header.png" />
		<br />
		<b>System Information</b>
		<br />
		<br />
		<table>
			<tr>
				<td>Free Space:</td>
				<td><?php echo formatSize(disk_free_space("/")); ?></td>
			</tr>
			<tr>
				<td>IP Address:</td>
				<td><?php echo $_SERVER['SERVER_ADDR']; ?></td>
			</tr>
			<tr>
				<td>Version:</td>
				<td>1.0a</td>
			</tr>
		</table>
		<br />
		<a href="index.php">Home</a>
	</center>
	</body>
</html>

<?php
	function formatSize($bytes) 
	{
		$symbols = array('B', 'KiB', 'MiB', 'GiB', 'TiB', 'PiB', 'EiB', 'ZiB', 'YiB');
		$exp = floor(log($bytes)/log(1024));

		return sprintf('%.2f ' . $symbols[$exp], ($bytes/pow(1024, floor($exp))));
	}
