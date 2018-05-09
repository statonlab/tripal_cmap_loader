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
  use DBTransaction;
  /*
   * Tests the form itself.  This means making sure select options populate etc...
   */
    public function testImporterForm()
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

  public function testImporterFormValidator() {
    $importer = new  \CmapImporter;

    $fmap = factory('chado.featuremap');
    $form_state = ['values']['featuremap_id'] = $fmap->featuremap_id;
    $form = [];
    $importer->formValidate($form, $form_state);
  }

  //  dont think we want to test a submit because we don't want to submit a job!  Maybe its OK though because itll roll back the db???

  

}
