<?php
	session_start();
?>

<?php
	if (isset($_POST["username"]))
	{
		$_SESSION["username"] = $_POST["username"];
		$_SESSION["isLogged"] = true;
		header("Location: index.php");
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
	<form action="login.php" method="post">
		<input type="text" name="username" placeholder="Username">
		<button type="submit">Login</button>
	</form>
</body>
</html>