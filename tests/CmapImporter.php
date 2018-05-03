<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use CmapImporter;

/**
 * Class ExampleTest
 *
 * Note that test classes must have a suffix of Test.php and the filename
 * must match the class name.
 *
 * @package Tests
 */
class ExampleTest extends TripalTestCase
{
    public function testInit()
    {
$this->assertTrue(TRUE);

$importer = new CmapImporter;

    }
}
