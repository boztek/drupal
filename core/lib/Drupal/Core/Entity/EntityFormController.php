<?php

/**
 * @file
 * Contains \Drupal\Core\Entity\EntityFormController.
 */

namespace Drupal\Core\Entity;

use Drupal\Core\Form\FormBase;
use Drupal\entity\EntityFormDisplayInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Language\Language;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Base class for entity form controllers.
 */
class EntityFormController extends FormBase implements EntityFormControllerInterface {

  /**
   * The name of the current operation.
   *
   * Subclasses may use this to implement different behaviors depending on its
   * value.
   *
   * @var string
   */
  protected $operation;

  /**
   * The module handler service.
   *
   * @var \Drupal\Core\Extension\ModuleHandlerInterface
   */
  protected $moduleHandler;

  /**
   * The entity being used by this form.
   *
   * @var \Drupal\Core\Entity\EntityInterface
   */
  protected $entity;

  /**
   * {@inheritdoc}
   */
  public function setOperation($operation) {
    // If NULL is passed, do not overwrite the operation.
    if ($operation) {
      $this->operation = $operation;
    }
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getBaseFormID() {
    // Assign ENTITYTYPE_form as base form ID to invoke corresponding
    // hook_form_alter(), #validate, #submit, and #theme callbacks, but only if
    // it is different from the actual form ID, since callbacks would be invoked
    // twice otherwise.
    $base_form_id = $this->entity->entityType() . '_form';
    if ($base_form_id == $this->getFormID()) {
      $base_form_id = '';
    }
    return $base_form_id;
  }

  /**
   * {@inheritdoc}
   */
  public function getFormID() {
    $entity_type = $this->entity->entityType();
    $bundle = $this->entity->bundle();
    $form_id = $entity_type;
    if ($bundle != $entity_type) {
      $form_id = $bundle . '_' . $form_id;
    }
    if ($this->operation != 'default') {
      $form_id = $form_id . '_' . $this->operation;
    }
    return $form_id . '_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    // During the initial form build, add this controller to the form state and
    // allow for initial preparation before form building and processing.
    if (!isset($form_state['controller'])) {
      $this->init($form_state);
    }

    // Retrieve the form array using the possibly updated entity in form state.
    $form = $this->form($form, $form_state);

    // Retrieve and add the form actions array.
    $actions = $this->actionsElement($form, $form_state);
    if (!empty($actions)) {
      $form['actions'] = $actions;
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
  }

  /**
   * Initialize the form state and the entity before the first form build.
   */
  protected function init(array &$form_state) {
    // Add the controller to the form state so it can be easily accessed by
    // module-provided form handlers there.
    $form_state['controller'] = $this;

    // Ensure we act on the translation object corresponding to the current form
    // language.
    $this->entity = $this->getTranslatedEntity($form_state);

    // Prepare the entity to be presented in the entity form.
    $this->prepareEntity();

    $form_display = entity_get_render_form_display($this->entity, $this->getOperation());

    // Let modules alter the form display.
    $form_display_context = array(
      'entity_type' => $this->entity->entityType(),
      'bundle' => $this->entity->bundle(),
      'form_mode' => $this->getOperation(),
    );
    $this->moduleHandler->alter('entity_form_display', $form_display, $form_display_context);

    $this->setFormDisplay($form_display, $form_state);

    // Invoke the prepare form hooks.
    $this->prepareInvokeAll('entity_prepare_form', $form_state);
    $this->prepareInvokeAll($this->entity->entityType() . '_prepare_form', $form_state);
  }

  /**
   * Returns the actual form array to be built.
   *
   * @see Drupal\Core\Entity\EntityFormController::build()
   */
  public function form(array $form, array &$form_state) {
    $entity = $this->entity;
    // @todo Exploit the Field API to generate the default widgets for the
    // entity properties.
    $info = $entity->entityInfo();
    if (!empty($info['fieldable'])) {
      field_attach_form($entity, $form, $form_state, $this->getFormLangcode($form_state));
    }

    // Add a process callback so we can assign weights and hide extra fields.
    $form['#process'][] = array($this, 'processForm');

    if (!isset($form['langcode'])) {
      // If the form did not specify otherwise, default to keeping the existing
      // language of the entity or defaulting to the site default language for
      // new entities.
      $form['langcode'] = array(
        '#type' => 'value',
        '#value' => !$entity->isNew() ? $entity->getUntranslated()->language()->id : language_default()->id,
      );
    }
    return $form;
  }

  /**
   * Process callback: assigns weights and hides extra fields.
   *
   * @see \Drupal\Core\Entity\EntityFormController::form()
   */
  public function processForm($element, $form_state, $form) {
    // Assign the weights configured in the form display.
    foreach ($this->getFormDisplay($form_state)->getComponents() as $name => $options) {
      if (isset($element[$name])) {
        $element[$name]['#weight'] = $options['weight'];
      }
    }

    // Hide extra fields.
    $extra_fields = field_info_extra_fields($this->entity->entityType(), $this->entity->bundle(), 'form');
    foreach ($extra_fields as $extra_field => $info) {
      if (!$this->getFormDisplay($form_state)->getComponent($extra_field)) {
        $element[$extra_field]['#access'] = FALSE;
      }
    }

    return $element;
  }

  /**
   * Returns the action form element for the current entity form.
   */
  protected function actionsElement(array $form, array &$form_state) {
    $element = $this->actions($form, $form_state);

    // We cannot delete an entity that has not been created yet.
    if ($this->entity->isNew()) {
      unset($element['delete']);
    }
    elseif (isset($element['delete'])) {
      // Move the delete action as last one, unless weights are explicitly
      // provided.
      $delete = $element['delete'];
      unset($element['delete']);
      $element['delete'] = $delete;
      $element['delete']['#button_type'] = 'danger';
    }

    if (isset($element['submit'])) {
      // Give the primary submit button a #button_type of primary.
      $element['submit']['#button_type'] = 'primary';
    }

    $count = 0;
    foreach (element_children($element) as $action) {
      $element[$action] += array(
        '#type' => 'submit',
        '#weight' => ++$count * 5,
      );
    }

    if (!empty($element)) {
      $element['#type'] = 'actions';
    }

    return $element;
  }

  /**
   * Returns an array of supported actions for the current entity form.
   */
  protected function actions(array $form, array &$form_state) {
    return array(
      // @todo Rename the action key from submit to save.
      'submit' => array(
        '#value' => $this->t('Save'),
        '#validate' => array(
          array($this, 'validate'),
        ),
        '#submit' => array(
          array($this, 'submit'),
          array($this, 'save'),
        ),
      ),
      'delete' => array(
        '#value' => $this->t('Delete'),
        // No need to validate the form when deleting the entity.
        '#submit' => array(
          array($this, 'delete'),
        ),
      ),
      // @todo Consider introducing a 'preview' action here, since it is used by
      // many entity types.
    );
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::validate().
   */
  public function validate(array $form, array &$form_state) {
    $entity = $this->buildEntity($form, $form_state);
    $entity_type = $entity->entityType();
    $entity_langcode = $entity->language()->id;

    $violations = array();

    foreach ($entity as $field_name => $field) {
      $field_violations = $field->validate();
      if (count($field_violations)) {
        $violations[$field_name] = $field_violations;
      }
    }

    // Map errors back to form elements.
    if ($violations) {
      foreach ($violations as $field_name => $field_violations) {
        $field_state = field_form_get_state($form['#parents'], $field_name, $form_state);
        $field_state['constraint_violations'] = $field_violations;
        field_form_set_state($form['#parents'], $field_name, $form_state, $field_state);
      }

      field_invoke_method('flagErrors', _field_invoke_widget_target($form_state['form_display']), $entity, $form, $form_state);
    }

    // @todo Remove this.
    // Execute legacy global validation handlers.
    unset($form_state['validate_handlers']);
    form_execute_handlers('validate', $form, $form_state);
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::submit().
   *
   * This is the default entity object builder function. It is called before any
   * other submit handler to build the new entity object to be passed to the
   * following submit handlers. At this point of the form workflow the entity is
   * validated and the form state can be updated, this way the subsequently
   * invoked handlers can retrieve a regular entity object to act on.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param array $form_state
   *   A reference to a keyed array containing the current state of the form.
   */
  public function submit(array $form, array &$form_state) {
    // Remove button and internal Form API values from submitted values.
    form_state_values_clean($form_state);

    $this->updateFormLangcode($form_state);
    $this->entity = $this->buildEntity($form, $form_state);
    return $this->entity;
  }

  /**
   * Form submission handler for the 'save' action.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param array $form_state
   *   A reference to a keyed array containing the current state of the form.
   */
  public function save(array $form, array &$form_state) {
    // @todo Perform common save operations.
  }

  /**
   * Form submission handler for the 'delete' action.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param array $form_state
   *   A reference to a keyed array containing the current state of the form.
   */
  public function delete(array $form, array &$form_state) {
    // @todo Perform common delete operations.
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::getFormLangcode().
   */
  public function getFormLangcode(array $form_state) {
    $entity = $this->entity;

    if (!empty($form_state['langcode'])) {
      $langcode = $form_state['langcode'];
    }
    else {
      // If no form langcode was provided we default to the current content
      // language and inspect existing translations to find a valid fallback,
      // if any.
      $translations = $entity->getTranslationLanguages();
      $langcode = language(Language::TYPE_CONTENT)->id;
      $fallback = language_multilingual() ? language_fallback_get_candidates() : array();
      while (!empty($langcode) && !isset($translations[$langcode])) {
        $langcode = array_shift($fallback);
      }
    }

    // If the site is not multilingual or no translation for the given form
    // language is available, fall back to the entity language.
    return !empty($langcode) ? $langcode : $entity->getUntranslated()->language()->id;
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::isDefaultFormLangcode().
   */
  public function isDefaultFormLangcode(array $form_state) {
    return $this->getFormLangcode($form_state) == $this->entity->getUntranslated()->language()->id;
  }

  /**
   * Updates the form language to reflect any change to the entity language.
   *
   * @param array $form_state
   *   A reference to a keyed array containing the current state of the form.
   */
  protected function updateFormLangcode(array &$form_state) {
    // Update the form language as it might have changed.
    if (isset($form_state['values']['langcode']) && $this->isDefaultFormLangcode($form_state)) {
      $form_state['langcode'] = $form_state['values']['langcode'];
    }
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::buildEntity().
   */
  public function buildEntity(array $form, array &$form_state) {
    $entity = clone $this->entity;
    // If you submit a form, the form state comes from caching, which forces
    // the controller to be the one before caching. Ensure to have the
    // controller of the current request.
    $form_state['controller'] = $this;
    // @todo Move entity_form_submit_build_entity() here.
    // @todo Exploit the Field API to process the submitted entity field.
    entity_form_submit_build_entity($entity->entityType(), $entity, $form, $form_state, array('langcode' => $this->getFormLangcode($form_state)));
    return $entity;
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::getEntity().
   */
  public function getEntity() {
    return $this->entity;
  }

  /**
   * Returns the translation object corresponding to the form language.
   *
   * @param array $form_state
   *   A keyed array containing the current state of the form.
   */
  protected function getTranslatedEntity(array $form_state) {
    $langcode = $this->getFormLangcode($form_state);
    return $this->entity->getTranslation($langcode);
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::setEntity().
   */
  public function setEntity(EntityInterface $entity) {
    $this->entity = $entity;
    return $this;
  }

  /**
   * Prepares the entity object before the form is built first.
   */
  protected function prepareEntity() {}

  /**
   * Invokes the specified prepare hook variant.
   *
   * @param string $hook
   *   The hook variant name.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   */
  protected function prepareInvokeAll($hook, array &$form_state) {
    $implementations = $this->moduleHandler->getImplementations($hook);
    foreach ($implementations as $module) {
      $function = $module . '_' . $hook;
      if (function_exists($function)) {
        // Ensure we pass an updated translation object and form display at
        // each invocation, since they depend on form state which is alterable.
        $args = array($this->getTranslatedEntity($form_state), $this->getFormDisplay($form_state), $this->operation, &$form_state);
        call_user_func_array($function, $args);
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getFormDisplay(array $form_state) {
    return isset($form_state['form_display']) ? $form_state['form_display'] : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function setFormDisplay(EntityFormDisplayInterface $form_display, array &$form_state) {
    $form_state['form_display'] = $form_display;
    return $this;
  }

  /**
   * Implements \Drupal\Core\Entity\EntityFormControllerInterface::getOperation().
   */
  public function getOperation() {
    return $this->operation;
  }

  /**
   * {@inheritdoc}
   */
  public function setModuleHandler(ModuleHandlerInterface $module_handler) {
    $this->moduleHandler = $module_handler;
    return $this;
  }

}
