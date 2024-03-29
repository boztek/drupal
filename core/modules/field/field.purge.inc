<?php

/**
 * @file
 * Provides support for field data purge after mass deletion.
 */

use Drupal\field\Entity\Field;
use Drupal\field\FieldException;

/**
 * @defgroup field_purge Field API bulk data deletion
 * @{
 * Cleans up after Field API bulk deletion operations.
 *
 * Field API provides functions for deleting data attached to individual
 * entities as well as deleting entire fields or field instances in a single
 * operation.
 *
 * When a single entity is deleted, the Entity storage controller performs the
 * following operations:
 * - Invoking the FieldInterface delete() method for each field on the
 *   entity. A file field type might use this method to delete uploaded files
 *   from the filesystem.
 * - Removing the data from storage.
 * - Invoking the global hook_entity_delete() for all modules that implement it.
 *   Each hook implementation receives the entity being deleted and can operate
 *   on whichever subset of the entity's bundle's fields it chooses to.
 *
 * Similar operations are performed on deletion of a single entity revision.
 *
 * When a field, bundle, or field instance is deleted, it is not practical to
 * perform those operations immediately on every affected entity in a single
 * page request; there could be thousands or millions of them. Instead, the
 * appropriate field data items, instances, and/or fields are marked as deleted
 * so that subsequent load or query operations will not return them. Later, a
 * separate process cleans up, or "purges", the marked-as-deleted data by going
 * through the three-step process described above and, finally, removing deleted
 * field and instance records.
 *
 * Purging field data is made somewhat tricky by the fact that, while
 * $entity->delete() has a complete entity to pass to the various deletion
 * steps, the Field API purge process only has the field data it has previously
 * stored. It cannot reconstruct complete original entities to pass to the
 * deletion operations. It is even possible that the original entity to which
 * some Field API data was attached has been itself deleted before the field
 * purge operation takes place.
 *
 * Field API resolves this problem by using stub entities during purge
 * operations, containing only the information from the original entity that
 * Field API knows about: entity type, ID, revision ID, and bundle. It also
 * contains the field data for whichever field instance is currently being
 * purged.
 *
 * See @link field Field API @endlink for information about the other parts of
 * the Field API.
 */

/**
 * Purges a batch of deleted Field API data, instances, or fields.
 *
 * This function will purge deleted field data in batches. The batch size
 * is defined as an argument to the function, and once each batch is finished,
 * it continues with the next batch until all have completed. If a deleted field
 * instance with no remaining data records is found, the instance itself will
 * be purged. If a deleted field with no remaining field instances is found, the
 * field itself will be purged.
 *
 * @param $batch_size
 *   The maximum number of field data records to purge before returning.
 */
function field_purge_batch($batch_size) {
  // Retrieve all deleted field instances. We cannot use field_info_instances()
  // because that function does not return deleted instances.
  $instances = field_read_instances(array('deleted' => TRUE), array('include_deleted' => TRUE));
  $factory = \Drupal::service('entity.query');
  $info = entity_get_info();
  foreach ($instances as $instance) {
    $entity_type = $instance['entity_type'];

    // We cannot purge anything if the entity type is unknown (e.g. the
    // providing module was uninstalled).
    // @todo Revisit after https://drupal.org/node/2080823.
    if (!isset($info[$entity_type])) {
      continue;
    }

    // EntityFieldQuery currently fails on conditions on comment bundle.
    // Remove when http://drupal.org/node/731724 is fixed.
    if ($entity_type == 'comment') {
      continue;
    }

    $ids = (object) array(
      'entity_type' => $entity_type,
      'bundle' => $instance['bundle'],
    );
    // field_purge_data() will need the field array.
    $field = $instance->getField();
    // Retrieve some entities.
    $query = $factory->get($entity_type)
      ->condition('id:' . $field['uuid'] . '.deleted', 1)
      ->range(0, $batch_size);
    // If there's no bundle key, all results will have the same bundle.
    if (!empty($info[$entity_type]['entity_keys']['bundle'])) {
      $query->condition($info[$entity_type]['entity_keys']['bundle'], $ids->bundle);
    }
    $results = $query->execute();
    if ($results) {
      foreach ($results as $revision_id => $entity_id) {
        $ids->revision_id = $revision_id;
        $ids->entity_id = $entity_id;
        $entity = _field_create_entity_from_ids($ids);
        \Drupal::entityManager()->getStorageController($entity_type)->onFieldItemsPurge($entity, $instance);
      }
    }
    else {
      // No field data remains for the instance, so we can remove it.
      field_purge_instance($instance);
    }
  }

  // Retrieve all deleted fields. Any that have no instances can be purged.
  $deleted_fields = \Drupal::state()->get('field.field.deleted') ?: array();
  foreach ($deleted_fields as $field) {
    $field = new Field($field);

    // We cannot purge anything if the entity type is unknown (e.g. the
    // providing module was uninstalled).
    // @todo Revisit after https://drupal.org/node/2080823.
    if (!isset($info[$field->entity_type])) {
      continue;
    }

    $instances = field_read_instances(array('field_id' => $field['uuid']), array('include_deleted' => 1));
    if (empty($instances)) {
      field_purge_field($field);
    }
  }
}

/**
 * Purges a field instance record from the database.
 *
 * This function assumes all data for the instance has already been purged and
 * should only be called by field_purge_batch().
 *
 * @param $instance
 *   The instance record to purge.
 */
function field_purge_instance($instance) {
  $state = \Drupal::state();
  $deleted_instances = $state->get('field.instance.deleted');
  unset($deleted_instances[$instance['uuid']]);
  $state->set('field.instance.deleted', $deleted_instances);

  // Clear the cache.
  field_info_cache_clear();

  // Invoke external hooks after the cache is cleared for API consistency.
  \Drupal::moduleHandler()->invokeAll('field_purge_instance', array($instance));
}

/**
 * Purges a field record from the database.
 *
 * This function assumes all instances for the field has already been purged,
 * and should only be called by field_purge_batch().
 *
 * @param $field
 *   The field record to purge.
 */
function field_purge_field($field) {
  $instances = field_read_instances(array('field_id' => $field['uuid']), array('include_deleted' => 1));
  if (count($instances) > 0) {
    throw new FieldException(t('Attempt to purge a field @field_name that still has instances.', array('@field_name' => $field['field_name'])));
  }

  $state = \Drupal::state();
  $deleted_fields = $state->get('field.field.deleted');
  unset($deleted_fields[$field['uuid']]);
  $state->set('field.field.deleted', $deleted_fields);

  // Notify the storage layer.
  \Drupal::entityManager()->getStorageController($field->entity_type)->onFieldPurge($field);

  // Clear the cache.
  field_info_cache_clear();

  // Invoke external hooks after the cache is cleared for API consistency.
  \Drupal::moduleHandler()->invokeAll('field_purge_field', array($field));
}

/**
 * @} End of "defgroup field_purge".
 */
