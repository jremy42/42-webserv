<?php
	session_start();
?>

<?php
	if ($_SESSION["isLogged"])
	{		
		header("Location: index.php");
		exit();
	}
	if (isset($_POST["username"]))
	{
		if($_POST["username"] == "")
		{
			header("Location: login.php");
			exit();
		}
		$_SESSION["username"] = $_POST["username"];
		$_SESSION["isLogged"] = true;
		header("Location: index.php");
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
		<h1>webserv site 4</h1>
		<a href="/index.html">Home</a>
	</div>
	
	<form action="login.php" method="post">
		<input type="text" name="username" placeholder="Username">
		<button type="submit">Login</button>
	</form>
</body>
</html>