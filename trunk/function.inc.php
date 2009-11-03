<?php
/**
 * Functions lib
 * 
 * @author Kim L
 * @category Project
 * @package None
 * @copyright Copyright (c)2009
 * @version $Id$
 */
 
/**
 * auto load class file
 * 
 * @access public 
 * @param string $classname 
 * @return void 
 */
function __autoload($classname)
{
	if (empty($classname)) {
		return;
	} 
	$filepath = dirname(__FILE__) . DIRECTORY_SEPARATOR . "lib";
	$filepath .= "/" . strtolower($classname) . ".class.php";
	if (is_file($filepath)) {
		require_once($filepath);
	} 
    
} 

function test()
{
    $ssh = new SSH();
}

/** End of file function.inc.php */