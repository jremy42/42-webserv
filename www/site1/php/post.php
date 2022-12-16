<?php
	ini_set('display_errors', 1);
	ini_set('display_startup_errors', 1);
	error_reporting(E_ALL);
?>
<!DOCTYPE html>
<html>
<head>
	<title>Our Website</title>
	<link rel="stylesheet" type="text/css" href="/css/style.css">

</head>
<body>
	<div class="header">
		<h1>Methode POST PHP</h1>
		<a href="../index.html">Home</a>
	</div>
	<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="post" enctype="multipart/form-data">
        <p>
            <label for id=input_file>Selectionnez un fichier : </label>
            <input type="file" name="input_file" id=input_file/>
        </p>
        <p>
            <label for id=filename>Donnez lui un nouveau nom : </label>
            <input type="text" name="filename" id=filename/>
        </p>
        <p>
            <input type="submit"/>
        </p>
    </form>
	<p>
	<?php
		if (isset($_FILES["input_file"]) && $_FILES["input_file"]["name"] != ""
			&& isset($_POST["filename"]) && $_POST["filename"] != "")
		{
			echo "filename is : " . $_POST["filename"];
			echo "<br/>";
			$try_upload = "/tmp/upload/" . $_POST["filename"];
			if (file_exists($try_upload))
			{
				echo $try_upload . " already exits on disk !";
				echo "<br/>";
			}
			else
			{
				if (!is_dir("/tmp/upload"))
				{
					echo "Trying to create missing upload dir :" . " /tmp/upload/";
					echo "<br/>";
					if (mkdir("/tmp/upload/"))
					{
						echo "Successfuly created upload dir :" . " /tmp/upload/";
						echo "<br/>";
					}
					else
					{
						echo "Failure creating upload dir :" . " /tmp/upload/";
						echo "<br/>";
					}

				}
				$ret = move_uploaded_file($_FILES["input_file"]["tmp_name"], $try_upload);
				if ($ret == true)
					echo "Successfully uploaded : " . $try_upload . "!";
				else
					echo "Failure uploading : " . $try_upload . "!";
				echo "<br/>";
			}
		}
		else
		{
			if (!isset($_FILES["input_file"]) || $_FILES["input_file"]["name"] == "")
			{
				echo "No file yet selected";
				echo "<br/>";
			}
			if (!isset($_POST["filename"]) || $_POST["filename"] == "")
			{
				echo "Missing filename";
				echo "<br/>";
			}
		}
	?>
</body>
</html>
