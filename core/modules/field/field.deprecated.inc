<?php

/**
 * @file
 * This file contains functions marked as deprecated.
 */

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityNG;
use Drupal\Core\Entity\Field\PrepareCacheInterface;
use Drupal\Core\Language\Language;
use Drupal\entity\Entity\EntityDisplay;
use Drupal\field\Field;


/**
 * Returns information about field types.
 *
 * @param $field_type
 *   (optional) A field type name. If omitted, all field types will be returned.
 *
 * @return
 *   Either a field type definition, or an array of all existing field types,
 *   keyed by field type name.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.entity.field.field_type')->getDefinition()
 *   or
 *   \Drupal::service('plugin.manager.entity.field.field_type')->getDefinitions()
 */
function field_info_field_types($field_type = NULL) {
  if ($field_type) {
    return \Drupal::service('plugin.manager.entity.field.field_type')->getDefinition($field_type);
  }
  else {
    return \Drupal::service('plugin.manager.entity.field.field_type')->getDefinitions();
  }
}

/**
 * Returns a field type's default settings.
 *
 * @param $type
 *   A field type name.
 *
 * @return
 *   The field type's default settings, or an empty array if type or settings
 *   are not defined.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.entity.field.field_type')->getDefaultSettings()
 */
function field_info_field_settings($type) {
  return \Drupal::service('plugin.manager.entity.field.field_type')->getDefaultSettings($type);
}

/**
 * Returns a field type's default instance settings.
 *
 * @param $type
 *   A field type name.
 *
 * @return
 *   The field type's default instance settings, or an empty array if type or
 *   settings are not defined.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.entity.field.field_type')->getDefaultInstanceSettings()
 */
function field_info_instance_settings($type) {
  return \Drupal::service('plugin.manager.entity.field.field_type')->getDefaultInstanceSettings($type);
}

/**
 * Returns information about field widgets from AnnotatedClassDiscovery.
 *
 * @param string $widget_type
 *   (optional) A widget type name. If omitted, all widget types will be
 *   returned.
 *
 * @return array
 *   Either a single widget type description, as provided by class annotations,
 *   or an array of all existing widget types, keyed by widget type name.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.field.widget')->getDefinition()
 *   or
 *   \Drupal::service('plugin.manager.field.widget')->getDefinitions()
 */
function field_info_widget_types($widget_type = NULL) {
  if ($widget_type) {
    return drupal_container()->get('plugin.manager.field.widget')->getDefinition($widget_type);
  }
  else {
    return drupal_container()->get('plugin.manager.field.widget')->getDefinitions();
  }
}

/**
 * Returns a field widget's default settings.
 *
 * @param $type
 *   A widget type name.
 *
 * @return
 *   The widget type's default settings, as provided by
 *   hook_field_widget_info(), or an empty array if type or settings are
 *   undefined.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.field.widget')->getDefaultSettings()
 */
function field_info_widget_settings($type) {
  return \Drupal::service('plugin.manager.field.widget')->getDefaultSettings($type);
}

/**
 * Returns information about field formatters from hook_field_formatter_info().
 *
 * @param string $formatter_type
 *   (optional) A formatter type name. If omitted, all formatter types will be
 *   returned.
 *
 * @return array
 *   Either a single formatter type description, as provided by class
 *   annotations, or an array of all existing formatter types, keyed by
 *   formatter type name.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.field.formatter')->getDefinition()
 *   or
 *   \Drupal::service('plugin.manager.field.formatter')->getDefinitions()
 */
function field_info_formatter_types($formatter_type = NULL) {
  if ($formatter_type) {
    return drupal_container()->get('plugin.manager.field.formatter')->getDefinition($formatter_type);
  }
  else {
    return drupal_container()->get('plugin.manager.field.formatter')->getDefinitions();
  }
}

/**
 * Returns a field formatter's default settings.
 *
 * @param $type
 *   A field formatter type name.
 *
 * @return
 *   The formatter type's default settings, as provided by
 *   hook_field_formatter_info(), or an empty array if type or settings are
 *   undefined.
 *
 * @deprecated as of Drupal 8.0. Use
 *   \Drupal::service('plugin.manager.field.formatter')->getDefaultSettings()
 */
function field_info_formatter_settings($type) {
  return \Drupal::service('plugin.manager.field.formatter')->getDefaultSettings($type);
}

/**
 * Returns a lightweight map of fields across bundles.
 *
 * The function only returns active, non deleted fields.
 *
 * @return
 *   An array keyed by entity type. Each value is an array which keys are
 *   field names and value is an array with two entries:
 *   - type: The field type.
 *   - bundles: The bundles in which the field appears.
 * Example:
 * @code
 * array(
 *   'node' => array(
 *     'body' => array(
 *       'bundles' => array(
 *         'page', 'article'
 *       ),
 *       'type' => 'text_with_summary',
 *     ),
 *   ),
 * );
 * @endcode
 *
 * @deprecated as of Drupal 8.0. Use
 *   Field::fieldInfo()->getFieldMap().
 */
function field_info_field_map() {
  return Field::fieldInfo()->getFieldMap();
}

/**
 * Returns data about an individual field.
 *
 * @param $entity_type
 *   The entity type.
 * @param $field_name
 *   The name of the field to retrieve. $field_name can only refer to a
 *   non-deleted, active field. For deleted fields, use
 *   field_info_field_by_id(). To retrieve information about inactive fields,
 *   use field_read_fields().
 *
 * @return
 *   The field array, as returned by field_read_fields(), with an
 *   additional element 'bundles', whose value is an array of all the bundles
 *   this field belongs to keyed by entity type. NULL if the field was not
 *   found.
 *
 * @see field_info_field_by_id()

 * @deprecated as of Drupal 8.0. Use
 *   Field::fieldInfo()->getField($field_name).
 */
function field_info_field($entity_type, $field_name) {
  return Field::fieldInfo()->getField($entity_type, $field_name);
}

/**
 * Returns data about an individual field, given a field ID.
 *
 * @param $field_id
 *   The ID of the field to retrieve. $field_id can refer to a deleted field,
 *   but not an inactive one.
 *
 * @return
 *   The field array, as returned by field_read_fields(), with an additional
 *   element 'bundles', whose value is an array of all the bundles this field
 *   belongs to.
 *
 * @see field_info_field()
 *
 * @deprecated as of Drupal 8.0. Use
 *   Field::fieldInfo()->getFieldById($field_id).
 */
function field_info_field_by_id($field_id) {
  return Field::fieldInfo()->getFieldById($field_id);
}

/**
 * Returns the same data as field_info_field_by_id() for every field.
 *
 * Use of this function should be avoided when possible, since it loads and
 * statically caches a potentially large array of information.
 *
 * When iterating over the fields present in a given bundle after a call to
 * field_info_instances($entity_type, $bundle), it is recommended to use
 * field_info_field() on each individual field instead.
 *
 * @return
 *   An array, each key is a field ID and the values are field arrays as
 *   returned by field_read_fields(), with an additional element 'bundles',
 *   whose value is an array of all the bundle this field belongs to.
 *
 * @see field_info_field()
 * @see field_info_field_by_id()
 *
 * @deprecated as of Drupal 8.0. Use
 *   Field::fieldInfo()->getFields().
 */
function field_info_field_by_ids() {
  return Field::fieldInfo()->getFields();
}

/**
 * Retrieves information about field instances.
 *
 * Use of this function to retrieve instances across separate bundles (i.e.
 * when the $bundle parameter is NULL) should be avoided when possible, since
 * it loads and statically caches a potentially large array of information.
 * Use field_info_field_map() instead.
 *
 * When retrieving the instances of a specific bundle (i.e. when both
 * $entity_type and $bundle_name are provided), the function also populates a
 * static cache with the corresponding field definitions, allowing fast
 * retrieval of field_info_field() later in the request.
 *
 * @param $entity_type
 *   (optional) The entity type for which to return instances.
 * @param $bundle_name
 *   (optional) The bundle name for which to return instances. If $entity_type
 *   is NULL, the $bundle_name parameter is ignored.
 *
 * @return
 *   If $entity_type is not set, return all instances keyed by entity type and
 *   bundle name. If $entity_type is set, return all instances for that entity
 *   type, keyed by bundle name. If $entity_type and $bundle_name are set,
 *   return all instances for that bundle.
 *
 * @deprecated as of Drupal 8.0. Use
 *   Field::fieldInfo()->getInstances(),
 *   Field::fieldInfo()->getInstances($entity_type) or
 *   Field::fieldInfo()->getBundleInstances($entity_type, $bundle_name).
 */
function field_info_instances($entity_type = NULL, $bundle_name = NULL) {
  $cache = Field::fieldInfo();

  if (!isset($entity_type)) {
    return $cache->getInstances();
  }

  if (!isset($bundle_name)) {
    return $cache->getInstances($entity_type);
  }

  return $cache->getBundleInstances($entity_type, $bundle_name);
}

/**
 * Returns an array of instance data for a specific field and bundle.
 *
 * The function populates a static cache with all fields and instances used in
 * the bundle, allowing fast retrieval of field_info_field() or
 * field_info_instance() later in the request.
 *
 * @param $entity_type
 *   The entity type for the instance.
 * @param $field_name
 *   The field name for the instance.
 * @param $bundle_name
 *   The bundle name for the instance.
 *
 * @return
 *   An associative array of instance data for the specific field and bundle;
 *   NULL if the instance does not exist.
 *
 * @deprecated as of Drupal 8.0. Use
 *   Field::fieldInfo()->getInstance($entity_type, $bundle_name, $field_name).
 */
function field_info_instance($entity_type, $field_name, $bundle_name) {
  return Field::fieldInfo()->getInstance($entity_type, $bundle_name, $field_name);
}

/**
 * Reads a single field record directly from the database.
 *
 * Generally, you should use the field_info_field() instead.
 *
 * This function will not return deleted fields. Use field_read_fields() instead
 * for this purpose.
 *
 * @param $entity_type
 *   The entity type.
 * @param $field_name
 *   The field name to read.
 * @param array $include_additional
 *   The default behavior of this function is to not return a field that is
 *   inactive. Setting $include_additional['include_inactive'] to TRUE will
 *   override this behavior.
 *
 * @return
 *   A field definition array, or FALSE.
 *
 * @deprecated as of Drupal 8.0. Use
 *   entity_load('field_entity', 'field_name').
 */
function field_read_field($entity_type, $field_name, $include_additional = array()) {
  $fields = field_read_fields(array('entity_type' => $entity_type, 'name' => $field_name), $include_additional);
  return $fields ? current($fields) : FALSE;
}

/**
 * Reads in fields that match an array of conditions.
 *
 * @param array $conditions
 *   An array of conditions to match against. Keys are names of properties found
 *   in field configuration files, and values are conditions to match.
 * @param array $include_additional
 *   The default behavior of this function is to not return fields that are
 *   inactive or have been deleted. Setting
 *   $include_additional['include_inactive'] or
 *   $include_additional['include_deleted'] to TRUE will override this behavior.
 *
 * @return
 *   An array of fields matching $params. If
 *   $include_additional['include_deleted'] is TRUE, the array is keyed by
 *   field ID, otherwise it is keyed by field name.
 *
 * @deprecated as of Drupal 8.0. Use
 *   entity_load_multiple_by_properties('field_entity', $conditions).
 */
function field_read_fields($conditions = array(), $include_additional = array()) {
  // Include inactive fields if specified in the $include_additional parameter.
  $include_inactive = isset($include_additional['include_inactive']) && $include_additional['include_inactive'];
  // Include deleted fields if specified either in the $include_additional or
  // the $conditions parameters.
  $include_deleted = (isset($include_additional['include_deleted']) && $include_additional['include_deleted']) || (isset($conditions['deleted']) && $conditions['deleted']);

  // Pass include_inactive and include_deleted to the $conditions array.
  $conditions['include_inactive'] = $include_inactive;
  $conditions['include_deleted'] = $include_deleted;

  return entity_load_multiple_by_properties('field_entity', $conditions);
}

/**
 * Reads a single instance record from the database.
 *
 * Generally, you should use field_info_instance() instead, as it provides
 * caching and allows other modules the opportunity to append additional
 * formatters, widgets, and other information.
 *
 * @param $entity_type
 *   The type of entity to which the field is bound.
 * @param $field_name
 *   The field name to read.
 * @param $bundle
 *   The bundle to which the field is bound.
 * @param array $include_additional
 *   The default behavior of this function is to not return an instance that has
 *   been deleted, or whose field is inactive. Setting
 *   $include_additional['include_inactive'] or
 *   $include_additional['include_deleted'] to TRUE will override this behavior.
 *
 * @return
 *   An instance structure, or FALSE.
 *
 * @deprecated as of Drupal 8.0. Use
 *   entity_load('field_instance', 'field_name').
 */
function field_read_instance($entity_type, $field_name, $bundle, $include_additional = array()) {
  $instances = field_read_instances(array('entity_type' => $entity_type, 'field_name' => $field_name, 'bundle' => $bundle), $include_additional);
  return $instances ? current($instances) : FALSE;
}

/**
 * Reads in field instances that match an array of conditions.
 *
 * @param $param
 *   An array of properties to use in selecting a field instance. Keys are names
 *   of properties found in field instance configuration files, and values are
 *   conditions to match.
 * @param $include_additional
 *   The default behavior of this function is to not return field instances that
 *   have been marked deleted, or whose field is inactive. Setting
 *   $include_additional['include_inactive'] or
 *   $include_additional['include_deleted'] to TRUE will override this behavior.
 *
 * @return
 *   An array of instances matching the arguments.
 *
 * @deprecated as of Drupal 8.0. Use
 *   entity_load_multiple_by_properties('field_instance', $conditions).
 */
function field_read_instances($conditions = array(), $include_additional = array()) {
  // Include instances of inactive fields if specified in the
  // $include_additional parameter.
  $include_inactive = isset($include_additional['include_inactive']) && $include_additional['include_inactive'];
  // Include deleted instances if specified either in the $include_additional
  // or the $conditions parameters.
  $include_deleted = (isset($include_additional['include_deleted']) && $include_additional['include_deleted']) || (isset($conditions['deleted']) && $conditions['deleted']);

  // Pass include_inactive and include_deleted to the $conditions array.
  $conditions['include_inactive'] = $include_inactive;
  $conditions['include_deleted'] = $include_deleted;

  return entity_load_multiple_by_properties('field_instance', $conditions);
}

/**
 * Adds form elements for all fields for an entity to a form structure.
 *
 * The form elements for the entity's fields are added by reference as direct
 * children in the $form parameter. This parameter can be a full form structure
 * (most common case for entity edit forms), or a sub-element of a larger form.
 *
 * By default, submitted field values appear at the top-level of
 * $form_state['values']. A different location within $form_state['values'] can
 * be specified by setting the '#parents' property on the incoming $form
 * parameter. Because of name clashes, two instances of the same field cannot
 * appear within the same $form element, or within the same '#parents' space.
 *
 * For each call to field_attach_form(), field values are processed by calling
 * field_attach_extract_form_values() on the same $form element.
 *
 * Sample resulting structure in $form:
 * @code
 *   '#parents' => The location of field values in $form_state['values'],
 *   '#entity_type' => The name of the entity type,
 *   '#bundle' => The name of the bundle,
 *   // One sub-array per field appearing in the entity, keyed by field name.
 *   // The structure of the array differs slightly depending on whether the
 *   // widget is 'single-value' (provides the input for one field value,
 *   // most common case), and will therefore be repeated as many times as
 *   // needed, or 'multiple-values' (one single widget allows the input of
 *   // several values, e.g checkboxes, select box...).
 *   'field_foo' => array(
 *     '#access' => TRUE if the current user has 'edit' grants for the field,
 *       FALSE if not.
 *     'widget' => array(
 *       '#field_name' => The name of the field,
 *       '#language' => $langcode,
 *       '#field_parents' => The 'parents' space for the field in the form,
 *          equal to the #parents property of the $form parameter received by
 *          field_attach_form(),
 *       '#required' => Whether or not the field is required,
 *       '#title' => The label of the field instance,
 *       '#description' => The description text for the field instance,
 *
 *       // Only for 'single' widgets:
 *       '#theme' => 'field_multiple_value_form',
 *       '#cardinality' => The field cardinality,
 *       // One sub-array per copy of the widget, keyed by delta.
 *       0 => array(
 *         '#entity_type' => The name of the entity type,
 *         '#bundle' => The name of the bundle,
 *         '#field_name' => The name of the field,
 *         '#field_parents' => The 'parents' space for the field in the form,
 *            equal to the #parents property of the $form parameter received by
 *            field_attach_form(),
 *         '#title' => The title to be displayed by the widget,
 *         '#default_value' => The field value for delta 0,
 *         '#required' => Whether the widget should be marked required,
 *         '#delta' => 0,
 *         // The remaining elements in the sub-array depend on the widget.
 *         '#type' => The type of the widget,
 *         ...
 *       ),
 *       1 => array(
 *         ...
 *       ),
 *
 *       // Only for multiple widgets:
 *       '#entity_type' => The name of the entity type,
 *       '#bundle' => $instance['bundle'],
 *       // The remaining elements in the sub-array depend on the widget.
 *       '#type' => The type of the widget,
 *       ...
 *     ),
 *     ...
 *   ),
 * )
 * @endcode
 *
 * Additionally, some processing data is placed in $form_state, and can be
 * accessed by field_form_get_state() and field_form_set_state().
 *
 * @param \Drupal\Core\Entity\EntityInterface $entity
 *   The entity for which to load form elements, used to initialize
 *   default form values.
 * @param $form
 *   The form structure to fill in. This can be a full form structure, or a
 *   sub-element of a larger form. The #parents property can be set to control
 *   the location of submitted field values within $form_state['values']. If
 *   not specified, $form['#parents'] is set to an empty array, placing field
 *   values at the top-level of $form_state['values'].
 * @param $form_state
 *   An associative array containing the current state of the form.
 * @param $langcode
 *   The language the field values are going to be entered, if no language
 *   is provided the default site language will be used.
 * @param array $options
 *   An associative array of additional options. See field_invoke_method() for
 *   details.
 *
 * @deprecated as of Drupal 8.0. Use the entity system instead.
 *
 * @see field_form_get_state()
 * @see field_form_set_state()
 */
function field_attach_form(EntityInterface $entity, &$form, &$form_state, $langcode = NULL, array $options = array()) {
  // Set #parents to 'top-level' by default.
  $form += array('#parents' => array());

  // Get the entity_form_display object for this form.
  $form_display = $form_state['form_display'];

  // If no language is provided use the default site language.
  $options['langcode'] = field_valid_language($langcode);
  $form += (array) field_invoke_method('form', _field_invoke_widget_target($form_display), $entity, $form, $form_state, $options);

  $form['#entity_type'] = $entity->entityType();
  $form['#bundle'] = $entity->bundle();

  // Let other modules make changes to the form.
  // Avoid \Drupal::moduleHandler()->invokeAll()
  // to let parameters be taken by reference.
  foreach (\Drupal::moduleHandler()->getImplementations('field_attach_form') as $module) {
    $function = $module . '_field_attach_form';
    $function($entity, $form, $form_state, $langcode);
  }
}

/**
 * Performs field validation against form-submitted field values.
 *
 * There are two levels of validation for fields in forms: widget validation and
 * and field validation.
 * - Widget validation steps are specific to a given widget's own form structure
 *   and UI metaphors. They are executed through FAPI's #element_validate
 *   property during normal form validation.
 * - Field validation steps are common to a given field type, independently of
 *   the specific widget being used in a given form. They are defined in the
 *   field type's implementation of hook_field_validate().
 *
 * This function performs field validation in the context of a form submission.
 * It converts field validation errors into form errors on the correct form
 * elements. Fieldable entity types should call this function during their own
 * form validation function.
 *
 * @param \Drupal\Core\Entity\EntityInterface $entity
 *   The entity being submitted. The actual field values will be read
 *   from $form_state['values'].
 * @param $form
 *   The form structure where field elements are attached to. This might be a
 *   full form structure, or a sub-element of a larger form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 * @param array $options
 *   An associative array of additional options. See field_invoke_method() for
 *   details.
 *
 * @deprecated as of Drupal 8.0. Use the entity system instead.
 */
function field_attach_form_validate(EntityInterface $entity, $form, &$form_state, array $options = array()) {
  $has_violations = FALSE;
  foreach ($entity as $field_name => $field) {
    $definition = $field->getDefinition();
    if (!empty($definition['configurable']) && (empty($options['field_name']) || $options['field_name'] == $field_name)) {
      $field_violations = $field->validate();
      if (count($field_violations)) {
        $has_violations = TRUE;

        // Place violations in $form_state.
        $field_state = field_form_get_state($form['#parents'], $field_name, $form_state);
        $field_state['constraint_violations'] = $field_violations;
        field_form_set_state($form['#parents'], $field_name, $form_state, $field_state);
      }
    }
  }

  if ($has_violations) {
    // Map errors back to form elements.
    field_invoke_method('flagErrors', _field_invoke_widget_target($form_state['form_display']), $entity, $form, $form_state, $options);
  }
}

/**
 * Populates an entity object with values from a form submission.
 *
 * Currently, this accounts for drag-and-drop reordering of field values, and
 * filtering of empty values.
 *
 * @param \Drupal\Core\Entity\EntityInterface $entity
 *   The entity being submitted. The actual field values will be read
 *   from $form_state['values'].
 * @param $form
 *   The form structure where field elements are attached to. This might be a
 *   full form structure, or a sub-element of a larger form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 * @param array $options
 *   An associative array of additional options. See field_invoke_method() for
 *   details.
 *
 * @deprecated as of Drupal 8.0. Use the entity system instead.
 */
function field_attach_extract_form_values(EntityInterface $entity, $form, &$form_state, array $options = array()) {
  // Extract field values from submitted values.
  $form_display = $form_state['form_display'];
  field_invoke_method('extractFormValues', _field_invoke_widget_target($form_display), $entity, $form, $form_state, $options);

  // Let other modules act on submitting the entity.
  // Avoid \Drupal::moduleHandler()->invokeAll()
  // to let $form_state be taken by reference.
  foreach (\Drupal::moduleHandler()->getImplementations('field_attach_extract_form_values') as $module) {
    $function = $module . 'field_attach_extract_form_values';
    $function($entity, $form, $form_state);
  }
}

/**
 * Prepares field data prior to display.
 *
 * This function lets field types and formatters load additional data needed for
 * display that is not automatically loaded during entity loading. It accepts an
 * array of entities to allow query optimization when displaying lists of
 * entities.
 *
 * field_attach_prepare_view() and field_attach_view() are two halves of the
 * same operation. It is safe to call field_attach_prepare_view() multiple times
 * on the same entity before calling field_attach_view() on it, but calling any
 * Field API operation on an entity between passing that entity to these two
 * functions may yield incorrect results.
 *
 * @param $entity_type
 *   The type of entities in $entities; e.g. 'node' or 'user'.
 * @param array $entities
 *   An array of entities, keyed by entity ID.
 * @param array $displays
 *   An array of entity display objects, keyed by bundle name.
 * @param $langcode
 *   (Optional) The language the field values are to be shown in. If no language
 *   is provided the current language is used.
 *
 * @deprecated as of Drupal 8.0. Use the entity system instead.
 */
function field_attach_prepare_view($entity_type, array $entities, array $displays, $langcode = NULL) {
  $options['langcode'] = array();

  // To ensure hooks are only run once per entity, only process items without
  // the _field_view_prepared flag.
  // @todo: resolve this more generally for both entity and field level hooks.
  $prepare = array();
  foreach ($entities as $id => $entity) {
    if (empty($entity->_field_view_prepared)) {
      // Add this entity to the items to be prepared.
      $prepare[$id] = $entity;

      // Determine the actual language code to display for each field, given the
      // language codes available in the field data.
      $options['langcode'][$id] = field_language($entity, NULL, $langcode);

      // Mark this item as prepared.
      $entity->_field_view_prepared = TRUE;
    }
  }

  // Then let the formatters do their own specific massaging. For each
  // instance, call the prepareView() method on the formatter object handed by
  // the entity display.
  $target_function = function ($instance) use ($displays) {
    if (isset($displays[$instance['bundle']])) {
      return $displays[$instance['bundle']]->getRenderer($instance['field_name']);
    }
  };
  $null = NULL;
  field_invoke_method_multiple('prepareView', $target_function, $prepare, $null, $null);
}

/**
 * Returns a renderable array for the fields on an entity.
 *
 * Each field is displayed according to the display options specified in the
 * $display parameter for the given view mode.
 *
 * field_attach_prepare_view() and field_attach_view() are two halves of the
 * same operation. It is safe to call field_attach_prepare_view() multiple times
 * on the same entity before calling field_attach_view() on it, but calling any
 * Field API operation on an entity between passing that entity to these two
 * functions may yield incorrect results.
 *
 * @param \Drupal\Core\Entity\EntityInterface $entity
 *   The entity with fields to render.
 * @param \Drupal\entity\Entity\EntityDisplay $display
 *   The entity display object.
 * @param $langcode
 *   The language the field values are to be shown in. If no language is
 *   provided the current language is used.
 * @param array $options
 *   An associative array of additional options. See field_invoke_method() for
 *   details. Note: key "langcode" will be overridden inside this function.
 *
 * @return array
 *   A renderable array for the field values.
 *
 * @deprecated as of Drupal 8.0. Use the entity system instead.
 */
function field_attach_view(EntityInterface $entity, EntityDisplay $display, $langcode = NULL, array $options = array()) {
  // Determine the actual language code to display for each field, given the
  // language codes available in the field data.
  $options['langcode'] = field_language($entity, NULL, $langcode);

  // For each instance, call the view() method on the formatter object handed
  // by the entity display.
  $target_function = function ($instance) use ($display) {
    return $display->getRenderer($instance['field_name']);
  };
  $null = NULL;
  $output = field_invoke_method('view', $target_function, $entity, $null, $null, $options);

  // Let other modules alter the renderable array.
  $view_mode = $display->originalMode;
  $context = array(
    'entity' => $entity,
    'view_mode' => $view_mode,
    'display_options' => $view_mode,
    'langcode' => $langcode,
  );
  drupal_alter('field_attach_view', $output, $context);

  // Reset the _field_view_prepared flag set in field_attach_prepare_view(),
  // in case the same entity is displayed with different settings later in
  // the request.
  unset($entity->_field_view_prepared);

  return $output;
}

/**
 * Returns the field items in the language they currently would be displayed.
 *
 * @param \Drupal\Core\Entity\EntityInterface $entity
 *   The entity containing the data to be displayed.
 * @param $field_name
 *   The field to be displayed.
 * @param $langcode
 *   (optional) The language code $entity->{$field_name} has to be displayed in.
 *   Defaults to the current language.
 *
 * @return
 *   An array with available field items keyed by delta.
 *
 * @deprecated as of Drupal 8.0. Use
 *   $entity->getTranslation($langcode)->{$field_name}
 */
function field_get_items(EntityInterface $entity, $field_name, $langcode = NULL) {
  $langcode = field_language($entity, $field_name, $langcode);
  return isset($entity->{$field_name}[$langcode]) ? $entity->{$field_name}[$langcode] : array();
}

/**
 * Helper function to get the default value for a field on an entity.
 *
 * @param \Drupal\Core\Entity\EntityInterface $entity
 *   The entity for the operation.
 * @param $field
 *   The field structure.
 * @param $instance
 *   The instance structure.
 * @param $langcode
 *   The field language to fill-in with the default value.
 *
 * @return array
 *   The default value for the field.
 *
 * @deprecated as of Drupal 8.0. Use
 *   $instance->getFieldDefaultValue($entity)
 */
function field_get_default_value(EntityInterface $entity, $field, $instance, $langcode = NULL) {
  return $instance->getFieldDefaultValue($entity);
}
