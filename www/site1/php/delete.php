<?php
	session_start();
?>

<?php
	if ($_SERVER["REQUEST_METHOD"] == "get")
	{
		echo "get request";
	}

	if ($_SERVER["REQUEST_METHOD"] == "DELETE")
	{
		$d = dir("/tmp/upload");
		if (!$d) {
			echo "Error: cannot open directory";
			header("Status: 404");
			exit;
		}
		while (false !== ($entry = $d->read())) {
			if ($entry != "." && $entry != "..")
				unlink("/tmp/upload/" . $entry);
		 }
		 $d->close();
	}
?>


<!DOCTYPE html>
<html>
<head>
	<title>Our Website</title>
	<link rel="stylesheet" type="text/css" href="/css/style.css">
	<script defer src="deletephp.js"></script>
</head>
<body>
	<div class="header">
		<h1>Methode Delete</h1>
		<a href="/index.html">Home</a>
	</div>
	<button id="deleteButton">Delete all /tmp/upload</button>
</body>

</html>