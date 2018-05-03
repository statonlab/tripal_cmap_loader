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
  public static $description = 'Import a cmap file into Chado';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = array('txt');


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
    // set the defaults
    $example = NULL;
    $map_type = NULL;

    // if we are re constructing the form from a failed validation or ajax callback
    // then use the $form_state['values'] values
    if (array_key_exists('values', $form_state)) {
      $map_type = $form_state['values']['map_type'];
      $example = $form_state['values']['example'];

    }


    $cv = tripal_get_cv(array('name' => 'sequence'));
    $cv_id = $cv->cv_id;
    $form['map_type'] = array(
      '#title' => t('Map Type'),
      '#type' => 'textfield',
      '#description' => t('Please enter the Sequence Ontology term (e.g. contig, polypeptide, mRNA) that describes the map type.'),
      '#default_value' => $map_type,
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/$cv_id",
    );

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
    // trim character fields
//    $form_state['values']['query_uniquename']  = trim($form_state['values']['query_uniquename']);
//    $form_state['values']['query_type']        = trim($form_state['values']['query_type']);
//    $form_state['values']['query_re']          = trim($form_state['values']['query_re']);

    // check the regular expression to make sure it is valid
    set_error_handler(function () {
    }, E_WARNING);
    $result = preg_match("/" . $form_state['values']['query_re'] . "/", NULL);
    restore_error_handler();
    if ($result === FALSE) {
      form_set_error('query_re', 'Invalid regular expression.');
    }
  }

  /**
   * @see TripalImporter::run()
   */
  public function run() {
    $arguments = $this->arguments['run_args'];
    $analysis_id = $arguments['analysis_id'];
    $cmap_file = trim($this->arguments['files'][0]['file_path']);
//    $parsego = $arguments['parsego'];
//    $query_re = $arguments['query_re'];
//    $query_type = $arguments['query_type'];

    $unit_type = 27; //CVTERM for the unit type.  IE centimorgans

    $map_type_id = 100; //cvterm for the map type.  IE.... chromosome? Linkage group?  let's give this variable a better name.
    $featuremap_id = $this->create_featuremap($unit_type);

    $this->parse_cmap($analysis_id, $cmap_file, $featuremap_id, $map_type_id);
  }

  private function parse_cmap($analysis_id, $file, $featuremap_id, $map_type_id) {

    $lines = file($file, FILE_SKIP_EMPTY_LINES);
    $prev_map_feature = null;

    foreach ($lines as $line_num => $line) {

      // split the line into it's columns
      $cols = explode("\t", $line);
      if (sizeof($cols) < 11) {
        throw new Exception("Improper number of columns on line $line_num");
      }
      $map_acc = $cols[0]; //the uniquename feature for the map ie chromosome
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

      $map_feature = $this->get_map_feature($map_acc, $map_name, $map_type_id);

      $marker_feature = $this->get_marker_feature($feature_acc, $feature_name, $feature_aliases);

      //insert into featurepos
$this->insert_featurepos($featuremap_id, $feature_id, $map_feature_id);
      //insert into featurerange



    }

  }


  private function get_map_feature($map_acc, $map_name, $map_type_id) {
    //get map feature if it already exists and return it
      $query = db_select("chado.feature", 'F')
      ->fields('F', ['feature_id'])
      ->condition('unique_name', $map_acc);
    //if it doesnt, insert and return it
    $map_feature_id = $query->execute()->fetchField();

    if (!$map_feature_id){
      $insert = db_insert('chado.feature', 'F');
      $insert->fields([$organism_id,$map_name, $map_acc, NULL, NULL,NULL, $map_type_id, FALSE, FALSE]);
$map_feature_id = $insert->execute();
    }

    return $map_feature_id;
  }

  private function get_marker_feature($feature_acc, $feature_name, $feature_aliases) {
    //get feature if it already exists and return it

    // if it doesnt, insert and return it
  }

  private function create_featuremap($unit_type) {

  }

}
