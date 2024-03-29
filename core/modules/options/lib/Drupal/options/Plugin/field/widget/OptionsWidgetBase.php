<?php

/**
 * @file
 * Contains \Drupal\options\Plugin\field\widget\OptionsWidgetBase.
 */

namespace Drupal\options\Plugin\field\widget;

use Drupal\Core\Entity\Field\FieldDefinitionInterface;
use Drupal\Core\Entity\Field\FieldInterface;
use Drupal\Core\Entity\Field\FieldItemInterface;
use Drupal\field\Plugin\Type\Widget\WidgetBase;

/**
 * Base class for the 'options_*' widgets.
 *
 * Field types willing to enable one or several of the widgets defined in
 * options.module (select, radios/checkboxes, on/off checkbox) need to
 * implement the AllowedValuesInterface to specify the list of options to
 * display in the widgets.
 *
 * @see \Drupal\Core\TypedData\AllowedValuesInterface
 */
abstract class OptionsWidgetBase extends WidgetBase {

  /**
   * Identifies a 'None' option.
   */
  const OPTIONS_EMPTY_NONE = 'option_none';

  /**
   * Identifies a 'Select a value' option.
   */
  const OPTIONS_EMPTY_SELECT = 'option_select';

  /**
   * Abstract over the actual field columns, to allow different field types to
   * reuse those widgets.
   *
   * @var string
   */
  protected $column;

  /**
   * {@inheritdoc}
   */
  public function __construct($plugin_id, array $plugin_definition, FieldDefinitionInterface $field_definition, array $settings) {
    parent::__construct($plugin_id, $plugin_definition, $field_definition, $settings);
    $property_names = $this->fieldDefinition->getFieldPropertyNames();
    $this->column = $property_names[0];
  }

  /**
   * {@inheritdoc}
   */
  public function formElement(FieldInterface $items, $delta, array $element, array &$form, array &$form_state) {
    // Prepare some properties for the child methods to build the actual form
    // element.
    $this->required = $element['#required'];
    $cardinality = $this->fieldDefinition->getFieldCardinality();
    $this->multiple = ($cardinality == FIELD_CARDINALITY_UNLIMITED) || ($cardinality > 1);
    $this->has_value = isset($items[0]->{$this->column});

    // Add our custom validator.
    $element['#element_validate'][] = array(get_class($this), 'validateElement');
    $element['#key_column'] = $this->column;

    // The rest of the $element is built by child method implementations.

    return $element;
  }

  /**
   * Form validation handler for widget elements.
   *
   * @param array $element
   *   The form element.
   * @param array $form_state
   *   The form state.
   */
  public static function validateElement(array $element, array &$form_state) {
    if ($element['#required'] && $element['#value'] == '_none') {
      form_error($element, t('!name field is required.', array('!name' => $element['#title'])));
    }

    // Massage submitted form values.
    // Drupal\field\Plugin\Type\Widget\WidgetBase::submit() expects values as
    // an array of values keyed by delta first, then by column, while our
    // widgets return the opposite.

    if (is_array($element['#value'])) {
      $values = array_values($element['#value']);
    }
    else {
      $values = array($element['#value']);
    }

    // Filter out the 'none' option. Use a strict comparison, because
    // 0 == 'any string'.
    $index = array_search('_none', $values, TRUE);
    if ($index !== FALSE) {
      unset($values[$index]);
    }

    // Transpose selections from field => delta to delta => field.
    $items = array();
    foreach ($values as $value) {
      $items[] = array($element['#key_column'] => $value);
    }
    form_set_value($element, $items, $form_state);
  }

  /**
   * Returns the array of options for the widget.
   *
   * @param \Drupal\Core\Entity\Field\FieldItemInterface $item
   *   The field item.
   *
   * @return array
   *   The array of options for the widget.
   */
  protected function getOptions(FieldItemInterface $item) {
    if (!isset($this->options)) {
      // Limit the settable options for the current user account.
      $options = $item->getSettableOptions(\Drupal::currentUser());

      // Add an empty option if the widget needs one.
      if ($empty_option = $this->getEmptyOption()) {
        switch ($this->getPluginId()) {
          case 'options_buttons':
            $label = t('N/A');
            break;

          case 'options_select':
            $label = ($empty_option == static::OPTIONS_EMPTY_NONE ? t('- None -') : t('- Select a value -'));
            break;
        }

        $options = array('_none' => $label) + $options;
      }

      $module_handler = \Drupal::moduleHandler();
      $context = array(
        'fieldDefinition' => $this->fieldDefinition,
        'entity' => $item->getEntity(),
      );
      $module_handler->alter('options_list', $options, $context);

      array_walk_recursive($options, array($this, 'sanitizeLabel'));

      // Options might be nested ("optgroups"). If the widget does not support
      // nested options, flatten the list.
      if (!$this->supportsGroups()) {
        $options = $this->flattenOptions($options);
      }

      $this->options = $options;
    }
    return $this->options;
  }

  /**
   * Determines selected options from the incoming field values.
   *
   * @param \Drupal\Core\Entity\Field\FieldInterface $items
   *   The field values.
   * @param int $delta
   *   (optional) The delta of the item to get options for. Defaults to 0.
   *
   * @return array
   *   The array of corresponding selected options.
   */
  protected function getSelectedOptions(FieldInterface $items, $delta = 0) {
    // We need to check against a flat list of options.
    $flat_options = $this->flattenOptions($this->getOptions($items[$delta]));

    $selected_options = array();
    foreach ($items as $item) {
      $value = $item->{$this->column};
      // Keep the value if it actually is in the list of options (needs to be
      // checked against the flat list).
      if (isset($flat_options[$value])) {
        $selected_options[] = $value;
      }
    }

    return $selected_options;
  }

  /**
   * Flattens an array of allowed values.
   *
   * @param array $array
   *   A single or multidimensional array.
   *
   * @return array
   *   The flattened array.
   */
  protected function flattenOptions(array $array) {
    $result = array();
    array_walk_recursive($array, function($a, $b) use (&$result) { $result[$b] = $a; });
    return $result;
  }

  /**
   * Indicates whether the widgets support optgroups.
   *
   * @return bool
   *   TRUE if the widget supports optgroups, FALSE otherwise.
   */
  protected function supportsGroups() {
    return FALSE;
  }

  /**
   * Sanitizes a string label to display as an option.
   *
   * @param string $label
   *   The label to sanitize.
   */
  static protected function sanitizeLabel(&$label) {
    // Allow a limited set of HTML tags.
    $label = field_filter_xss($label);
  }

  /**
   * Returns the empty option to add to the list of options, if any.
   *
   * @return string|null
   *   Either static::OPTIONS_EMPTY_NONE, static::OPTIONS_EMPTY_SELECT, or NULL.
   */
  protected function getEmptyOption() { }

}
