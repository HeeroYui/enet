<?php
// force display of error
error_reporting(E_ALL);
ini_set('display_errors', '1');

date_default_timezone_set('Europe/Paris');


echo "plop<br/>\n";
//echo $_POST;
if (    isset($_POST) == true
     && empty($_POST) == false) {
	foreach ($_POST as $key => $value) {
		echo "key='".$key."' val='".$value."'<br>\n";
	}
}
if (    isset($HTTP_RAW_POST_DATA) == true
     && empty($HTTP_RAW_POST_DATA) == false) {
	echo "raw data :'".$HTTP_RAW_POST_DATA."'<br>\n";
}

if (    isset($_SERVER)
     && isset($_SERVER["CONTENT_TYPE"]) ) {
	echo "content Type : '".$_SERVER["CONTENT_TYPE"]."'<br>\n";
}

?>