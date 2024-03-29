<?php

/**
 * @file
 * Contains \Drupal\node\Controller\NodeController.
 */

namespace Drupal\node\Controller;

use Drupal\Core\Entity\EntityInterface;
use Drupal\node\NodeInterface;

/**
 * Returns responses for Node routes.
 */
class NodeController {

  /**
   * @todo Remove node_add_page().
   */
  public function addPage() {
    module_load_include('pages.inc', 'node');
    return node_add_page();
  }

  /**
   * @todo Remove node_add().
   */
  public function add(EntityInterface $node_type) {
    module_load_include('pages.inc', 'node');
    return node_add($node_type);
  }

  /**
   * @todo Remove node_page_view().
   */
  public function viewPage(NodeInterface $node) {
    return node_page_view($node);
  }

  /**
   * @todo Remove node_show().
   */
  public function revisionShow($node_revision) {
    $node_revision = entity_revision_load('node', $node_revision);
    return node_show($node_revision, TRUE);
  }

  /**
   * @todo Remove node_revision_overview().
   */
  public function revisionOverview(NodeInterface $node) {
    module_load_include('pages.inc', 'node');
    return node_revision_overview($node);
  }

}
