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
    $organism = factory('chado.organism')->create();

    $form = [];
    $form_state = [];
    $form = $importer->form($form, $form_state);
    $this->assertNotEmpty($form);
    $options = $form['featuremap_id']['#options'];

    $this->assertGreaterThan(1, count($options));//please select is an option by default.

    $org_options = $form['organism_id']['#options'];

    $this->assertGreaterThan(1, count($org_options));//please select is an option by default.

  }

//  /**
//   * @group form
//   */
//  public function testImporterFormValidator() {
//    $importer = new  \CmapImporter;
//
//    $fmap = factory('chado.featuremap')->create();
//    $form_state = [];
//
//    $form_state['values']['featuremap_id'] = $fmap->featuremap_id;
//    $form = [];
//    $form = $importer->form($form, $form_state);
//    $form = $importer->formValidate($form, $form_state);
//  }


  /**
   * Data provider for the map features (chromosomes)
   *
   * @return array
   */
  public function chromosome_name_uname_provider() {
    return [
      ['A', 'C_mollisima_A'],
      ['L', 'C_mollisima_L',],
    ];
  }

  /**
   * Test that the map features (chromosomes) are created
   *
   * @dataProvider chromosome_name_uname_provider
   */
  public function testImporterCreatesChromosomeFeatures($name, $uname) {
    ob_start();
    $this->run_importer();
    ob_end_clean();

    //were features from beginning and end of file loaded?
    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename', 'type_id']);
    $query->condition('CF.name', $name);
    $result = $query->execute()->fetchObject();

    $this->assertEquals($name, $result->name);
    $this->assertEquals($uname, $result->uniquename);

    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename']);
    $query->condition('CF.uniquename', $uname);
    $result = $query->execute()->fetchObject();

    $this->assertEquals($name, $result->name);
    $this->assertEquals($uname, $result->uniquename);

  }

  /**
   * test case for when the feature already exists.
   *
   * @param $name
   * @param $uname
   *
   * @group failing
   * @throws \Exception
   * @dataProvider chromosome_name_uname_provider
   */
  public function testImporterUpdatesExistingFeatures($name, $uname) {
    $importer = new  \CmapImporter;
    $cv_id = $this->get_so_id();//should only allow SO terms...
    $organism = factory('chado.organism')->create();
    $analysis = factory('chado.analysis')->create();
    $featuremap = factory('chado.featuremap')->create();
    $type = factory('chado.cvterm')->create(['cv_id' => $cv_id]);
    $file = __DIR__ . '/../example/c_moll_mini.cmap';

    //Create the chromosome feature we're testing beforehand with some sequence so we'll know it was overwritten
    $seq = 'AAA';
    factory('chado.feature')->create([
      'name' => $name,
      'uniquename' => $uname,
      'residues' => $seq,
      'type_id' => $type->cvterm_id
    ]);

    ob_start();
    $importer->parse_cmap($analysis->analysis_id, $file, $featuremap->featuremap_id, $type->cvterm_id, $organism->organism_id);
    ob_end_clean();

    //Was everything loaded even though this feature existed?

    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename', 'type_id', 'residues']);
    $query->condition('CF.name', $name);
    $result = $query->execute()->fetchObject();

    $this->assertEquals($name, $result->name);
    $this->assertEquals($uname, $result->uniquename);
    $this->assertEquals($seq, $result->residues);
  }

  /**
   * @param $name
   * @param $uname
   * @param $start
   * @param $type_name
   * @param $mapping_feature
   *
   * @group failing
   * @dataProvider marker_provider
   */
  public function testImporterCreatesMarkers($name, $uname, $start, $type_name, $mapping_feature) {
    ob_start();
    $this->run_importer();
    ob_end_clean();

    //were features loaded?
    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename', 'type_id']);
    $query->join('chado.cvterm', 'CV', 'CF.type_id = CV.cvterm_id');
    $query->fields('CV', ['name']);
    $query->condition('CF.uniquename', $uname);
    $result = $query->execute()->fetchObject();
    $this->assertEquals($name, $result->name);
    $this->assertEquals($uname, $result->uniquename);
    $this->assertEquals($type_name, $result->cv_name);
  }

  /**
   * @param $name
   * @param $uname
   * @param $start
   * @param $type_name
   * @param $mapping_feature
   *
   * @dataProvider marker_provider
   * @group failing
   * @group featurepos
   */
  public function testImporterPopulatesFeaturePos($name, $uname, $start, $type_name, $mapping_feature) {
    ob_start();
    $this->run_importer();
    ob_end_clean();

    //were features loaded?
    $query = db_select('chado.featurepos', 'CFP');
    $query->fields('CFP', [
      'featuremap_id',
      'map_feature_id',
      'mappos',
    ]);
    $query->join('chado.feature', 'CF', 'CF.feature_id = CFP.feature_id');
    $query->join('chado.feature', 'CFTWO', 'CFTWO.feature_id = CFP.map_feature_id');
    $query->fields('CFTWO', ['name']);
    $query->condition('CF.uniquename', $uname);
    $result = $query->execute()->fetchObject();
    $this->assertNotFalse($result);
    $this->assertEquals($start, $result->mappos);
    $this->assertEquals($mapping_feature, $result->name);
  }

  public function marker_provider() {
    return [

      ['CmSNP00665', 'CmSNP00665', '50.4', 'SNP', 'L'],
      ['CmSI0928', 'CmSI0928', '44.8', 'microsatellite', 'L'],
      ['CmSI0407', 'CmSI0407', '7.5', 'microsatellite', 'A'],
    ];

  }


  /**
   * Runs the importer directly, which is great because it bypasses the importer creating a new transaction.
   * @throws \Exception
   */
  private function run_importer() {

    $importer = new  \CmapImporter;
    // $this->create_import_job($importer);

    //Not allowed.  dbtransaction exception.
    // $this->run();

    $cv_id = $this->get_so_id();//should only allow SO terms...

    $organism = factory('chado.organism')->create();
    $analysis = factory('chado.analysis')->create();
    $featuremap = factory('chado.featuremap')->create();
    $type = factory('chado.cvterm')->create(['cv_id' => $cv_id]);
    $file = __DIR__ . '/../example/c_moll_mini.cmap';

    $importer->parse_cmap($analysis->analysis_id, $file, $featuremap->featuremap_id, $type->cvterm_id, $organism->organism_id);


  }

  /**
   * Creates an importer job.  We dont use this because it opens a db transaction.
   * @param $importer
   */
  private function create_import_job(&$importer) {

    $analysis = factory('chado.analysis')->create();
    $fmap = factory('chado.featuremap')->create();
    $run_args = [
      'analysis_id' => $analysis->analysis_id,
      'featuremap_id' => $fmap->featuremap_id,
    ];

    $importer->create($run_args, $file_details = ['file_local' => __DIR__ . '/../example/c_moll_mini.cmap']);

  }

  /**
   * get the chado cv id for the sequence ontology.
   *
   * @return mixed
   */
  private function get_so_id() {
    $query = db_select('chado.cv', 'CV');
    $query->fields('CV', ['cv_id']);
    $query->condition('CV.name', 'sequence');
    $cv_id = $query->execute()->fetchField();
    return $cv_id;
  }
}
