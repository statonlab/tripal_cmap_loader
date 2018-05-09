<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use StatonLab\TripalTestSuite\DBTransaction;

require_once(__DIR__ . '/../includes/TripalImporter/CmapImporter.inc');

/**
 * Class CmapImporterTest
 *
 * Tests the cmap importer.
 *
 */
class CmapImporterTest extends TripalTestCase {

  use DBTransaction;

  /*
   * Tests the form itself.  This means making sure select options populate etc...
   */
  public function testImporterForm() {
    $importer = new  \CmapImporter;
    $this->assertNotNull($importer);

    $featuremap = factory('chado.featuremap')->create();

    $form = [];
    $form_state = [];
    $form = $importer->form($form, $form_state);
    $this->assertNotEmpty($form);
    $options = $form['featuremap_id']['#options'];

    $this->assertGreaterThan(1, count($options));//please select is an option by default.
  }

  public function testImporterFormValidator() {
    $importer = new  \CmapImporter;

    $fmap = factory('chado.featuremap')->create();
    $form_state = [];

    $form_state['values']['featuremap_id'] = $fmap->featuremap_id;
    $form = [];
    $importer->formValidate($form, $form_state);
  }


  public function testImporterLoadingStuff() {

    $importer = new  \CmapImporter;

    $analysis = factory('chado.analysis')->create();
    $fmap = factory('chado.featuremap')->create();
    $run_args = [
      'analysis_id' => $analysis->analysis_id,
      'featuremap_id' => $fmap->featuremap_id,
    ];

    $importer->create($run_args, $file_details = ['file_local' => __DIR__ . '/../example/c_mollisima_example.cmap']);


  }
  //  dont think we want to test a submit because we don't want to submit a job!  Maybe its OK though because itll roll back the db???


}
