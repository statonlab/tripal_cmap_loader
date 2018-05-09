<?php

class CmapImporter extends TripalImporter {

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Chado Cmap loader';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'cmap_loader';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = '<p>This importer will import a cmap file into Chado.  A cmap formatted map is a TSV seperated spreadhseet with 11 columns.</p>
    <p>This importer follows the Chado featuremap module.  Each Map is a single entry in featuremap. Each featuremap consists of many marker features (SSRs, QTL regions, known sequences) mapped onto another feature (A chromosome, linkage group, contig...).</p>
';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = ['txt', 'cmap'];
  //I THINK MAP IS REDUNDANT WITH ANALYSIS?

  public static $use_analysis = FALSE;
  public static $require_analysis = FALSE;


  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = 'Please provide the cmap file.';

  /**
   * The title that should appear above the upload button.
   */
  public static $upload_title = 'Cmap File';

  /**
   * Text that should appear on the button at the bottom of the importer
   * form.
   */
  public static $button_text = 'Import Cmap file';

  /**
   * @see TripalImporter::form()
   */
  public function form($form, &$form_state) {

    $form['instructions']= [
      '#markup' => '<p>This loader makes several assumptions.
<ul>
<li>Each map item has a map accession (column 0) and map name (column 1).  These correspond to the map feature\'s UNIQUE NAME and NAME, respectively.
</li>
<li>If these features are not already present in Chado via accession == uniquename, they will be created.
</li>
<li>The feature type you specify below denotes how these map features will be loaded into Chado.  Valid map feature types include linkage group, scaffold, or chromosome.
</li>
<li>feature_name and feature_accession columns correspond to the marker feature name and unique name.
</li>
<li>if the features cannot be found via accession == uniquename, they will be created using the feature type specified in the feature type accession column. </li>
</ul>
</p>'
    ];
    $featuremap_options = $this->get_featuremap_options();

    $featuremap_options[0] = '--Please Select--';

    ksort($featuremap_options);

    $form['featuremap_id'] = [
      '#title' => t('Featuremap'),
      '#description' => t('Each map corresponds to a single featuremap in Chado.  Please create a featuremap for this file before using the loader.'),
      '#type' => 'select',
      '#options' => $featuremap_options,
    ];

    $cv = tripal_get_cv(['name' => 'sequence']);
    $cv_id = $cv->cv_id;
    $form['map_type'] = [
      '#title' => t('Map Feature Type'),
      '#type' => 'textfield',
      '#description' => t('Please enter the Sequence Ontology term (e.g. contig, polypeptide, mRNA) that describes feature map type.  Valid examples include chromosome, linkage group, or contig.'),
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/$cv_id",
    ];

    return $form;
  }

  /**
   * @see TripalImporter::formSubmit()
   */
  public function formSubmit($form, &$form_state) {
    global $user;

    return '';
  }

  /**
   * @see TripalImporter::formValidate()
   */
  public function formValidate($form, &$form_state) {
    if ($form_state['values']['featuremap_id'] === 0) {
      form_set_error('featuremap_id', t('please select a featurmap (map).  If a featuremap does not exist, you must create one.'));
    };

    $map_type = $form_state['values']['map_type'];
    //check that this is a valid SO term....

  }

  /**
   * @see TripalImporter::run()
   *
   * some basic mapping terminology:
   *
   * featuremap: this refers to the whole map.
   * map_feature - this is the FEATURE that is serving as the reference.  It
   *   might be a chromosome, linkage group, or contig. feature: the feature
   *   that is being mapped onto map_feature.  It might be an SSR, QTL region,
   *   or a generic marker.
   *
   *
   */

  public function run() {
    $arguments = $this->arguments['run_args'];
    $analysis_id = $arguments['analysis_id'];
    $featuremap_id = $arguments['featuremap_id'];

    dpm($featuremap_id);
    $cmap_file = trim($this->arguments['files'][0]['file_path']);

    $map_type_id = 100; //cvterm for the map type.  IE.... chromosome? Linkage group?  let's give this variable a better name.

    $this->parse_cmap($analysis_id, $cmap_file, $featuremap_id, $map_type_id);
  }

  public function parse_cmap($analysis_id, $file, $featuremap_id, $map_type_id) {

    $lines = file($file, FILE_SKIP_EMPTY_LINES);
    $prev_map_feature = NULL;

    $line_count = 0;

    foreach ($lines as $line_num => $line) {

      // split the line into it's columns
      $cols = explode("\t", $line);
      if (sizeof($cols) < 11) {
        throw new Exception("Improper number of columns on line $line_num");
      }
      $map_acc = $cols[0]; //the uniquename feature for the mapping ie chromosome
      $map_name = $cols[1]; //name for map feature
      $map_start = $cols[2];
      $map_stop = $cols[3];
      $feature_acc = $cols[4];
      $feature_name = $cols[5];
      $feature_aliases = $cols[6];
      $feature_start = $cols[7];
      $feature_stop = $cols[8];
      $feature_type_acc = $cols[9];
      $is_landmark = $cols[10];

      if ($map_acc == 'map_acc') {
        //skip header
        continue;
      }

      if ($line_count % 1000 === 0){
        print("processing marker {$line_count}");
      }

      $map_feature = $this->get_map_feature($map_acc, $map_name, $map_type_id);

      $marker_feature = $this->get_marker_feature($feature_acc, $feature_name, $feature_aliases);

      //insert into featurepos
      $this->insert_featurepos($featuremap_id, $marker_feature, $map_feature);
      //insert into featurerange
      if ($feature_start != $feature_stop){
        $this->insert_featurerange($featuremap_id, $marker_feature, $map_feature, $feature_start, $feature_stop);
      }
      $line_count++;
    }

    print("\nSuccess");
  }


  private function get_map_feature($map_acc, $map_name, $map_type_id) {
    //get map feature if it already exists and return it
    $query = db_select("chado.feature", 'F')
      ->fields('F', ['feature_id'])
      ->condition('unique_name', $map_acc);
    //if it doesnt, insert and return it
    $map_feature_id = $query->execute()->fetchField();

    if (!$map_feature_id) {
      $insert = db_insert('chado.feature', 'F');
      $insert->fields([
        $organism_id,
        $map_name,
        $map_acc,
        NULL,
        NULL,
        NULL,
        $map_type_id,
        FALSE,
        FALSE,
      ]);
      $map_feature_id = $insert->execute();
    }

    return $map_feature_id;
  }

  private function insert_featurepos($featuremap_id, $marker_feature, $map_feature){

  }

  private function get_marker_feature($feature_acc, $feature_name, $feature_aliases) {
    //get feature if it already exists and return it

    // if it doesnt, insert and return it
  }

  private function insert_featurerange($featuremap_id, $marker_feature, $map_feature, $feature_start, $feature_stop) {

  }

  /**
   * create keyed array of featuremaps in chado.
   *
   * @return array
   */
  function get_featuremap_options() {

    $out = [];
    $query = db_select('chado.featuremap', 'FM');
    $query->fields('FM', ['featuremap_id', 'name']);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) {
      $out[$result->featuremap_id] = $result->name;
    }
    return $out;
  }

}
