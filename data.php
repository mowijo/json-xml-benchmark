<?php

ini_set('display_errors',1);
ini_set('display_startup_errors',1);
error_reporting(-1);

$format = $_GET["encoder"];
$entries = $_GET["entries"];

$list = array(); 
for( $i = 0; $i < $entries; $i++)
{
  $a = array("name" => "filename".$i.".extension", "mtime" => "1997-07-16T19:20:30+00:00", "size" => 1231234, "md5"=>"sdfasfarf4r5sf");
  $list[] = $a;
}

$s = "";

if($format == "buildin-json")
{
  $start =  microtime(true);
  $s = json_encode($list);
  $end =  microtime (true);
}


if($format == "simplexml-attibutes")
{
  $start = microtime(true);
  $xml = new SimpleXMLElement('<xml/>');

  foreach($list as $element)
  {
      $track = $xml->addChild('file');
      $track->addAttribute('name', $element["name"]);
      $track->addAttribute('mtime', $element["mtime"]);
      $track->addAttribute('size', $element["size"]);
      $track->addAttribute('md5', $element["md5"]);

  }
  $s = $xml->asXML();
  $end = microtime(true);
}

if($format == "simplexml-childnodes")
{
  $start = microtime(true);
  $xml = new SimpleXMLElement('<xml/>');

  foreach($list as $element)
  {
      $track = $xml->addChild('file');
      $track->addChild('name', $element["name"]);
      $track->addChild('mtime', $element["mtime"]);
      $track->addChild('size', $element["size"]);
      $track->addChild('md5', $element["md5"]);

  }
  $s = $xml->asXML();
  $end = microtime(true);
}


header("X-encodingDuration: ".($end-$start)*1000000);	//In micreoseconds  seconds
header("X-encodedSize: ".strlen($s));
header("X-encoder: ".($format));

echo $s;

?>

