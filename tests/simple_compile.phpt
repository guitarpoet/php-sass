--TEST--
Check for simple compile
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php 
$error = "";
$ret = sass_compile("data", "foo{width:2*2px;}", array('output_style' => 3), $error);
if($ret)
	echo $ret;
else
	echo $error;
--EXPECT--
foo{width:4px}
