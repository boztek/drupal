<?php

/**
 * @file
 * Builds placeholder replacement tokens for node-related data.
 */

use Drupal\Core\Language\Language;

/**
 * Implements hook_token_info().
 */
function node_token_info() {
  $type = array(
    'name' => t('Nodes'),
    'description' => t('Tokens related to individual content items, or "nodes".'),
    'needs-data' => 'node',
  );

  // Core tokens for nodes.
  $node['nid'] = array(
    'name' => t("Content ID"),
    'description' => t('The unique ID of the content item, or "node".'),
  );
  $node['vid'] = array(
    'name' => t("Revision ID"),
    'description' => t("The unique ID of the node's latest revision."),
  );
  $node['tnid'] = array(
    'name' => t("Translation set ID"),
    'description' => t("The unique ID of the original-language version of this node, if one exists."),
  );
  $node['type'] = array(
    'name' => t("Content type"),
    'description' => t("The type of the node."),
  );
  $node['type-name'] = array(
    'name' => t("Content type name"),
    'description' => t("The human-readable name of the node type."),
  );
  $node['title'] = array(
    'name' => t("Title"),
    'description' => t("The title of the node."),
  );
  $node['body'] = array(
    'name' => t("Body"),
    'description' => t("The main body text of the node."),
  );
  $node['summary'] = array(
    'name' => t("Summary"),
    'description' => t("The summary of the node's main body text."),
  );
  $node['langcode'] = array(
    'name' => t('Language code'),
    'description' => t('The language code of the language the node is written in.'),
  );
  $node['url'] = array(
    'name' => t("URL"),
    'description' => t("The URL of the node."),
  );
  $node['edit-url'] = array(
    'name' => t("Edit URL"),
    'description' => t("The URL of the node's edit page."),
  );

  // Chained tokens for nodes.
  $node['created'] = array(
    'name' => t("Date created"),
    'description' => t("The date the node was posted."),
    'type' => 'date',
  );
  $node['changed'] = array(
    'name' => t("Date changed"),
    'description' => t("The date the node was most recently updated."),
    'type' => 'date',
  );
  $node['author'] = array(
    'name' => t("Author"),
    'description' => t("The author of the node."),
    'type' => 'user',
  );

  return array(
    'types' => array('node' => $type),
    'tokens' => array('node' => $node),
  );
}

/**
 * Implements hook_tokens().
 */
function node_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $token_service = \Drupal::token();

  $url_options = array('absolute' => TRUE);
  if (isset($options['langcode'])) {
    $url_options['language'] = language_load($options['langcode']);
    $langcode = $options['langcode'];
  }
  else {
    $langcode = Language::LANGCODE_DEFAULT;
  }
  $sanitize = !empty($options['sanitize']);

  $replacements = array();

  if ($type == 'node' && !empty($data['node'])) {
    $node = $data['node'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        // Simple key values on the node.
        case 'nid':
          $replacements[$original] = $node->id();
          break;

        case 'vid':
          $replacements[$original] = $node->getRevisionId();
          break;

        case 'tnid':
          $replacements[$original] = $node->tnid->value;
          break;

        case 'type':
          $replacements[$original] = $sanitize ? check_plain($node->getType()) : $node->getType();
          break;

        case 'type-name':
          $type_name = node_get_type_label($node);
          $replacements[$original] = $sanitize ? check_plain($type_name) : $type_name;
          break;

        case 'title':
          $replacements[$original] = $sanitize ? check_plain($node->getTitle()) : $node->getTitle();
          break;

        case 'body':
        case 'summary':
          if (($items = $node->getTranslation($langcode)->get('body')) && !$items->isEmpty()) {
            $item = $items[0];
            $instance = field_info_instance('node', 'body', $node->getType());
            $field_langcode = field_language($node, 'body', $langcode);

            // If the summary was requested and is not empty, use it.
            if ($name == 'summary' && !empty($item->summary)) {
              $output = $sanitize ? $item->summary_processed : $item->summary;
            }
            // Attempt to provide a suitable version of the 'body' field.
            else {
              $output = $sanitize ? $item->processed : $item->value;
              // A summary was requested.
              if ($name == 'summary') {
                // Generate an optionally trimmed summary of the body field.

                // Get the 'trim_length' size used for the 'teaser' mode, if
                // present, or use the default trim_length size.
                $display_options = entity_get_display('node', $node->getType(), 'teaser')->getComponent('body');
                if (isset($display_options['settings']['trim_length'])) {
                  $length = $display_options['settings']['trim_length'];
                }
                else {
                  $settings = \Drupal::service('plugin.manager.field.formatter')->getDefaultSettings('text_summary_or_trimmed');
                  $length = $settings['trim_length'];
                }

                $output = text_summary($output, $instance['settings']['text_processing'] ? $item->format : NULL, $length);
              }
            }
            $replacements[$original] = $output;
          }
          break;

        case 'langcode':
          $replacements[$original] = $sanitize ? check_plain($node->language()->id) : $node->language()->id;
          break;

        case 'url':
          $replacements[$original] = url('node/' . $node->id(), $url_options);
          break;

        case 'edit-url':
          $replacements[$original] = url('node/' . $node->id() . '/edit', $url_options);
          break;

        // Default values for the chained tokens handled below.
        case 'author':
          $account = $node->getAuthor() ? $node->getAuthor() : user_load(0);
          $replacements[$original] = $sanitize ? check_plain($account->label()) : $account->label();
          break;

        case 'created':
          $replacements[$original] = format_date($node->getCreatedTime(), 'medium', '', NULL, $langcode);
          break;

        case 'changed':
          $replacements[$original] = format_date($node->getChangedTime(), 'medium', '', NULL, $langcode);
          break;
      }
    }

    if ($author_tokens = $token_service->findWithPrefix($tokens, 'author')) {
      $replacements += $token_service->generate('user', $author_tokens, array('user' => $node->getAuthor()), $options);
    }

    if ($created_tokens = $token_service->findWithPrefix($tokens, 'created')) {
      $replacements += $token_service->generate('date', $created_tokens, array('date' => $node->getCreatedTime()), $options);
    }

    if ($changed_tokens = $token_service->findWithPrefix($tokens, 'changed')) {
      $replacements += $token_service->generate('date', $changed_tokens, array('date' => $node->getChangedTime()), $options);
    }
  }

  return $replacements;
}
