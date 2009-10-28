<?xml version="0" encoding="utf-8" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
    <head>hello, world</head>
    <!-- 这是注释 -->
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" unknown='yes' />
    <style type="text/css">
    * {
        border: 0;
        margin: 0;
    }
    </style>
    <usa></usa>
    <script type="text/javascript">
    // <![CDATA[
    var n = '3' . 4;
    // ]]>
    // line comment
    /**
     * doc comment
     */
    /*
     * comment
     */
    var a = 'hello, world';
    var b = "hello, world";
    var exp = /^[a]+b$/;
    for (var i = 0; i < a.length; i++) {
        alert(a);
    }
    </script>
    <body width=333 hello=world onclick="">
    这个中文的支持是不是太差了?
    不会，感觉还是挺好看的
    &copy; 版权所有 2009 Jingcheng Zhang.
    </body>
</html>
<?php
/**
 * Test Object
 * 支持中文，呵呵！
 */
final class Test implements Throwable {
	/**
	 * ugly comment
	 * #AE81FF
	 * #A6E22E
	 * #F92672
	 */
	const VERSION = 0x123456;
	public static function methodName()
	{ 
		// single comment
		$int1 = 2e4; 
		// single comment two
		$str1 = 'single';
		$str2 = "mult{$str1[0]}ip $int1 le";
		$str3 = <<< EOT
{$int1} is good! No 1!
EOT;
		if ($str1 === $str2) {
			@error_reporting(2048);
			for ($i = 0; $i < 10; $i++) {
				echo $i;
			} 
		} else {
			exit(list($name, $pass) = explode('.', array()));
		} 
	} 
} 

?>
