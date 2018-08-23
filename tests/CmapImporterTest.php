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

  /**
   * @group pass
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


  /**
   * @throws \Exception
   * @group pass
   */
  public function testImporterCreatesLocusFeature() {

    ob_start();
    $this->run_importer();
    ob_end_clean();

    $name = 'CmSNP00329';

    //the term for marker locus is biological_region
    $term = chado_get_cvterm(['id' => 'SO:0001411']);

    //were features from beginning and end of file loaded?
    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename', 'type_id']);
    $query->condition('CF.uniquename', $name);
    $query->condition('CF.type_id', $term->cvterm_id);
    $result = $query->execute()->fetchObject();
    $this->assertNotEmpty($result);
    $this->assertEquals($name, $result->uniquename);

  }

  /**
   * @group form
   * @group pass
   */
  public function testImporterFormValidator() {
    $importer = new  \CmapImporter;

    $fmap = factory('chado.featuremap')->create();
    $form_state = [];

    $form = [];
    $form = $importer->form($form, $form_state);
    $form_state['values']['featuremap_id'] = $fmap->featuremap_id;
    $form_state['values']['map_type'] = 'not a so term';
    $importer->formValidate($form, $form_state);
  }


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
   * @group pass
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
      'type_id' => $type->cvterm_id,
    ]);

    $importer->parse_cmap($analysis->analysis_id, $file, $featuremap->featuremap_id, $type->cvterm_id, $organism->organism_id);

    //Was everything loaded even though this feature existed?

    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename', 'type_id', 'residues']);
    $query->condition('CF.name', $name);
    $query->condition('CF.type_id', $type->cvterm_id);
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
   * @dataProvider marker_provider
   */
  public function testImporterCreatesMarkers($name, $uname, $start, $type_name, $mapping_feature) {
    ob_start();
    $this->run_importer();
    ob_end_clean();
    $marker_type_id = chado_get_cvterm([
      'cv_id' => [
        'name' => 'sequence',
      ],
      'name' => $type_name,
    ]);

    //were features loaded?
    $query = db_select('chado.feature', 'CF');
    $query->fields('CF', ['name', 'uniquename', 'type_id']);
    $query->join('chado.cvterm', 'CV', 'CF.type_id = CV.cvterm_id');
    $query->fields('CV', ['name']);
    $query->condition('CF.uniquename', $uname);
    $query->condition('CV.cvterm_id', $marker_type_id->cvterm_id);
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

  /**
   * @param $name
   * @param $uname
   * @param $start
   * @param $type_name
   * @param $mapping_feature
   * @group featurepos
   * @dataProvider marker_provider
   */
  public function testImporterAddsPropsForFeaturepos($name, $uname, $start, $type_name, $mapping_feature) {

    ob_start();
    $this->run_importer();

    ob_end_clean();
    
       $marker_type_id = chado_get_cvterm([
         'cv_id' => [
           'name' => 'sequence',
         ],
         'name' => $type_name,
       ]);

    //was featureposprop loaded?
    $query = db_select('chado.featurepos', 'CFP');
    $query->join('chado.feature', 'CF', 'CF.feature_id = CFP.feature_id');
    $query->join('chado.featureposprop', 'FPP', 'CFP.featurepos_id = FPP.featurepos_id');
    $query->fields('FPP', ['value']);
    $query->condition('CF.uniquename', $uname);
    $query->condition('CF.type_id', $marker_type_id->cvterm_id);
    $value = $query->execute()->fetchField();
    $this->assertNotNull($value);
    $this->assertEquals($start, $value);

  }

  public function marker_provider() {
    return [
      ['CmSNP00665', 'CmSNP00665', '50.4', 'SNP', 'L'],
      ['CmSI0928', 'CmSI0928', '44.8', 'microsatellite', 'L'],
      ['CmSI0407', 'CmSI0407', '7.5', 'microsatellite', 'A'],
    ];

  }


  /**
   * Runs the importer directly, which is great because it bypasses the
   * importer creating a new transaction.
   *
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
