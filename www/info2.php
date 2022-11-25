<?php

$say = htmlspecialchars($_GET['say']);
$to = htmlspecialchars($_GET['to']);

echo '[', $say, ' ', $to, ']';

?>
