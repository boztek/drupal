<?php

/**
 * @file
 * Provide Views data and handlers for field.module.
 *
 * @ingroup views_module_handlers
 */

use Drupal\Component\Utility\NestedArray;
use Drupal\Core\Entity\DatabaseStorageController;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Drupal\field\FieldInterface;

/**
 * Implements hook_views_data().
 *
 * Field modules can implement hook_field_views_data() to override the default
 * behavior for adding fields.
 */
function field_views_data() {
  $data = array();
  $module_handler = \Drupal::moduleHandler();

  foreach (field_info_fields() as $field) {
    if (_field_views_is_sql_entity_type($field)) {
      $result = (array) $module_handler->invoke($field['module'], 'field_views_data', array($field));
      if (empty($result)) {
        $result = field_views_field_default_views_data($field);
      }
      $module_handler->alter('field_views_data', $result, $field);

      if (is_array($result)) {
        $data = NestedArray::mergeDeep($result, $data);
      }
    }
  }

  return $data;
}

/**
 * Implements hook_views_data_alter().
 *
 * Field modules can implement hook_field_views_data_views_data_alter() to
 * alter the views data on a per field basis. This is weirdly named so as
 * not to conflict with the drupal_alter('field_views_data') in
 * field_views_data.
 */
function field_views_data_alter(&$data) {
  foreach (field_info_fields() as $field) {
    if (_field_views_is_sql_entity_type($field)) {
      $function = $field['module'] . '_field_views_data_views_data_alter';
      if (function_exists($function)) {
        $function($data, $field);
      }
    }
  }
}

/**
 * Determines whether the entity type the field appears in is SQL based.
 *
 * @param \Drupal\field\FieldInterface $field
 *   The field definition.
 *
 * @return bool
 *   True if the entity type uses DatabaseStorageController.
 */
function _field_views_is_sql_entity_type(FieldInterface $field) {
  $entity_manager = \Drupal::entityManager();
  try {
    if ($entity_manager->getStorageController($field->entity_type) instanceof DatabaseStorageController) {
      return TRUE;
    }
  }
  catch (\InvalidArgumentException $e) {
    // Disabled entity type, nothing to do.
  }
}

/**
 * Returns the label of a certain field.
 *
 * Therefore it looks up in all bundles to find the most used instance.
 */
function field_views_field_label($entity_type, $field_name) {
  $label_counter = array();
  $all_labels = array();
  // Count the amount of instances per label per field.
  $instances = field_info_instances($entity_type);
  foreach ($instances as $bundle => $bundle_instances) {
    if (isset($bundle_instances[$field_name])) {
      $instance = $bundle_instances[$field_name];
      $label_counter[$instance->label] = isset($label_counter[$instance->label]) ? ++$label_counter[$instance->label] : 1;
      $all_labels[$instance->label] = TRUE;
    }
  }
  if (empty($label_counter)) {
    return array($field_name, $all_labels);
  }
  // Sort the field labels by it most used label and return the most used one.
  arsort($label_counter);
  $label_counter = array_keys($label_counter);
  return array($label_counter[0], $all_labels);
}

/**
 * Default views data implementation for a field.
 *
 * @param \Drupal\field\FieldInterface $field
 *   The field definition.
 *
 * @return array
 *   The default views data for the field.
 */
function field_views_field_default_views_data(FieldInterface $field) {
  $data = array();

  // Check the field type is available.
  if (!\Drupal::service('plugin.manager.entity.field.field_type')->getDefinition($field['type'])) {
    return $data;
  }
  // Check the field has instances.
  if (empty($field['bundles'])) {
    return $data;
  }

  $field_name = $field['field_name'];

  // Grab information about the entity type tables.
  $entity_manager = \Drupal::entityManager();
  $entity_type = $field->entity_type;
  $entity_info = $entity_manager->getDefinition($entity_type);
  if (!isset($entity_info['base_table'])) {
    return $data;
  }
  $entity_table = $entity_info['base_table'];
  $entity_tables = array($entity_table => $entity_type);
  $supports_revisions = !empty($entity_info['entity_keys']['revision']) && !empty($entity_info['revision_table']);
  if ($supports_revisions) {
    $entity_revision_table = $entity_info['revision_table'];
    $entity_tables[$entity_revision_table] = $entity_type;
  }

  // Description of the field tables.
  $field_tables = array(
    EntityStorageControllerInterface::FIELD_LOAD_CURRENT => array(
      'table' => DatabaseStorageController::_fieldTableName($field),
      'alias' => "{$entity_type}__{$field->name}",
    ),
  );
  if ($supports_revisions) {
    $field_tables[EntityStorageControllerInterface::FIELD_LOAD_REVISION] = array(
      'table' => DatabaseStorageController::_fieldRevisionTableName($field),
      'alias' => "{$entity_type}_revision__{$field->name}",
    );
  }

  // Build the relationships between the field table and the entity tables.
  $table_alias = $field_tables[EntityStorageControllerInterface::FIELD_LOAD_CURRENT]['alias'];
  $data[$table_alias]['table']['join'][$entity_table] = array(
    'left_field' => $entity_info['entity_keys']['id'],
    'field' => 'entity_id',
    'extra' => array(
      array('field' => 'deleted', 'value' => 0, 'numeric' => TRUE),
    ),
  );
  if ($supports_revisions) {
    $table_alias = $field_tables[EntityStorageControllerInterface::FIELD_LOAD_REVISION]['alias'];
    $data[$table_alias]['table']['join'][$entity_revision_table] = array(
      'left_field' => $entity_info['entity_keys']['revision'],
      'field' => 'revision_id',
      'extra' => array(
        array('field' => 'deleted', 'value' => 0, 'numeric' => TRUE),
      ),
    );
  }

  // Override Node to Content.
  $group_name = ($entity_info['label'] == t('Node')) ? t('Content') : $entity_info['label'];
  // Get the list of bundles the field appears in.
  $bundles_names = $field->getBundles();
  // Build the list of additional fields to add to queries.
  $add_fields = array('delta', 'langcode', 'bundle');
  foreach (array_keys($field['columns']) as $column) {
    $add_fields[] = DatabaseStorageController::_fieldColumnName($field, $column);
  }
  // Determine the label to use for the field. We don't have a label available
  // at the field level, so we just go through all instances and take the one
  // which is used the most frequently.
  list($label, $all_labels) = field_views_field_label($entity_type, $field_name);

  // Expose data for the field as a whole.
  foreach ($field_tables as $type => $table_info) {
    $table = $table_info['table'];
    $table_alias = $table_info['alias'];

    if ($type == EntityStorageControllerInterface::FIELD_LOAD_CURRENT) {
      $group = $group_name;
      $field_alias = $field_name;
    }
    else {
      $group = t('@group (historical data)', array('@group' => $group_name));
      $field_alias = $field_name . '-revision_id';
    }

    $data[$table_alias][$field_alias] = array(
      'group' => $group,
      'title' => $label,
      'title short' => $label,
      'help' =>  t('Appears in: @bundles.', array('@bundles' => implode(', ', $bundles_names))),
    );

    // Go through and create a list of aliases for all possible combinations of
    // entity type + name.
    $aliases = array();
    $also_known = array();
    foreach ($all_labels as $label_name => $true) {
      if ($type == EntityStorageControllerInterface::FIELD_LOAD_CURRENT) {
        if ($label != $label_name) {
          $aliases[] = array(
            'base' => $entity_table,
            'group' => $group_name,
            'title' => $label_name,
            'help' => t('This is an alias of @group: @field.', array('@group' => $group_name, '@field' => $label)),
          );
          $also_known[] = t('@group: @field', array('@group' => $group_name, '@field' => $label_name));
        }
      }
      elseif ($supports_revisions && $label != $label_name) {
        $aliases[] = array(
          'base' => $table,
          'group' => t('@group (historical data)', array('@group' => $group_name)),
          'title' => $label_name,
          'help' => t('This is an alias of @group: @field.', array('@group' => $group_name, '@field' => $label)),
        );
        $also_known[] = t('@group (historical data): @field', array('@group' => $group_name, '@field' => $label_name));
      }
    }
    if ($aliases) {
      $data[$table_alias][$field_alias]['aliases'] = $aliases;
      $data[$table_alias][$field_alias]['help'] .= ' ' . t('Also known as: !also.', array('!also' => implode(', ', $also_known)));
    }

    $keys = array_keys($field['columns']);
    $real_field = reset($keys);
    $data[$table_alias][$field_alias]['field'] = array(
      'table' => $table,
      'id' => 'field',
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      // Provide a real field for group by.
      'real field' => $field_alias . '_' . $real_field,
      'additional fields' => $add_fields,
      'entity_tables' => $entity_tables,
      // Default the element type to div, let the UI change it if necessary.
      'element type' => 'div',
      'is revision' => $type == EntityStorageControllerInterface::FIELD_LOAD_REVISION,
    );
  }

  // Expose data for each field property individually.
  foreach ($field['columns'] as $column => $attributes) {
    $allow_sort = TRUE;

    // Identify likely filters and arguments for each column based on field type.
    switch ($attributes['type']) {
      case 'int':
      case 'mediumint':
      case 'tinyint':
      case 'bigint':
      case 'serial':
      case 'numeric':
      case 'float':
        $filter = 'numeric';
        $argument = 'numeric';
        $sort = 'standard';
        break;
      case 'text':
      case 'blob':
        // It does not make sense to sort by blob or text.
        $allow_sort = FALSE;
      default:
        $filter = 'string';
        $argument = 'string';
        $sort = 'standard';
        break;
    }

    if (count($field['columns']) == 1 || $column == 'value') {
      $title = t('@label (!name)', array('@label' => $label, '!name' => $field_name));
      $title_short = $label;
    }
    else {
      $title = t('@label (!name:!column)', array('@label' => $label, '!name' => $field_name, '!column' => $column));
      $title_short = t('@label:!column', array('@label' => $label, '!column' => $column));
    }

    // Expose data for the property.
    foreach ($field_tables as $type => $table_info) {
      $table = $table_info['table'];
      $table_alias = $table_info['alias'];

      if ($type == EntityStorageControllerInterface::FIELD_LOAD_CURRENT) {
        $group = $group_name;
      }
      else {
        $group = t('@group (historical data)', array('@group' => $group_name));
      }
      $column_real_name = DatabaseStorageController::_fieldColumnName($field, $column);

      // Load all the fields from the table by default.
      $field_sql_schema = DatabaseStorageController::_fieldSqlSchema($field);
      $additional_fields = array_keys($field_sql_schema[$table]['fields']);

      $data[$table_alias][$column_real_name] = array(
        'group' => $group,
        'title' => $title,
        'title short' => $title_short,
        'help' =>  t('Appears in: @bundles.', array('@bundles' => implode(', ', $bundles_names))),
      );

      // Go through and create a list of aliases for all possible combinations of
      // entity type + name.
      $aliases = array();
      $also_known = array();
      foreach ($all_labels as $label_name => $true) {
        if ($label != $label_name) {
          if (count($field['columns']) == 1 || $column == 'value') {
            $alias_title = t('@label (!name)', array('@label' => $label_name, '!name' => $field_name));
          }
          else {
            $alias_title = t('@label (!name:!column)', array('@label' => $label_name, '!name' => $field_name, '!column' => $column));
          }
          $aliases[] = array(
            'group' => $group_name,
            'title' => $alias_title,
            'help' => t('This is an alias of @group: @field.', array('@group' => $group_name, '@field' => $title)),
          );
          $also_known[] = t('@group: @field', array('@group' => $group_name, '@field' => $title));
        }
      }
      if ($aliases) {
        $data[$table_alias][$column_real_name]['aliases'] = $aliases;
        $data[$table_alias][$column_real_name]['help'] .= ' ' . t('Also known as: !also.', array('!also' => implode(', ', $also_known)));
      }

      $data[$table_alias][$column_real_name]['argument'] = array(
        'field' => $column_real_name,
        'table' => $table,
        'id' => $argument,
        'additional fields' => $additional_fields,
        'field_name' => $field_name,
        'entity_type' => $entity_type,
        'empty field name' => t('- No value -'),
      );
      $data[$table_alias][$column_real_name]['filter'] = array(
        'field' => $column_real_name,
        'table' => $table,
        'id' => $filter,
        'additional fields' => $additional_fields,
        'field_name' => $field_name,
        'entity_type' => $entity_type,
        'allow empty' => TRUE,
      );
      if (!empty($allow_sort)) {
        $data[$table_alias][$column_real_name]['sort'] = array(
          'field' => $column_real_name,
          'table' => $table,
          'id' => $sort,
          'additional fields' => $additional_fields,
          'field_name' => $field_name,
          'entity_type' => $entity_type,
        );
      }

      // Expose additional delta column for multiple value fields.
      if ($field['cardinality'] > 1 || $field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
        $title_delta = t('@label (!name:delta)', array('@label' => $label, '!name' => $field_name));
        $title_short_delta = t('@label:delta', array('@label' => $label));

        $data[$table_alias]['delta'] = array(
          'group' => $group,
          'title' => $title_delta,
          'title short' => $title_short_delta,
          'help' => t('Delta - Appears in: @bundles.', array('@bundles' => implode(', ', $bundles_names))),
        );
        $data[$table_alias]['delta']['field'] = array(
          'id' => 'numeric',
        );
        $data[$table_alias]['delta']['argument'] = array(
          'field' => 'delta',
          'table' => $table,
          'id' => 'numeric',
          'additional fields' => $additional_fields,
          'empty field name' => t('- No value -'),
          'field_name' => $field_name,
          'entity_type' => $entity_type,
        );
        $data[$table_alias]['delta']['filter'] = array(
          'field' => 'delta',
          'table' => $table,
          'id' => 'numeric',
          'additional fields' => $additional_fields,
          'field_name' => $field_name,
          'entity_type' => $entity_type,
          'allow empty' => TRUE,
        );
        $data[$table_alias]['delta']['sort'] = array(
          'field' => 'delta',
          'table' => $table,
          'id' => 'standard',
          'additional fields' => $additional_fields,
          'field_name' => $field_name,
          'entity_type' => $entity_type,
        );
      }

      // Expose additional language column for translatable fields.
      if (!empty($field['translatable'])) {
        $title_language = t('@label (!name:language)', array('@label' => $label, '!name' => $field_name));
        $title_short_language = t('@label:language', array('@label' => $label));

        $data[$table_alias]['language'] = array(
          'group' => $group,
          'title' => $title_language,
          'title short' => $title_short_language,
          'help' => t('Language - Appears in: @bundles.', array('@bundles' => implode(', ', $bundles_names))),
        );
        $data[$table_alias]['language']['field'] = array(
          'id' => 'language',
        );
        $data[$table_alias]['language']['argument'] = array(
          'field' => 'language',
          'table' => $table,
          'id' => 'language',
          'additional fields' => $additional_fields,
          'empty field name' => t('- No value -'),
          'field_name' => $field_name,
          'entity_type' => $entity_type,
        );
        $data[$table_alias]['language']['filter'] = array(
          'field' => 'language',
          'table' => $table,
          'id' => 'language',
          'additional fields' => $additional_fields,
          'field_name' => $field_name,
          'entity_type' => $entity_type,
          'allow empty' => TRUE,
        );
        $data[$table_alias]['language']['sort'] = array(
          'field' => 'language',
          'table' => $table,
          'id' => 'standard',
          'additional fields' => $additional_fields,
          'field_name' => $field_name,
          'entity_type' => $entity_type,
        );
      }
    }
  }

  return $data;
}

/**
 * Have a different filter handler for lists. This should allow to select values of the list.
 */
function list_field_views_data($field) {
  $data = field_views_field_default_views_data($field);
  foreach ($data as $table_name => $table_data) {
    foreach ($table_data as $field_name => $field_data) {
      if (isset($field_data['filter']) && $field_name != 'delta') {
        $data[$table_name][$field_name]['filter']['id'] = 'field_list';
      }
      if (isset($field_data['argument']) && $field_name != 'delta') {
        if ($field['type'] == 'list_text') {
          $data[$table_name][$field_name]['argument']['id'] = 'field_list_string';
        }
        else {
          $data[$table_name][$field_name]['argument']['id'] = 'field_list';
        }
      }
    }
  }
  return $data;
}
