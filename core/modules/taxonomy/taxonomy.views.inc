<?php

/**
 * @file
 * Provide views data and handlers for taxonomy.module.
 *
 * @ingroup views_module_handlers
 */

use Drupal\Core\Entity\DatabaseStorageController;
use Drupal\field\FieldInterface;

/**
 * Implements hook_views_data().
 */
function taxonomy_views_data() {
  $data = array();

 // taxonomy_term_data table

  $data['taxonomy_term_data']['table']['group']  = t('Taxonomy term');
  $data['taxonomy_term_data']['table']['base'] = array(
    'field' => 'tid',
    'title' => t('Term'),
    'help' => t('Taxonomy terms are attached to nodes.'),
    'access query tag' => 'term_access',
  );
  $data['taxonomy_term_data']['table']['entity type'] = 'taxonomy_term';
  $data['taxonomy_term_data']['table']['wizard_id'] = 'taxonomy_term';


  // The term data table
  $data['taxonomy_term_data']['table']['join'] = array(
    // This is provided for many_to_one argument
    'taxonomy_index' => array(
      'field' => 'tid',
      'left_field' => 'tid',
    ),
  );

  // tid field
  $data['taxonomy_term_data']['tid'] = array(
    'title' => t('Term ID'),
    'help' => t('The tid of a taxonomy term.'),
    'field' => array(
      'id' => 'numeric',
    ),
    'sort' => array(
      'id' => 'standard',
    ),
    'argument' => array(
      'id' => 'taxonomy',
      'name field' => 'name',
      'zero is null' => TRUE,
    ),
    'filter' => array(
      'title' => t('Term'),
      'help' => t('Taxonomy term chosen from autocomplete or select widget.'),
      'id' => 'taxonomy_index_tid',
      'hierarchy table' => 'taxonomy_term_hierarchy',
      'numeric' => TRUE,
    ),
  );

  // raw tid field
  $data['taxonomy_term_data']['tid_raw'] = array(
    'title' => t('Term ID'),
    'help' => t('The tid of a taxonomy term.'),
    'real field' => 'tid',
    'filter' => array(
      'id' => 'numeric',
      'allow empty' => TRUE,
    ),
  );

  $data['taxonomy_term_data']['tid_representative'] = array(
    'relationship' => array(
      'title' => t('Representative node'),
      'label'  => t('Representative node'),
      'help' => t('Obtains a single representative node for each term, according to a chosen sort criterion.'),
      'id' => 'groupwise_max',
      'relationship field' => 'tid',
      'outer field' => 'taxonomy_term_data.tid',
      'argument table' => 'taxonomy_term_data',
      'argument field' =>  'tid',
      'base'   => 'node',
      'field'  => 'nid',
      'relationship' => 'node:term_node_tid'
    ),
  );

  // Term name field
  $data['taxonomy_term_data']['name'] = array(
    'title' => t('Name'),
    'help' => t('The taxonomy term name.'),
    'field' => array(
      'id' => 'taxonomy',
    ),
    'sort' => array(
      'id' => 'standard',
    ),
    'filter' => array(
      'id' => 'string',
      'help' => t('Taxonomy term name.'),
    ),
    'argument' => array(
      'id' => 'string',
      'help' => t('Taxonomy term name.'),
      'many to one' => TRUE,
      'empty field name' => t('Uncategorized'),
    ),
  );

  // taxonomy weight
  $data['taxonomy_term_data']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The term weight field'),
    'field' => array(
      'id' => 'numeric',
    ),
    'sort' => array(
      'id' => 'standard',
    ),
    'filter' => array(
      'id' => 'numeric',
    ),
    'argument' => array(
      'id' => 'numeric',
    ),
  );

  // Term description
  $data['taxonomy_term_data']['description'] = array(
    'title' => t('Term description'),
    'help' => t('The description associated with a taxonomy term.'),
    'field' => array(
      'id' => 'markup',
      'format' => array('field' => 'format'),
      'click sortable' => FALSE,
    ),
    'filter' => array(
      'id' => 'string',
    ),
  );

  // Term vocabulary
  $data['taxonomy_term_data']['vid'] = array(
    'title' => t('Vocabulary'),
    'help' => t('Filter the results of "Taxonomy: Term" to a particular vocabulary.'),
    'filter' => array(
      'id' => 'bundle',
    ),
  );

  $data['taxonomy_term_data']['langcode'] = array(
    'title' => t('Language'), // The item it appears as on the UI,
    'help' => t('Language of the term'),
    'field' => array(
      'id' => 'taxonomy_term_language',
    ),
    'sort' => array(
      'id' => 'standard',
    ),
    'filter' => array(
      'id' => 'language',
    ),
    'argument' => array(
      'id' => 'language',
    ),
  );

  // Link to edit the term
  $data['taxonomy_term_data']['edit_term'] = array(
    'field' => array(
      'title' => t('Term edit link'),
      'help' => t('Provide a simple link to edit the term.'),
      'id' => 'term_link_edit',
      'click sortable' => FALSE,
    ),
  );

  $data['taxonomy_term_data']['changed'] = array(
    'title' => t('Updated date'),
    'help' => t('The date the term was last updated.'),
    'field' => array(
      'id' => 'date',
    ),
    'sort' => array(
      'id' => 'date'
    ),
    'filter' => array(
      'id' => 'date',
    ),
  );

  $data['taxonomy_term_data']['changed_fulldate'] = array(
    'title' => t('Updated date'),
    'help' => t('Date in the form of CCYYMMDD.'),
    'argument' => array(
      'field' => 'changed',
      'id' => 'date_fulldate',
    ),
  );

  $data['taxonomy_term_data']['changed_year_month'] = array(
    'title' => t('Updated year + month'),
    'help' => t('Date in the form of YYYYMM.'),
    'argument' => array(
      'field' => 'changed',
      'id' => 'date_year_month',
    ),
  );

  $data['taxonomy_term_data']['changed_year'] = array(
    'title' => t('Updated year'),
    'help' => t('Date in the form of YYYY.'),
    'argument' => array(
      'field' => 'changed',
      'id' => 'date_year',
    ),
  );

  $data['taxonomy_term_data']['changed_month'] = array(
    'title' => t('Updated month'),
    'help' => t('Date in the form of MM (01 - 12).'),
    'argument' => array(
      'field' => 'changed',
      'id' => 'date_month',
    ),
  );

  $data['taxonomy_term_data']['changed_day'] = array(
    'title' => t('Updated day'),
    'help' => t('Date in the form of DD (01 - 31).'),
    'argument' => array(
      'field' => 'changed',
      'id' => 'date_day',
    ),
  );

  $data['taxonomy_term_data']['changed_week'] = array(
    'title' => t('Updated week'),
    'help' => t('Date in the form of WW (01 - 53).'),
    'argument' => array(
      'field' => 'changed',
      'id' => 'date_week',
    ),
  );

  // Content translation field.
  if (\Drupal::moduleHandler()->moduleExists('content_translation')) {
    $data['taxonomy_term_data']['translation_link'] = array(
      'title' => t('Translation link'),
      'help' => t('Provide a link to the translations overview for taxonomy terms.'),
      'field' => array(
        'id' => 'content_translation_link',
      ),
    );
  }

  // taxonomy_index table

  $data['taxonomy_index']['table']['group']  = t('Taxonomy term');

  $data['taxonomy_index']['table']['join'] = array(
    'taxonomy_term_data' => array(
      // links directly to taxonomy_term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links directly to node via nid
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'taxonomy_term_hierarchy' => array(
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['taxonomy_index']['nid'] = array(
    'title' => t('Content with term'),
    'help' => t('Relate all content tagged with a term.'),
    'relationship' => array(
      'id' => 'standard',
      'base' => 'node',
      'base field' => 'nid',
      'label' => t('node'),
      'skip base' => 'node',
    ),
  );

  // @todo This stuff needs to move to a node field since
  // really it's all about nodes.
  // tid field
  $data['taxonomy_index']['tid'] = array(
    'group' => t('Content'),
    'title' => t('Has taxonomy term ID'),
    'help' => t('Display content if it has the selected taxonomy terms.'),
    'argument' => array(
      'id' => 'taxonomy_index_tid',
      'name table' => 'taxonomy_term_data',
      'name field' => 'name',
      'empty field name' => t('Uncategorized'),
      'numeric' => TRUE,
      'skip base' => 'taxonomy_term_data',
    ),
    'filter' => array(
      'title' => t('Has taxonomy term'),
      'id' => 'taxonomy_index_tid',
      'hierarchy table' => 'taxonomy_term_hierarchy',
      'numeric' => TRUE,
      'skip base' => 'taxonomy_term_data',
      'allow empty' => TRUE,
    ),
  );

  // term_hierarchy table

  $data['taxonomy_term_hierarchy']['table']['group']  = t('Taxonomy term');

  $data['taxonomy_term_hierarchy']['table']['join'] = array(
    'taxonomy_term_hierarchy' => array(
      // links to self through left.parent = right.tid (going down in depth)
      'left_field' => 'tid',
      'field' => 'parent',
    ),
    'taxonomy_term_data' => array(
      // links directly to taxonomy_term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['taxonomy_term_hierarchy']['parent'] = array(
    'title' => t('Parent term'),
    'help' => t('The parent term of the term. This can produce duplicate entries if you are using a vocabulary that allows multiple parents.'),
    'relationship' => array(
      'base' => 'taxonomy_term_data',
      'field' => 'parent',
      'label' => t('Parent'),
    ),
    'filter' => array(
      'help' => t('Filter the results of "Taxonomy: Term" by the parent pid.'),
      'id' => 'numeric',
    ),
    'argument' => array(
      'help' => t('The parent term of the term.'),
      'id' => 'taxonomy',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function taxonomy_views_data_alter(&$data) {
  $data['node']['term_node_tid'] = array(
    'title' => t('Taxonomy terms on node'),
    'help' => t('Relate nodes to taxonomy terms, specifiying which vocabulary or vocabularies to use. This relationship will cause duplicated records if there are multiple terms.'),
    'relationship' => array(
      'id' => 'node_term_data',
      'label' => t('term'),
      'base' => 'taxonomy_term_data',
    ),
    'field' => array(
      'title' => t('All taxonomy terms'),
      'help' => t('Display all taxonomy terms associated with a node from specified vocabularies.'),
      'id' => 'taxonomy_index_tid',
      'no group by' => TRUE,
      'click sortable' => FALSE,
    ),
  );

  $data['node']['term_node_tid_depth'] = array(
    'help' => t('Display content if it has the selected taxonomy terms, or children of the selected terms. Due to additional complexity, this has fewer options than the versions without depth.'),
    'real field' => 'nid',
    'argument' => array(
      'title' => t('Has taxonomy term ID (with depth)'),
      'id' => 'taxonomy_index_tid_depth',
      'accept depth modifier' => TRUE,
    ),
    'filter' => array(
      'title' => t('Has taxonomy terms (with depth)'),
      'id' => 'taxonomy_index_tid_depth',
    ),
  );

  $data['node']['term_node_tid_depth_modifier'] = array(
    'title' => t('Has taxonomy term ID depth modifier'),
    'help' => t('Allows the "depth" for Taxonomy: Term ID (with depth) to be modified via an additional contextual filter value.'),
    'argument' => array(
      'id' => 'taxonomy_index_tid_depth_modifier',
    ),
  );
}

/**
 * Implements hook_field_views_data().
 *
 * Views integration for taxonomy_term_reference fields. Adds a term relationship to the default
 * field data.
 *
 * @see field_views_field_default_views_data()
 */
function taxonomy_field_views_data(FieldInterface $field) {
  $data = field_views_field_default_views_data($field);
  foreach ($data as $table_name => $table_data) {
    foreach ($table_data as $field_name => $field_data) {
      if (isset($field_data['filter']) && $field_name != 'delta') {
        $data[$table_name][$field_name]['filter']['id'] = 'taxonomy_index_tid';
        $data[$table_name][$field_name]['filter']['vocabulary'] = $field['settings']['allowed_values'][0]['vocabulary'];
      }
    }

    // Add the relationship only on the tid field.
    $data[$table_name][$field['field_name'] . '_target_id']['relationship'] = array(
      'id' => 'standard',
      'base' => 'taxonomy_term_data',
      'base field' => 'tid',
      'label' => t('term from !field_name', array('!field_name' => $field['field_name'])),
    );

  }

  return $data;
}

/**
 * Implements hook_field_views_data_views_data_alter().
 *
 * Views integration to provide reverse relationships on term references.
 */
function taxonomy_field_views_data_views_data_alter(array &$data, FieldInterface $field) {
  $entity_type = $field['entity_type'];
  $entity_info = entity_get_info($entity_type);
  $pseudo_field_name = 'reverse_' . $field['field_name'] . '_' . $entity_type;

  list($label,) = field_views_field_label($entity_type, $field['field_name']);
  $entity_label = ($entity_info['label'] == t('Node')) ? t('Content') : $entity_info['label'];

  $data['taxonomy_term_data'][$pseudo_field_name]['relationship'] = array(
    'title' => t('@entity using @field', array('@entity' => $entity_label, '@field' => $label)),
    'help' => t('Relate each @entity with a @field set to the term.', array('@entity' => $entity_label, '@field' => $label)),
    'id' => 'entity_reverse',
    'field_name' => $field['field_name'],
    'entity_type' => $field['entity_type'],
    'field table' => DatabaseStorageController::_fieldTableName($field),
    'field field' => $field['field_name'] . '_target_id',
    'base' => $entity_info['base_table'],
    'base field' => $entity_info['entity_keys']['id'],
    'label' => t('!field_name', array('!field_name' => $field['field_name'])),
    'join_extra' => array(
      0 => array(
        'field' => 'deleted',
        'value' => 0,
        'numeric' => TRUE,
      ),
    ),
  );
}

/**
 * Helper function to set a breadcrumb for taxonomy.
 */
function views_taxonomy_set_breadcrumb(&$breadcrumb, &$argument) {
  if (empty($argument->options['set_breadcrumb'])) {
    return;
  }

  $args = $argument->view->args;
  $parents = taxonomy_get_parents_all($argument->argument);
  foreach (array_reverse($parents) as $parent) {
    // Unfortunately parents includes the current argument. Skip.
    if ($parent->id() == $argument->argument) {
      continue;
    }
    if (!empty($argument->options['use_taxonomy_term_path'])) {
      $path = $parent->uri();
      $path = $path['path'];
    }
    else {
      $args[$argument->position] = $parent->id();
      $path = $argument->view->getUrl($args);
    }
    $breadcrumb[$path] = check_plain($parent->label());
  }
}
