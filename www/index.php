<?php
	session_start();

	if (!isset($_SESSION["username"]))
	{
		header("Location: login.php");
		exit();
	}
?>

<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Document</title>
</head>
<body>
	<div>
		Hello <?php
		if (isset($_SESSION["isLogged"]))
			echo $_SESSION['username'];
		else
			echo "Error";
			?>
	</div>
</body>
</html>