<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use StatonLab\TripalTestSuite\DBTransaction;


require_once(__DIR__ . '/../includes/TripalImporter/CmapImporter.inc');
//use tripal_cmap_loader;

/**
 * Class ExampleTest
 *
 * Note that test classes must have a suffix of Test.php and the filename
 * must match the class name.
 *
 * @package Tests
 */
class CmapImporterTest extends TripalTestCase {

  use DBTransaction;

  /*
   * Tests the form itself.  This means making sure select options populate etc...
   */
  public function testImporterForm() {
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
    $form_state =[];

      $form_state['values']['featuremap_id'] = $fmap->featuremap_id;
    $form = [];
    $importer->formValidate($form, $form_state);
  }


  public function testImporterLoadingStuff() {

    $importer = new  \CmapImporter;

    $analysis = factory('chado.analysis');
    $fmap = factory('chado.featuremap');
    $run_args = [
      'analysis_id' => $analysis->analysis_id,
      'featuremap_id' => $fmap->featuremap_id,
    ];

    $importer->create($run_args, $file_details = ['file_local' => __DIR__ . '/../example/c_mollisima_example.cmap']);


  }
  //  dont think we want to test a submit because we don't want to submit a job!  Maybe its OK though because itll roll back the db???


}
