--TEST--
Check for simple compile
--SKIPIF--
<?php if (!extension_loaded("sass")) echo "skip"; ?>
--FILE--
<?php 
$error = "";
$ret = sass_compile("data", "foo{width:2*2px;}", array('output_style' => 3), $error);
if($ret)
	echo 1;
else
	echo $error;
--EXPECT--
1
