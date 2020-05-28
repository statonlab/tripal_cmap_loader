<?php
/**
 * @file
 * Contains all field specific code outside the classes.
 */

/**
 * Implements hook_bundle_fields_info().
 *
 * This hook tells Drupal/Tripal about your new field type. Make sure you've
 * created the field (handles basic storage of your data), widget (provides
 * user UI to set data), and formatter (describes display of data on Entity
 * Page) classes. These should be located in the following directory: [your
 * module]/includes/TripalFields/[classname].inc with one file per class. Your
 * field name should be [cv name]__[cvterm name] and the classes should be
 * named [field name], [field_name]_widget, [field name]_formatter for the
 * field, widget and formatter respectively. MAKE SURE YOU'VE CLEARED THE CACHE
 * SINCE ADDING THESE FILES so Tripal magic can find them or the following will
 * fail.
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene)
 *   this hook is being called for. However, since this hook creates field
 *   types (by definition not tied to a specific Tripal Content Type (bundle))
 *   and since a field type will only be created if it doesn't already exist,
 *   this parameter doesn't actually matter. NOTE: If you do need to determine
 *   the bundle in this hook, we suggest inspecting the data_table since the
 *   label can be changed by site administrators.
 *
 * @return
 *   An array of field definitions. Each field in this array will be created if
 *   it doesn't already exist. To trigger create of fields when developing call
 *   tripal_refresh_bundle_fields() for the specific bundle.
 */
function tripal_cmap_loader_bundle_fields_info($entity_type, $bundle) {
  $fields = [];

  // Because we are expecting data housed in Chado we can use the 'data_table'
  // property of the bundle to determine if this bundle is really the one
  // we want the field to be associated with.
  if (isset($bundle->data_table) AND ($bundle->data_table == 'featuremap')) {
    // First add my term.
    tripal_insert_cvterm([
      'id' => 'local:cmap_viewer',
      'name' => 'cmap_viewer',
      'cv_name' => 'local',
      'definition' => 'A cmap visualization field.',
    ]);

    // Then describe the field defined by that term.
    $field_name = 'local__cmap_viewer';
    $fields[$field_name] = [
      'field_name' => $field_name,
      'type' => 'text',
      'cardinality' => 1,
      'locked' => FALSE,
      'module' => 'text',
      'storage' => [
        'type' => 'field_sql_storage',
      ],
      'settings' => [
        'max_length' => 10485760,
      ],
    ];
  }

  if (isset($bundle->data_table) AND ($bundle->data_table == 'organism')) {

    // First add my term.
    tripal_insert_cvterm([
      'id' => 'data:1274',
      'name' => 'Map',
      'cv_name' => 'EDAM',
      'definition' => 'A map of (typically one) DNA sequence annotated with positional or non-positional features.',
    ]);

    // Then describe the field defined by that term.
    $field_name = 'data__map';
    $field_type = 'data__map';
    $fields[$field_name] = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
    ];
  }

  return $fields;
}

/**
 * Implements hook_bundle_instances_info().
 *
 * This hook tells Drupal/Tripal to create a field instance of a given field
 * type on a specific Tripal Content type (otherwise known as the bundle). Make
 * sure to implement hook_bundle_create_fields() to create your field type
 * before trying to create an instance of that field.
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene)
 *   the field instances are being created for. Thus this hook is called once
 *   per Tripal Content Type on your site. The name of the bundle is the
 *   machine name of the type (e.g. bio_data_1) and the label of the bundle
 *   (e.g. Organism) is what you see in the interface. Since the label can be
 *   changed by site admin, we suggest checking the data_table to determine if
 *   this is the entity you want to add field instances to.
 *
 * @return
 *   An array of field instance definitions. This is where you can define the
 *   defaults for any settings you use in your field. Each entry in this array
 *   will be used to create an instance of an already existing field.
 */
function tripal_cmap_loader_bundle_instances_info($entity_type, $bundle) {
  $instances = [];
  if (isset($bundle->data_table) AND ($bundle->data_table == 'featuremap')) {
    $field_name = 'local__cmap_viewer';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'cMap',
      'description' => 'A cmap visualization field.',
      'required' => FALSE,
      'storage' => [
        'type' => 'field_sql_storage',
      ],
      'settings' => [
        'auto_attach' => FALSE,
        'text_processing' => 0,
        'term_vocabulary' => 'local',
        // The name of the term.
        'term_name' => 'cmap_viewer',
        // The unique ID (i.e. accession) of the term.
        'term_accession' => 'cmap_viewer',
      ],
      'widget' => [
        'type' => 'text_textfield',
        'required' => FALSE,
        'label' => 'Test copied',
        'settings' => [
          'size' => 64,
        ],
      ],
      'display' => [
        'default' => [
          'label' => 'hidden',
          'type' => 'local__cmap_viewer_formatter',
          'settings' => [],
        ],
      ],
    ];
  }

  if (isset($bundle->data_table) AND ($bundle->data_table == 'organism')) {
    $field_name = 'data__map';
    $field_type = 'data__map';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Map',
      'description' => 'A map of (typically one) DNA sequence annotated with positional or non-positional features.',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => FALSE,
        //'chado_table' => 'featuremap_organism',
        //'chado_column' => 'featuremap_organism_id',
        //'base_table' => 'organism',
      ],
      'widget' => [
        'type' => 'data__map_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'hidden',
          'type' => 'data__map_formatter',
          'settings' => [],
        ],
      ],
    ];
  }

  return $instances;
}
