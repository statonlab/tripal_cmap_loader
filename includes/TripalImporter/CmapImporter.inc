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

    $form['instructions'] = [
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
</p>',
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

    $organism_options = $this->get_organism_options();

    $organism_options[0] = '--Please Select--';

    ksort($organism_options);

    $form['organism_id'] = [
      '#title' => t('Organism'),
      '#description' => t('If features are not found in Chado, they will be created.  This is the organism the features will be created for.'),
      '#type' => 'select',
      '#options' => $organism_options,
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

    $featuremap_id = $form_state['values']['featuremap_id'];

    $record = ['table' => 'featuremap', 'id' => $featuremap_id];

    $property  = ['cv_name' => 'local', 'type_name' => 'featuremap_type'];

    $typeprop = chado_get_property($record, $property);

    if (!$typeprop){
      form_set_error('featuremap_id', t('The featuremap you selected does not have the featuremap_type property set and will cause problems with the TripalMap module.  Please set the property.  See the README for more help'));

    }

    if ($form_state['values']['organism_id'] === 0) {
      form_set_error('organism_id', t('please select an organism.'));
    };
    $map_type = $form_state['values']['map_type'];
    $term_id = $this->get_so_cvterm($map_type);

    if (!$term_id) {
      form_set_error('map_type', t('Term not found in sequence ontology.'));
    }
    $form['map_type_id'] = [
      '#type' => 'value',
      '#value' => $term_id,
    ];
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
    $organism_id = $arguments['organism_id'];
    $map_type = $arguments['map_type'];
    $map_type_id = $this->get_so_cvterm($map_type);

    $cmap_file = trim($this->arguments['files'][0]['file_path']);


    $this->parse_cmap($analysis_id, $cmap_file, $featuremap_id, $map_type_id, $organism_id);
  }

  /**
   * Parses and loads a cmap file.
   *
   * @param $analysis_id
   * @param $file
   * @param $featuremap_id
   * @param $map_type_id
   * @param $organism_id
   *
   * @throws \Exception
   */
  public function parse_cmap($analysis_id, $file, $featuremap_id, $map_type_id, $organism_id) {

    $lines = file($file, FILE_SKIP_EMPTY_LINES);
    $prev_map_feature = NULL;

    $line_count = 0;
    $type_id_mapper = [];//Keep track of type IDs so we dont have to look up each time.

    foreach ($lines as $line_num => $line) {


      // split the line into it's columns
      $cols = explode("\t", $line);
      if (sizeof($cols) < 12) {
        throw new Exception("Improper number of columns on line $line_num.\n  This module expects a 12 column file.");
      }
      $map_acc = $cols[0]; //the uniquename feature for the mapping ie chromosome
      $map_name = $cols[1]; //name for map feature
      $map_start = $cols[2];
      $map_stop = $cols[3];
      $locus_acc = $cols[4];
      $feature_acc = $cols[5];
      $feature_name = $cols[6];
      $feature_aliases = $cols[7];
      $feature_start = $cols[8];
      $feature_stop = $cols[9];
      $feature_type_acc = $cols[10];
      $is_landmark = $cols[11];

      if ($map_acc == 'map_acc') {
        //skip header
        continue;
      }

      if ($line_count % 1000 === 0) {
        print("processing marker {$line_count}");
      }

      if (!isset($type_id_mapper[$feature_type_acc])) {
        $marker_type_id = $this->get_so_cvterm($feature_type_acc);
        if (!$marker_type_id) {
          throw new Exception("{$feature_type_acc} is not loaded in the sequence ontology.");
        }
        $type_id_mapper[$feature_type_acc] = $marker_type_id;
      }
      else {
        $marker_type_id = $type_id_mapper[$feature_type_acc];
      }

      $map_feature = $this->get_map_feature($map_acc, $map_name, $map_type_id, $organism_id);


      $marker_feature = $this->get_marker_feature($feature_acc, $feature_name, $feature_aliases, $marker_type_id, $organism_id);

      $locus_feature = $this->get_locus_feature($locus_acc, $marker_feature, $organism_id);

      //insert into featurepos
      if ($feature_start === $feature_stop) {
        $this->insert_featurepos($featuremap_id, $marker_feature, $map_feature, $feature_start);
      }
      //insert into featurerange
      if ($feature_start != $feature_stop) {
        $this->insert_featurerange($featuremap_id, $marker_feature, $map_feature, $feature_start, $feature_stop);
      }
      $line_count++;
    }
  }

  /**
   * Gets the map feature.  Inserts first if necessary.
   *
   * @param $map_acc
   * @param $map_name
   * @param $map_type_id
   * @param $organism_id
   *
   * @return mixed
   */
  private function get_map_feature($map_acc, $map_name, $map_type_id, $organism_id) {
    //get map feature if it already exists and return it
    $query = db_select("chado.feature", 'F');
    $query->fields('F', ['feature_id']);
    $query->condition('uniquename', $map_acc);
    $query->condition('type_id', $map_type_id);
    $map_feature_id = $query->execute()->fetchField();

    //if it doesn't exist insert and return it
    if (!$map_feature_id) {
      $insert = db_insert('chado.feature');
      $insert->fields([
        'organism_id' => $organism_id,
        'name' => $map_name,
        'uniquename' => $map_acc,
        'type_id' => $map_type_id,
      ]);
      $map_feature_id = $insert->execute();
    }
    return $map_feature_id;
  }

  /**
   * Retrieves the marker feature.  Inserts first if necessary.
   *
   * @param $feature_acc
   * @param $feature_name
   * @param $feature_aliases
   * @param $type_name
   * @param $organism_id
   *
   * @return mixed
   */
  private function get_marker_feature($feature_acc, $feature_name, $feature_aliases, $type_id, $organism_id) {

    //get feature if it already exists and return it
    //get map feature if it already exists and return it
    $query = db_select("chado.feature", 'F');
    $query->fields('F', ['feature_id']);
    $query->condition('uniquename', $feature_acc);
    $query->condition('type_id', $type_id);
    //if it doesnt, insert and return it
    $feature_id = $query->execute()->fetchField();

    if (!$feature_id) {
      $insert = db_insert('chado.feature');
      $insert->fields([
        'organism_id' => $organism_id,
        'name' => $feature_name,
        'uniquename' => $feature_acc,
        'type_id' => $type_id,
      ]);
      $feature_id = $insert->execute();
    }
    return $feature_id;
    // if it doesnt, insert and return it
  }

  /**
   * Insert into feature position.
   *
   * @param $featuremap_id
   * @param $marker_feature
   * @param $map_feature
   * @param $position
   */
  private function insert_featurepos($featuremap_id, $marker_feature, $map_feature, $position) {

    //TODO: we should deal with duplicate constraints.

    //$start_term = chado_get_cvterm(['id' => 'SIO:000943'])->cvterm_id;

    $pos_type_term = chado_get_cvterm(['id' => 'local:marker_position_type'])->cvterm_id;

    $values = [
      'featuremap_id' => $featuremap_id,
      'feature_id' => $marker_feature,
      'map_feature_id' => $map_feature,
      'mappos' => $position,
    ];

    $featurepos_record = chado_insert_record('featurepos', $values);
    //add "start" prop
    $record = ['table' => 'featurepos', 'id' => $featurepos_record];
    $property = ['type_id' => $pos_type_term, 'value' => 'start'];
    $options = ['update_if_present' => TRUE];
    chado_insert_property($record, $property, $options);
  }

  /**
   * Inserts into featurerange.  Cmap files will only have a range with a
   * discrete start/stop.  The featurerange table can support start/stop ranges
   * but cmap does not.
   *
   * @param $featuremap_id
   * @param $marker_feature
   * @param $map_feature
   * @param $feature_start
   * @param $feature_stop
   */
  private function insert_featurerange($featuremap_id, $marker_feature, $map_feature, $feature_start, $feature_stop) {


    $featurepos_record = $this->insert_featurepos($featuremap_id, $marker_feature, $map_feature, $feature_start);

    $start_term = chado_get_cvterm(['id' => 'SIO:000943'])->cvterm_id;
    //add "start" prop
    $record = ['table' => 'featurepos', 'id' => $featurepos_record];
    $property = ['type_id' => $start_term, 'value' => $feature_start];
    $options = ['update_if_present' => TRUE];
    chado_insert_property($record, $property, $options);

    //stop prop
    $stop_term = chado_get_cvterm(['id' => 'SIO:000953'])->cvterm_id;
    $record = ['table' => 'featurepos', 'id' => $featurepos_record];
    $property = ['type_id' => $stop_term, 'value' => $feature_start];
    $options = ['update_if_present' => TRUE];
    chado_insert_property($record, $property, $options);

    //TODO: QTL peak support goes here
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

  /**
   * Gets the organisms on the site in [organism_id => abbreviation].  For use
   * with select box.
   *
   * @return array
   */
  private function get_organism_options() {
    $out = [];
    $query = db_select('chado.organism', 'O');
    $query->fields('O', ['organism_id', 'abbreviation']);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) {
      $out[$result->organism_id] = $result->abbreviation;
    }
    return $out;
  }

  /**
   * Given text term name, get the sequence ontology cvterm_id.
   *
   * @param $term
   *
   * @return mixed
   */
  private function get_so_cvterm($term) {
    $query = db_select("chado.cvterm", 'CVT');
    $query->fields('CVT', ['cvterm_id']);
    $query->join('chado.cv', 'CV', 'CV.cv_id = CVT.cv_id');
    $query->condition('CVT.name', $term);
    $query->condition('CV.name', 'sequence');
    $type_id = $query->execute()->fetchField();
    return $type_id;
  }

  /**
   * Adds a parent marker locus feature.  This feature uses the SO term biological_region.
   * @param $locus_acc
   * @param $marker_feature
   * @param $organism_id
   *
   * @return mixed
   */
  private function get_locus_feature($locus_acc, $marker_feature, $organism_id) {
    $term = chado_get_cvterm(['id' => 'SO:0001411']);
    $locus_type_id = $term->cvterm_id;
    $term = chado_get_cvterm(['id' => 'OBO_REL:instance_of']);
    $relationship_type_id = $term->cvterm_id;

    //get feature if it already exists and return it
    $query = db_select("chado.feature", 'F');
    $query->fields('F', ['feature_id']);
    $query->condition('uniquename', $locus_acc);
    $query->condition('type_id', $locus_type_id);
    $locus_id = $query->execute()->fetchField();

    if ($locus_id) {

      $query = db_select('chado.feature_relationship', 'FR');
        $query->fields('FR', ['feature_relationship_id']);
      $query->condition('subject_id', $locus_id)
        ->condition('object_id', $marker_feature)
        ->condition('type_id', $relationship_type_id);
      $relationship = $query->execute()
        ->fetchField();

      if ($relationship) {
        return $locus_id;
      }

      //add the relationship before returning
      $insert = db_insert('chado.feature_relationship');
      $insert->fields([
        'subject_id' => $locus_id,
        'object_id' => $marker_feature,
        'type_id' => $relationship_type_id,
      ]);
      $insert->execute();
    }

    //if it doesn't exist insert and return it
    if (!$locus_id) {
      $insert = db_insert('chado.feature');
      $insert->fields([
        'organism_id' => $organism_id,
        'name' => $locus_acc,
        'uniquename' => $locus_acc,
        'type_id' => $locus_type_id,
      ]);
      $locus_id = $insert->execute();

      //add the relationship too

      $insert = db_insert('chado.feature_relationship');
      $insert->fields([
        'subject_id' => $locus_id,
        'object_id' => $marker_feature,
        'type_id' => $relationship_type_id,
      ]);
      $insert->execute();
    }
    return $locus_id;

  }

}
