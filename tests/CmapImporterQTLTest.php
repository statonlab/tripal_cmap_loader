<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

require_once(__DIR__ . '/../includes/TripalImporter/CmapImporter.inc');


class CmapImporterQTLTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
   use DBTransaction;


  /**
   * @group qtl
   * @throws \Exception
   */
  public function testImporterInsertsDifferentStartAndStops() {

    $importer = new  \CmapImporter;
    $this->assertNotNull($importer);

    $organism = factory('chado.organism')->create();
    $analysis = factory('chado.analysis')->create();
    $featuremap = factory('chado.featuremap')->create();
    $type = factory('chado.cvterm')->create();
    $file = __DIR__ . '/../example/guess_qtl.cmap';


    $importer->parse_cmap($analysis->analysis_id, $file, $featuremap->featuremap_id, $type->cvterm_id, $organism->organism_id);

    $stop_term = chado_get_cvterm(['id' => 'SIO:000953'])->cvterm_id;

    $start_term = chado_get_cvterm(['id' => 'SIO:000943'])->cvterm_id;

    $featureposse = db_select('chado.featurepos', 'fp')->fields('fp')->condition('fp.featuremap_id', $featuremap->featuremap_id)->execute();

    foreach ($featureposse as $featurepos) {

      $startprop = db_select('chado.featureposprop', 'fpp')->fields('fpp')->condition('type_id', $start_term)->condition('featurepos_id', $featurepos->featurepos_id)->execute()->fetchObject();


      $stopprop = db_select('chado.featureposprop', 'fpp')->fields('fpp')->condition('type_id', $stop_term)->condition('featurepos_id', $featurepos->featurepos_id)->execute()->fetchObject();

      $this->assertNotNull($stopprop);
      $this->assertNotNull($startprop);

      $this->assertObjectHasAttribute('value', $stopprop);
      $this->assertObjectHasAttribute('value', $startprop);
      $difference = (float) $stopprop->value -  (float) $startprop->value;


      $this->assertGreaterThan(0, $difference);
    }

  }
}
