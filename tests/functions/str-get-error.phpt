--TEST--
Check for str-get with error arguments
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php 
$error = "";
$ret = sass_compile("data", "foo{width:str-get(a, a);}", array('output_style' => 3), $error);
if($ret)
        echo $ret;
else
        echo $error;
--EXPECT--
stdin:1: error in C function str-get: Argument in str-get is not right!
Backtrace:
	stdin:0, in function `str-get`
	stdin:0
