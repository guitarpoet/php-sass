--TEST--
Check for convert-unit with error arguments
--SKIPIF--
<?php if (!extension_loaded("sass")) print "skip"; ?>
--FILE--
<?php 
$error = "";
$ret = sass_compile("data", "foo{width:convert-unit(1px, em);}", array('output_style' => 3), $error);
if($ret)
        echo $ret;
else
        echo $error;
--EXPECT--
foo{width:1em}
