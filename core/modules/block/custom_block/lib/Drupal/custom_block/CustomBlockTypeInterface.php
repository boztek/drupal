<?php

/**
 * @file
 * Contains \Drupal\custom_block\Entity\CustomBlockTypeInterface.
 */

namespace Drupal\custom_block;

use Drupal\Core\Config\Entity\ConfigEntityInterface;

/**
 * Provides an interface defining a custom block type entity.
 */
interface CustomBlockTypeInterface extends ConfigEntityInterface {
  /**
   * Returns the description of the block type.
   *
   * @return string
   *   The description of the type of this block.
   */
  public function getDescription();

 /**
  * Sets the description of the block type to the given value.
  *
  * @param string
  *   The desired description.
  *
  * @return \Drupal\custom_block\CustomBlockTypeInterface
  *   The class instance this method is called on.
  */
  public function setDescription($description);
}
