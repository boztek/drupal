<?php

/**
 * @file
 * Field Info API, providing information about available fields and field types.
 */

use Drupal\Core\Cache\CacheBackendInterface;
use Drupal\Core\Language\Language;
use Drupal\field\Field;

/**
 * @defgroup field_info Field Info API
 * @{
 * Obtains information about Field API configuration.
 *
 * The Field Info API exposes information about field types, fields, instances,
 * bundles, widget types, display formatters, behaviors, and settings defined by
 * or with the Field API.
 *
 * See @link field Field API @endlink for information about the other parts of
 * the Field API.
 */

/**
 * Clears the field info cache without clearing the field data cache.
 *
 * This is useful when deleted fields or instances are purged. We need to remove
 * the purged records, but no actual field data items are affected.
 */
function field_info_cache_clear() {
  drupal_static_reset('field_view_mode_settings');
  drupal_static_reset('field_form_mode_settings');
  drupal_static_reset('field_available_languages');

  // @todo: Remove this when field_attach_*_bundle() bundle management
  // functions are moved to the entity API.
  entity_info_cache_clear();

  // Clear typed data definitions.
  \Drupal::typedData()->clearCachedDefinitions();
  \Drupal::service('plugin.manager.entity.field.field_type')->clearCachedDefinitions();

  Field::fieldInfo()->flush();
}

/**
 * Determines the behavior of a widget with respect to an operation.
 *
 * @param string $op
 *   The name of the operation. Currently supported: 'default_value',
 *   'multiple_values'.
 * @param array $instance
 *   The field instance array.
 *
 * @return int
 *   One of these values:
 *   - FIELD_BEHAVIOR_NONE: Do nothing for this operation.
 *   - FIELD_BEHAVIOR_CUSTOM: Use the widget's callback function.
 *   - FIELD_BEHAVIOR_DEFAULT: Use field.module default behavior.
 */
function field_behaviors_widget($op, $instance) {
  $info = array();
  if ($component = entity_get_form_display($instance['entity_type'], $instance['bundle'], 'default')->getComponent($instance['field_name'])) {
    $info = \Drupal::service('plugin.manager.field.widget')->getDefinition($component['type']);
  }
  return isset($info[$op]) ? $info[$op] : FIELD_BEHAVIOR_DEFAULT;
}

/**
 * Returns all field definitions.
 *
 * Use of this function should be avoided when possible, since it loads and
 * statically caches a potentially large array of information. Use
 * field_info_field_map() instead.
 *
 * When iterating over the fields present in a given bundle after a call to
 * field_info_instances($entity_type, $bundle), it is recommended to use
 * field_info_field() on each individual field instead.
 *
 * @return
 *   An array of field definitions, keyed by field UUID.
 *
 * @see field_info_field_map()
 */
function field_info_fields() {
  // Filter out deleted fields.
  return array_filter(Field::fieldInfo()->getFields(), function ($field) {
    return !$field->deleted;
  });
}

/**
 * Returns a list and settings of pseudo-field elements in a given bundle.
 *
 * If $context is 'form', an array with the following structure:
 * @code
 *   array(
 *     'name_of_pseudo_field_component' => array(
 *       'label' => The human readable name of the component,
 *       'description' => A short description of the component content,
 *       'weight' => The weight of the component in edit forms,
 *     ),
 *     'name_of_other_pseudo_field_component' => array(
 *       // ...
 *     ),
 *   );
 * @endcode
 *
 * If $context is 'display', an array with the following structure:
 * @code
 *   array(
 *     'name_of_pseudo_field_component' => array(
 *       'label' => The human readable name of the component,
 *       'description' => A short description of the component content,
 *       // One entry per view mode, including the 'default' mode:
 *       'display' => array(
 *         'default' => array(
 *           'weight' => The weight of the component in displayed entities in
 *             this view mode,
 *           'visible' => TRUE if the component is visible, FALSE if hidden, in
 *             displayed entities in this view mode,
 *         ),
 *         'teaser' => array(
 *           // ...
 *         ),
 *       ),
 *     ),
 *     'name_of_other_pseudo_field_component' => array(
 *       // ...
 *     ),
 *   );
 * @endcode
 *
 * @param $entity_type
 *   The type of entity; e.g. 'node' or 'user'.
 * @param $bundle
 *   The bundle name.
 * @param $context
 *   The context for which the list of pseudo-fields is requested. Either 'form'
 *   or 'display'.
 *
 * @return
 *   The array of pseudo-field elements in the bundle.
 */
function field_info_extra_fields($entity_type, $bundle, $context) {
  $info = Field::fieldInfo()->getBundleExtraFields($entity_type, $bundle);

  return isset($info[$context]) ? $info[$context] : array();
}

/**
 * @} End of "defgroup field_info".
 */
