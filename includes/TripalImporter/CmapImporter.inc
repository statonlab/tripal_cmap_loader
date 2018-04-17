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

    $this->parse_cmap($analysis_id, $cmap_file);
  }

  private function parse_cmap($analysis_id, $file) {

  }

}
