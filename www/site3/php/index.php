<?php
	session_start();

	if (!isset($_SESSION["username"]))
	{
		header("Location: login.php");
		exit();
	}
	if (isset($_POST["logout"]))
	{
		session_destroy();
		header("Location: login.php");
		exit();
	}
?>



<!DOCTYPE html>
<html lang="en">
<head>
	<title>Our Website</title>
	<link rel="stylesheet" type="text/css" href="/css/style.css">

</head>
<body>

	<div class="header">
		<h1>HELLO <?php
		if (isset($_SESSION["isLogged"]))
			echo $_SESSION['username'];
		else
			echo "Error";
			?></h1>
		<a href="/index.html">Home</a>
	</div>
	<form action="index.php" method="post">
		<button type="submit" name="logout">Logout</button>
	</form>
</body>
</html>