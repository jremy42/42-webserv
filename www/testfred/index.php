<!DOCTYPE html>
<html>
	<head>
		<title>Test</title>
		<meta charset="utf-8">
	</head>
	<body>
		<p>Start of form</p>
		<form method="post">
			<div>
				<label for="id_user_name">Your name :</label>
				<input type="text" name="user_name" id="id_user_name">
			</div>
			<div>
				<input type="submit" value="Send">
			</div>
		</form>
		<p>Start of php</p>
		<?php
			if(isset($_POST["user_name"]))
				$var = $_POST["user_name"];
			else
				$var = "Default_user_name";
			echo 'Hello ' . $var . '!'
		?>
	</body>
</html>
