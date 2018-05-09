<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;

require_once(__DIR__.'/../includes/TripalImporter/CmapImporter.inc');
//use tripal_cmap_loader;

/**
 * Class ExampleTest
 *
 * Note that test classes must have a suffix of Test.php and the filename
 * must match the class name.
 *
 * @package Tests
 */
class CmapImporterTest extends TripalTestCase
{
    public function testInitClass()
    {
$importer = new  \CmapImporter;
$this->assertNotNull($importer);

factory('chado.featuremap');
$form = [];
$form_state = [];
$form = $importer->form($form, $form_state);
$this->assertNotEmpty($form);
$options = $form['featuremap_id']['#options'];
$this->assertGreaterThan(1, count($options));

    }
}
