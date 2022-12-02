<?php

$msg = "";
if ($_SERVER['REQUEST_METHOD'] == 'POST') 
{
    $name = $_POST["vscode"];
	 if ($name == '') {
        $msg = "You must enter all fields";
     }
    if ($name == "vscode")
    {
        $msg = "good Response";

    }
    else
    {
        $msg = "bad Response";
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Exemple Webserv</title>
    <style>
            body
      {
          background-color: black;
          margin: 0;
          padding: 0;
          border: 0;
      }

      .title
      {
          font-family: Outfit,Avenir,Helvetica,Arial,sans-serif;
          display: flex;
          align-items: center;
          justify-content: center;
          color: white;
          background: #1d1d20;
          font-size: 4em;
          margin-block-start: 0;
          margin-block-end: 0px;
          margin-inline-start: 0px;
          margin-inline-end: 0px;
      }

      #snakeboard {
          position: absolute;
          top: 60%;
          left: 50%;
          transform: translate(-50%, -50%);
        }

        #score {
          font-family: Outfit,Avenir,Helvetica,Arial,sans-serif;
          display: flex;
          align-items: center;
          justify-content: center;
          color: white;
          font-size: 1em;
          margin-block-start: 0;
          margin-block-end: 0px;
          margin-inline-start: 0px;
          margin-inline-end: 0px;
        }
    </style>
</head>
<body>
    <h1 class="title">form</h1>
    <div id="score">
      <p> quel est ton editeur de texte preferer</p>
    </div>
    <div id="score">
    <form name="form" action="<?= $_SERVER['PHP_SELF'] ?>" method="post" >
		<table class="form" border="0">
			<tr>
			<td></td>
				<td style="color:red;">
				<?php echo $msg; ?></td>
			</tr> 
			
			<tr>
				<th><label for="name"><strong>reponse</strong></label></th>
				<td><input class="inp-text" name="vscode" id="vscode" type="text" size="30" /></td>
			</tr>
			<td></td>
				<td class="submit-button-right">
				<input class="send_btn" type="submit" value="Submit" alt="Submit" title="Submit" />
				
				<input class="send_btn" type="reset" value="Reset" alt="Reset" title="Reset" /></td>
				
			</tr>
		</table>
	</form>
    </div>

</body>
</html>
