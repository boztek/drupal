<?php

/**
 * @file
 * Contains \Drupal\block\Entity\BlockInterface.
 */

namespace Drupal\block;

use Drupal\Core\Config\Entity\ConfigEntityInterface;

/**
 * Provides an interface defining a block entity.
 */
interface BlockInterface extends ConfigEntityInterface {

  /**
   * Indicates the block label (title) should be displayed to end users.
   */
  const BLOCK_LABEL_VISIBLE = 'visible';

  /**
   * Returns the plugin instance.
   *
   * @return \Drupal\block\BlockPluginInterface
   *   The plugin instance for this block.
   */
  public function getPlugin();

  /**
   * Returns the plugin settings.
   *
   * @return array
   *   The plugin settings for this block.
   */
  public function settings();

  /**
   * Returns the region name.
   *
   * @return string
   *   The name of the region this block is placed in.
   */
  public function getRegion();

  /**
   * Sets the region to the region with the given name.
   *
   * @param string $region
   *   The name of the desired region this block will be placed in.
   *
   * @return \Drupal\block\BlockInterface
   *   The class instance this method is called on.
   */
  public function setRegion($region);

  /**
   * Returns the weight.
   *
   * @return int
   *   The weight of this block.
   */
  public function getWeight();

  /**
   * Sets the weight to the given value.
   *
   * @param int $weight
   *   The desired weight.
   *
   * @return \Drupal\block\BlockInterface
   *   The class instance this method is called on.
   */
  public function setWeight($weight);

  /**
   * Returns the visibility restrictions information.
   *
   * @return array
   *   The visibility restrictions information keyed by type for this block.
   */
  public function getVisibility();

  /**
   * Sets the visibility restrictions information as a whole.
   *
   * @param array $visibility
   *   All visibility restrictions information keyed by type for this block.
   *
   * @return \Drupal\block\BlockInterface
   *   The class instance this method is called on.
   */
  public function setVisibility(array $visibility);

}
