<!DOCTYPE html>
<html>
	<body>
		<?php
		$say = htmlspecialchars($_GET['say']);
		$to = htmlspecialchars($_GET['to']);
		echo "[", $say, " ", $to, "]";
		?>
		<br />

		<?php
		echo "PATH_INFO -> [", $_SERVER['PATH_INFO'], "]";
		?>
		<br />

		<?php
		echo "QUERY_STRING -> [", $_SERVER['QUERY_STRING'], "]";
		?>
		<br />
		<?php
		echo "REMOTE_ADDR -> [", $_SERVER['REMOTE_ADDR'], "]";
		?>
		<br />

	</body>
</html>
